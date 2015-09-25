/*
 *  LibVT_OpenCL.cpp
 *
 *
 *  Created by Julian Mayer on 20.03.10.
 *  Copyright 2010 A. Julian Mayer.
 *
 */

/*
 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "LibVT_Internal.h"
#include "LibVT.h"

#if OPENCL_BUFFERREDUCTION
extern vtData vt;
extern vtConfig c;

//void verify_pagetable_results(const char *str, unsigned char *gpu_pagetable_results, int w, int h, int m, uint32_t *res, int len);


const char * kernel_source = "\n" \
"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable                                                                       \n" \
"                                                                                                                                         \n" \
"kernel void buffer_to_quadtree(image2d_t img, global uchar *pagetable, constant int *offsets,                                            \n" \
"                               constant int *widths, constant int frame, constant int reduction_multiplier)                              \n" \
"{                                                                                                                                        \n" \
"    int image_width = get_image_width(img);                                                                                              \n" \
"    int image_height = get_image_height(img);                                                                                            \n" \
"    int x = get_global_id(0) * reduction_multiplier;                                                                                     \n" \
"    int y = get_global_id(1) * reduction_multiplier;                                                                                     \n" \
"                                                                                                                                         \n" \
"    if ((x < image_width) && (y < image_height))                                                                                         \n" \
"    {                                                                                                                                    \n" \
"        float4 clr = read_imagef(img, CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST, (float2)(x, y));     \n" \
"        uchar indx_x, indx_y, indx_mip;                                                                                                  \n" \
"        indx_x = convert_uchar_sat(clr.x * 255.0f);                                                                                      \n" \
"        indx_y = convert_uchar_sat(clr.y * 255.0f);                                                                                      \n" \
"        indx_mip = convert_uchar_sat(clr.z * 255.0f);                                                                                    \n" \
"                                                                                                                                         \n" \
"        int index = offsets[indx_mip] + indx_y * widths[indx_mip] + indx_x;                                                              \n" \
"        pagetable[index] = frame;                                                                                                        \n" \
"    }                                                                                                                                    \n" \
"}                                                                                                                                        \n" \
"                                                                                                                                         \n" \
" // notice: this kernel is based on the CUDA kernel by Charles-Frederik Hollemeersch, Bart Pieters, Peter Lambert, and Rik Van de Walle  \n" \
" // as published in 'Accelerating Virtual Texturing Using CUDA', copyright may apply													  \n" \
"kernel void quadtree_to_list(global uchar *pagetable, constant int numItems, global uint *tileList, constant int frame)                  \n" \
"{                                                                                                                                        \n" \
"    int x = get_global_id(0);                                                                                                            \n" \
"                                                                                                                                         \n" \
"    if (x < numItems)                                                                                                                    \n" \
"    {                                                                                                                                    \n" \
"        if (pagetable[x] == frame)                                                                                                       \n" \
"        {                                                                                                                                \n" \
"            int i = atom_inc(tileList);                                                                                                  \n" \
"            tileList[i+1] = x;                                                                                                           \n" \
"        }                                                                                                                                \n" \
"    }                                                                                                                                    \n" \
"}                                                                                                                                        \n" \
"                                                                                                                                         \n" \
"kernel void quadtree_clear(global uchar *pagetable, constant int numItems)                                                               \n" \
"{                                                                                                                                        \n" \
"    int x = get_global_id(0);                                                                                                            \n" \
"                                                                                                                                         \n" \
"    if (x < numItems)                                                                                                                    \n" \
"    {                                                                                                                                    \n" \
"        pagetable[x] = 0;                                                                                                                \n" \
"    }                                                                                                                                    \n" \
"}                                                                                                                                        \n" \
"                                                                                                                                         \n";


void vtPrepareOpenCL(const GLuint requestTexture)
{

	cl_device_id		devices[16];
	cl_int              numDevices, i;
	size_t              ret_size;
	cl_int              err;
	int					numItems = vt.pageTableMipOffsets[c.mipChainLength-1]+1;

	assert(!LONG_MIP_CHAIN); // TODO: make this compatible

	vt.cl_device = NULL;
	if (requestTexture)
	{
		vt.requestTexture = requestTexture;
		assert(READBACK_MODE == kCustomReadback);
	}
	else
	{
		vt.requestTexture = vt.fboColorTexture;
		assert(READBACK_MODE != kCustomReadback);
	}

    // create a shared context from a GL context
#ifdef WIN32
	cl_context_properties properties[] = { CL_GL_CONTEXT_KHR, (cl_context_properties) wglGetCurrentContext() };
#else
	cl_context_properties properties[] = { CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()), 0 };
#endif

	vt.cl_shared_context = clCreateContext(properties, 0, 0, 0, 0, 0);
	if (!vt.cl_shared_context)
		vt_fatal("clCreateContext() failed\n");

    // compute the number of devices
	err = clGetContextInfo(vt.cl_shared_context, CL_CONTEXT_DEVICES, 0, NULL, &ret_size);
	if(!ret_size || err != CL_SUCCESS)
		vt_fatal("clGetContextInfo() failed\n");

	numDevices = ret_size/sizeof(cl_device_id);

    // get the device list
	err = clGetContextInfo(vt.cl_shared_context, CL_CONTEXT_DEVICES, ret_size, devices, &ret_size);
	if(err)
		vt_fatal("clGetContextInfo() failed\n");

	// get the GPU device and queue
	for(i=0; i<numDevices; i++)
	{
		cl_device_type deviceType;

		err = clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &deviceType, &ret_size);
		if(err)
			vt_fatal("clGetDeviceInfo() failed\n");

		if(deviceType == CL_DEVICE_TYPE_GPU)
		{
			int error;

			vt.cl_device = devices[i];
			vt.cl_queue = clCreateCommandQueue(vt.cl_shared_context, vt.cl_device, 0, &error);

			size_t param_value_size_ret;
			err = clGetDeviceInfo(vt.cl_device, CL_DEVICE_EXTENSIONS, 0, NULL, &param_value_size_ret);
			if(err != CL_SUCCESS)
				vt_fatal("clGetDeviceInfo() failed\n");

			char *extensions_string = (char *)malloc(param_value_size_ret);
			err = clGetDeviceInfo(vt.cl_device, CL_DEVICE_EXTENSIONS, param_value_size_ret, extensions_string, NULL);
			if(err != CL_SUCCESS)
				vt_fatal("clGetDeviceInfo() failed\n");

			if (!strstr(extensions_string, "cl_khr_global_int32_base_atomics"))
			{
				free(extensions_string);
				vt_fatal("cl_khr_global_int32_base_atomics is required\n");
			}

			free(extensions_string);

			break;
		}
	}

	if (vt.cl_device == NULL)
		vt_fatal("no device found\n");


	// init program and kernels
	vt.program_bufferreduction = clCreateProgramWithSource(vt.cl_shared_context, 1,(const char **) &  kernel_source, NULL, &err);
	if(!vt.program_bufferreduction || err)
		vt_fatal("clCreateProgramWithSource() failed. (%d)\n", err);

    err = clBuildProgram(vt.program_bufferreduction, 1, &vt.cl_device, NULL, NULL, NULL);
    if(err != CL_SUCCESS)
    {
        char    buffer[2048] = "";
        clGetProgramBuildInfo(vt.program_bufferreduction, vt.cl_device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
		vt_fatal("clBuildProgram() failed.\n Log:\n%s\n", buffer);
    }

    vt.kernel_buffer_to_quadtree = clCreateKernel(vt.program_bufferreduction, "buffer_to_quadtree", &err);
    if(!vt.kernel_buffer_to_quadtree || err)
  		vt_fatal("clCreateKernel() failed creating kernel void buffer_to_quadtree(). (%d)\n", err);

	vt.kernel_quadtree_to_list = clCreateKernel(vt.program_bufferreduction, "quadtree_to_list", &err);
    if(!vt.kernel_quadtree_to_list || err)
  		vt_fatal("clCreateKernel() failed creating kernel void quadtree_to_list(). (%d)\n", err);

	vt.kernel_quadtree_clear = clCreateKernel(vt.program_bufferreduction, "quadtree_clear", &err);
    if(!vt.kernel_quadtree_clear || err)
  		vt_fatal("clCreateKernel() failed creating kernel void quadtree_clear(). (%d)\n", err);

	// init memory buffers
	vt.mem_quadtree = clCreateBuffer(vt.cl_shared_context, CL_MEM_READ_WRITE, (numItems * sizeof(unsigned char)), NULL, &err);
    if (!vt.mem_quadtree || err)
		vt_fatal("clCreateBuffer() failed. (%d)\n", err);

	vt.mem_offsets = clCreateBuffer(vt.cl_shared_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (12 * sizeof(int)), &vt.pageTableMipOffsets, &err);
    if (!vt.mem_offsets || err)
		vt_fatal("clCreateBuffer() failed. (%d)\n", err);

	int w[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	for (uint8_t i = 0; i < c.mipChainLength; i++)
		w[i] = (uint16_t) (c.virtTexDimensionPages >> i);
	vt.mem_widths = clCreateBuffer(vt.cl_shared_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (12 * sizeof(int)), &w, &err);
    if (!vt.mem_widths || err)
		vt_fatal("clCreateBuffer() failed. (%d)\n", err);

	vt.mem_list = clCreateBuffer(vt.cl_shared_context, CL_MEM_WRITE_ONLY, (c.physTexDimensionPages * c.physTexDimensionPages * 2 * sizeof(uint32_t) + 1), NULL, &err);
	if (!vt.mem_list || err)
		vt_fatal("clCreateBuffer() failed. (%d)\n", err);



	vt.list_buffer = (uint32_t *)malloc((c.physTexDimensionPages * c.physTexDimensionPages * 2 * sizeof(uint32_t) + 1));

	int reduction = 1 << OPENCL_REDUCTION_SHIFT;
    clSetKernelArg(vt.kernel_buffer_to_quadtree, 1, sizeof(cl_mem), &vt.mem_quadtree);
    clSetKernelArg(vt.kernel_buffer_to_quadtree, 2, sizeof(cl_mem), &vt.mem_offsets);
    clSetKernelArg(vt.kernel_buffer_to_quadtree, 3, sizeof(cl_mem), &vt.mem_widths);
    clSetKernelArg(vt.kernel_buffer_to_quadtree, 5, sizeof(int), &reduction);

    clSetKernelArg(vt.kernel_quadtree_to_list, 0, sizeof(cl_mem), &vt.mem_quadtree);
	clSetKernelArg(vt.kernel_quadtree_to_list, 1, sizeof(int), &numItems);
    clSetKernelArg(vt.kernel_quadtree_to_list, 2, sizeof(cl_mem), &vt.mem_list);

	clSetKernelArg(vt.kernel_quadtree_clear, 0, sizeof(cl_mem), &vt.mem_quadtree);
	clSetKernelArg(vt.kernel_quadtree_clear, 1, sizeof(int), &numItems);
}

void vtReshapeOpenCL(const uint16_t _w, const uint16_t _h)
{
 	cl_int              err;
	size_t              workgroup_size;
    size_t              num_groups;
	int					numItems = vt.pageTableMipOffsets[c.mipChainLength-1]+1;

	clGetKernelWorkGroupInfo(vt.kernel_buffer_to_quadtree, vt.cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &workgroup_size, NULL);
    {
        size_t  gsize[2];

        if (workgroup_size <= 256)
        {
            gsize[0] = 16;
            gsize[1] = workgroup_size / 16;
        }
        else if (workgroup_size <= 1024)
        {
            gsize[0] = workgroup_size / 16;
            gsize[1] = 16;
        }
        else
        {
            gsize[0] = workgroup_size / 32;
            gsize[1] = 32;
        }

        vt.local_work_size_k1[0] = gsize[0];
        vt.local_work_size_k1[1] = gsize[1];

        vt.global_work_size_k1[0] = ((_w + gsize[0] - 1) / gsize[0]);
        vt.global_work_size_k1[1] = ((_h + gsize[1] - 1) / gsize[1]);

        num_groups = vt.global_work_size_k1[0] * vt.global_work_size_k1[1];
        vt.global_work_size_k1[0] *= gsize[0];
        vt.global_work_size_k1[1] *= gsize[1];

		//printf("K1: global_work_size %i %i local_work_size %i %i num_groups %i workgroup_size %i \n\n", (int)vt.global_work_size_k1[0],  (int)vt.global_work_size_k1[1],  (int)vt.local_work_size_k1[0], (int)vt.local_work_size_k1[1], (int)num_groups, (int)workgroup_size);
	}

	clGetKernelWorkGroupInfo(vt.kernel_quadtree_to_list, vt.cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &workgroup_size, NULL);
	{
		vt.global_work_size_k2 = (numItems / workgroup_size + 1) * workgroup_size;
		vt.local_work_size_k2 = workgroup_size;

		//printf("K2: workgroup_size %i numItems %i bla %i\n", (int)workgroup_size, (int)numItems, (int)((numItems / workgroup_size + 1) * workgroup_size));
	}


	if (!vt.mem_shared_texture_buffer)
	{
		assert(vt.requestTexture);

		vt.mem_shared_texture_buffer = clCreateFromGLTexture2D (vt.cl_shared_context, CL_MEM_READ_ONLY, GL_TEXTURE_RECTANGLE_ARB, 0, vt.requestTexture, &err);
		if (!vt.mem_shared_texture_buffer || err)
			vt_fatal("clCreateFromGLTexture2D() failed. (%d)\n", err);

		clSetKernelArg(vt.kernel_buffer_to_quadtree, 0, sizeof(cl_mem), &vt.mem_shared_texture_buffer);
	}
}

void vtPerformOpenCLBufferReduction()
{
  	cl_int              err;
//	int					numItems = vt.pageTableMipOffsets[c.mipChainLength-1]+1;
	static int			frame = 1;
	uint8_t				framecounter = (frame % 255) + 1;

	frame ++;

	glFlush();
	//glFinish(); // a finish instead of a flush may be neccessary for some implementations. alternatively fence objects should do it.

	err = clEnqueueAcquireGLObjects(vt.cl_queue, 1, &vt.mem_shared_texture_buffer, 0, NULL, NULL);
	if (err)
		vt_fatal("clEnqueueAcquireGLObjects() failed. (%d)\n", err);

	int zero = 0;
	clEnqueueWriteBuffer(vt.cl_queue, vt.mem_list, CL_FALSE, 0, sizeof(int), &zero, 0, NULL, NULL); // reset the first element of the list, the element counter

	if (framecounter == 1) // clear quadtree
	{
		//printf("this one does more\n");
		err = clEnqueueNDRangeKernel(vt.cl_queue, vt.kernel_quadtree_clear, 1, NULL, &vt.global_work_size_k2, &vt.local_work_size_k2, 0, NULL, NULL);
		if (err)
			vt_fatal("clEnqueueNDRangeKernel() failed for kernel_quadtree_clear kernel. (%d)\n", err);
	}

	clSetKernelArg(vt.kernel_buffer_to_quadtree, 4, sizeof(uint8_t), &framecounter);
    clSetKernelArg(vt.kernel_quadtree_to_list, 3, sizeof(uint8_t), &framecounter);



    err = clEnqueueNDRangeKernel(vt.cl_queue, vt.kernel_buffer_to_quadtree, 2, NULL, vt.global_work_size_k1, vt.local_work_size_k1, 0, NULL, NULL);
	if (err)
		vt_fatal("clEnqueueNDRangeKernel() failed for buffer_to_quadtree kernel. (%d)\n", err);


	err = clEnqueueNDRangeKernel(vt.cl_queue, vt.kernel_quadtree_to_list, 1, NULL, &vt.global_work_size_k2, &vt.local_work_size_k2, 0, NULL, NULL);
	if (err)
		vt_fatal("clEnqueueNDRangeKernel() failed for quadtree_to_list kernel. (%d)\n", err);



//    unsigned char *gpu_pagetable_results = (unsigned char *)malloc(numItems*sizeof(unsigned char));
//    err = clEnqueueReadBuffer(vt.cl_queue, vt.mem_quadtree, CL_FALSE, 0, (numItems*sizeof(unsigned char)), gpu_pagetable_results, 0, NULL, NULL);
//    if (err)
//		vt_fatal("clEnqueueReadBuffer() failed. (%d)\n", err);

    err = clEnqueueReadBuffer(vt.cl_queue, vt.mem_list, CL_FALSE, 0, ((c.physTexDimensionPages * c.physTexDimensionPages * 2 * sizeof(uint32_t) + 1)), vt.list_buffer, 0, NULL, NULL);
    if (err)
		vt_fatal("clEnqueueReadBuffer() failed. (%d)\n", err);

	err = clEnqueueReleaseGLObjects(vt.cl_queue, 1, &vt.mem_shared_texture_buffer, 0, NULL, NULL);
	if (err)
		vt_fatal("clEnqueueReleaseGLObjects() failed. (%d)\n", err);


//	verify_pagetable_results("RGBA 8-bit", NULL, c.virtTexDimensionPages, c.virtTexDimensionPages, c.mipChainLength, vt.list_buffer, (c.physTexDimensionPages * c.physTexDimensionPages * 2 * sizeof(int) + 1));

//	free(gpu_pagetable_results);
}

void vtExtractNeededPagesOpenCL()
{
	queue<uint32_t> tmpPages;
	const clock_t clocks = vt.thisFrameClock = clock();

	clFinish(vt.cl_queue); // finish kernel and readback


	// erase pages that were requested in previous frames. if they are still neccesarry they will be readded
	{       // lock
		LOCK(vt.neededPagesMutex)

		while(!vt.neededPages.empty())
		{
			uint32_t pageInfo = vt.neededPages.front();

			vt.neededPages.pop_front();

			uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
			uint8_t mip = EXTRACT_MIP(pageInfo);
			*((uint8_t *)&PAGE_TABLE(mip, x_coord, y_coord)) = kTableFree;
		}
	}       // unlock

	vt.necessaryPageCount = vt.list_buffer[0]-1;
//	printf("%i\n", vt.necessaryPageCount);

	for (uint32_t i = 1; i < vt.list_buffer[0]; i++)
	{
		for (int8_t m = c.mipChainLength-1; m >= 0; m--)
		{
			uint32_t page = vt.list_buffer[i];

			if (page >= vt.pageTableMipOffsets[m])
			{
				page -= vt.pageTableMipOffsets[m];

				int y = page / (c.virtTexDimensionPages >> m);
				int x = page - (y * (c.virtTexDimensionPages >> m));


//				printf("%i   %i %i %i\n", r, l, x, y);


				const uint32_t pageEntry = PAGE_TABLE(m, x, y);


				if ((uint8_t) pageEntry == kTableFree) // if page is not mapped, add it to the download list and make sure we don't handle it again this frame
				{
					const uint32_t pageInfo = MAKE_PAGE_INFO(m, x, y);

					tmpPages.push(pageInfo);
#if DEBUG_LOG > 0
					printf("Requesting page: Mip:%u %u/%u\n", m, x, y);
#endif

					// we just want to set the alpha channel, luckly this byte is right there on little endian
					// setting just the lowest byte matters for the fallback-entry-mode, else a non-mapped page is empty anyway
					*((uint8_t *)&PAGE_TABLE(m, x, y)) = kTableMappingInProgress;

					vtcTouchCachedPage(pageInfo);

				}
				else if ((uint8_t) pageEntry == kTableMapped)	// if the page is mapped we need to mark it used
				{
					const uint8_t yInTexture = BYTE2(pageEntry), xInTexture = BYTE3(pageEntry);

					fast_assert((xInTexture < c.physTexDimensionPages) && (yInTexture < c.physTexDimensionPages));

					vt.textureStorageInfo[xInTexture][yInTexture].clockUsed = clocks;	// touch page in physical texture

					vtcTouchCachedPage(MAKE_PAGE_INFO(m, x, y));			// touch page in RAM cache
				}


				break;
			}
		}
	}



	queue<uint32_t>	cachedPages;
	queue<uint32_t>	nonCachedPages;

	vtcSplitPagelistIntoCachedAndNoncachedLOCK(&tmpPages, &cachedPages, &nonCachedPages);

	if (!nonCachedPages.empty())
	{	// lock
		LOCK(vt.neededPagesMutex)

		while(!nonCachedPages.empty())
		{
			vt.neededPages.push_back(nonCachedPages.front());nonCachedPages.pop();
		}

#if ENABLE_MT
		vt.neededPagesAvailableCondition.notify_one(); // wake up page loading thread if it is sleeping
#endif
	}	// unlock

	if (!cachedPages.empty()) // pass needed pages that are cached right to newPages so they don't have to roundtrip to another possibly busy thread. this is a optimization just for the MT path.
	{	// lock
		LOCK(vt.newPagesMutex)

		while(!cachedPages.empty())
		{
			const uint32_t pageInfo = cachedPages.front();

			vt.newPages.push(pageInfo);cachedPages.pop();

#if DEBUG_LOG > 0
			const uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
			const uint8_t mip = EXTRACT_MIP(pageInfo);
			printf("Loading page from RAM-cache: Mip:%u %u/%u\n", mip, x_coord, y_coord);
#endif
		}
	}	// unlock


}

void vtShutdownOpenCL()
{
	clReleaseKernel(vt.kernel_buffer_to_quadtree);
	clReleaseKernel(vt.kernel_quadtree_to_list);
	clReleaseKernel(vt.kernel_quadtree_clear);

    clReleaseProgram(vt.program_bufferreduction);
    clReleaseMemObject(vt.mem_quadtree);
	clReleaseMemObject(vt.mem_list);
	clReleaseMemObject(vt.mem_offsets);
	clReleaseMemObject(vt.mem_widths);
	clReleaseMemObject(vt.mem_shared_texture_buffer);

    clReleaseCommandQueue(vt.cl_queue);
    clReleaseContext(vt.cl_shared_context);

	free(vt.list_buffer);
}


//void verify_pagetable_results(const char *str, unsigned char *gpu_pagetable_results, int w, int h, int m, uint32_t *res, int len)
//{
//	int k1 = 0;
//
//	for (int n=0; n < m; n++)
//	{
//		for (int x=0; x<(w>>n); x++)
//		{
//			for (int y=0; y<(h>>n); y++)
//			{
//				if (gpu_pagetable_results[vt.pageTableMipOffsets[n] + y * (w>>n) + x] != 0)
//				{
//					k1++;
//				//	printf("%i  %i %i is %i\n",n, x, y, gpu_pagetable_results[vt.pageTableMipOffsets[n] + y * (w>>n) + x]);
//
//				}
//			}
//		}
//	}
//	printf(" k1 %i %i\n", k1);
//
//	for (int i = 1; i < res[0]; i++)
//	{
//		for (int8_t l = c.mipChainLength-1; l >= 0; l--)
//		{
//			int r = res[i];
//
//			if (r >= vt.pageTableMipOffsets[l])
//			{
//				r -= vt.pageTableMipOffsets[l];
//
//				int y = r / (c.virtTexDimensionPages >> l);
//				int x = r - (y * (c.virtTexDimensionPages >> l));
////				printf("%i   %i %i %i\n", r, l, x, y);
//				break;
//			}
//		}
//	}
//}
#endif

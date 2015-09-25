//
//  Simulation.m
//  VTDemo
//
//  Created by Julian Mayer on 31.07.09.
/*	Copyright (c) 2010 A. Julian Mayer
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#import "Simulation.h"
#import "LibVT_Internal.h"
#import "LibVT.h"
#import "LibVT_Config.h"
#include "LibVT_Utilities.h"

NSMutableArray *tmpMsg;
//void * files[32][32];
//uint32_t files_sizes[32][32];

#ifndef TARGET_OS_MAC
#define MICROSTART		;
#define MICROSTOP		;
#else
#define MICROSTART(x)	(x) = GetNanoseconds() / 1000
#define MICROSTOP(x)	(x) = ((GetNanoseconds() / 1000) - (x))
#endif

#include <deque>
#include <list>

#define TILE_DIR "/Users/julian/Documents/Development/VirtualTexturing/_texdata/8k_jpg"
#define EXT "jpg"
uint32_t picsize = 256;
void tLoadNeededPages();
void tDecompressNeededPages();
void * vtuLoadImageMemMAC(void *file_data, uint32_t file_size);
void * vtuLoadImageMemJPEG(void *file_data, uint32_t file_size);
void * vtuLoadImageMemPNG(void *file_data, uint32_t file_size);

queue<uint32_t>			vtneededPages;

boost::mutex			vtcompressedPagesMutex;

uint32_t			vtcompressedPages[1024];
uint32_t				vtcompressedPagesIndex;
//vector<uint32_t>			vtcompressedPages;
map<uint32_t, void *>	vtcompressedPagesData;
map<uint32_t, uint64_t>	vtcompressedPagesSize;
boost::condition		vtneededPagesAvailableCondition;
boost::condition		vtcompressedPagesAvailableCondition;
boost::thread			vtbackgroundThread;
boost::thread			vtbackgroundThread2;

uint64_t images;
uint64_t image2;

//@implementation Simulation

//- (id)init
//{
//	if ((self = [super init]))
int main(int argc, char *argv[])
	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NanosecondsInit();
		{
			int x, y;
			usleep(1000000);	// 1 s
			system("/usr/bin/purge");	// clear page cache (OS)
			usleep(10000000);	// 10 s
			for (x = 0; x < 32; x++)
			{

				for (y = 0; y < 32; y++)
				{
					void * image_data = vtuLoadFile([[NSString stringWithFormat:@"/Users/julian/Documents/Development/VirtualTexturing/_texdata/8k/tiles_b0_level0/tile_0_%u_%u.bmp", x, y] UTF8String], 0, 0);	 // clear disk buffer (HW)
					free(image_data);
				}
			}
			usleep(40000000);	// 40 s


			for (uint8_t x = 0; x < 32; x++)
				for (uint8_t y = 0; y < 32; y++)
					vtneededPages.push(MAKE_PAGE_INFO(0, x, y));

//			vtbackgroundThread = boost::thread(&tLoadNeededPages);
			vtbackgroundThread2 = boost::thread(&tDecompressNeededPages);

			MICROSTART(images);
			image2=images;

		}
		//vtcompressedPages.reserve(1024);

		char buf[255];

				while(!vtneededPages.empty())
				{
					const uint32_t pageInfo = vtneededPages.front();vtneededPages.pop();
					const uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
					const uint8_t mip = EXTRACT_MIP(pageInfo);
					void *image_data;


					snprintf(buf, 255, "%s%stiles_b%u_level%u%stile_%u_%u_%u.%s", TILE_DIR, PATH_SEPERATOR, 0, mip, PATH_SEPERATOR, mip, x_coord, y_coord, EXT);
					//printf("Loading page from Disk: Mip:%u %u/%u\n", mip, x_coord, y_coord);

					uint64_t size = 0;
					image_data = vtuLoadFile(buf, 0, &size);


					{	// lock
					//	LOCK(vtcompressedPagesMutex)
						vtcompressedPagesData.insert(pair<uint32_t, void *>(pageInfo, image_data));
					//	printf("%i\n", size);
						vtcompressedPagesSize.insert(pair<uint32_t, uint64_t>(pageInfo, size));
						//vtcompressedPages.push_back(pageInfo);
						vtcompressedPages[vtcompressedPagesIndex] = (pageInfo);
						vtcompressedPagesIndex++;
					}	// unlock

					if (x_coord == 31 & y_coord == 31)
					{
						MICROSTOP(image2);
						NSLog(@"load took %f ms %f MP/s", image2 / 1000.0, (32 * 32 / 16.0) / (image2 / 1000000.0));
						vtbackgroundThread2.join();

					}
				}


		exit(1);
	}
//	return self;
//}
//@end




void tDecompressNeededPages()
{
	char buf[255];
	static int decomp = 0;
	try {
		while (1)
		{
			queue<uint32_t>	tmpNewPages;
			queue<uint32_t>	neededPages;

			{
				LOCK(vtcompressedPagesMutex)


				uint8_t i = 0;	// limit to 5 pages at once
				//while(!vtcompressedPages.empty() && i < 5) // TODO: all this copying could use preallocation of necessary space (not only here)
				while(decomp < vtcompressedPagesIndex && i < 5)
				{
//					neededPages.push(vtcompressedPages.back());vtcompressedPages.pop_back();
					neededPages.push(vtcompressedPages[decomp]);
					decomp++;
					i ++;
				}
			}

			while(!neededPages.empty())
			{
				const uint32_t pageInfo = neededPages.front();neededPages.pop();
				void *image_data;
				static int times = 0;
				uint32_t size;

				{	// lock
					LOCK(vtcompressedPagesMutex)

					image_data = vtcompressedPagesData.find(pageInfo)->second;
					size = vtcompressedPagesSize.find(pageInfo)->second;
				} // unlock

				void *unc_image_data = vtuLoadImageMemJPEG(image_data, size);
				times ++;
				free(unc_image_data);

				if (times == 1024)
				{
					MICROSTOP(images);
					NSLog(@"extract took %f ms %f MP/s", images / 1000.0, (32 * 32 / 16.0) / (images / 1000000.0));
					exit(1);
				}
			}

		}
	}
	catch (boost::thread_interrupted const&)
	{
	}
}


void * vtuLoadImageMemMAC(void *file_data, uint32_t file_size)
{
	//	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)imagePath, strlen(imagePath), 0);
	//	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL(url, NULL);
	//	if (!imageSourceRef)
	//		vt_fatal("Error: tile directory must be corrupt (or wrong MIP_CHAIN_LENGTH?), there is no file at: %s", imagePath);

	CGDataProviderRef dataprov = CGDataProviderCreateWithData(NULL, file_data, file_size, NULL);
	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithDataProvider(dataprov, NULL);

	CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);

	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);
	CGRect rect = {{0, 0}, {width, height}};

	void *image_data = calloc(width * 4, height);

	CGContextRef bitmapContext = CGBitmapContextCreate(image_data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host );
	CGContextTranslateCTM (bitmapContext, 0, height);
	CGContextScaleCTM (bitmapContext, 1.0, -1.0);

	CGContextDrawImage(bitmapContext, rect, imageRef);

	CGContextRelease(bitmapContext);
	CGImageRelease(imageRef);
	CFRelease(imageSourceRef);
	//	CFRelease(url);


	return image_data;
}

#include "libjpeg-turbo/jpeglib.h"
void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char const *buffer, size_t bufsize);
void * vtuLoadImageMemJPEG(void *file_data, uint32_t file_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	//FILE *f = fopen(filename, "rb");
	char *image_data = (char *)malloc((picsize * picsize * 3));
	char *rows[picsize];

	//	assert(f);

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	//	jpeg_stdio_src(&cinfo, f);
//	jpeg_mem_src(&cinfo,(unsigned char *) file_data, file_size);
	jpeg_memory_src(&cinfo,(unsigned char *) file_data, file_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	assert((cinfo.output_width == picsize) && (cinfo.output_height == picsize) && (cinfo.output_components == 3));

	for (uint16_t i = 0; i < picsize; i++)
		rows[i] = image_data + i * picsize * 3;

	while (cinfo.output_scanline < cinfo.output_height)
    {
		jpeg_read_scanlines(&cinfo, (JSAMPLE **)&rows[cinfo.output_scanline], 256);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);


	//	fclose(f);

	return image_data;
}


#include <png.h>

typedef struct {
	const unsigned char *data;
	png_size_t size;
	png_size_t seek;
} pngData;

static void _readPngData(png_structp png_ptr, png_bytep data, png_size_t length)
{
	pngData *d = (pngData*) png_get_io_ptr(png_ptr);

	if (d) {
		for (png_size_t i = 0; i < length; i++) {
			if (d->seek >= d->size) break;
			data[i] = d->data[d->seek++];
		}
	}
}

void * vtuLoadImageMemPNG(void *file_data, uint32_t file_size)
{
	png_structp png_ptr;
	png_infop info_ptr;
	//	FILE *f = fopen(filename, "rb");
	char *image_data = (char *)malloc(picsize * picsize * 3);
	char *rows[picsize];

	//	assert(f);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	assert(png_ptr);


	pngData d;
	d.data = (const unsigned char *)file_data;
	d.size = file_size;
	d.seek = 0;
	png_set_read_fn(png_ptr, (void *) &d, _readPngData);

	info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);


	assert(setjmp(png_jmpbuf(png_ptr)) == 0);

	//	png_init_io(png_ptr, f);

	for (uint16_t i = 0; i < picsize; i++)
		rows[i] = image_data + i * picsize * 3;

	png_set_rows(png_ptr, info_ptr, (png_byte **)&rows);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_get_rows(png_ptr, info_ptr);

	assert((png_get_image_width(png_ptr, info_ptr) == picsize) && (png_get_image_height(png_ptr, info_ptr) == picsize) && (png_get_bit_depth(png_ptr, info_ptr) == 8) && (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB));

	return image_data;
}




struct my_src_mgr{
	struct jpeg_source_mgr pub;

	JOCTET eoi_buffer[2];
};
static void init_source(j_decompress_ptr cinfo){
}

static int fill_input_buffer(j_decompress_ptr cinfo){
	return 1;
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes){
	my_src_mgr *src = (my_src_mgr *)cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long)src->pub.bytes_in_buffer){

			num_bytes -= (long)src->pub.bytes_in_buffer;
			fill_input_buffer(cinfo);
		}
	}

	src->pub.next_input_byte += num_bytes;
	src->pub.bytes_in_buffer -= num_bytes;
}

static void term_source(j_decompress_ptr cinfo){
}


void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char const *buffer, size_t bufsize){

	my_src_mgr *src;
	if (! cinfo->src){

		cinfo->src = (jpeg_source_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(my_src_mgr));
	}

	src = (my_src_mgr *)cinfo->src;
	src->pub.init_source = init_source;

	src->pub.fill_input_buffer = fill_input_buffer;
	src->pub.skip_input_data = skip_input_data;

	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = term_source;

	src->pub.next_input_byte = buffer;
	src->pub.bytes_in_buffer = bufsize;
}

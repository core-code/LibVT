/*
 *  LibVT_Internal.h
 *
 *
 *  Created by Julian Mayer on 07.10.09.
 *  Copyright 2009 A. Julian Mayer. 
 *
 */

/*
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "LibVT_Config.h"


#include <time.h>
#include <assert.h>
#include <math.h>
#include <sys/types.h>



#ifdef WIN32
	#define PATH_SEPERATOR "\\"
	#if OPENCL_BUFFERREDUCTION
		#include <CL/opencl.h>
	#endif
	//#include <windows.h>

	#if defined(NO_GLEE)
		#include <GL/gl.h>
		#include "glext.h"
		//#include "wglext.h"
		#include "opengl_win32.h"
	#else
		#include "Glee.h"
	#endif
	#include <stdio.h>
	#include <string.h>
	#include <stdarg.h>

	#if defined(_MSC_VER)
		#ifndef snprintf
			#define snprintf _snprintf
		#endif
		#ifndef log2f
			inline float log2f(double x)
			{
				 static const double xxx = 1.0/log(2.0);
				 return (float)(log(x)*xxx);
			}
		#endif
		#include "dirent_win32.h"
	#else
		#include <dirent.h>
	#endif
#elif defined(__APPLE__)
	#import <TargetConditionals.h>
	#define PATH_SEPERATOR "/"
	#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
		#ifdef __OBJC__
			#import <OpenGLES/EAGL.h>
			#import <OpenGLES/EAGLDrawable.h>
		#endif

		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
		#define glBindFramebufferEXT glBindFramebuffer
		#define glOrtho glOrthof
		#define GL_FRAMEBUFFER_EXT	GL_FRAMEBUFFER
		#define glDeleteFramebuffersEXT glDeleteFramebuffers
		#define glGenFramebuffersEXT glGenFramebuffers
		#define glFramebufferTexture2DEXT glFramebufferTexture2D
		#define glCheckFramebufferStatusEXT glCheckFramebufferStatus
		#define GL_MAX_TEXTURE_IMAGE_UNITS_ARB GL_MAX_TEXTURE_IMAGE_UNITS
		#define GL_UNSIGNED_INT_8_8_8_8_REV	GL_UNSIGNED_BYTE
		#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
		#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
	#else
		#include <OpenGL/OpenGL.h>
		#include <Carbon/Carbon.h>
	#endif
	#include <dirent.h>
	#if OPENCL_BUFFERREDUCTION
		#include <OpenCL/opencl.h>
	#endif
#elif defined(linux)
	#define PATH_SEPERATOR "/"
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include "opengl_linux.h"
	#include <dirent.h>
	#include <stdio.h>
	#include <string.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <locale.h>
#else
	#error COULD_NOT_GUESS_TARGET_SYSTEM
#endif

#include <iostream>
#include <queue>
#include <map>


#include <vector>
#include <string>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/cstdint.hpp>

void vtShutdown();
static inline void vt_fatal(const char *err, ...) {va_list ap; va_start (ap, err); vtShutdown(); vfprintf (stderr, err, ap); va_end (ap); exit(1); }

using namespace std;
using namespace boost;


enum {
	kCustomReadback = 1,
	kBackbufferReadPixels = 2,
	kBackbufferGetTexImage = 3,
	kFBOReadPixels = 4,
	kFBOGetTexImage = 5
};

enum {
	kTableFree = 0,
	kTableMappingInProgress = 1,
	kTableMapped = 0xFF
};

#define MIPPED_PHYSTEX				(VT_MIN_FILTER == GL_NEAREST_MIPMAP_NEAREST || \
									VT_MIN_FILTER == GL_LINEAR_MIPMAP_NEAREST || \
									VT_MIN_FILTER == GL_NEAREST_MIPMAP_LINEAR || \
									VT_MIN_FILTER == GL_LINEAR_MIPMAP_LINEAR)
#define READBACK_MODE_NONE			(READBACK_MODE == kCustomReadback)
#define READBACK_MODE_FBO			(READBACK_MODE >= kFBOReadPixels)
#define READBACK_MODE_BACKBUFFER	((READBACK_MODE < kFBOReadPixels) && (!READBACK_MODE_NONE))
#define READBACK_MODE_GET_TEX_IMAGE	(READBACK_MODE == kFBOGetTexImage || READBACK_MODE == kBackbufferGetTexImage)
#define READBACK_MODE_READ_PIXELS	(READBACK_MODE == kFBOReadPixels || READBACK_MODE == kBackbufferReadPixels)

#define DecompressionPNG 4
#define DecompressionJPEG 8
#define DecompressionAllFormats 16

#define DecompressionLibPNG 4
#define DecompressionSTBIPNG 5
#define DecompressionLibJPEG 8
#define DecompressionLibJPEGTurbo 9
#define DecompressionSTBIJPEG 11
#define DecompressionMac 16
#define DecompressionDevil 17
#define DecompressionImageMagick 18


#define BYTE1(v)					((uint8_t) (v))
#define BYTE2(v)					((uint8_t) (((uint32_t) (v)) >> 8))
#define BYTE3(v)					((uint8_t) (((uint32_t) (v)) >> 16))
#define BYTE4(v)					((uint8_t) (((uint32_t) (v)) >> 24))

#if LONG_MIP_CHAIN
	#define MAKE_PAGE_INFO(m, x, y) ((x << 20) + (y << 8) + m)
	#define EXTRACT_Y(page)			((uint16_t) ((((uint32_t) (page)) >> 8) & 0xFFF))
	#define EXTRACT_X(page)			((uint16_t) ((((uint32_t) (page)) >> 20) & 0xFFF))
#else
	#define MAKE_PAGE_INFO(m, x, y) ((x << 16) + (y << 8) + m)
	#define EXTRACT_Y(page)			(BYTE2(page))
	#define EXTRACT_X(page)			(BYTE3(page))
#endif


#define PAGE_TABLE(m, x, y)			(vt.pageTables[(m)][(y) * (c.virtTexDimensionPages >> (m)) + (x)])
#define EXTRACT_MIP(page)			(BYTE1(page))

#if LONG_MIP_CHAIN
	#define MIP_INFO(mip)			(c.mipChainLength - 1 - mip)
#else
	#define MIP_INFO(mip)			(vt.mipTranslation[mip])
#endif

#if ENABLE_MT
    #define LOCK(x)					boost::mutex::scoped_lock scoped_lock(x);
#else
    #define LOCK(x)
#endif

#ifdef DEBUG
    #define fast_assert(x)
#else
    #define fast_assert(x)			assert((x))
#endif

#define touchMipRow(mip, row) {	vt.mipLevelTouched[mip] = true; \
								vt.mipLevelMinrow[mip] = (vt.mipLevelMinrow[mip] < row) ? vt.mipLevelMinrow[mip] : row; \
								vt.mipLevelMaxrow[mip] = (vt.mipLevelMaxrow[mip] > row) ? vt.mipLevelMaxrow[mip] : row; }

#define MAX_PHYS_TEX_DIMENSION_PAGES 64

struct storageInfo
{
	clock_t		clockUsed;
	uint16_t	x, y;
	uint8_t		mip;
//	bool		active;
};

struct vtConfig // TODO: constify?
{
    uint32_t		pageDimension;

    string			tileDir, pageCodec;

    uint8_t			pageBorder, mipChainLength;

    // derived values:
    uint32_t		pageMemsize, maxCachedPages, physTexDimensionPages, virtTexDimensionPages, residentPages;
    GLenum			pageDataFormat, pageDataType, pageDXTCompression;
};

struct vtData
{
	uint16_t				mipTranslation[12];
	uint32_t				pageTableMipOffsets[12];
	GLuint					fbo, fboColorTexture, fboDepthTexture, physicalTexture, pageTableTexture, mipcalcTexture, pboReadback, pboPagetable, pboPhystex;


	bool					mipLevelTouched[12];
	uint16_t				mipLevelMinrow[12];
	uint16_t				mipLevelMaxrow[12];
	storageInfo				textureStorageInfo[MAX_PHYS_TEX_DIMENSION_PAGES][MAX_PHYS_TEX_DIMENSION_PAGES];	// yes allocating this to the max size is a memory waste - it consumes 50k - but a vector of vectors is 1 magnitude slower

	uint16_t				necessaryPageCount, newPageCount, missingPageCount;
	float					bias;
	uint32_t				*readbackBuffer, **pageTables;
	clock_t					thisFrameClock;
	uint32_t				w, h, real_w, real_h;
	double					projectionMatrix[4][4];
	double					fovInDegrees;
	deque<uint32_t>			neededPages;
	queue<uint32_t>			newPages;
	map<uint32_t, void *>	cachedPages;
	map<uint32_t, clock_t>	cachedPagesAccessTimes;

#if ENABLE_MT
	boost::condition		neededPagesAvailableCondition;
	boost::mutex			neededPagesMutex;
	boost::mutex			newPagesMutex;
	boost::mutex			cachedPagesMutex;
	boost::thread			backgroundThread;
#endif

#if ENABLE_MT > 1
	boost::thread			backgroundThread2;
	boost::mutex			compressedMutex;
	queue<uint32_t>			newCompressedPages;
	map<uint32_t, void *>	compressedPages;
	map<uint32_t, uint32_t>	compressedPagesSizes;
	boost::condition		compressedPagesAvailableCondition;
#endif

#if OPENCL_BUFFERREDUCTION
	cl_context				cl_shared_context;
	cl_device_id			cl_device;
	cl_command_queue		cl_queue;
	cl_program				program_bufferreduction;
    cl_kernel				kernel_buffer_to_quadtree, kernel_quadtree_to_list, kernel_quadtree_clear;
    cl_mem					mem_quadtree, mem_offsets, mem_widths, mem_shared_texture_buffer, mem_list;
	size_t					global_work_size_k1[2], local_work_size_k1[2];
	size_t					global_work_size_k2, local_work_size_k2;
	uint32_t				*list_buffer;
	GLuint					requestTexture;
#endif
};

void vtLoadNeededPages();
void vtLoadNeededPagesDecoupled();
void vtDecompressNeededPagesDecoupled();
void vtCachePages(queue<uint32_t> pagesToCache);


void vtcRemoveCachedPageLOCK(uint32_t pageInfo);
void vtcTouchCachedPage(uint32_t pageInfo);
void vtcSplitPagelistIntoCachedAndNoncachedLOCK(queue<uint32_t> *s, queue<uint32_t> *cached, queue<uint32_t> *nonCached);
bool vtcIsPageInCacheLOCK(uint32_t pageInfo);
void vtcInsertPageIntoCacheLOCK(uint32_t pageInfo, void * image_data);
void * vtcRetrieveCachedPageLOCK(uint32_t pageInfo);
void vtcReduceCacheIfNecessaryLOCK(clock_t currentTime);
void _vtcRemoveCachedPage(uint32_t pageInfo);

void vtPrepareOpenCL();
void vtReshapeOpenCL(const uint16_t _w, const uint16_t _h);


void vtUnmapPage(int mipmap_level, int x_coord, int y_coord, int x_storage_location, int y_storage_location);
void vtUnmapPageCompleteley(int mipmap_level, int x_coord, int y_coord, int x_storage_location, int y_storage_location);


char		vtuFileExists(char *path);
void *		vtuLoadFile(const char *filePath, const uint32_t offset, uint32_t *file_size);
uint32_t *	vtuDownsampleImageRGBA(const uint32_t *tex);
uint32_t *	vtuDownsampleImageRGB(const uint32_t *tex);
void		vtuPerspective(double m[4][4], double fovy, double aspect,	double zNear, double zFar);



void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size);
void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size);
void * vtuCompressRGBA_DXT1(void *rgba);
void * vtuCompressRGBA_DXT5(void *rgba);

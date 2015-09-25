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

//NSMutableArray *tmpMsg;
void * files[32][32];
uint32_t files_sizes[32][32];

#ifndef TARGET_OS_MAC
#define MICROSTART		;
#define MICROSTOP		;
#else
#define MICROSTART(x)	(x) = GetNanoseconds() / 1000
#define MICROSTOP(x)	(x) = ((GetNanoseconds() / 1000) - (x))
#endif

void * vtuLoadImageMemJPEGSTBI(void *file_data, uint32_t file_size);
void * vtuLoadImageMemPNG(void *file_data, uint32_t file_size);
void * vtuLoadImageMemPNGSTBI(void *file_data, uint32_t file_size);
void * vtuLoadImageMemDEVIL(void *file_data, uint32_t file_size);
void * vtuLoadImageMemMAC(void *file_data, uint32_t file_size);
void * vtuLoadImageMemJPEG(void *file_data, uint32_t file_size);
void * vtuLoadImageMemJPEGT(void *file_data, uint32_t file_size);
void * vtuLoadImageMemIM(void *file_data, uint32_t file_size);

//#define TEST_DEVIL 1

extern vtConfig c;
uint32_t picsize = 256;

//@implementation Simulation
//
//- (id)init
//{
//	if ((self = [super init]))
int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NanosecondsInit();






		{
			int x, y;

			for (x = 0; x < 32; x++)
			{

				for (y = 0; y < 32; y++)
				{
					files[x][y] = vtuLoadFile([[NSString stringWithFormat:@"/Users/julian/Documents/Development/VirtualTexturing/_texdata/ny_final/tiles_b1_level4/tile_4_%u_%u.jpg", x, y] UTF8String], 0, &files_sizes[x][y]);
				}
			}
		}
		//system("/usr/bin/purge");
		//usleep(20000);

		for (int i = 0; i < 6; i++)
		{
			int x, y;

			uint64_t images;
			MICROSTART(images);

			for (x = 0; x < 32; x++)
			{

				for (y = 0; y < 32; y++)
				{
					void * image_data = vtuLoadImageMemJPEG(files[x][y], files_sizes[x][y]);//, &picsize);
					free(image_data);
				}
			}

			MICROSTOP(images);
			NSLog(@"extract took %f ms %f MP/s", images / 1000.0, (x * y / 16.0) / (images / 1000000.0));


		}

		exit(1);
}
//@end


#include "libjpeg-turbo/jpeglib.h"
void jpeg_memory_src(j_decompress_ptr cinfo, unsigned char const *buffer, size_t bufsize);

void * vtuLoadImageMemJPEG(void *file_data, uint32_t file_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	//FILE *f = fopen(filename, "rb");
	char *image_data = (char *)malloc((256 * 256 * 3));
	char *rows[256];

	//	assert(f);


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	//	jpeg_stdio_src(&cinfo, f);
	//jpeg_mem_src(&cinfo,(unsigned char *) file_data, file_size);
	jpeg_memory_src(&cinfo,(unsigned char *) file_data, file_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	//	assert((cinfo.output_width == picsize) && (cinfo.output_height == picsize) && (cinfo.output_components == 3));

	for (uint16_t i = 0; i < 256; i++)
		rows[i] = image_data + i * 256 * 3;

	while (cinfo.output_scanline < cinfo.output_height)
    {
		jpeg_read_scanlines(&cinfo, (JSAMPLE **)&rows[cinfo.output_scanline], 256);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);


	//	fclose(f);

	return image_data;
}


#define STBI_HEADER_FILE_ONLY
#include "stb_image.cc"
void * vtuLoadImageMemJPEGSTBI(void *file_data, uint32_t file_size)
{
	int width, height, bitdepth;

	//void * data = stbi_jpeg_load(filename, &width, &height, &bitdepth, STBI_rgb);
	void * data = stbi_jpeg_load_from_memory((const stbi_uc *)file_data, file_size, &width, &height, &bitdepth, STBI_rgb);

	assert(data);
	//	assert((width == picsize) && (height == picsize));

	return data;
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

#define STBI_HEADER_FILE_ONLY
#include "stb_image.cc"
void * vtuLoadImageMemPNGSTBI(void *file_data, uint32_t file_size)
{
	int width, height, bitdepth;

	//void * data = stbi_png_load(filename, &width, &height, &bitdepth, STBI_rgb);
	void * data = stbi_png_load_from_memory((const stbi_uc *)file_data, file_size, &width, &height, &bitdepth, STBI_rgb);

	//	assert(data);
	//	assert((width == picsize) && (height == picsize));

	return data;
}


//namespace MC
//{
//#include <magick/MagickCore.h>
//}
//void * vtuLoadImageMemIM(void *file_data, uint32_t file_size)
//{
//	static int times = 0;
//	if (times == 0)
//	{	MagickCoreGenesis("/Users/julian/Documents/Development/_BUILD/Debug/VTDemo.app/Contents/MacOS/VTDemo", MC::MagickFalse); times++;}
//	MC::ExceptionInfo *exception = MC::AcquireExceptionInfo();
//
//	MC::ImageInfo *image_info = CloneImageInfo((MC::ImageInfo *) NULL);
//
//	//  strcpy(image_info->filename, imagePath);
//	//	MC::Image *image = ReadImage(image_info, exception);
//	char *image_data = (char *)malloc(picsize * picsize * 8);
//
//	MC::Image *image = BlobToImage(image_info, file_data, file_size, exception);
//	free(file_data);
//
//	assert(image);
//
//	const MC::PixelPacket *p = GetVirtualPixels(image, 0, 0, image->columns, image->rows, exception);
//
//	memcpy(image_data, p, picsize * picsize * 8);
//	DestroyImage(image);
//
//	return image_data;
//}

//#include "turbojpeg/turbojpeg.h"
//void * vtuLoadImageMemJPEGT(void *file_data, uint32_t file_size)
//{
//	static int times = 0;
//	static tjhandle j;
//	char *image_data = (char *)malloc(picsize * picsize * 3);
//
//	if (times == 0)
//		j = tjInitDecompress();
//
//	tjDecompress(j, (unsigned char*)file_data, file_size, (unsigned char*)image_data, picsize, 0 , picsize, 3, TJ_FORCESSE2);
//
//	return image_data;
//}

void * vtuLoadImageMemMAC(void *file_data, uint32_t file_size)
{
//	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)imagePath, strlen(imagePath), 0);
//	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL(url, NULL);
//	if (!imageSourceRef)
//		vt_fatal("Error: tile directory must be corrupt (or wrong MIP_CHAIN_LENGTH?), there is no file at: %s", imagePath);

	CGDataProviderRef dataprov = CGDataProviderCreateWithData(NULL, file_data, file_size, NULL);
	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithDataProvider(dataprov, NULL);

	CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);


	CGRect rect = {{0, 0}, {picsize, picsize}};

	void *image_data = calloc(picsize * 4, picsize);

	CGContextRef bitmapContext = CGBitmapContextCreate(image_data, picsize, picsize, 8, picsize * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host );

	CGContextSetBlendMode(bitmapContext, kCGBlendModeCopy);

	CGContextDrawImage(bitmapContext, rect, imageRef);

	CGContextRelease(bitmapContext);
	CGImageRelease(imageRef);
	CFRelease(imageSourceRef);
//	CFRelease(url);


	return image_data;
}

#ifdef TEST_DEVIL
#include "IL/il.h"
void * vtuLoadImageMemDEVIL(void *file_data, uint32_t file_size)
{
	int format = 0;
	ILuint ilName;
	char *data;

	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);

	ilGenImages(1, &ilName);
	ilBindImage(ilName);
//	if (!ilLoadImage(imagePath))
//		vt_fatal("Error: tile directory must be corrupt (or wrong MIP_CHAIN_LENGTH?), there is no file at: %s", imagePath);
	ilLoadL(IL_TYPE_UNKNOWN, file_data, file_size);

	data = (char *) ilGetData();
	format = ilGetInteger(IL_IMAGE_FORMAT);

//	if ((format != IL_BGR) && (format != IL_RGB))
//		vt_fatal("Error: texture at: %s seems to be of wrong kind (%u)", imagePath, format);

//	if (*pic_size == 0)
//		*pic_size = ilGetInteger(IL_IMAGE_WIDTH);
//	else
//		assert((ilGetInteger(IL_IMAGE_WIDTH) == *pic_size) && (ilGetInteger(IL_IMAGE_HEIGHT) == *pic_size));
	//ilDeleteImages(1, &ilName); // TODO: this ain't optimal - we leak DevIL resources cause deleting the image would also release the cached data

	return data;
}

#endif

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

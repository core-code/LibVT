/*
 *  LibVT_ImageDecompression.cpp
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

#include "LibVT_Internal.h"
#include "LibVT.h"


extern vtConfig c;

#if IMAGE_DECOMPRESSION_LIBRARY == DecompressionMac


void * _vtuDecompressMac(CGImageSourceRef imageSourceRef, uint32_t *pic_size, const char *imagePath)
{
	CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);

	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);
	CGRect rect = {{0, 0}, {width, height}};

	void *image_data = calloc(width * 4, height);

	CGContextRef bitmapContext = CGBitmapContextCreate(image_data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host );
	CGContextTranslateCTM (bitmapContext, 0, height);
	CGContextScaleCTM (bitmapContext, 1.0, -1.0);

	CGContextDrawImage(bitmapContext, rect, imageRef);

#ifndef RENDER_INFO_TO_PAGES
	CGContextSetLineWidth(bitmapContext, (*pic_size - c.pageBorder)/50.0);
	CGContextBeginPath(bitmapContext);
	CGContextMoveToPoint(bitmapContext, 0, 0);
	CGContextAddLineToPoint(bitmapContext, (*pic_size - c.pageBorder), 0);
	CGContextAddLineToPoint(bitmapContext, (*pic_size - c.pageBorder), (*pic_size - c.pageBorder));
	CGContextAddLineToPoint(bitmapContext, 0, (*pic_size - c.pageBorder));
	CGContextAddLineToPoint(bitmapContext, 0, 0);
	CGContextStrokePath(bitmapContext);
	if(imagePath)
	{
		CGContextSelectFont(bitmapContext, "Arial", (*pic_size - c.pageBorder) / 5.0, kCGEncodingMacRoman);
		CGContextSetLineWidth(bitmapContext, (*pic_size - c.pageBorder)/100.0);
		CGContextSetCharacterSpacing(bitmapContext, 0);
		CGContextSetTextDrawingMode(bitmapContext, kCGTextFillStroke);
		CGContextSetRGBFillColor(bitmapContext, 0, 1, 0, .5);
		CGContextSetRGBStrokeColor(bitmapContext, 0, 0, 1, 1);
		CGContextShowTextAtPoint(bitmapContext, (*pic_size - c.pageBorder) / 10.0, (*pic_size - c.pageBorder) / 2.0, &imagePath[strlen(imagePath) - 11], 7);
	}
#endif

	CGContextRelease(bitmapContext);
	CGImageRelease(imageRef);
	CFRelease(imageSourceRef);

	if (*pic_size == 0)
		*pic_size = width;
	else
		assert((width == *pic_size) && (height == *pic_size));

	return image_data;
}


void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	CGDataProviderRef dataprov = CGDataProviderCreateWithData(NULL, file_data, file_size, NULL);
	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithDataProvider(dataprov, NULL);

	return _vtuDecompressMac(imageSourceRef, pic_size, NULL);
}

void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)imagePath, strlen(imagePath), 0);
	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL(url, NULL);

	CFRelease(url);
	if (!imageSourceRef)
		vt_fatal("Error: tile directory must be corrupt (or wrong MIP_CHAIN_LENGTH?), there is no file at: %s", imagePath);


	return _vtuDecompressMac(imageSourceRef, pic_size, imagePath);
}

#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionDevil

static bool ilInited = FALSE;

#include "IL/il.h"
void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	int format = 0;
	ILuint ilName;
	char *data;

	if (!ilInited)
	{
		ilInit();
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
		ilEnable(IL_ORIGIN_SET);
		ilInited = TRUE;
	}

	ilGenImages(1, &ilName);
	ilBindImage(ilName);
	if (!ilLoadImage(imagePath))
		vt_fatal("Error: tile directory must be corrupt (or wrong MIP_CHAIN_LENGTH?), there is no file at: %s", imagePath);


	data = (char *) ilGetData();
	format = ilGetInteger(IL_IMAGE_FORMAT);

	if ((format != IL_BGR) && (format != IL_RGB))
		vt_fatal("Error: texture at: %s seems to be of wrong kind (%u)", imagePath, format);

	if (*pic_size == 0)
		*pic_size = ilGetInteger(IL_IMAGE_WIDTH);
	else
		assert((ilGetInteger(IL_IMAGE_WIDTH) == *pic_size) && (ilGetInteger(IL_IMAGE_HEIGHT) == *pic_size));

	char *image_data = (char *)malloc(c.pageMemsize);
	memcpy(image_data, data, c.pageMemsize);
	ilDeleteImages(1, &ilName);

	return image_data;
}

void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	int format = 0;
	ILuint ilName;
	char *data;

	if (!ilInited)
	{
		ilInit();
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
		ilEnable(IL_ORIGIN_SET);
		ilInited = TRUE;
	}

	ilGenImages(1, &ilName);
	ilBindImage(ilName);
	ilLoadL(IL_TYPE_UNKNOWN, file_data, file_size);

	data = (char *) ilGetData();
	format = ilGetInteger(IL_IMAGE_FORMAT);

	if ((format != IL_BGR) && (format != IL_RGB))
		vt_fatal("Error: texture seems to be of wrong kind (%u)", format);

	if (*pic_size == 0)
		*pic_size = ilGetInteger(IL_IMAGE_WIDTH);
	else
		assert((ilGetInteger(IL_IMAGE_WIDTH) == *pic_size) && (ilGetInteger(IL_IMAGE_HEIGHT) == *pic_size));

	char *image_data = (char *)malloc(c.pageMemsize);
	memcpy(image_data, data, c.pageMemsize);
	ilDeleteImages(1, &ilName);

	return image_data;
}

#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionImageMagick

namespace MC
{
#include <magick/MagickCore.h>
}
void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	static int times = 0;
	if (times == 0)
	{	MagickCoreGenesis("/Users/julian/Documents/Development/_BUILD/Debug/VTDemo.app/Contents/MacOS/VTDemo", MC::MagickFalse); times++;}

	MC::ExceptionInfo *exception = MC::AcquireExceptionInfo();

	MC::ImageInfo *image_info = CloneImageInfo((MC::ImageInfo *) NULL);

	strcpy(image_info->filename, imagePath);
	MC::Image *image = ReadImage(image_info, exception);
	char *image_data = (char *)malloc(c.pageMemsize);

	assert(image);

	const MC::PixelPacket *p = GetVirtualPixels(image, 0, 0, image->columns, image->rows, exception);

	if (*pic_size == 0)
		*pic_size = image->columns;
	else
		assert((image->columns == *pic_size) && (image->rows == *pic_size));

	memcpy(image_data, p, c.pageMemsize);
	DestroyImage(image);

	return image_data;
}

void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	static int times = 0;
	if (times == 0)
	{	MagickCoreGenesis("/Users/julian/Documents/Development/_BUILD/Debug/VTDemo.app/Contents/MacOS/VTDemo", MC::MagickFalse); times++;}

	MC::ExceptionInfo *exception = MC::AcquireExceptionInfo();

	MC::ImageInfo *image_info = CloneImageInfo((MC::ImageInfo *) NULL);
	MC::Image *image = BlobToImage(image_info, file_data, file_size, exception);
	char *image_data = (char *)malloc(c.pageMemsize);

	assert(image);

	const MC::PixelPacket *p = GetVirtualPixels(image, 0, 0, image->columns, image->rows, exception);

	if (*pic_size == 0)
		*pic_size = image->columns;
	else
		assert((image->columns == *pic_size) && (image->rows == *pic_size));

	memcpy(image_data, p, c.pageMemsize);
	DestroyImage(image);

	return image_data;
}

#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionLibJPEG

#include "jpeglib.h"
void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *f = fopen(imagePath, "rb");



	assert(f);

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	assert(cinfo.output_components == 3);

	char *image_data = (char *)malloc(cinfo.output_width * cinfo.output_width * 3);
	char *rows[cinfo.output_width];
	
	for (uint16_t i = 0; i < cinfo.output_width; i++)
		rows[i] = image_data + (cinfo.output_width - 1 - i) * cinfo.output_width * 3;

	while (cinfo.output_scanline < cinfo.output_height)
    {
		jpeg_read_scanlines(&cinfo, (JSAMPLE **)&rows[cinfo.output_scanline], cinfo.output_width);
	}

	if (*pic_size == 0)
		*pic_size = cinfo.output_width;

	assert((cinfo.output_width == *pic_size) && (cinfo.output_height == *pic_size));

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);


	fclose(f);

	return image_data;
}
void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	char *image_data = (char *)malloc(c.pageMemsize);
	char *rows[c.pageDimension];


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo,(unsigned char *) file_data, file_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	assert(cinfo.output_components == 3);

	for (uint16_t i = 0; i < c.pageDimension; i++)
		rows[i] = image_data + (c.pageDimension - 1 - i) * c.pageDimension * 3;

	while (cinfo.output_scanline < cinfo.output_height)
    {
		jpeg_read_scanlines(&cinfo, (JSAMPLE **)&rows[cinfo.output_scanline], c.pageDimension);
	}

	if (*pic_size == 0)
		*pic_size = cinfo.output_width;
	else
		assert((cinfo.output_width == *pic_size) && (cinfo.output_height == *pic_size));

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);


	return image_data;
}

#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionLibPNG


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

void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	png_structp png_ptr;
	png_infop info_ptr;
	FILE *f = fopen(imagePath, "rb");
	char *image_data = (char *)malloc(c.pageMemsize);
	char *rows[c.pageDimension];

	assert(f);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	assert(png_ptr);

	info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr);


	assert(setjmp(png_jmpbuf(png_ptr)) == 0);

	png_init_io(png_ptr, f);

	for (uint16_t i = 0; i < c.pageDimension; i++)
		rows[i] = image_data + i * c.pageDimension * 3;

	png_set_rows(png_ptr, info_ptr, (png_byte **)&rows);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_get_rows(png_ptr, info_ptr);

	assert((png_get_bit_depth(png_ptr, info_ptr) == 8) && (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB));

	if (*pic_size == 0)
		*pic_size = png_get_image_height(png_ptr, info_ptr);
	else
		assert((png_get_image_height(png_ptr, info_ptr) == *pic_size) && (png_get_image_width(png_ptr, info_ptr) == *pic_size));

	return image_data;
}


void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	png_structp png_ptr;
	png_infop info_ptr;
	char *image_data = (char *)malloc(c.pageMemsize);
	char *rows[c.pageDimension];


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


	for (uint16_t i = 0; i < c.pageDimension; i++)
		rows[i] = image_data + i * c.pageDimension * 3;

	png_set_rows(png_ptr, info_ptr, (png_byte **)&rows);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_get_rows(png_ptr, info_ptr);

	assert((png_get_bit_depth(png_ptr, info_ptr) == 8) && (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB));

	if (*pic_size == 0)
		*pic_size = png_get_image_height(png_ptr, info_ptr);
	else
		assert((png_get_image_height(png_ptr, info_ptr) == *pic_size) && (png_get_image_width(png_ptr, info_ptr) == *pic_size));

	return image_data;
}

#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionSTBIJPEG

#define STBI_HEADER_FILE_ONLY
#include "stb_image.cc"
void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	int width, height, bitdepth;

	char * data = (char *)stbi_jpeg_load(imagePath, &width, &height, &bitdepth, STBI_rgb);

	assert(data);

	if (*pic_size == 0)
		*pic_size = width;
	else
		assert(((uint32_t)width == *pic_size) && ((uint32_t)height == *pic_size));

	char *image_data = (char *)malloc(c.pageMemsize);
	for (uint16_t i = 0; i < c.pageDimension; i++)
		memcpy(image_data + i * c.pageDimension * 3, data + (c.pageDimension-i-1) * c.pageDimension * 3, c.pageDimension * 3); //TODO: this isn't performance optimal

	free(data);

	return image_data;
}

void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	int width, height, bitdepth;

        char * data = (char *)stbi_jpeg_load_from_memory((const stbi_uc *)file_data, file_size, &width, &height, &bitdepth, STBI_rgb);

	assert(data);

	if (*pic_size == 0)
		*pic_size = width;
	else
                assert(((uint32_t)width == *pic_size) && ((uint32_t)height == *pic_size));

        char *image_data = (char *)malloc(c.pageMemsize);
        for (uint16_t i = 0; i < c.pageDimension; i++)
                memcpy(image_data + i * c.pageDimension * 3, data + (c.pageDimension-i-1) * c.pageDimension * 3, c.pageDimension * 3); //TODO: this isn't performance optimal

        free(data);

        return image_data;
}

#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionSTBIPNG

#define STBI_HEADER_FILE_ONLY
#include "stb_image.cc"

void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	int width, height, bitdepth;

	void * data = stbi_png_load(filename, &width, &height, &bitdepth, STBI_rgb);

	assert(data);

	if (*pic_size == 0)
		*pic_size = width;
	else
		assert((width == *pic_size) && (height == *pic_size));

	return data;
}

void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	int width, height, bitdepth;

	void * data = stbi_png_load_from_memory((const stbi_uc *)file_data, file_size, &width, &height, &bitdepth, STBI_rgb);

	assert(data);

	if (*pic_size == 0)
		*pic_size = width;
	else
		assert((width == *pic_size) && (height == *pic_size));

	return data;
}
#elif IMAGE_DECOMPRESSION_LIBRARY == DecompressionLibJPEGTurbo
#include "libjpeg-turbo/jpeglib.h"
struct my_src_mgr{
	struct jpeg_source_mgr pub;

	JOCTET eoi_buffer[2];
};
static void init_source(j_decompress_ptr cinfo){
}

static boolean fill_input_buffer(j_decompress_ptr cinfo){
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

void * vtuDecompressImageBuffer(const void *file_data, uint32_t file_size, uint32_t *pic_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	char *image_data = (char *)malloc(REALTIME_DXT_COMPRESSION ? c.pageDimension * c.pageDimension * 4 : c.pageMemsize);
	char *rows[c.pageDimension];


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_memory_src(&cinfo,(unsigned char *) file_data, file_size);

	jpeg_read_header(&cinfo, TRUE);
	if (REALTIME_DXT_COMPRESSION)	cinfo.out_color_space = JCS_EXT_RGBX;
	jpeg_start_decompress(&cinfo);


	if (cinfo.output_components == (REALTIME_DXT_COMPRESSION ? 4 : 3))
	{
		for (uint16_t i = 0; i < c.pageDimension; i++)
			rows[i] = image_data + (c.pageDimension - 1 - i) * c.pageDimension * (REALTIME_DXT_COMPRESSION ? 4 : 3);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, (JSAMPLE **)&rows[cinfo.output_scanline], 256);
		}

		if (*pic_size == 0)
			*pic_size = cinfo.output_width;
		else
			assert((cinfo.output_width == *pic_size) && (cinfo.output_height == *pic_size));

		jpeg_finish_decompress(&cinfo);
	}
	else
		printf("WARNING: skipping decompressing JPG with wrong number of components\n");

	jpeg_destroy_decompress(&cinfo);



	return image_data;
}
void * vtuDecompressImageFile(const char *imagePath, uint32_t *pic_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *f = fopen(imagePath, "rb");

	assert(f);

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	if (REALTIME_DXT_COMPRESSION)	cinfo.out_color_space = JCS_EXT_RGBX;
	jpeg_start_decompress(&cinfo);

	if (*pic_size == 0)
		*pic_size = cinfo.output_width;
	
	assert((cinfo.output_width == *pic_size) && (cinfo.output_height == *pic_size));

	char *image_data = (char *)malloc(cinfo.output_width * cinfo.output_width * (REALTIME_DXT_COMPRESSION ? 4 : 3));
	char *rows[cinfo.output_width];
	
	if (cinfo.output_components == (REALTIME_DXT_COMPRESSION ? 4 : 3))
	{
		for (uint16_t i = 0; i < cinfo.output_width; i++)
			rows[i] = image_data + (cinfo.output_width - 1 - i) * cinfo.output_width * (REALTIME_DXT_COMPRESSION ? 4 : 3);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, (JSAMPLE **)&rows[cinfo.output_scanline], cinfo.output_width);
		}

		jpeg_finish_decompress(&cinfo);
	}
	else
		printf("WARNING: skipping decompressing JPG with wrong number of components, path: %s\n", imagePath);

	jpeg_destroy_decompress(&cinfo);


	fclose(f);

	return image_data;
}
#else
	#error NO_IMAGE_DECOMPRESSION_LIBRARY
#endif

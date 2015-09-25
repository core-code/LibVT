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

#import "Core3D.h"
#import "LibVT_Internal.h"
#import "LibVT.h"
#import "LibVT_Config.h"

//void * files[32][32];
//uint32_t files_sizes[32][32];


#define MICROSTART(x)	(x) = GetNanoseconds() / 1000
#define MICROSTOP(x)	(x) = ((GetNanoseconds() / 1000) - (x))



#define TILE_DIR "/Users/julian/Documents/Development/VirtualTexturing/_texdata/8k_png"
#define EXT "png"
void * files[32][32];
uint32_t picsize = 256;
uint64_t images;

#define STBGL_COMPRESSED_RGB_S3TC_DXT1    0x83F0
#define STBGL_COMPRESSED_RGBA_S3TC_DXT1   0x83F1
#define STBGL_COMPRESSED_RGBA_S3TC_DXT3   0x83F2
#define STBGL_COMPRESSED_RGBA_S3TC_DXT5   0x83F3
#define STB_DEFINE
#include "stb_dxt.h"
#include "squish.h"
#include "libdxt.h"
#include "txc_dxtn.h"

static void stbgl__compress(uint8 *p, uint8 *rgba, int w, int h, int output_desc, uint8 *end);

int main(int argc, char *argv[])
{
	NanosecondsInit();
	int x, y;
	char buf[255];

	for (x = 0; x < 32; x++)
	{
		for (y = 0; y < 32; y++)
		{
			snprintf(buf, 255, "/Users/julian/Documents/Development/VirtualTexturing/_texdata/8k_png/tiles_b0_level0/tile_0_%u_%u.png", x, y);
			files[x][y] = vtuDecompressImageFile(buf, &picsize);
		}
	}

	stb__InitDXT();

	for (int i = 0; i < 6; i++)
	{
		int x, y;

		uint64_t images;
		MICROSTART(images);

		for (x = 0; x < 32; x++)
		{

			for (y = 0; y < 32; y++)
			{
				uint8 *out = (uint8 *)malloc((picsize+3)*(picsize+3)/16*8);
				uint8 *end_out = out + ((picsize+3)*(picsize+3))/16*8;

				//stbgl__compress(out, (uint8 *) files[x][y], picsize, picsize, STBGL_COMPRESSED_RGBA_S3TC_DXT5, end_out);
				//squish::CompressImage((const squish::u8*) files[x][y], picsize, picsize, out, squish::kDxt5 | squish::kColourRangeFit, NULL);
				CompressDXT((const byte *)files[x][y], out, picsize, picsize, FORMAT_DXT1, 1);
				//tx_compress_dxtn(4, picsize, picsize, (const GLubyte *)files[x][y], GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, out, picsize );
				free(out);

			}
		}

		MICROSTOP(images);
		printf("extract took %f ms %f MP/s\n", images / 1000.0, (x * y / 16.0) / (images / 1000000.0));
	}
}


static void stbgl__compress(uint8 *p, uint8 *rgba, int w, int h, int output_desc, uint8 *end)
{
	int i,j,y,y2;
	int alpha = (output_desc == STBGL_COMPRESSED_RGBA_S3TC_DXT5);
	for (j=0; j < w; j += 4) {
		int x=4;
		for (i=0; i < h; i += 4) {
			uint8 block[16*4];
			if (i+3 >= w) x = w-i;
			for (y=0; y < 4; ++y) {
				if (j+y >= h) break;
				memcpy(block+y*16, rgba + w*4*(j+y) + i*4, x*4);
			}
			if (x < 4) {
				switch (x) {
					case 0: assert(0);
					case 1:
						for (y2=0; y2 < y; ++y2) {
							memcpy(block+y2*16+1*4, block+y2*16+0*4, 4);
							memcpy(block+y2*16+2*4, block+y2*16+0*4, 8);
						}
						break;
					case 2:
						for (y2=0; y2 < y; ++y2)
							memcpy(block+y2*16+2*4, block+y2*16+0*4, 8);
						break;
					case 3:
						for (y2=0; y2 < y; ++y2)
							memcpy(block+y2*16+3*4, block+y2*16+1*4, 4);
						break;
				}
			}
			y2 = 0;
			for(; y<4; ++y,++y2)
				memcpy(block+y*16, block+y2*16, 4*4);
			stb_compress_dxt_block(p, block, alpha, 0);
			p += alpha ? 16 : 8;
		}
	}
	assert(p <= end);
}

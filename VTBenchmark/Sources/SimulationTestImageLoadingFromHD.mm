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

#define TILE_DIR "/Users/julian/Documents/Development/VirtualTexturing/_texdata/8k_png"
#define EXT "png"


uint64_t images;


int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NanosecondsInit();

	NSArray *exts = [[NSArray alloc] initWithObjects:@"bmp", @"dxt1", @"dxt5", nil];
	for (NSString *ext in exts)
	{
		for (int i = 0; i < 5; i ++)
		{
			int x, y;

			usleep(1000000);	// 1 s
			system("/usr/bin/purge");	// clear page cache (OS)
			usleep(10000000);	// 10 s
			for (x = 0; x < 32; x++)
			{

				for (y = 0; y < 32; y++)
				{
					void * image_data = vtuLoadFile([[NSString stringWithFormat:@"/Users/julian/Documents/Development/VirtualTexturing/_texdata/16k/tiles_b0_level0/tile_0_%u_%u.bmp", x, y] UTF8String], 0, 0);	 // clear disk buffer (HW)
					free(image_data);
				}
			}
			usleep(40000000);	// 40 s



			MICROSTART(images);

			for (x = 0; x < 32; x++)
			{

				for (y = 0; y < 32; y++)
				{
					void * image_data = vtuLoadFile([[NSString stringWithFormat:@"/Users/julian/Documents/Development/VirtualTexturing/_texdata/8k_%@/tiles_b0_level0/tile_0_%u_%u.%@", ext, x, y, ext]  UTF8String], 0, 0);
					free(image_data);
				}
			}

			MICROSTOP(images);
			NSLog(@"extract took %f ms %f MP/s %@", images / 1000.0, (x * y / 16.0) / (images / 1000000.0), ext);
		}
	}
}

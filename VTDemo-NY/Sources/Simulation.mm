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
#include "LibVT.h"
#include "LibVT_Internal.h"

float positions[60 * 60][6];


#define PLAY_DEMO		0
#define SCENE			kSceneTerrain
#define FIXED_TILES_PATH   // either load from a specific directory or from where the binary is


#define kSceneNY		1
#define kSceneTerrain	2

#ifdef FIXED_TILES_PATH
#ifdef WIN32
#define TILES_BASEPATH	@"C:\\"
#else
//#define TILES_BASEPATH	@"/Users/julian/Documents/Development/VirtualTexturing/_texdata_longmipchain/"
#define TILES_BASEPATH	@"/Users/julian/Documents/Development/VirtualTexturing/_texdata/"
#endif
#else
#define TILES_BASEPATH	[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent]
#endif




@implementation Simulation

- (id)init
{
	if ((self = [super init]))
	{
		//GLuint bla = LoadTexture(@"/Users/julian/Documents/Development/VirtualTexturing/_texdata_sources/texture_8k.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 2.0);

		//globalSettings.disableTextureCompression = YES;


#ifndef TARGET_OS_IPHONE
//		Light *light = [[[Light alloc] init] autorelease];
//		[light setPosition:vector3f(300, 300, 0)];
//		[light setLightAmbient:vector4f(0.7, 0.7, 0.7, 1.0)];
//		[[scene lights] addObject:light];
#endif

		[[scene camera] setAxisConfiguration:AXIS_CONFIGURATION(kXAxis, kYAxis, kZAxis)];
		[[scene camera] setFarPlane:20000];
		[[scene camera] setNearPlane:0.5];


#if SCENE == kSceneNY
		mesh = [[CollideableMesh alloc] initWithOctreeNamed:@"terrain"];

		VirtualTexturingNode *vtnode = [[VirtualTexturingNode alloc] initWithTileStore:[TILES_BASEPATH stringByAppendingPathComponent:@"ny_final"] format:@"jpg" border:1 miplength:10 tilesize:256];
//		VirtualTexturingNode *vtnode = [[[VirtualTexturingNode alloc] initWithTileStore:[TILES_BASEPATH stringByAppendingPathComponent:@"32k_b1_jpg"] format:@"jpg" border:1 miplength:8 tilesize:256] autorelease];
//		VirtualTexturingNode *vtnode = [[[VirtualTexturingNode alloc] initWithTileStore:[TILES_BASEPATH stringByAppendingPathComponent:@"ny_final_128"] format:@"jpg" border:1 miplength:11 tilesize:128] autorelease];

		for (int i = 1; i < 17; i++)
		{
			Mesh *b = [[[Mesh alloc] initWithOctreeNamed:[NSString stringWithFormat:@"%i_bo_lod1", i]] autorelease]; // got to use only the low NY mesh if we want to run on 512er cards, for 1024 we can use both and to LoD
			//Mesh *b = [[[Mesh alloc] initWithOctreeNamed:[NSString stringWithFormat:@"%i_bo_128", i]] autorelease];
			//Mesh *b = [[[Mesh alloc] initWithOctreeNamed:[NSString stringWithFormat:@"%i_bo", i]] autorelease];

			//LODNode *b = [[LODNode alloc] initWithOctreesNamed:[NSArray arrayWithObjects:[NSString stringWithFormat:@"%i_bo", i], [NSString stringWithFormat:@"%i_bo_lod1", i], nil] andFactor:1.1];
			[[vtnode children] addObject:b];
			//[[scene objects] addObject:b];
			//((Mesh *)b)->texName = bla; // this is only if we want to use a normal texture instead for performance testing
		}

#elif SCENE == kSceneTerrain
		mesh = [[CollideableMesh alloc] initWithOctreeNamed:@"testscene"];


		VirtualTexturingNode *vtnode;
		char ext [5] = "    ";
        uint8_t border, length;
        uint32_t dim;
        bool success = vtScan([[TILES_BASEPATH stringByAppendingPathComponent:@"texdata"] UTF8String], ext, &border, &length, &dim);
		
        if (success)
        {
#if (LONG_MIP_CHAIN)
            if (length > 9)
#else
			if ((length > 0) && (length <= 9))
#endif
			{
				vtnode = [[VirtualTexturingNode alloc] initWithTileStore:[TILES_BASEPATH stringByAppendingPathComponent:@"texdata"] format:[NSString stringWithUTF8String:ext] border:border miplength:length tilesize:dim];
			}
			else
			{
				printf("Error: %s not same MIPMAPCHAINLENGTH mode as binary.", [[TILES_BASEPATH stringByAppendingPathComponent:@"texdata"] UTF8String]);
				exit(1);
			}
        }
		else
		{
			printf("Error: %s not a valid tile store", [[TILES_BASEPATH stringByAppendingPathComponent:@"texdata"] UTF8String]);
			exit(1);
		}


#endif

		[[vtnode children] addObject:[mesh autorelease]];
		[[scene objects] addObject:[vtnode autorelease]];

//		[[scene objects] addObject:mesh];
//		glActiveTexture(GL_TEXTURE0 + 5);
//		glEnable(GL_TEXTURE_2D);
//
//		((Mesh *)mesh)->texName = bla;  // this is only if we want to use a normal texture instead for performance testing
//
//		glActiveTexture(GL_TEXTURE0);


		[self resetCamera];
#ifndef WIN32
		Skybox *skybox = [[Skybox alloc] initWithSurroundTextureNamed:@"north_east_south_west"];
		[skybox setSize:11000 ];
		[[scene objects] addObject:skybox];
#endif
#ifdef DISABLE_VBL_AND_BENCH
	//	globalSettings.disableVBLSync = YES;
#else
#ifndef WIN32
		[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(fpstimer) userInfo:NULL repeats:YES];
#endif
#endif
		if (0)
			[NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:self selector:@selector(recordPositionTimer) userInfo:NULL repeats:YES];

		if (PLAY_DEMO)
		{
#if SCENE == kSceneNY
			void *pos = vtuLoadFile([[[NSBundle mainBundle] pathForResource:@"positions-ny2" ofType:@"bin"] UTF8String], 0, NULL);
#elif SCENE == kSceneTerrain
			void *pos = vtuLoadFile([[[NSBundle mainBundle] pathForResource:@"positions" ofType:@"bin"] UTF8String], 0, NULL);
#endif
			memcpy(&positions, pos, sizeof(positions));

			[NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:self selector:@selector(updateTimer) userInfo:NULL repeats:YES];
		}

		speedModifier = 1.0;
	}
	return self;
}

- (void)recordPositionTimer
{
	static int times = 0;

	if (times < 60 * 60)
	{
		vector3f pos = [[scene camera] position];
		vector3f rot = [[scene camera] rotation];

		positions[times][0] = pos[0];
		positions[times][1] = pos[1];
		positions[times][2] = pos[2];
		positions[times][3] = rot[0];
		positions[times][4] = rot[1];
		positions[times][5] = rot[2];
	}
	if (times == 60 * 60)
	{
		FILE * pFile;
		pFile = fopen ( "/Users/julian/Desktop/positions.bin" , "wb" );
		fwrite (positions , 1 , sizeof(positions) , pFile );
		fclose (pFile);
	}
	times++;
}

- (void)fpstimer
{
	globalInfo.fps = frames;
	frames = 0;
}

- (void)updateTimer
{
	if (PLAY_DEMO)
	{
		static int times = 0;

		if (times < 60 * 60)
		{
			[[scene camera] setPosition:vector3f(positions[times][0], positions[times][1], positions[times][2])];
			[[scene camera] setRotation:vector3f(positions[times][3], positions[times][4], positions[times][5])];
		}
		else
		{
			vtShutdown();
			exit(1);
		}

		times++;

	}
}

- (void)update
{
#ifdef WIN32
	[self updateTimer];
#endif
}

- (void)render
{
	frames++;
	static BOOL ringbufferW[20], ringbufferA[20], ringbufferS[20], ringbufferD[20];
	vector3f movement = vector3f(0, 0, 0);

	ringbufferW[frames%20] = ringbufferA[frames%20] = ringbufferS[frames%20] = ringbufferD[frames%20] = NO;

	for (NSString *keyHit in pressedKeys)
	{
		switch ([keyHit intValue])
		{
			case 's':
				ringbufferS[frames%20] = YES;
				break;
			case 'w':
				ringbufferW[frames%20] = YES;
				break;
			case 'a':
				ringbufferA[frames%20] = YES;
				break;
			case 'd':
				ringbufferD[frames%20] = YES;
				break;
		}
	}

	for (int i = 0; i < 20; i ++)	movement[2] -= ringbufferW[i];
	for (int i = 0; i < 20; i ++)	movement[2] += ringbufferS[i];
	for (int i = 0; i < 20; i ++)	movement[0] += ringbufferD[i];
	for (int i = 0; i < 20; i ++)	movement[0] -= ringbufferA[i];

#define DIST 1.0f
	matrix33f_c m;
	matrix_rotation_euler(m, cml::rad([[scene camera] rotation][0]), cml::rad([[scene camera] rotation][1]), cml::rad([[scene camera] rotation][2]), cml::euler_order_xyz);
	movement = transform_vector(m, movement);

#if SCENE == kSceneNY
	vector3f npos = [[scene camera] position] + (movement * speedModifier / 8.0); 
#elif SCENE == kSceneTerrain
	vector3f npos = [[scene camera] position] + (movement * speedModifier / 1.0); 
#endif

	
	if (!PLAY_DEMO)
	{
	vector3f intersectionPoint = [mesh intersectWithLineStart:vector3f(npos[0], npos[1] - DIST, npos[2]) end:vector3f(npos[0], 1000, npos[2])];

	if (intersectionPoint[1] != FLT_MAX)
		npos = vector3f(npos[0], intersectionPoint[1] + DIST, npos[2]);
	}

	[[scene camera] setPosition:npos];

}

- (void)mouseDragged:(vector2f)delta withFlags:(uint32_t)flags
{
	vector3f rot = [[scene camera] rotation];

	 rot[1] -= delta[0] / 10.0;
	 rot[0] -= delta[1] / 10.0;

	 [[scene camera] setRotation:rot];
}

#if !TARGET_OS_IPHONE && !defined(linux)
- (void)rightMouseUp:(NSEvent *)event
{
	speedModifier = 1.0;
}
#endif

- (void)scrollWheel:(float)delta
{
	speedModifier += delta / 100.0;

	if (speedModifier > 4.0) speedModifier = 4.0;
	if (speedModifier < 0.01) speedModifier = 0.01;
}

- (void)resetCamera
{
//	[[scene camera] setPosition:vector3f(30, 45, 0)];
//	[[scene camera] setRotation:vector3f(-12, 0, 0)];

#if SCENE == kSceneNY

	[[scene camera] setPosition:vector3f(100, 100, 0)];
	[[scene camera] setRotation:vector3f(-90, 0, 0)];
#elif SCENE == kSceneTerrain

	[[scene camera] setPosition:vector3f(0, 12000, 0)];
	[[scene camera] setRotation:vector3f(-90, 0, 0)];
#endif
//	[[scene camera] setRotation:vector3f(-15, -180, 0)];

}
@end

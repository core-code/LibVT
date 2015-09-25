//
//  Scene.m
//  Core3D
//
//  Created by Julian Mayer on 16.11.07.
//  Copyright 2007 - 2010 A. Julian Mayer.
//
/*
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#import "Core3D.h"

NSMutableArray *tmpMsg;
NSMutableArray *pressedKeys;

Info globalInfo;
Settings globalSettings;
Scene *scene = nil;

@implementation Scene

@synthesize camera, lights, objects, simulator;

+ sharedScene
{
	if (scene == nil)
		scene = [[self alloc] init];

	return scene;
}

- (id)init
{
	if ((self = [super init]))
	{
#ifdef WIN32

#endif
		globalInfo.renderpass = kMainRenderPass;
		globalInfo.frame = 0;


		if ([[NSString stringWithUTF8String:(const char *)glGetString(GL_VENDOR)] hasPrefix:@"NVIDIA"])		globalInfo.gpuVendor = kNVIDIA;
		else if ([[NSString stringWithUTF8String:(const char *)glGetString(GL_VENDOR)] hasPrefix:@"ATI"])		globalInfo.gpuVendor = kATI;

		globalSettings.shadowFiltering = (shadowFilteringEnum) [[NSUserDefaults standardUserDefaults] integerForKey:@"shadowFiltering"];
		globalSettings.shadowMode = (shadowModeEnum) [[NSUserDefaults standardUserDefaults] integerForKey:@"shadowMode"];

		std::srand(time(NULL));
#ifdef WIN32
		init_opengl_function_pointers();
#endif
		NanosecondsInit();

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
#ifndef GL_ES_VERSION_2_0
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif

	//	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);


		lights = [[NSMutableArray alloc] initWithCapacity:8];
		objects = [[NSMutableArray alloc] initWithCapacity:20];
		camera = [[Camera alloc] init];
	}

	return self;
}

- (void)reshape:(NSArray *)size
{
	[camera reshape:size];

	[objects makeObjectsPerformSelector:@selector(reshape:) withObject:size];
}

- (void)processKeys
{
#ifndef TARGET_OS_IPHONE
	NSString *keyToErase = nil;
	for (NSString *keyHit in pressedKeys)
	{
		keyToErase = keyHit;
		switch ([keyHit intValue])
		{
			case NSF1FunctionKey:
				globalSettings.enablePostprocessing = !globalSettings.enablePostprocessing;
				[tmpMsg replaceObjectsInRange:NSMakeRange(0, 1) withObjectsFromArray:[NSArray arrayWithObjects:[NSDate date], globalSettings.enablePostprocessing ? @"PostProcessing ON" : @"PostProcessing OFF", nil]];
				break;
			case NSF2FunctionKey:
				globalSettings.displayFPS = !globalSettings.displayFPS;
				break;
			case NSF3FunctionKey:
				globalSettings.doWireframe = !globalSettings.doWireframe;
				[tmpMsg replaceObjectsInRange:NSMakeRange(0, 1) withObjectsFromArray:[NSArray arrayWithObjects:[NSDate date], globalSettings.doWireframe ? @"Wireframe ON" : @"Wireframe OFF", nil]];
				break;
			case NSF4FunctionKey:
				globalSettings.displayNormals = !globalSettings.displayNormals;
				[tmpMsg replaceObjectsInRange:NSMakeRange(0, 1) withObjectsFromArray:[NSArray arrayWithObjects:[NSDate date], globalSettings.displayNormals ? @"Displaying Normals ON" : @"Displaying Normals OFF", nil]];
				break;
			case NSF5FunctionKey:
				globalSettings.displayOctree = !globalSettings.displayOctree;
				[tmpMsg replaceObjectsInRange:NSMakeRange(0, 1) withObjectsFromArray:[NSArray arrayWithObjects:[NSDate date], globalSettings.displayOctree ? @"Displaying Octree ON" : @"Displaying Octree OFF", nil]];
				break;
			case NSF6FunctionKey:
				globalSettings.disableVFC = !globalSettings.disableVFC;
				[tmpMsg replaceObjectsInRange:NSMakeRange(0, 1) withObjectsFromArray:[NSArray arrayWithObjects:[NSDate date], globalSettings.disableVFC ? @"VFC OFF" : @"VFC ON", nil]];
				break;
			case NSF7FunctionKey:
				globalSettings.disableTex = !globalSettings.disableTex;
				[tmpMsg replaceObjectsInRange:NSMakeRange(0, 1) withObjectsFromArray:[NSArray arrayWithObjects:[NSDate date], globalSettings.disableVFC ? @"TEX OFF" : @"TEX ON", nil]];
				break;
			default:
				keyToErase = nil;
				break;
		}
	}
	if (keyToErase)	[pressedKeys removeObject:keyToErase];
#endif
}

- (void)update
{
//uint64_t micro = GetNanoseconds() / 1000
	globalInfo.frame++;

	[self processKeys];

	[simulator update];
	[camera update];
	[lights makeObjectsPerformSelector:@selector(update)];
	[objects makeObjectsPerformSelector:@selector(update)];

//	NSLog(@"frame update %i", GetNanoseconds() / 1000 - micro);
}

- (void)render
{
//	uint64_t micro = GetNanoseconds() / 1000
	globalInfo.renderedFaces = globalInfo.visitedNodes = globalInfo.drawCalls = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	[camera identity];

	[camera transform];

	[lights makeObjectsPerformSelector:@selector(render)];

	[objects makeObjectsPerformSelector:@selector(render)];

	[simulator render];

	glError()
//	NSLog(@"frame render %i", GetNanoseconds() / 1000 - micro);
}
@end

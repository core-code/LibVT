//
//  VirtualTexturingNode.m
//  VTDemo
//
//  Created by Julian Mayer on 12.08.09.
/*	Copyright (c) 2010 A. Julian Mayer
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#import "Simulation.h"
#import "LibVT_Internal.h"
#import "LibVT.h"
#import "LibVT_Config.h"
//#import "Mesh.h"

extern vtData vt;

#define NORMAL_TEXUNIT 5

#if !TARGET_OS_IPHONE
BMFont		*hudFont;
#endif
uint8_t		debugMode;

@implementation VirtualTexturingNode

- (id)init
{
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}

static GLuint ct, dt, fbo, info;


- (id)initWithTileStore:(NSString *)path format:(NSString *)format border:(char)border miplength:(char)miplength tilesize:(int)tilesize
{
	if ((self = [super init]))
	{


		vtInit([path UTF8String], [format UTF8String], border, miplength, tilesize);

		char *prelude = vtGetShaderPrelude();

		combined = LoadShaders(@"combined", [NSString stringWithUTF8String:prelude]);



		free(prelude);

		vtPrepare(combined, combined);

		glGenTextures(1, &info);
		vtPrepareOpenCL(info);

#if !TARGET_OS_IPHONE
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(applicationWillTerminate:)
													 name:NSApplicationWillTerminateNotification object:nil];
#endif
	}

	return self;
}



- (void)reshapeNode:(NSArray *)size
{
	int w = [[size objectAtIndex:0] intValue], h = [[size objectAtIndex:1] intValue];

	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	if (!ct) glGenTextures(1, &ct);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ct);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	assert(info);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, info);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	if (!dt) glGenTextures(1, &dt);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, dt);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_NONE );
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	if (!fbo) glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, ct, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, info, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, dt, 0);

	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
		vt_fatal("Error: couldn't setup FBO %04x\n", (unsigned int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	vtReshape([[size objectAtIndex:0] intValue], [[size objectAtIndex:1] intValue], (float)[[scene camera] fov], [[scene camera] nearPlane], [[scene camera] farPlane]);
}

- (void)render
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum mrt[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
	glDrawBuffers(2, mrt);

	//if (globalInfo.frame > 1)
	vtMapNewPages();


	glUseProgram(combined);
		glUniform1f(glGetUniformLocation(combined, "mip_bias"), vtGetBias());
		[children makeObjectsPerformSelector:@selector(render)];
	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	vtPerformOpenCLBufferReduction();
	vtExtractNeededPagesOpenCL();


	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
    glBlitFramebufferEXT(0, 0, vt.w, vt.h,
                       0, 0, vt.real_w, vt.real_h,
                       GL_COLOR_BUFFER_BIT,
                       GL_NEAREST);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
	NSLog(@"minMS: %f maxMS: %f avgMS: %f", minMS, maxMS, allMS / (globalInfo.frame - 120));

	vtShutdown();
}
@end

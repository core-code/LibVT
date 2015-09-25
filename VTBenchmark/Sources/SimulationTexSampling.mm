//
//  Simulation.m
//  VTBenchmark
//
//  Created by Julian Mayer on 07.04.10.
/*	Copyright (c) 2010 A. Julian Mayer
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <OpenGL/glext.h>

void _RenderTexture(GLuint size);
GLuint LoadTexture2D(NSString *imagePath, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy);
GLuint LoadTextureArray(NSArray *imagePaths, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy);
GLuint LoadTexture3D(NSArray *imagePaths, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy);
#define MICROSTART(x)	(x) = GetNanoseconds() / 1000
#define MICROSTOP(x)	(x) = ((GetNanoseconds() / 1000) - (x))

#import "Simulation.h"

@implementation Simulation


- (id)init {
    if ((self = [super init]))
	{

		NSString *texPath1 = [[NSBundle mainBundle] pathForResource:@"1" ofType:@"png"];
		NSString *texPath2 = [[NSBundle mainBundle] pathForResource:@"2" ofType:@"png"];
		NSString *texPath3 = [[NSBundle mainBundle] pathForResource:@"3" ofType:@"png"];
		NSString *texPath4 = [[NSBundle mainBundle] pathForResource:@"4" ofType:@"png"];

	//	texName = LoadTexture3D([NSArray arrayWithObjects:texPath1, texPath2, texPath3, texPath4, nil], GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);
		glActiveTexture(GL_TEXTURE1);
		texName = LoadTexture2D(texPath1, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);	
		glActiveTexture(GL_TEXTURE2);
		texName2 = LoadTexture2D(texPath2, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);	
		glActiveTexture(GL_TEXTURE3);
		texName3 = LoadTexture2D(texPath3, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);	
		glActiveTexture(GL_TEXTURE4);
		texName4 = LoadTexture2D(texPath4, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);	
		
	//	arrayProg = LoadShaders(@"array", nil);
		threedProg = LoadShaders(@"3d", nil);
		multiProg = LoadShaders(@"multi", nil);		
		
//		NSString *texPath = [[NSBundle mainBundle] pathForResource:@"0" ofType:@"png"];
//		texName = LoadTexture3D(texPath, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);
		
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(applicationWillTerminate:)
													 name:NSApplicationWillTerminateNotification object:nil];
		
		glUseProgram(multiProg);
		glUniform1i(glGetUniformLocation(multiProg, "one"), 1);
		glUniform1i(glGetUniformLocation(multiProg, "two"), 2);
		glUniform1i(glGetUniformLocation(multiProg, "three"), 3);
		glUniform1i(glGetUniformLocation(multiProg, "four"), 4);
		glUseProgram(0);

	}
    return self;
}

- (void)update
{
}

- (void)render
{
	uint64_t images;
	MICROSTART(images);
	
	//glUseProgram(arrayProg);
//	glUseProgram(threedProg);
	glUseProgram(multiProg);
	
//	glBindTexture(GL_TEXTURE_3D, texName);
	_RenderTexture(1024);
	glUseProgram(0);
	
	MICROSTOP(images);
	//NSLog(@"%f ms ", images / 1000.0);
	
	if (globalInfo.frame > 120)
	{
		allMS += images / 1000.0;
	}
}

- (void)dealloc
{
    [super dealloc];
	
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
	NSLog(@"avgMS: %f", allMS / (globalInfo.frame - 120));
}
@end


void _RenderTexture(GLuint size)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	myColor(1.0, 1.0, 1.0, 1.0);
	//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	
	GLint showWidth = (size > globalInfo.width) ? globalInfo.width : size;
	GLint showHeight = (size > globalInfo.height) ? globalInfo.height : size;
	
	myClientStateVTN(kNeedDisabled, kNeedDisabled, kNeedDisabled);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(((GLfloat)showWidth/(GLfloat) globalInfo.width )*2.0f-1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(((GLfloat)showWidth/(GLfloat) globalInfo.width )*2.0f-1.0f, ((GLfloat)showHeight/(GLfloat) globalInfo.height)*2.0f-1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, ((GLfloat)showHeight/(GLfloat) globalInfo.height)*2.0f-1.0f);
	glEnd();
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	globalInfo.drawCalls++;
}
GLuint LoadTexture2D(NSString *imagePath, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy)
{
	if (!imagePath)
		fatal("Error: can't load nil texture");
	
	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:imagePath], NULL);
	CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);
	GLuint texName;
	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);
	CGRect rect = {{0, 0}, {width, height}};
	void *data = calloc(width * 4, height);

	CGContextRef bitmapContext = CGBitmapContextCreate (data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host ); //

	
	CGContextTranslateCTM (bitmapContext, 0, height);
	CGContextScaleCTM (bitmapContext, 1.0, -1.0);
	
	CGContextDrawImage(bitmapContext, rect, imageRef);
	CGContextRelease(bitmapContext);
	
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, mipmap);
	
	
	if (anisontropy > 1.0)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisontropy);
	
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	glPopClientAttrib();
	CGImageRelease(imageRef);
	CFRelease(imageSourceRef);

	
	free(data);
	
	return texName;
}


GLuint LoadTextureArray(NSArray *imagePaths, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy)
{
	GLuint texName;
	
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures(1, &texName);
	glBindTexture(0x8C1A, texName);
	
	glTexParameteri(0x8C1A, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(0x8C1A, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(0x8C1A, GL_GENERATE_MIPMAP, mipmap);
	glTexParameteri( 0x8C1A,
					GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	
	if (anisontropy > 1.0)
		glTexParameterf(0x8C1A, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisontropy);
	
	glTexImage3D(0x8C1A, 0, GL_RGBA8, 512, 512, 4, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
    for (int i = 0; i < 4; i++)
	{
		if (![imagePaths objectAtIndex:i])
			fatal("Error: can't load nil texture");
		
		CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:[imagePaths objectAtIndex:i]], NULL);
		CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);
		size_t width = CGImageGetWidth(imageRef);
		size_t height = CGImageGetHeight(imageRef);
		CGRect rect = {{0, 0}, {width, height}};
		void *data = calloc(width * 4, height);
		
		CGContextRef bitmapContext = CGBitmapContextCreate (data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host ); //
		
		
		CGContextTranslateCTM (bitmapContext, 0, height);
		CGContextScaleCTM (bitmapContext, 1.0, -1.0);
		
		CGContextDrawImage(bitmapContext, rect, imageRef);
		CGContextRelease(bitmapContext);
		
        glTexSubImage3D( 0x8C1A, 0, 0, 0, i, width, height, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
		
		free(data);
		CGImageRelease(imageRef);
		CFRelease(imageSourceRef);

    }

	glPopClientAttrib();

	
	
	
	return texName;
}
GLuint LoadTexture3D(NSArray *imagePaths, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy)
{
	GLuint texName;
	
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 512);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_3D, texName);
	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP, mipmap);
	glTexParameteri( GL_TEXTURE_3D,
					GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	
	if (anisontropy > 1.0)
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisontropy);
	
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 512, 512, 4, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
    for (int i = 0; i < 4; i++)
	{
		if (![imagePaths objectAtIndex:i])
			fatal("Error: can't load nil texture");
		
		CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:[imagePaths objectAtIndex:i]], NULL);
		CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);
		size_t width = CGImageGetWidth(imageRef);
		size_t height = CGImageGetHeight(imageRef);
		CGRect rect = {{0, 0}, {width, height}};
		void *data = calloc(width * 4, height);
		
		CGContextRef bitmapContext = CGBitmapContextCreate (data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host ); //
		
		
		CGContextTranslateCTM (bitmapContext, 0, height);
		CGContextScaleCTM (bitmapContext, 1.0, -1.0);
		
		CGContextDrawImage(bitmapContext, rect, imageRef);
		CGContextRelease(bitmapContext);
		
        glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
		
		free(data);
		CGImageRelease(imageRef);
		CFRelease(imageSourceRef);
		
    }
	
	glPopClientAttrib();
	
	
	
	
	return texName;
}



//
//  VirtualTexturingNode.h
//  VTDemo
//
//  Created by Julian Mayer on 12.08.09.
/*	Copyright (c) 2010 A. Julian Mayer
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef __APPLE__
#import "BMFont.h"
#endif

#define MICROSTART(x)	(x) = GetNanoseconds() / 1000
#define MICROSTOP(x)	(x) = ((GetNanoseconds() / 1000) - (x))

@interface VirtualTexturingNode : SceneNode
{
	int anis;
	float minMS, maxMS;
	double allMS;

	float bias;
	GLuint readbackShader, renderVTShader, combined;
}

- (id)initWithTileStore:(NSString *)path format:(NSString *)format border:(char)border miplength:(char)miplength tilesize:(int)tilesize;
- (void)renderHUD:(float)duration prepassTime:(float)prepass readbackTime:(float)readback extractTime:(float)extract uploadTime:(float)upload;
@end

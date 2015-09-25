//
//  CocoaOpenGLView.m
//  Core3D
//
//  Created by Julian Mayer on 14.11.07.
//  Copyright 2007 - 2010 A. Julian Mayer.
//
/*
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#import "Core3D.h"
#import "CocoaOpenGLView.h"
#include "Simulation.h"


extern char buf[256];


@implementation CocoaOpenGLView

- (void)awakeFromNib
{
#ifndef WIN32
	ProcessSerialNumber psn;
	GetCurrentProcess(&psn);
	SetFrontProcess(&psn);
#endif

	timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode]; // ensure timer fires during resize

	pressedKeys = [[NSMutableArray alloc] initWithCapacity:5];

	[[self window] zoom:self];
}

- (void)animationTimer:(NSTimer *)timer
{
	//[self drawRect:[self bounds]]; // redraw now instead dirty to enable updates during live resize
	[self setNeedsDisplay:YES];
}

- (void)prepareOpenGL
{
	scene = [Scene sharedScene];

	id sim = [[[NSClassFromString([[NSBundle mainBundle] objectForInfoDictionaryKey:@"SimulationClass"]) alloc] init] autorelease];
	if (sim)
		[scene setSimulator:sim];
	else
		fatal("Error: there is no valid simulation class");

#ifndef WIN32
	const GLint swap = !globalSettings.disableVBLSync;
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swap);
#else
	typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapIntervalEXT(!globalSettings.disableVBLSync);
#endif
}

- (void)reshape
{
#ifndef WIN32
	[[self openGLContext] update];
#endif
	[scene reshape:[NSArray arrayWithObjects:[NSNumber numberWithInt:[self bounds].size.width], [NSNumber numberWithInt:[self bounds].size.height], nil]];
}

- (void)drawRect:(NSRect)rect
{
	[scene update];
	[scene render];

	[[self openGLContext] flushBuffer];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
	return YES;
}

- (void)keyDown:(NSEvent *)theEvent
{
	[pressedKeys addObject:[[NSNumber numberWithUnsignedInt:[[theEvent characters] characterAtIndex:0]] stringValue]];
}

- (void)keyUp:(NSEvent *)theEvent
{
#ifdef WIN32
	[pressedKeys removeObject:[[NSNumber numberWithUnsignedInt:[[theEvent characters] characterAtIndex:0] + 32] stringValue]];
#else
	[pressedKeys removeObject:[[NSNumber numberWithUnsignedInt:[[theEvent characters] characterAtIndex:0]] stringValue]];
#endif
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if ([[scene simulator] respondsToSelector:@selector(mouseDown:)])		[[scene simulator] mouseDown:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
	if ([[scene simulator] respondsToSelector:@selector(mouseUp:)])			[[scene simulator] mouseUp:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	if ([[scene simulator] respondsToSelector:@selector(rightMouseDown:)])	[[scene simulator] rightMouseDown:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	if ([[scene simulator] respondsToSelector:@selector(rightMouseUp:)])	[[scene simulator] rightMouseUp:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	if ([[scene simulator] respondsToSelector:@selector(mouseDragged:withFlags:)])
		[(Simulation *)[scene simulator] mouseDragged:vector2f([theEvent deltaX], [theEvent deltaY]) withFlags:(uint32_t)[theEvent modifierFlags]];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	if ([[scene simulator] respondsToSelector:@selector(scrollWheel:)])		[(Simulation *)[scene simulator] scrollWheel:[theEvent deltaY]];
}
@end

int main(int argc, char *argv[])
{
#ifndef WIN32
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];

	if (!PreCheckOpenGL())
	{
		NSRunAlertPanel(@"Error", @"Core3D requires OpenGL 2.0 and EXT_framebuffer_object!", @"OK, I'll upgrade", nil, nil);
		exit(1);
	}

	[NSBundle loadNibNamed:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSMainNibFile"] owner:NSApp];
	[pool release];
	[NSApp run];
#else
	return NSApplicationMain(argc,  (const char **) argv);
#endif
}

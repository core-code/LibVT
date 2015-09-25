/*

File: iPhoneEAGL2View.h
Abstract: Convenience class that wraps the CAEAGLLayer from CoreAnimation into a
UIView subclass.

Version: 1.7

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the following terms, and your
use, installation, modification or redistribution of this Apple software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject
to these terms, Apple grants you a personal, non-exclusive license, under
Apple's copyrights in this original Apple software (the "Apple Software"), to
use, reproduce, modify and redistribute the Apple Software, with or without
modifications, in source and/or binary forms; provided that if you redistribute
the Apple Software in its entirety and without modifications, you must retain
this notice and the following text and disclaimers in all such redistributions
of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Inc. may be used
to endorse or promote products derived from the Apple Software without specific
prior written permission from Apple.  Except as expressly stated in this notice,
no other rights or licenses, express or implied, are granted by Apple herein,
including but not limited to any patent rights that may be infringed by your
derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR
DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF
CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

//CLASSES:

@class iPhoneEAGL2View;
@class Scene;
//PROTOCOLS:

@protocol iPhoneEAGL2ViewDelegate <NSObject>
- (void) didResizeEAGLSurfaceForView:(iPhoneEAGL2View *)view; //Called whenever the EAGL surface has been resized
@end

//CLASS INTERFACE:

/*
 This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
 The view content is basically an EAGL surface you render your OpenGL scene into.
 Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
 */
@interface iPhoneEAGL2View : UIView <UIAccelerometerDelegate>
{
@private
	NSString				*_format;
	GLuint					_depthFormat;
	BOOL					_autoresize;
	EAGLContext				*_context;
	GLuint					_framebuffer;
	GLuint					_renderbuffer;
	GLuint					_depthBuffer;
	CGSize					_size;
	BOOL					_hasBeenCurrent, displayLinkSupported;
	id<iPhoneEAGL2ViewDelegate>	_delegate;
	id displayLink;

	NSTimer *timer;
	Scene *scene;
}
- (id)initWithCoder:(NSCoder *)coder;

@property (readonly) GLuint framebuffer;
@property (readonly) NSString *pixelFormat;
@property (readonly) GLuint depthFormat;
@property (readonly) EAGLContext *context;

@property BOOL autoresizesSurface; //NO by default - Set to YES to have the EAGL surface automatically resized when the view bounds change, otherwise the EAGL surface contents is rendered scaled
@property (readonly, nonatomic) CGSize surfaceSize;

@property (assign) id<iPhoneEAGL2ViewDelegate> delegate;


- (void) swapBuffers; //This also checks the current OpenGL error and logs an error if needed

- (void) start;

@end

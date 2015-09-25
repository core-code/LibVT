#import "Utilities.h"


#ifndef GL_OES_VERSION_1_0
// Based on Apple sample code:
// http://developer.apple.com/samplecode/GLSLShowpiece/listing6.html

GLuint LoadShaders(NSString *shadername, NSString *preprocessorDefines)
{
#define LOAD_SHADER(shaderString, shader_string, shader, shader_compiled, GL_SHADER)\
if ((shaderString))																	\
{																					\
if (preprocessorDefines)															\
(shaderString) = [preprocessorDefines stringByAppendingString:(shaderString)];		\
(shader) = glCreateShader((GL_SHADER));												\
(shader_string) = (GLchar *) [(shaderString) UTF8String];							\
glShaderSource((shader), 1, &(shader_string), NULL);								\
glCompileShader((shader));															\
glGetShaderiv((shader), GL_COMPILE_STATUS, &(shader_compiled));						\
GLint infoLogLength;																\
glGetShaderiv((shader), GL_INFO_LOG_LENGTH, &infoLogLength);						\
if (infoLogLength > 0)																\
{																					\
infoLog = (char *) malloc((infoLogLength + 1) * sizeof(char));						\
glGetShaderInfoLog ((shader), infoLogLength, NULL, infoLog);						\
NSLog(@"Warning: shader log: %s\n", infoLog);										\
}																					\
}

	char * infoLog = 0;
	NSString *vertexString = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:shadername ofType:@"vert"] encoding:NSUTF8StringEncoding error:NULL];
	NSString *fragmentString = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:shadername ofType:@"frag"] encoding:NSUTF8StringEncoding error:NULL];
	GLuint vertex_shader = 0, fragment_shader = 0, program_object;
	const GLchar *vertex_string, *fragment_string;
	GLint vertex_compiled = 1, fragment_compiled = 1, linked;
	NSString *openglESSupport = @"#ifdef GL_ES\nprecision highp float;\n#endif\n";
	if (!vertexString && !fragmentString) fatal("Error: can't load empty shaders");

	if (preprocessorDefines == nil) preprocessorDefines = openglESSupport;
	else preprocessorDefines = [openglESSupport stringByAppendingString:preprocessorDefines];
	LOAD_SHADER(vertexString, vertex_string, vertex_shader, vertex_compiled, GL_VERTEX_SHADER)
	LOAD_SHADER(fragmentString, fragment_string, fragment_shader, fragment_compiled, GL_FRAGMENT_SHADER)

	if (!vertex_compiled || !fragment_compiled)
		fatal("Error: couldn't compile shaders: \n%s\nVERTEX SHADER:\n%s\n\nFRAGMENT SHADER:\n%s\n", infoLog, [vertexString UTF8String], [fragmentString UTF8String]); // should do cleanup if we don't wanna panic here


	program_object = glCreateProgram();
	if (vertex_shader != 0)
	{
		glAttachShader(program_object, vertex_shader);
		glDeleteShader(vertex_shader);
	}
	if (fragment_shader != 0)
	{
		glAttachShader(program_object, fragment_shader);
		glDeleteShader(fragment_shader);
	}
	glLinkProgram(program_object);
	glGetProgramiv(program_object, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint infoLogLength;
		glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &infoLogLength);
		char * infoLog = (char *) malloc((infoLogLength + 1) * sizeof(char));
		glGetProgramInfoLog (program_object, infoLogLength, NULL, infoLog);

		fatal("Error: couldn't link shaders:\n%s\n%s\n%s\n", infoLog, [vertexString UTF8String], [fragmentString UTF8String]);
	}
	return program_object;
}
#endif


// Based on Apple sample code:
// http://developer.apple.com/DOCUMENTATION/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_texturedata/chapter_10_section_5.html#//apple_ref/doc/uid/TP40001987-CH407-SW31
#ifdef __APPLE__
GLuint LoadTexture(NSString *imagePath, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy)
{
	if (!imagePath)
		fatal("Error: can't load nil texture");

#ifndef TARGET_OS_IPHONE
	CGImageSourceRef imageSourceRef = CGImageSourceCreateWithURL((CFURLRef)[NSURL fileURLWithPath:imagePath], NULL);
	CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSourceRef, 0, NULL);
#else
	CGImageRef imageRef = [UIImage imageWithContentsOfFile:imagePath].CGImage;
#endif
	GLuint texName;
	size_t width = CGImageGetWidth(imageRef);
	size_t height = CGImageGetHeight(imageRef);
	CGRect rect = {{0, 0}, {width, height}};
	void *data = calloc(width * 4, height);
#ifndef TARGET_OS_IPHONE // does CGColorSpaceCreateDeviceRGB() really leak?
	CGContextRef bitmapContext = CGBitmapContextCreate (data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host ); //
#else
	CGContextRef bitmapContext = CGBitmapContextCreate (data, width, height, 8, width * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast);
#endif

	CGContextTranslateCTM (bitmapContext, 0, height);
	CGContextScaleCTM (bitmapContext, 1.0, -1.0);

	CGContextDrawImage(bitmapContext, rect, imageRef);
	CGContextRelease(bitmapContext);

#ifndef TARGET_OS_IPHONE
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
#ifdef GL_GENERATE_MIPMAP
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, mipmap);
#else
	glGenerateMipmap(GL_TEXTURE_2D);
#endif

#ifndef TARGET_OS_IPHONE
	if (anisontropy > 1.0)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisontropy);


	glTexImage2D(GL_TEXTURE_2D, 0, globalSettings.disableTextureCompression ? GL_RGBA : GL_COMPRESSED_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
	glPopClientAttrib();
	CGImageRelease(imageRef);
	CFRelease(imageSourceRef);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
#endif

	free(data);

	return texName;
}
#elif defined(WIN32)
#include "IL/il.h"
GLuint LoadTexture(NSString *imagePath, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy)
{
//	int format = 0;
//	ILuint ilName;
//	char *data;
//	NSSize imgSize;
//	GLuint texName;
//
//	ilInit();
//	ilGenImages(1, &ilName);
//	ilBindImage(ilName);
//	ilLoadImage((char* )[imagePath UTF8String]);
//
//	data = (char *)ilGetData();
//	imgSize = NSMakeSize(ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));;
//
//	//glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
//	//glPixelStorei(GL_UNPACK_ROW_LENGTH, imgSize.width); 	// Set proper unpacking row length for bitmap.
//	//glPixelStorei (GL_UNPACK_ALIGNMENT, 1);				// Set byte aligned unpacking (needed for 3 byte per pixel bitmaps).
//
//	// Generate a new texture name if one was not provided.
//	glGenTextures (1, &texName);
//	glBindTexture (GL_TEXTURE_2D, texName);
//
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_LINEAR_MIPMAP_LINEAR);// minFilter
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // magFilter
//	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,  GL_TRUE); // mipmap
//	if (anisontropy > 1.0)
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisontropy);
//
//	format = ilGetInteger(IL_IMAGE_FORMAT);
//
//	// RGB 24 bit bitmap, or RGBA 32 bit bitmap.
//	if (format == IL_RGB || format == IL_BGR || format == IL_RGBA || format == IL_BGRA)
//	{
//		glTexImage2D(GL_TEXTURE_2D, 0,
//					 ((format == IL_RGB) || (format == IL_BGR)) ? GL_RGB8 : GL_RGBA8,
//					 imgSize.width,
//					 imgSize.height,
//					 0,
//					 format,
//					 GL_UNSIGNED_BYTE,
//					 data);
//
//	}
//	else
//		fatal("Error: couldn't load texture");
//
//	//glPopClientAttrib();
//
//	// Clean up.
//	ilDeleteImages(1, &ilName);
//
//	return texName;
}
#endif

GLuint LoadTextureNamed(NSString *name, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy)
{
	for (NSString *ext in IMG_EXTENSIONS)
	{
		NSString *path = [[NSBundle mainBundle] pathForResource:name ofType:ext];
		if (path)
			return LoadTexture(path, minFilter, magFilter, mipmap, anisontropy);
	}

	NSLog(@"Warning: could not find texture named: %@", name);

	return 0;
}

#ifndef GL_ES_VERSION_2_0
void DrawFullscreenQuad(short screenWidth, short screenHeight, short textureWidth, short textureHeight)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	matrix44f_c orthographicMatrix;
	matrix_orthographic_RH(orthographicMatrix, 0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f, cml::z_clip_neg_one);
	glLoadMatrixf(orthographicMatrix.data());

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	GLshort vertices[] = {0, 0,  screenWidth, 0,  screenWidth, screenHeight,  0, screenHeight};
	GLshort texCoords[] = {0, 0,  textureWidth, 0,  textureWidth, textureHeight,  0, textureHeight};
	GLubyte indices[] = {0,1,3, 1,2,3};

	myClientStateVTN(kNeedEnabled, kNeedEnabled, kNeedDisabled);

	glTexCoordPointer(2, GL_SHORT, 0, texCoords);
	glVertexPointer(2, GL_SHORT, 0, vertices);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	globalInfo.drawCalls++;
}
#endif

char AABoxInFrustum(const float frustum[6][4], float x, float y, float z, float ex, float ey, float ez) // adapted code from glm and lighthouse tutorial
{
	int p;
	int result = kInside, out,in;	// TODO: optimiziation: http://www.lighthouse3d.com/opengl/viewfrustum/index.php?gatest3

	for(p = 0; p < 6; p++)
	{
		out = 0;
		in = 0;

		if (frustum[p][0]*(x-ex) + frustum[p][1]*(y-ey) + frustum[p][2]*(z-ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x+ex) + frustum[p][1]*(y-ey) + frustum[p][2]*(z-ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x-ex) + frustum[p][1]*(y+ey) + frustum[p][2]*(z-ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x+ex) + frustum[p][1]*(y+ey) + frustum[p][2]*(z-ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x-ex) + frustum[p][1]*(y-ey) + frustum[p][2]*(z+ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x+ex) + frustum[p][1]*(y-ey) + frustum[p][2]*(z+ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x-ex) + frustum[p][1]*(y+ey) + frustum[p][2]*(z+ez) + frustum[p][3] < 0) out++; else in++;
		if (frustum[p][0]*(x+ex) + frustum[p][1]*(y+ey) + frustum[p][2]*(z+ez) + frustum[p][3] < 0) out++; else in++;

		if (!in)			// if all corners are out
			return (kOutside);
		else if (out)		// if some corners are out and others are in
			result = kIntersecting;
	}

	return(result);
}

#ifndef WIN32
#ifndef TARGET_OS_IPHONE
void RenderTexture(GLuint size)
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

void RenderAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	glVertex3f(minX, minY, minZ);
	glVertex3f(maxX, minY, minZ);

	glVertex3f(minX, minY, minZ);
	glVertex3f(minX, maxY, minZ);

	glVertex3f(minX, minY, minZ);
	glVertex3f(minX, minY, maxZ);

	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(minX, maxY, maxZ);

	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(maxX, minY, maxZ);

	glVertex3f(maxX, maxY, maxZ);
	glVertex3f(maxX, maxY, minZ);

	glVertex3f(minX, maxY, minZ);
	glVertex3f(maxX, maxY, minZ);

	glVertex3f(minX, maxY, minZ);
	glVertex3f(minX, maxY, maxZ);

	glVertex3f(minX, minY, maxZ);
	glVertex3f(maxX, minY, maxZ);

	glVertex3f(minX, minY, maxZ);
	glVertex3f(minX, maxY, maxZ);

	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, minY, maxZ);

	glVertex3f(maxX, minY, minZ);
	glVertex3f(maxX, maxY, minZ);
}

BOOL PreCheckOpenGL(void)
{
	BOOL hasOpenGL2 = FALSE;
	CGLPixelFormatAttribute attribs[] = {kCGLPFAAccelerated, (CGLPixelFormatAttribute)NULL};
	CGLPixelFormatObj pixelFormat = NULL;
	long numPixelFormats = 0;
	CGLContextObj myCGLContext = 0, curr_ctx = CGLGetCurrentContext ();
	CGLChoosePixelFormat(attribs, &pixelFormat, (GLint*)&numPixelFormats);
	if (pixelFormat)
	{
		CGLCreateContext(pixelFormat, NULL, &myCGLContext);
		CGLDestroyPixelFormat(pixelFormat);
		CGLSetCurrentContext(myCGLContext);

		if (myCGLContext)
		{
			hasOpenGL2 = ([[[NSString stringWithUTF8String:(const char *)glGetString(GL_VERSION)] substringToIndex:1] intValue] >= 2);
		}
	}
	CGLDestroyContext(myCGLContext);
	CGLSetCurrentContext(curr_ctx);

	return hasOpenGL2;
}
#endif
#endif


#ifdef WIN32
#include <windows.h>
LARGE_INTEGER freq;
void NanosecondsInit()
{
	QueryPerformanceFrequency(&freq);
}
uint64_t GetNanoseconds()
{
#ifdef DEBUG
	if (freq.QuadPart == 0)
		fatal("Error: Nano timer not inited");
#endif
	LARGE_INTEGER ntime;
	QueryPerformanceCounter(&ntime);

	return (uint64_t)((double)ntime.QuadPart * 1000.0 /((double)freq.QuadPart / (1000.0 * 1000.0)));
}

#else
mach_timebase_info_data_t sTimebaseInfo;
void NanosecondsInit()
{
	mach_timebase_info(&sTimebaseInfo);
}

uint64_t GetNanoseconds()
{
#ifdef DEBUG
	if (sTimebaseInfo.denom == 0)
		fatal("Error: Nano timer not inited");
#endif

	return (mach_absolute_time() * sTimebaseInfo.numer / sTimebaseInfo.denom);
}
#endif



#ifdef TARGET_OS_IPHONE
SOUND_TYPE LoadSound(NSString *name)
{
	SystemSoundID soundID;
	AudioServicesCreateSystemSoundID((CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:(name) ofType:@"caf"]], &soundID);
	return soundID;
}
void PlaySound(SOUND_TYPE soundID)
{
	AudioServicesPlaySystemSound(soundID);
}
void UnloadSound(SOUND_TYPE soundID)
{
	AudioServicesDisposeSystemSoundID(soundID);
	soundID = 0;
}
#else
SOUND_TYPE LoadSound(NSString *name)
{
#ifdef WIN32
	return [[NSSound soundNamed:name] retain];
#elif defined(__APPLE__)
	NSString *aiffPath = [[NSBundle mainBundle] pathForResource:(name) ofType:@"aiff"];
	NSString *wavPath = [[NSBundle mainBundle] pathForResource:(name) ofType:@"wav"];

	if (!aiffPath && !wavPath)
	{	fatal("Error: there is no sound named: %s", [name UTF8String]); }

	return  [[NSSound alloc] initWithContentsOfURL:(aiffPath ? [NSURL fileURLWithPath:aiffPath] : [NSURL fileURLWithPath:wavPath]) byReference:NO];
#endif
}
void PlaySound(SOUND_TYPE soundID)
{
	[soundID play];
}
void UnloadSound(SOUND_TYPE soundID)
{
	[soundID release];
	soundID = nil;
}
#endif

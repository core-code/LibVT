#include "windows.h"

#if defined(_MSC_VER)
#include <stdio.h>
#include <gl/gl.h>
#include "glext.h"
#include "wglext.h"
#endif

// opengl 1.3
PFNGLACTIVETEXTUREPROC glActiveTexture = 0;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = 0;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = 0;
// opengl 1.5
PFNGLGENBUFFERSPROC glGenBuffers = 0;
PFNGLBINDBUFFERPROC glBindBuffer = 0;
PFNGLBUFFERDATAPROC glBufferData = 0;
PFNGLMAPBUFFERPROC glMapBuffer = 0;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = 0;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0;
// opengl 2.0
PFNGLCREATESHADERPROC glCreateShader = 0;
PFNGLSHADERSOURCEPROC glShaderSource = 0;
PFNGLCOMPILESHADERPROC glCompileShader = 0;
PFNGLGETSHADERIVPROC glGetShaderiv = 0;
PFNGLDELETESHADERPROC glDeleteShader = 0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0;
PFNGLATTACHSHADERPROC glAttachShader = 0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0;
PFNGLUSEPROGRAMPROC glUseProgram = 0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0;
PFNGLUNIFORM1IPROC glUniform1i = 0;
PFNGLUNIFORM1IVPROC glUniform1iv = 0;
PFNGLUNIFORM1FPROC glUniform1f = 0;
PFNGLUNIFORM3FPROC glUniform3f = 0;
PFNGLUNIFORM3FVPROC glUniform3fv = 0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0;
// GL_EXT_framebuffer_object
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = 0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = 0;

static inline void oglwin32_fatal(const char *err, ...) {va_list ap; va_start (ap, err); vfprintf (stderr, err, ap); va_end (ap); exit(1); }

void init_opengl_function_pointers()
{
	glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)wglGetProcAddress("glCompressedTexSubImage2D");
	if (!glCompressedTexSubImage2D) oglwin32_fatal("glCompressedTexSubImage2D is NULL");
	glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)wglGetProcAddress("glCompressedTexImage2D");
	if (!glCompressedTexImage2D) oglwin32_fatal("glCompressedTexImage2D is NULL");
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	if (!glGenBuffers) oglwin32_fatal("glGenBuffers is NULL");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	if (!glBindBuffer) oglwin32_fatal("glBindBuffer is NULL");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	if (!glBufferData) oglwin32_fatal("glBufferData is NULL");
	glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
	if (!glMapBuffer) oglwin32_fatal("glMapBuffer is NULL");
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
	if (!glUnmapBuffer) oglwin32_fatal("glUnmapBuffer is NULL");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	if (!glDeleteBuffers) oglwin32_fatal("glDeleteBuffers is NULL");
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	if (!glCreateShader) oglwin32_fatal("glCreateShader is NULL");
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	if (!glShaderSource) oglwin32_fatal("glShaderSource is NULL");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	if (!glCompileShader) oglwin32_fatal("glCompileShader is NULL");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	if (!glGetShaderiv) oglwin32_fatal("glGetShaderiv is NULL");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	if (!glDeleteShader) oglwin32_fatal("glDeleteShader is NULL");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	if (!glCreateProgram) oglwin32_fatal("glCreateProgram is NULL");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	if (!glAttachShader) oglwin32_fatal("glAttachShader is NULL");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	if (!glLinkProgram) oglwin32_fatal("glLinkProgram is NULL");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	if (!glGetProgramiv) oglwin32_fatal("glGetProgramiv is NULL");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	if (!glUseProgram) oglwin32_fatal("glUseProgram is NULL");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	if (!glGetUniformLocation) oglwin32_fatal("glGetUniformLocation is NULL");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	if (!glUniform1i) oglwin32_fatal("glUniform1i is NULL");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1iv");
	if (!glUniform1iv) oglwin32_fatal("glUniform1iv is NULL");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	if (!glUniform1f) oglwin32_fatal("glUniform1f is NULL");
	glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
	if (!glUniform3f) oglwin32_fatal("glUniform3f is NULL");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");
	if (!glUniform3fv) oglwin32_fatal("glUniform3fv is NULL");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");
	if (!glUniform4fv) oglwin32_fatal("glUniform4fv is NULL");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	if (!glUniformMatrix4fv) oglwin32_fatal("glUniformMatrix4fv is NULL");
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	if (!glGenFramebuffersEXT) oglwin32_fatal("glGenFramebuffersEXT is NULL");
	glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	if (!glGenRenderbuffersEXT) oglwin32_fatal("glGenRenderbuffersEXT is NULL");
	glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	if (!glBindFramebufferEXT) oglwin32_fatal("glBindFramebufferEXT is NULL");
	glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
	if (!glDeleteFramebuffersEXT) oglwin32_fatal("glDeleteFramebuffersEXT is NULL");
	glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	if (!glFramebufferTexture2DEXT) oglwin32_fatal("glFramebufferTexture2DEXT is NULL");
	glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	if (!glBindRenderbufferEXT) oglwin32_fatal("glBindRenderbufferEXT is NULL");
	glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	if (!glRenderbufferStorageEXT) oglwin32_fatal("glRenderbufferStorageEXT is NULL");
	glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	if (!glFramebufferRenderbufferEXT) oglwin32_fatal("glFramebufferRenderbufferEXT is NULL");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
	if (!glCheckFramebufferStatusEXT) oglwin32_fatal("glCheckFramebufferStatusEXT is NULL");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
	if (!glDrawBuffers) oglwin32_fatal("glDrawBuffers is NULL");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	if (!glGetProgramInfoLog) oglwin32_fatal("glGetProgramInfoLog is NULL");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	if (!glGetShaderInfoLog) oglwin32_fatal("glGetShaderInfoLog is NULL");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	if (!glActiveTexture) oglwin32_fatal("glActiveTexture is NULL");
}

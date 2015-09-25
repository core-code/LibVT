//
//  LibVT Virtual Texturing Shaders
//  Based on Sean Barrett's public domain "Sparse Virtual Textures" demo shaders
//
/*	Copyright (c) 2010 A. Julian Mayer
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#ifdef GL_ES
uniform mat4 matViewProjection;
attribute vec4 vertex;
attribute vec2 texcoord0;
varying vec2 texcoord;
void main( void )
{
    gl_Position = matViewProjection * vertex;
    texcoord    = texcoord0.xy;
}
#else
void main(void)
{
	gl_TexCoord[0]	= gl_MultiTexCoord0;
	gl_Position		= gl_ModelViewProjectionMatrix *  gl_Vertex;
}
#endif
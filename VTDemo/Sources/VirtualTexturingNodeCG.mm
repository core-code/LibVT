//
//  VirtualTexturingNodeCG.m
//  VTDemo
//
//  Created by Julian Mayer on 19.06.10.
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

#include <Cg/cg.h>    /* Can't include this?  Is Cg Toolkit installed! */
#include <Cg/cgGL.h>

extern vtData vt;
extern vtConfig c;

static CGcontext	cgContext;
static CGprofile	cgVertexProfile, cgFragmentProfile;
static CGprogram	cgReadbackVertexProgram, cgReadbackFragmentProgram, cgRenderVertexProgram, cgRenderFragmentProgram;
static CGparameter	cgRenderParamMipBias, cgReadbackParamMipBias, cgRenderParamModelViewMatrix, cgReadbackParamModelViewMatrix, cgReadbackParamMipcalcTexture, cgRenderParamPhysicalTexture, cgRenderParamPageTableTexture;

static void checkForCgError(const char *situation)
{
	CGerror error;
	const char *string = cgGetLastErrorString(&error);
	
	if (error != CG_NO_ERROR) {
		printf("%s: %s: %s\n",
			   "LibVT", situation, string);
		if (error == CG_COMPILER_ERROR) {
			printf("%s\n", cgGetLastListing(cgContext));
		}
		exit(1);
	}
}

char * loadTextFile(const char *filePath)
{
	uint32_t fs = 0;
	char *fileData;
	FILE *f;
	
	f = fopen(filePath, "rb");
	fseek(f, 0 , SEEK_END);
	fs = ftell(f);
	fseek(f, 0, SEEK_SET);
	fileData = (char *) calloc(1, fs+1);
	fread(fileData, 1, fs, f);
	fclose (f);
	
	return fileData;
}

@implementation VirtualTexturingNode

- (id)init
{
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}



- (id)initWithTileStore:(NSString *)path format:(NSString *)format border:(char)border miplength:(char)miplength tilesize:(int)tilesize
{
	if ((self = [super init]))
	{
		vtInit([path UTF8String], [format UTF8String], border, miplength, tilesize);
		

		
		char *prelude = vtGetShaderPrelude();
		string p_s = string(prelude);

		char * rb_v = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/readback_vert.cg");
		char * rb_f = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/readback_frag.cg");
		char * rVT_v = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/renderVT_vert.cg");
		char * rVT_f = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/renderVT_frag.cg");
		
		string rb_v_s = p_s + string(rb_v);
		string rb_f_s = p_s + string(rb_f);
		string rVT_v_s = p_s + string(rVT_v);
		string rVT_f_s = p_s + string(rVT_f);
				
		
		vtPrepare(0, 0);
		
		
		//printf(prelude);

		cgContext = cgCreateContext();
		checkForCgError("creating context");
		cgGLSetDebugMode(CG_FALSE);
		cgSetParameterSettingMode(cgContext, CG_DEFERRED_PARAMETER_SETTING);
		cgGLSetManageTextureParameters(cgContext, false);
		cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
		cgGLSetOptimalOptions(cgVertexProfile);
		checkForCgError("selecting vertex profile");
		cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
		cgGLSetOptimalOptions(cgFragmentProfile);
		checkForCgError("selecting fragment profile");
		
		
		// readback
		cgReadbackVertexProgram = cgCreateProgram(cgContext, CG_SOURCE, rb_v_s.c_str(), cgVertexProfile, "readback_vert", NULL);             
		checkForCgError("creating vertex program from file");
		cgGLLoadProgram(cgReadbackVertexProgram);
		checkForCgError("loading vertex program");
		
		cgReadbackParamModelViewMatrix = cgGetNamedParameter(cgReadbackVertexProgram, "ModelViewMatrix");
		checkForCgError("getting ModelViewMatrix parameter");
			
		
		cgReadbackFragmentProgram = cgCreateProgram(cgContext, CG_SOURCE, rb_f_s.c_str(), cgFragmentProfile, "readback_frag", NULL);              
		checkForCgError("creating fragment program from file");
		cgGLLoadProgram(cgReadbackFragmentProgram);
		checkForCgError("loading fragment program");
		
		cgReadbackParamMipcalcTexture = cgGetNamedParameter(cgReadbackFragmentProgram, "mipcalcTexture");
		checkForCgError("getting mipcalcTexture parameter");
		cgGLSetTextureParameter(cgReadbackParamMipcalcTexture, vt.mipcalcTexture);
		checkForCgError("setting mipcalcTexture texture");
		cgGLEnableTextureParameter(cgReadbackParamMipcalcTexture);
		checkForCgError("enabling mipcalcTexture texture");

		cgReadbackParamMipBias = cgGetNamedParameter(cgReadbackFragmentProgram, "mip_bias");
		checkForCgError("getting mip_bias parameter");	


		
		// renderVT
		cgRenderVertexProgram = cgCreateProgram(cgContext, CG_SOURCE, rVT_v_s.c_str(), cgVertexProfile, "rendervt_vert", NULL);             
		checkForCgError("creating vertex program from file");
		cgGLLoadProgram(cgRenderVertexProgram);
		checkForCgError("loading vertex program");
		
		cgRenderParamModelViewMatrix = cgGetNamedParameter(cgRenderVertexProgram, "ModelViewMatrix");
		checkForCgError("getting ModelViewMatrix parameter");
		
		
		cgRenderFragmentProgram = cgCreateProgram(cgContext, CG_SOURCE, rVT_f_s.c_str(), cgFragmentProfile, "rendervt_frag", NULL);              
		checkForCgError("creating fragment program from file");
		cgGLLoadProgram(cgRenderFragmentProgram);
		checkForCgError("loading fragment program");
		
		cgRenderParamPhysicalTexture = cgGetNamedParameter(cgRenderFragmentProgram, "physicalTexture");
		checkForCgError("getting physicalTexture parameter");
		cgGLSetTextureParameter(cgRenderParamPhysicalTexture, vt.physicalTexture);
		checkForCgError("setting physicalTexture texture");
		cgGLEnableTextureParameter(cgRenderParamPhysicalTexture);
		checkForCgError("enabling physicalTexture texture");
		
		cgRenderParamPageTableTexture = cgGetNamedParameter(cgRenderFragmentProgram, "pageTableTexture");
		checkForCgError("getting pageTableTexture parameter");
		cgGLSetTextureParameter(cgRenderParamPageTableTexture, vt.pageTableTexture);
		checkForCgError("setting pageTableTexture texture");
		cgGLEnableTextureParameter(cgRenderParamPageTableTexture);
		checkForCgError("enabling pageTableTexture texture");

		cgRenderParamMipBias = cgGetNamedParameter(cgRenderFragmentProgram, "mip_bias");
		checkForCgError("getting mip_bias parameter");

		
		free(prelude);
		free(rb_v);
		free(rb_f);
		free(rVT_v);
		free(rVT_f);
		
		[[NSNotificationCenter defaultCenter] addObserver:self
											  selector:@selector(applicationWillTerminate:)
											  name:NSApplicationWillTerminateNotification object:nil];		
		
	}
	
	return self;
}



- (void)reshapeNode:(NSArray *)size
{
	vtReshape([[size objectAtIndex:0] intValue], [[size objectAtIndex:1] intValue], (float)[[scene camera] fov], [[scene camera] nearPlane], [[scene camera] farPlane]);
}


- (void)render
{
	cgGLEnableProfile(cgVertexProfile);
	checkForCgError("enabling vertex profile");
	
	cgGLEnableProfile(cgFragmentProfile);
	checkForCgError("enabling fragment profile");
	
	
	
	
	
	vtPrepareReadback();
	
	cgGLBindProgram(cgReadbackVertexProgram);
	checkForCgError("binding vertex program");
	
	
	cgGLBindProgram(cgReadbackFragmentProgram);
	checkForCgError("binding fragment program");
	
	
	
	cgGLSetStateMatrixParameter(cgReadbackParamModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgSetParameter1f(cgReadbackParamMipBias, vtGetBias());

	
	cgGLEnableTextureParameter(cgReadbackParamMipcalcTexture);
	
	
	
	globalInfo.renderpass = (renderPassEnum) (kRenderPassUpdateVFC | kRenderPassUseTexture);
	[children makeObjectsPerformSelector:@selector(render)];
	
	
	vtPerformReadback();
	vtExtractNeededPages(NULL);
	vtMapNewPages();
	
	
	
	
	cgGLBindProgram(cgRenderVertexProgram);
	checkForCgError("binding vertex program");
	
	
	cgGLBindProgram(cgRenderFragmentProgram);
	checkForCgError("binding fragment program");
	
	
	
	cgGLSetStateMatrixParameter(cgRenderParamModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgSetParameter1f(cgRenderParamMipBias, vtGetBias());
	
	
	cgGLEnableTextureParameter(cgRenderParamPhysicalTexture);
	cgGLEnableTextureParameter(cgRenderParamPageTableTexture);

	globalInfo.renderpass = (renderPassEnum) (kRenderPassSetMaterial | kRenderPassUseTexture);
	[children makeObjectsPerformSelector:@selector(render)];

	
	
	globalInfo.renderpass = (renderPassEnum) kMainRenderPass;
	
	
	
	cgGLDisableProfile(cgVertexProfile);
	checkForCgError("disabling vertex profile");
	
	cgGLDisableProfile(cgFragmentProfile);
	checkForCgError("disabling fragment profile");
	
}


- (void)applicationWillTerminate:(NSNotification*)notification
{
	NSLog(@"minMS: %f maxMS: %f avgMS: %f", minMS, maxMS, allMS / (globalInfo.frame - 120));
	
	vtShutdown();
}

- (void)renderHUD:(float)duration prepassTime:(float)prepass readbackTime:(float)readback extractTime:(float)extract uploadTime:(float)upload
{}

@end

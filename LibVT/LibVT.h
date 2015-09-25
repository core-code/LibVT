/*
 *  LibVT.h
 *
 *
 *  Created by Julian Mayer on 07.10.09.
 *  Copyright 2009 A. Julian Mayer. 
 *
 */

/*
 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


/*!
 * @file	LibVT.h
 * @brief	The external header file for LibVT, declares public functions.
 */


/*!
 * @fn vtInit(const char *_tileDir, const char *_pageExtension, const uint8_t _pageBorder, const uint8_t _mipChainLength, const uint16_t _pageDimension)
 * @brief Initializes LibVT and must be called previous to all other functions.
 * @param[in] _tileDir			The full path to the tile directory.
 * @param[in] _pageExtension	This is the extension and format of the stored page tiles, Values: "jpg", "bmp", "png", "dxt1", "dxt5"
 * @param[in] _pageBorder		This is the border each page has in pixels, Values:	0-8
 * @param[in] _mipChainLength	The length of the mipchain, determined by the virt. tex. size and the page size, Values: 2 - 11
 * @param[in] _pageDimension	This is the width/height of a single page in pixels, Values: 64, 128, 256 or 512
*/
void		vtInit(const char *_tileDir, const char *_pageExtension, const uint8_t _pageBorder, const uint8_t _mipChainLength, const uint16_t _pageDimension);

/*!
 * @fn vtPrepare(const GLuint readbackShader, const GLuint renderVTShader)
 * @brief Prepares the OpenGL resources used by LibVT. It must be called after vtInit() and previous to all other functions.
 * @pre OpenGL must be ready/callable.
 * @post Has the following side effect: glUseProgram(0); glActiveTexture(GL_TEXTURE0);
 * @param[in] readbackShader The shader program created with glCreateProgram() from the readback.[vert/frag] shader and prepended by the shader prelude from vtGetShaderPrelude(). Can pass 0 if you bind the uniform samplers to the texunits yourself or use the Cg shaders where this isn't necessary.
 * @param[in] renderVTShader The shader program created with glCreateProgram() from the renderVT.[vert/frag] shader and prepended by the shader prelude from vtGetShaderPrelude(). Can pass 0 if you bind the uniform samplers to the texunits yourself or use the Cg shaders where this isn't necessary.
 */
void		vtPrepare(const GLuint readbackShader, const GLuint renderVTShader);

/*!
 * @fn vtPrepareOpenCL(const GLuint requestTexture)
 * @brief Prepares the OpenCL resources used by LibVT. Must only be called if OPENCL_BUFFERREDUCTION is 1. It must be called after vtPrepare() and previous to all other functions.
 * @pre OpenGL and OpenCL must be ready/callable.
 * @param[in] requestTexture  If READBACK_MODE is kCustomReadback you are managing the FBO yourself (i.e. a single pass OpenCL solution) and you must pass the texture to extract from. Else you can pass 0, let LibVT manage the FBO and READBACK_MODE must be set to something else.
 */
void		vtPrepareOpenCL(const GLuint requestTexture);

/*!
 * @fn vtGetShaderPrelude()
 * @brief Returns the shader prelude that must be prepended to the virtual texturing shaders bevore compiling them.
 * @return The shader prelude buffer, must be free()ed after usage.
 */
char *		vtGetShaderPrelude();


/*!
 * @fn vtReshape(const uint16_t _w, const uint16_t _h, const float fovInDegrees, const float nearPlane, const float farPlane)
 * @brief Sets up LibVT for usage with current rendering parameters. Must be called once at start after vtPrepare() and previous to other functions, and then everytime width/height/fov/nearPlane/farPlance change. Can't be called between vtPerformReadback() and vtExtractNeeded().
 * @post If USE_FBO is activated it has the following side effect: glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); glDisable(GL_TEXTURE_RECTANGLE_ARB); glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

 * @param[in] _w			The new width.
 * @param[in] _h			The new height.
 * @param[in] fovInDegrees	The new field of view in degrees. Pass 0 is you are using orthographic and not perspectivic projection. Not used if no PREPASS_RESOLUTION_REDUCTION_SHIFT takes place.
 * @param[in] nearPlane		The new near plane as it would be passed to gluPerspective(). Not used if no PREPASS_RESOLUTION_REDUCTION_SHIFT takes place.
 * @param[in] farPlane		The new far plane as it would be passed to gluPerspective(). Not used if no PREPASS_RESOLUTION_REDUCTION_SHIFT takes place.
 */
void		vtReshape(const uint16_t _w, const uint16_t _h, const float fovInDegrees, const float nearPlane, const float farPlane);


/*!
 * @fn vtPrepareReadback()
 * @brief Must be called every frame previous to rendering the virtual textured geometry for the readback pass using the readbackShader if READBACK_MODE is not kCustomReadback.
 * @post If USE_FBO is activated it binds a framebuffer, if PREPASS_RESOLUTION_REDUCTION_SHIFT is not zero the viewport and projection matrix are modified.
 */
void		vtPrepareReadback();


/*!
 * @fn vtPerformReadback()
 * @brief Must be called every frame after rendering the virtual textured geometry for the readback pass using the readbackShader if READBACK_MODE is not kCustomReadback.
 * @post If USE_FBO is activated the side effects are: glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0); glDisable(GL_TEXTURE_RECTANGLE_ARB);  if PREPASS_RESOLUTION_REDUCTION_SHIFT is non-zero the side effect is: glMatrixMode(GL_MODELVIEW);
 */
void		vtPerformReadback();


/*!
 * @fn vtExtractNeededPages(const uint32_t *ext_buffer_BGRA)
 * @brief Must be called every frame after vtPerformReadback(). If you do not use vtPrepareReadback() and vtPerformReadback() but do the readback yourself (kCustomReadback, e.g. OSG) you must pass the RGBA buffer, as parameter else it must be NULL.
 *
 * @param[in] ext_buffer_BGRA			If your READBACK_MODE is kCustomReadback pass the valid buffer you have read back yourself, else pass NULL.
*/
void		vtExtractNeededPages(const uint32_t *ext_buffer_BGRA);


/*!
 * @fn vtMapNewPages()
 * @brief Must be called every frame before rendering the virtual textured geometry for the main pass using the renderVTShader.
 * @post Often has the following side effect: glActiveTexture(GL_TEXTURE0);
*/
void		vtMapNewPages();


/*!
 * @fn vtShutdown()
 * @brief Must be called before the program exits if multithreading is enabled.
 */
void		vtShutdown();


/*!
 * @fn vtScan(const char *_tileDir, char * _pageExtension, uint8_t *_pageBorder, uint8_t *_mipChainLength, uint32_t *_pageDimension)
 * @brief Scans the tile storage directory at the given path and determines the configuration.
 * @param[in] _tileDir			The path of the tile directory to scan.
 * @param[out] _pageExtension	A pointer to a char array of minimum length 5 where the determined tile extension will be stored.
 * @param[out] _pageBorder		A pointer to a uint8_t where the size of the tile border will be stored.
 * @param[out] _mipChainLength	A pointer to a uint8_t where the size of the mip chain length will be stored.
 * @param[out] _pageDimension	A pointer to a uint32_t where the size of the tiles will be stored.
 * @return Returns whether the directory seems to be a valid tile store.
 */
bool		vtScan(const char *_tileDir, char * _pageExtension, uint8_t *_pageBorder, uint8_t *_mipChainLength, uint32_t *_pageDimension);


/*!
 * @fn vtGetBias()
 * @brief Provides the bias that should be used for the float bias for the uniform "mip_bias" for both shaders.
 * @return The bias to use as uniform for the shaders.
 */
float		vtGetBias();


/*!
 * @fn vtPerformOpenCLBufferReduction()
 * @brief Must be called to reduce the buffer when doing OpenCL integration, before vtExtractNeededPagesOpenCL().
 */
void		vtPerformOpenCLBufferReduction();


/*!
 * @fn vtExtractNeededPagesOpenCL()
 * @brief Must be called to after buffer reduction when doing OpenCL integration, to extract the neccessary pages.
 */
void		vtExtractNeededPagesOpenCL();


/*!
 * @mainpage LibVT Index Page
 *
 * @section intro_usage LibVT Usage
 *
 * To successfully use LibVT in your realtime rendering application you must follow these steps:
 *
 * \b 1.) Adapt your application to use a (single) texture atlas. This means putting all textures (of the objects that should be virtual textured) into a single texture and adjusting the texture coordinates accordingly. You can find more information about rendering with a texure atlas here: http://download.nvidia.com/developer/NVTextureSuite/Atlas_Tools/Texture_Atlas_Whitepaper.pdf <br>
 NOTE: You don't have to use virtual texturing for every texture in your application and probably shouldn't. e.g. font rendering, particle rendering, skybox rendering or any other objects can be excluded. The following discussion doesn't apply to these objects and their textures shouldn't be put in the texture atlas.<br>
 NOTE: When placing your textures in the atlas you should place the sub-textures at multiples of the virtual texture tile size so that the virtual texturing system prevents artifacts (Default: 256px)<br>
 NOTE: The texture atlas you are generating has to have specific dimensions. For virtual texturing without (bilinear) filtering it is exactly "power of two" dimensions, with filtering (tile borders!) slightly smaller depending on the virtual texture tile size. You can of course leave empty white space in the texture atlas or generate ideally sized subtextures from your source artwork. Anyway the adjustment of the texture coordinates must be correct.<br>
 IMPORTANT: you can use the tool LibVT-Scripts/generateTextureAtlas.py to generate a texture atlas and a texture coordinate offset file. It depends on python and imagemagick. To apply the texture coordinate offsets you can use LibVT-Scripts/offsetObjTexcoords.py *if* you are using .Obj files.<br><br>
 * \b 2.) Your single texture atlas texture will now likely be larger than the maximum texture size (roughly: Intel: 2k^2 ATI: 4k-16k^2 NVIDIA: 8k^2). Create a version downscaled to your maximum texture size. Run your application and render with the downscaled texture atlas. This means every object uses the same texture and the texture coordinates have to be adjusted. If you had any texture streaming or on-demand-loading code in your application you can erase it since you are using just a single texture now which fits into GPU memory. Everything should still work fine, except a lowered texture resolution. If not you don't need to continue, rather find and fix your problem ;-)<br><br>
 * \b 3.) Use the python script "generateVirtualTexureTiles.py" to preprocess the full-resolution version of your texture atlas. Usage options for this script are documented elsewhere. This generates the virtual texture tile store.<br><br>
 * \b 4.) Adjust the values in "LibVT_Config.h" to match your generated virtual texture tile store, your application and it's rendersettings.<br><br>
 * \b 5.) Now adjust your realtime application to use LibVT as documented: <br>
 * At startup call \link vtInit() vtInit() \endlink with the path to your tile store, the border width, the mipchain length and the tilesize<br>
 * Call \link vtGetShaderPrelude() vtGetShaderPrelude() \endlink to obtain the prelude to prepend to the shaders and load the readback and renderVT shaders.<br>
 * When OpenGL is callable call \link vtPrepare() vtPrepare() \endlink and pass it the shader objects.<br>
 * Call  \link vtReshape() vtReshape() \endlink now with the screen width, height, as well as fov, nearplane and farplane (only imporant in readback reduction mode). This call must also be made every time any of these values change, i.e. at viewport resize time.<br>
 * Now in the renderloop, call \link vtPrepareReadback() vtPrepareReadback() \endlink, render with the readback shader, call \link vtPerformReadback() vtPerformReadback() \endlink, \link vtExtractNeededPages() vtExtractNeededPages() \endlink, \link vtMapNewPages() vtMapNewPages() \endlink, and then render with the renderVT shader. Additionally pass the result of \link vtGetBias() vtGetBias() \endlink to both shaders as value for "mip_bias" each frame if you have the dynamic lod adjustment turned on.<br>
 * At shutdown call  \link vtShutdown() vtShutdown() \endlink<br><br>
 * \b EXAMPLES:<br>
 * \ref SimpleExample Simple LibVT usage example<br>
 * \ref OpenCLExample LibVT usage example for a single pass solution using MRT and OpenCL buffer reduction<br>
 * \ref CGExample LibVT usage example with Cg instead of GLSL<br>
 * <br>
 *	NOTE: The "readback" shader has to be used exactly as provided and can't be modified. As seen in the sample you have to use this shader during rendering your virtual textured objects in the pre-pass.<br>
 *	NOTE: The "renderVT" shader can be modified as you wish, but you have to retain the virtual texture functions and use sampleVirtualTexture(calulateVirtualTextureCoordinates()) to sample from the virtual texture. Don't forget to prepend the prelude to the shadercode.<br>
 *  NOTE: As seen in the code, after you've finished rendering the virtual textures objects you can render other objects to your liking. You can use any shaders you wish. Be careful never to use texture units that LibVT uses, namely as defined in "LibVT_Config.h" TEXUNIT_FOR_PAGETABLE, TEXUNIT_FOR_PHYSTEX and TEXUNIT_FOR_MIPCALC.<br>
 *	NOTE: Many other requirements are listed throughout the remaining documentation. Read it carefully!<br><br><br>
 *
 *
 *
 * @section intro_pipeline LibVT Content Pipeline
 *
 * This section describes the usage of the preprocessing scripts of LibVT, i.e. how to adapt your content for virtual texturing rendering. We start with an short description of the individual scripts and then give three increasingly complex examples. Make sure you have already read the above section "LibVT Usage". Please also note that all scripts have "built in help", pass them bogus and they will print out their usage instructions including possible options.<br>

 *
 * \b generateTextureAtlas.py: Produces one big texture (i.e. a texture atlas) out of many small individual (sub-)textures. It also produces a "coordinate offset file". <br>
 * \b offsetObjTexcoords.py: Applies a "coordinate offset file" to a Wavefront Object File (.Obj).<br>
 * \b generateVirtualTextureTiles.py: Takes one big texture or texture atlas and produces the "virtual texture tile store" needed for virtual texturing rendering. This script has a strict requirement on the input image size, which depends on the tile size and the tile border size. Find out the neccessary size for your desired border and tilesize before creating a texture atlas of the wrong size!<br>
 * \b mergeVirtualTextureTiles.py: Merges 4, 16 or 64 "virtual texture tile stores" into a single, larger one.<br>
 * \b convertVirtualTextureTiles.py: Converts a "virtual texture tile stores" into another image compression format (e.g. "bmp" -> "jpg")<br><br>
 *
 * Here are multiple examples, detailing the pipeline in simple and more complex cases. Please read through the simpler cases even if you are only interested in the complex one, since not all the information is provided 3 times.
 *
 * \subsection case_one Simple Case
 *
 * Terrain textured with a single 32k texture:<br>
 *
 * @code generateVirtualTextureTiles -b=1 -t=128 -f=jpg imputimage.png @endcode
 * This produces a "virtual texture tile store" using tiles with a size of 128 and a border of 1. The tile format is jpg. The inputimage.png must be 32256^2 pixel.<br>
 * The output is saved in a folder in the current directory with a name similar to the input image name, without the extension.
 * @code generateVirtualTextureTiles -b=4 -t=256 -f=jpg imputimage.png @endcode
 * This produces a "virtual texture tile store" using tiles with a size of 256 and a border of 4. The tile format is jpg. The inputimage.png must be 31744^2 pixel.
 *
 *
 * \subsection case_two Atlas Case
 *
 * Scene composed of multiple textured objects with a combined texture requirement of 32k^2:<br>
 *
 * Since the scene contains multiple distinct textures they have to be combined into a single texture atlas. Please note you can also do this with alternative texture atlas tools, but only the usage of the script contained in LibVT is described here. Before you start you must decide on the tilesize and bordersize to use, because this affects the required atlas size. In this example we want to use 128 tiles with 1px border, so lets start by finding out the required atlas size for this settings:<br>
 *
 * @code generateVirtualTextureTiles -b=1 -t=128 somepicture_which_surely_is_the_wrong_size.bmp @endcode
 *
 * This gives us:
 * @code Error: input image must be in: 4032^2, 8064^2, 16128^2, 32256^2, 64512^2, 129024^2 @endcode
 *
 * So now we know that the required atlas size for the settings we want is 32256.
 *
 * @code generateTextureAtlas -d=output_dir/ -s=32256 tex*.jpg @endcode
 * This produces a texture atlas named "_texture_atlas00.bmp" out of all images starting with "tex" and of format jpg in the current directory. It also produces a texture coordinate offset file called "_texture_atlas_offset.off". All output files are stored to "output_dir". On windows it helps to pass "-c=\path\to\imagemagick\" to prevent the script from picking up the windows "convert" tool instead of the image magick one.

 * @code offsetObjTexcoords -o=scene_object.obj -t=_texture_atlas_offset.off -d=output_dir/ @endcode
 * This applies the texture coordinate offsets from the offset file onto the .Obj file "scene_object.obj". Of course the .Obj file must be accompanied by a .Mtl file and the relative path and the filename of the .Mtl file must be given in the "mtllib" section (i.e. everything works unless you moved/renamed the .Mtl file). "scene_object_transformed.obj" and "scene_object_transformed.mtl" are saved in "output_dir". You can apply the offsets to all your obj files in one go if you have a proper shell, here is code for bash:
 * @code for i in *obj; do offsetObjTexcoords -o=$i -t=_texture_atlas_offset.off; done @endcode
 * And here is code for "cmd.exe":
 * @code for %i in (*.obj) do offsetObjTexcoords -o=%i -t=_texture_atlas_offset.off @endcode
 *
 * The texture atlas has been created, the offsets have been applied, the last step is producing the actual "virtual texture tile store" out of the atlas:
 *
 * @code generateVirtualTextureTiles -b=1 -t=128 -f=jpg _texture_atlas00.bmp @endcode
 * This produces a "virtual texture tile store" using tiles with a size of 128 and a border of 1 out of the texture atlas. The tile format is jpg. The _texture_atlas00.bmp must be exatly 32256^2 pixel (and it is, see options to generateTextureAtlas above).
 *
 * \subsection case_three Big Atlas Case
 *
 * Scene composed of multiple textured objects with a combined texture requirement of 128k^2:<br>
 *
 * This case is similar to the above one, but images of size 32k^2 or larger are nearly impossible to process, necessitating more intermediate files and more intermediate steps. Note that this example uses 64 intermediate files, you could probably get by with only 16, and producing a 64k^2 virtual texture could get by even with only 4 intermediate files. Adapt as neccessary.
 *
 * @code generateTextureAtlas -d=output_dir/ -s=129024 -n=3 tex*.jpg @endcode
 * This produces a texture atlas named "_texture_atlas00.bmp" out of all images starting with "tex" and of format jpg in the current directory. It also produces a texture coordinate offset file called "_texture_atlas_offset.off". All output files are stored to "output_dir". The -n=3 option tells the script to produce 4^3, i.e. 64 output images to avoid creating a impossible large intermediate file.
 *
 * @code offsetObjTexcoords -o=scene_object.obj -t=_texture_atlas_offset.off -d=output_dir/ @endcode
 * This applies the texture coordinate offsets from the offset file onto the .Obj file "scene_object.obj". Of course the .Obj file must be accompanied by a .Mtl file and the relative path and the filename of the .Mtl file must be given in the "mtllib" section (i.e. everything works unless you moved/renamed the .Mtl file). "scene_object_transformed.obj" and "scene_object_transformed.mtl" are saved in "output_dir". You can apply the offsets to all your obj files in one go if you have a proper shell, here is code for bash:
 * @code for i in *obj; do offsetObjTexcoords -o=$i -t=_texture_atlas_offset.off; done @endcode
 *
 * The texture atlas has been created, the offsets have been applied, we now create 64 virtual textures out of the 64 parts of the texture atlas:
 *
 * @code for i in _t*bmp; do generateVirtualTextureTiles -b=1 -f=bmp -t=128 $i; done @endcode
 *
 * This produces a "virtual texture tile store" out of each of the parts of the texture atlas. We need to use a uncompressed format (bmp) because we process the tiles further. This example assumes a proper shell, either adapt the example or do the dumb thing and repeat this with 00 replaced with 01, 02, 04, 05, 06, 07, 10, 11...up to 77:
 *
 * @code generateVirtualTextureTiles -b=1 -f=bmp -t=128 _texture_atlas00.bmp @endcode
 *
 * The 64 individual virtual texture tile stored need to be merged:
 *
 * @code mergeVirtualTextureTiles -o=merged/ -l=8 -b=1 _texture_atlas00 _texture_atlas10 _texture_atlas20 _texture_atlas30 _texture_atlas40 _texture_atlas50 _texture_atlas60 _texture_atlas70 _texture_atlas01 _texture_atlas11 _texture_atlas21 _texture_atlas31 _texture_atlas41 _texture_atlas51 _texture_atlas61 _texture_atlas71 _texture_atlas02 _texture_atlas12 _texture_atlas22 _texture_atlas32 _texture_atlas42 _texture_atlas52 _texture_atlas62 _texture_atlas72 _texture_atlas03 _texture_atlas13 _texture_atlas23 _texture_atlas33 _texture_atlas43 _texture_atlas53 _texture_atlas63 _texture_atlas73 _texture_atlas04 _texture_atlas14 _texture_atlas24 _texture_atlas34 _texture_atlas44 _texture_atlas54 _texture_atlas64 _texture_atlas74 _texture_atlas05 _texture_atlas15 _texture_atlas25 _texture_atlas35 _texture_atlas45 _texture_atlas55 _texture_atlas65 _texture_atlas75 _texture_atlas06 _texture_atlas16 _texture_atlas26 _texture_atlas36 _texture_atlas46 _texture_atlas56 _texture_atlas66 _texture_atlas76 _texture_atlas07 _texture_atlas17 _texture_atlas27 _texture_atlas37 _texture_atlas47 _texture_atlas57 _texture_atlas67 _texture_atlas77 @endcode
 *
 * Note that the order is important and must be similar even with 16 or 4 intermediate atlas folders, i.e. for 4 folders its "_texture_atlas00 _texture_atlas10 _texture_atlas01 _texture_atlas11". If your border is not 1, adapt the -b option. If your mipmap-chain length is different (different tilesize or virtual texture size), adapt the -l option. The -l option must be set to specify the mipmap chain length of the input folders, i.e. if they have tiles_b0_level0 throughout tiles_b0_level7, you must specify -l=8.<br>
 * Now, the only thing left to do is convert this virtual texture tile store, because a store of this size in bmp is quite big:
 *
 * @code convertVirtualTextureTiles -b=1 -fi=bmp -fo=jpg merged @endcode
 *
 * This converts the tile store in place, now the store in "merged/" is the finished version.<br>
 *
 * \subsection case_add Addendum
 *
 * The generateTextureAtlas script has a "-i=" option to place a number of input image in a regular predefined grid. This allows controlling the placement of some input images in the atlas. This can be useful if you want some images placed in a special way, e.g. because they cover terrain and had to be split because of size limitations. Here is a example that demonstrates how the atlas for the new york scene was built:
 *
 * @code generateTextureAtlas -s=129024 -n=3 -i=4:4:__p1ground.png:__p2ground.png:__p3ground.png:__p4ground.png:__p5ground.png:__p6ground.png:__p7ground.png:__p8ground.png:__p9ground.png:__p10ground.png:__p11ground.png:__p12ground.png:__p13ground.png:__p14ground.png:__p15ground.png:__p16ground.png small*.jpg p1b* p2b* p3b* p5b* p6b* p8b* p9b* p11b* p12b* p13b* p14b* p15b* p16b* @endcode
 *
 */


/*! \page SimpleExample Simple LibVT usage example
 *
 * @code
 * void init() // called at startup
 * {
 * 	vtInit("/Path/to/the/tile/dir/", "jpg", 0, 8, 256); // jpeg tiles, no border, mipchain length 8, and 256x256 tiles
 *
 * 	char *prelude = vtGetShaderPrelude();
 *
 * 	readbackShader = loadShadersWithPrelude("readback", prelude);
 * 	renderVTShader = loadShadersWithPrelude("renderVT", prelude);
 *
 * 	free(prelude);
 *
 * 	vtPrepare(readbackShader, renderVTShader); // opengl must be ready when you call this
 *
 * 	renderViewHasBeenResizedOrFovChanged(640, 480, 90.0);
 * }
 *
 * void renderViewHasBeenResizedOrFovChanged(int newW, int newH, float newFov) // called at start and when w/h/fov change
 * {
 * 	float nearPlane = 1.0f, farPlane = 7000.0f;
 *
 * 	vtReshape(newW, newH, newFov, nearPlane, farPlane);
 * }
 *
 * void render() // called every frame
 * {
 * 	vtPrepareReadback();
 * 		glUseProgram(readbackShader);
 * 		glUniform1f(glGetUniformLocation(readbackShader, "mip_bias"), vtGetBias());
 *
 * 			renderVirtualTexturedObjects();
 *
 * 		glUseProgram(0);
 * 	vtPerformReadback();
 *
 *  // IN PBO MODE: you can burn a lot of CPU cycles here because the GPU is busy transfering the readback buffer back,
 *  //              and if you call vtExtractNeededPages() too "early" it will start by blocking until the transfer is done
 *
 * 	vtExtractNeededPages(NULL); // turn on PBO readback and move this to the very beginning of the frame to delay the readback until the next frame
 * 	vtMapNewPages();
 *
 *
 * 	glUseProgram(renderVTShader);
 * 		glUniform1f(glGetUniformLocation(renderVTShader, "mip_bias"), vtGetBias());
 * 		renderVirtualTexturedObjects();
 * 	glUseProgram(0);
 *
 * 	renderSomeOtherObjects();
 * }
 *
 * void shutdown() // called at shutdown
 * {
 * 	vtShutdown();
 * }
 * @endcode
 *
 */


/*! \page CGExample LibVT usage example with Cg instead of GLSL
 *
 * @code
 * static CGcontext	cgContext;
 * static CGprofile	cgVertexProfile, cgFragmentProfile;
 * static CGprogram	cgReadbackVertexProgram, cgReadbackFragmentProgram, cgRenderVertexProgram, cgRenderFragmentProgram;
 * static CGparameter	cgRenderParamMipBias, cgReadbackParamMipBias, cgRenderParamModelViewMatrix, cgReadbackParamModelViewMatrix, cgReadbackParamMipcalcTexture, cgRenderParamPhysicalTexture, cgRenderParamPageTableTexture;
 *
 * void init() // called at startup
 * {
 * 	vtInit("/Path/to/the/tile/dir/", "jpg", 0, 8, 256); // jpeg tiles, no border, mipchain length 8, and 256x256 tiles
 *
 * 	string p_s = string(prelude);
 * 	
 * 	char * rb_v = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/readback_vert.cg");
 * 	char * rb_f = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/readback_frag.cg");
 * 	char * rVT_v = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/renderVT_vert.cg");
 * 	char * rVT_f = loadTextFile("/Users/julian/Documents/Development/VirtualTexturing/LibVT/renderVT_frag.cg");
 * 	
 * 	string rb_v_s = p_s + string(rb_v);
 * 	string rb_f_s = p_s + string(rb_f);
 * 	string rVT_v_s = p_s + string(rVT_v);
 * 	string rVT_f_s = p_s + string(rVT_f);
 * 			
 * 	
 * 	vtPrepare(0, 0); // pass 0 because we have no opengl shader objects and we don't need to set the sampler uniforms for the Cg shaders anyway this is done with the prelude
 * 	
 * 	
 * 	cgContext = cgCreateContext();
 * 	cgGLSetDebugMode(CG_FALSE);
 * 	cgSetParameterSettingMode(cgContext, CG_DEFERRED_PARAMETER_SETTING);
 * 	cgGLSetManageTextureParameters(cgContext, false);
 * 	cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
 * 	cgGLSetOptimalOptions(cgVertexProfile);
 * 	cgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
 * 	cgGLSetOptimalOptions(cgFragmentProfile);
 * 	
 * 	
 * 	// readback shader
 * 	cgReadbackVertexProgram = cgCreateProgram(cgContext, CG_SOURCE, rb_v_s.c_str(), cgVertexProfile, "readback_vert", NULL);             
 * 	cgGLLoadProgram(cgReadbackVertexProgram);
 * 	
 * 	cgReadbackParamModelViewMatrix = cgGetNamedParameter(cgReadbackVertexProgram, "ModelViewMatrix");
 * 		
 * 	
 * 	cgReadbackFragmentProgram = cgCreateProgram(cgContext, CG_SOURCE, rb_f_s.c_str(), cgFragmentProfile, "readback_frag", NULL);              
 * 	cgGLLoadProgram(cgReadbackFragmentProgram);
 * 	
 * 	cgReadbackParamMipcalcTexture = cgGetNamedParameter(cgReadbackFragmentProgram, "mipcalcTexture");
 * 	cgGLSetTextureParameter(cgReadbackParamMipcalcTexture, vt.mipcalcTexture);
 * 	cgGLEnableTextureParameter(cgReadbackParamMipcalcTexture);
 * 	
 * 	cgReadbackParamMipBias = cgGetNamedParameter(cgReadbackFragmentProgram, "mip_bias");
 * 	
 * 	
 * 	
 * 	// renderVT shader
 * 	cgRenderVertexProgram = cgCreateProgram(cgContext, CG_SOURCE, rVT_v_s.c_str(), cgVertexProfile, "rendervt_vert", NULL);             
 * 	cgGLLoadProgram(cgRenderVertexProgram);
 * 	
 * 	cgRenderParamModelViewMatrix = cgGetNamedParameter(cgRenderVertexProgram, "ModelViewMatrix");
 * 	
 * 	
 * 	cgRenderFragmentProgram = cgCreateProgram(cgContext, CG_SOURCE, rVT_f_s.c_str(), cgFragmentProfile, "rendervt_frag", NULL);              
 * 	cgGLLoadProgram(cgRenderFragmentProgram);
 * 	
 * 	cgRenderParamPhysicalTexture = cgGetNamedParameter(cgRenderFragmentProgram, "physicalTexture");
 * 	cgGLSetTextureParameter(cgRenderParamPhysicalTexture, vt.physicalTexture);
 * 	cgGLEnableTextureParameter(cgRenderParamPhysicalTexture);
 * 	
 * 	cgRenderParamPageTableTexture = cgGetNamedParameter(cgRenderFragmentProgram, "pageTableTexture");
 * 	cgGLSetTextureParameter(cgRenderParamPageTableTexture, vt.pageTableTexture);
 * 	cgGLEnableTextureParameter(cgRenderParamPageTableTexture);
 * 	
 * 	cgRenderParamMipBias = cgGetNamedParameter(cgRenderFragmentProgram, "mip_bias");
 * 	
 * 	
 * 	free(prelude);
 * 	free(rb_v);
 * 	free(rb_f);
 * 	free(rVT_v);
 * 	free(rVT_f);
 *
 * 	renderViewHasBeenResizedOrFovChanged(640, 480, 90.0);
 * }
 *
 * void renderViewHasBeenResizedOrFovChanged(int newW, int newH, float newFov) // called at start and when w/h/fov change
 * {
 * 	float nearPlane = 1.0f, farPlane = 7000.0f;
 *
 * 	vtReshape(newW, newH, newFov, nearPlane, farPlane);
 * }
 *
 * void render() // called every frame
 * {
 * 	cgGLEnableProfile(cgVertexProfile);	
 * 	cgGLEnableProfile(cgFragmentProfile);	
 * 	
 * 		
 * 	vtPrepareReadback();
 * 	
 * 	cgGLBindProgram(cgReadbackVertexProgram);
 * 	cgGLBindProgram(cgReadbackFragmentProgram);
 * 	
 * 	
 * 	cgGLSetStateMatrixParameter(cgReadbackParamModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
 * 	cgSetParameter1f(cgReadbackParamMipBias, vtGetBias());
 * 	cgGLEnableTextureParameter(cgReadbackParamMipcalcTexture);  // dont need Cg messing with texture units pls!
 * 	
 * 	
 * 	 renderVirtualTexturedObjects();
 * 	
 * 	vtPerformReadback();
 *
 *
 *  // IN PBO MODE: you can burn a lot of CPU cycles here because the GPU is busy transfering the readback buffer back,
 *  //              and if you call vtExtractNeededPages() too "early" it will start by blocking until the transfer is done
 *
 * 	vtExtractNeededPages(NULL);  // turn on PBO readback and move this to the very beginning of the frame to delay the readback until the next frame
 * 	vtMapNewPages();
 * 	
 * 	
 * 	
 * 	cgGLBindProgram(cgRenderVertexProgram);
 * 	cgGLBindProgram(cgRenderFragmentProgram);
 * 	
 * 	
 * 	
 * 	cgGLSetStateMatrixParameter(cgRenderParamModelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
 * 	cgSetParameter1f(cgRenderParamMipBias, vtGetBias());
 * 	
 * 	
 * 	cgGLEnableTextureParameter(cgRenderParamPhysicalTexture);  // dont need Cg messing with texture units pls!
 * 	cgGLEnableTextureParameter(cgRenderParamPageTableTexture); // dont need Cg messing with texture units pls!
 * 	
 * 		renderVirtualTexturedObjects();
 * 	
 * 	cgGLDisableProfile(cgVertexProfile);
 * 	cgGLDisableProfile(cgFragmentProfile);
 * 	
 * 	renderSomeOtherObjects();
 *
 * }
 *
 * void shutdown() // called at shutdown
 * {
 * 	vtShutdown();
 * }
 * @endcode
 *
 */

/*! \page OpenCLExample LibVT usage example for a single pass solution using MRT and OpenCL buffer reduction
 *
 * @code
 * static GLuint ct, dt, fbo, info;
 *
 * void init() // called at startup
 * {
 * 	vtInit("/Path/to/the/tile/dir/", "jpg", 0, 8, 256); // jpeg tiles, no border, mipchain length 8, and 256x256 tiles
 *
 * 	char *prelude = vtGetShaderPrelude();
 *
 * 	combined = loadShadersWithPrelude("combined", prelude); // use the combined shader for a single MRT pass
 *
 * 	free(prelude);
 *
 * 	vtPrepare(combined, combined); // opengl must be ready when you call this
 *
 *	glGenTextures(1, &info); // create a texture that the tile determination buffer will be rendered into
 *	vtPrepareOpenCL(info); // the opencl stuff needs to know this texture. we could pass 0 in a dual pass OpenCL solution where the FBO would still be managed by LibVT
 * 
 * 	renderViewHasBeenResizedOrFovChanged(640, 480, 90.0);
 * }
 *
 * void renderViewHasBeenResizedOrFovChanged(int newW, int newH, float newFov) // called at start and when w/h/fov change
 * {
 * 	float nearPlane = 1.0f, farPlane = 7000.0f;
 *
 *	glEnable(GL_TEXTURE_RECTANGLE_ARB);
 * 	
 * 	if (!ct) glGenTextures(1, &ct);  // create FBO to render into 
 * 	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ct);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 * 	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, newW, newH, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
 * 	
 * 	assert(info); // we already created this and passed it to LibVT
 * 	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, info);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 * 	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, newW, newH, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
 * 	
 * 	if (!dt) glGenTextures(1, &dt);
 * 	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, dt);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
 * 	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_NONE );
 * 	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24, newW, newH, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
 * 	
 * 	if (!fbo) glGenFramebuffersEXT(1, &fbo);
 * 	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
 * 	
 * 	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, ct, 0);
 * 	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_RECTANGLE_ARB, info, 0);
 * 	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, dt, 0);
 * 	
 * 	if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
 * 		vt_fatal("Error: couldn't setup FBO %04x\n", (unsigned int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
 * 	
 * 	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
 * 	
 * 	glDisable(GL_TEXTURE_RECTANGLE_ARB);
 * 	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
 * 	
 * 	vtReshape(newW, newH, newFov, nearPlane, farPlane);
 * }
 *
 * void render() // called every frame
 * {
 * 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); // setup FBO to render into
 * 		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 * 	
 * 		GLenum mrt[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT}; // yes we gonna do MRT, wohooo!
 * 		glDrawBuffers(2, mrt);
 * 	
 * 		vtMapNewPages();
 * 	
 * 	
 * 		glUseProgram(combined); // need to use the proper shader
 *			glUniform1f(glGetUniformLocation(combined, "mip_bias"), vtGetBias());
 *
 * 			renderVirtualTexturedObjects();
 *
 * 		glUseProgram(0);
 * 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
 * 	
 * 		vtPerformOpenCLBufferReduction(); // run the OpenCL kernel on the info texture
 * 		vtExtractNeededPagesOpenCL();  // finish the kernel, read back the results and extract the needed pages from it, pass to to the loading threead
 * 	
 * 	
 * 		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, fbo); // blit from FBO to screen
 * 		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
 * 		glBlitFramebufferEXT(0, 0, vt.real_w, vt.real_h,
 * 	                       0, 0, vt.real_w, vt.real_h,
 * 	                       GL_COLOR_BUFFER_BIT,
 * 	                       GL_NEAREST);
 * 		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
 * 		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
 * 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
 *
 *		renderSomeOtherObjects();
 * }
 *
 * void shutdown() // called at shutdown
 * {
 * 	vtShutdown();
 * }
 * @endcode
 *
 */

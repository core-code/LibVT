/*
 *  LibVT.cpp
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

#include "LibVT_Internal.h"
#include "LibVT.h"

vtData vt;
vtConfig c;


void vtInit(const char *_tileDir, const char *_pageExtension, const uint8_t _pageBorder, const uint8_t _mipChainLength, const uint16_t _pageDimension)
{
	if (c.tileDir != "") vt_fatal("Error: calling vtInit() twice ain't good!\n");

	assert((VT_MAG_FILTER == GL_NEAREST) || (VT_MAG_FILTER == GL_LINEAR));
	assert((VT_MIN_FILTER == GL_NEAREST) || (VT_MIN_FILTER == GL_LINEAR) || (VT_MIN_FILTER == GL_NEAREST_MIPMAP_NEAREST) || (VT_MIN_FILTER == GL_LINEAR_MIPMAP_NEAREST) || (VT_MIN_FILTER == GL_NEAREST_MIPMAP_LINEAR) || (VT_MIN_FILTER == GL_LINEAR_MIPMAP_LINEAR));
	assert(TEXUNIT_FOR_PHYSTEX !=  TEXUNIT_FOR_PAGETABLE);
	#if LONG_MIP_CHAIN
		assert((_mipChainLength >= 10) && (_mipChainLength <= 11));
	#else
		assert((_mipChainLength >= 2) && (_mipChainLength <= 9));
	#endif
	assert((_pageDimension == 64) || (_pageDimension == 128) || (_pageDimension == 256) || (_pageDimension == 512));
	assert((PREPASS_RESOLUTION_REDUCTION_SHIFT >= 0) && (PREPASS_RESOLUTION_REDUCTION_SHIFT <= 4));
	assert((MAX_RAMCACHE_MB >= 50));
	assert((HIGHEST_MIP_LEVELS_TO_KEEP >= 0) && (HIGHEST_MIP_LEVELS_TO_KEEP <= 5) && ((float)HIGHEST_MIP_LEVELS_TO_KEEP <= _mipChainLength));
	assert(!(READBACK_MODE_NONE && USE_PBO_READBACK));
	if (OPENCL_BUFFERREDUCTION) assert((READBACK_MODE_FBO || READBACK_MODE == kBackbufferGetTexImage || READBACK_MODE == kCustomReadback));
	if (OPENCL_BUFFERREDUCTION) assert(!USE_PBO_READBACK);
#if GL_ES_VERSION_2_0
	assert(READBACK_MODE == kBackbufferReadPixels);
	assert(ANISOTROPY == 0);
	assert((VT_MIN_FILTER == GL_NEAREST) || (VT_MIN_FILTER == GL_LINEAR));
	assert(USE_PBO_READBACK == 0);
	assert(USE_PBO_PAGETABLE == 0);
	assert(USE_PBO_PHYSTEX == 0);
	assert(FALLBACK_ENTRIES == 1);
#endif
	if (LONG_MIP_CHAIN && !USE_MIPCALC_TEXTURE) printf("Warning: expect artifacts when using LONG_MIP_CHAIN && !USE_MIPCALC_TEXTURE\n");

	// initialize and calculate configuration
	c.tileDir = string(_tileDir);
	c.pageCodec = string(_pageExtension);

	if (c.pageCodec == "dxt1" || REALTIME_DXT_COMPRESSION)
	{
		if (REALTIME_DXT_COMPRESSION) assert((IMAGE_DECOMPRESSION_LIBRARY == DecompressionLibJPEGTurbo) || (IMAGE_DECOMPRESSION_LIBRARY == DecompressionMac));

		c.pageDXTCompression = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		c.pageMemsize = (_pageDimension * _pageDimension / 2);

		assert(!MIPPED_PHYSTEX) ;
	}
	else if (c.pageCodec == "dxt5")
	{
		c.pageDXTCompression = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		c.pageMemsize = (_pageDimension * _pageDimension);
	}
	else
	{
#ifdef TARGET_OS_IPHONE
		if (IMAGE_DECOMPRESSION_LIBRARY == DecompressionMac || TARGET_OS_IPHONE)
#else
		if (IMAGE_DECOMPRESSION_LIBRARY == DecompressionMac )			
#endif			
		{
			c.pageDataFormat = GL_BGRA;
			c.pageDataType = GL_UNSIGNED_INT_8_8_8_8_REV;
#if GL_ES_VERSION_2_0
			c.pageDataFormat = GL_RGBA;
			c.pageDataType = GL_UNSIGNED_BYTE;
#endif
			c.pageMemsize = (_pageDimension * _pageDimension * 4);
		}
		else if (IMAGE_DECOMPRESSION_LIBRARY == DecompressionImageMagick)
		{
			c.pageDataFormat = GL_RGB;
			c.pageDataType = GL_SHORT;
			c.pageMemsize = (_pageDimension * _pageDimension * 8);
		}
		else
		{
#if !GL_ES_VERSION_2_0
			c.pageDataFormat = (c.pageCodec == "bmp") ? GL_BGR : GL_RGB;
#else
			c.pageDataFormat = GL_RGB;
#endif
			c.pageDataType = GL_UNSIGNED_BYTE;
			c.pageMemsize = (_pageDimension * _pageDimension * 3);
		}
	}
	c.physTexDimensionPages = PHYS_TEX_DIMENSION / _pageDimension;
	c.maxCachedPages = (int)((float)MAX_RAMCACHE_MB / ((float)c.pageMemsize / (1024.0 * 1024.0)));
	for (uint8_t i = 0; i < float(HIGHEST_MIP_LEVELS_TO_KEEP); i++)
		c.residentPages += (uint32_t) powf(4, i);

	c.pageBorder = _pageBorder;
	c.mipChainLength = _mipChainLength;
	c.virtTexDimensionPages = 2 << (c.mipChainLength - 2);
	c.pageDimension = _pageDimension;
	#if VT_MAG_FILTER == GL_LINEAR || VT_MIN_FILTER == GL_LINEAR || VT_MIN_FILTER == GL_LINEAR_MIPMAP_NEAREST || VT_MIN_FILTER == GL_LINEAR_MIPMAP_LINEAR
		if (c.pageBorder > 1 && c.pageBorder > ANISOTROPY / 2)
			printf("Warning: PAGE_BORDER bigger than necessary for filtering with GL_LINEAR* and given ANISOTROPY\n");
		else if (c.pageBorder < 1)
			printf("Warning: PAGE_BORDER must be at least 1 for filtering with GL_LINEAR*\n");
		else if (c.pageBorder < float(ANISOTROPY) / 2.0)
			printf("Warning: PAGE_BORDER too small for given ANISOTROPY\n");
	#else
		if (c.pageBorder > 0)
			printf("Warning: PAGE_BORDER not necessary for filtering with GL_NEAREST*\n");
	#endif


	// init translation tables, offsets and allocate page table
	uint32_t offsetCounter = 0;
	for (uint8_t i = 0; i < c.mipChainLength; i++)
	{
		vt.mipTranslation[i] = (uint16_t) ((c.virtTexDimensionPages >> i) - 1); // we do -1 here so we can add +1 in the shader to allow for a mip chain length 9 which results in the translation being 255/256, this is not ideal performance wise...
		vt.pageTableMipOffsets[i] = offsetCounter;
		offsetCounter += (c.virtTexDimensionPages >> i) * (c.virtTexDimensionPages >> i);
	}

	vt.pageTables = (uint32_t **) malloc(sizeof(uint32_t *) * c.mipChainLength);
	assert(vt.pageTables);

	uint32_t *pageTableBuffer = (uint32_t *) calloc(1, 4 * offsetCounter);
	assert(pageTableBuffer);

	for (uint8_t i = 0; i < c.mipChainLength; i++)
		vt.pageTables[i] = (uint32_t *)(pageTableBuffer + vt.pageTableMipOffsets[i]);



	// check the tile store
	char buf[255];
	for (uint8_t i = 0; i < 16; i++)
	{
		snprintf(buf, 255, "%s%stiles_b%u_level%u%stile_%u_0_0.%s", _tileDir, PATH_SEPERATOR, c.pageBorder, i, PATH_SEPERATOR, i, c.pageCodec.c_str());


		if (vtuFileExists(buf) != (i < c.mipChainLength))
			vt_fatal("Error: %s doesn't seem to be a page store with MIP_CHAIN_LENGTH = %u, c.pageCodec.c_str() = %s and c.pageBorder = %u!", c.tileDir.c_str(), c.mipChainLength, c.pageCodec.c_str(), c.pageBorder);
	}

	// precache some pages
	queue<uint32_t>	pagesToCache;
	for (uint8_t i = c.mipChainLength - HIGHEST_MIP_LEVELS_TO_PRECACHE; i < c.mipChainLength; i++)
		for (uint8_t x = 0; x < (c.virtTexDimensionPages >> i); x++)
			for (uint8_t y = 0; y < (c.virtTexDimensionPages >> i); y++)
				pagesToCache.push(MAKE_PAGE_INFO(i, x, y));
	vtCachePages(pagesToCache);

	// push the resident pages
	for (uint8_t i = c.mipChainLength - HIGHEST_MIP_LEVELS_TO_KEEP; i < c.mipChainLength; i++)
		for (uint8_t x = 0; x < (c.virtTexDimensionPages >> i); x++)
			for (uint8_t y = 0; y < (c.virtTexDimensionPages >> i); y++)
				vt.neededPages.push_back(MAKE_PAGE_INFO(i, x, y));


    #if ENABLE_MT == 1
        vt.backgroundThread = boost::thread(&vtLoadNeededPages);
	#elif ENABLE_MT == 2
		vt.backgroundThread = boost::thread(&vtLoadNeededPagesDecoupled);
		vt.backgroundThread2 = boost::thread(&vtDecompressNeededPagesDecoupled);
    #endif

	if (c.pageDXTCompression && (c.pageBorder % 4 != 0)) printf("Warning: PAGE_BORDER should be a multiple of 4 for DXT compression\n");
	assert(c.physTexDimensionPages <= MAX_PHYS_TEX_DIMENSION_PAGES);
	assert(!((MIPPED_PHYSTEX == 1) && ((USE_PBO_PHYSTEX == 1) || (c.pageDXTCompression)))); // TODO: support these combinations
}

bool vtScan(const char *_tileDir, char * _pageExtension, uint8_t *_pageBorder, uint8_t *_mipChainLength, uint32_t *_pageDimension)
{
	bool success = false;
	DIR *dp;
	struct dirent *ep;
	string tilestring = string("");
	string codec = string("    ");

	*_mipChainLength = (uint8_t)0;

	dp = opendir (_tileDir);
	if (dp != NULL)
	{
		while ((ep = readdir(dp)))
		{
			int level, border;
			string dir = string(ep->d_name);

			if (dir.find("tiles_b", 0) != string::npos)
			{
				sscanf(ep->d_name, "tiles_b%d_level%d", &border, &level);

				*_pageBorder = border;
				if (tilestring == "") tilestring = string(ep->d_name);
				if (++level > *_mipChainLength) *_mipChainLength = level;
			}
		}
		closedir(dp);

		dp = opendir (string(string(_tileDir) + string("/") + string (tilestring)).c_str());
		if (dp != NULL)
		{
			while ((ep = readdir(dp)))
			{
				string file = string(ep->d_name);

				if (file.find("tile_", 0) != string::npos)
				{
					uint32_t len = (uint32_t) file.length();
					codec = file.substr(len - 4);
					if (codec[0] == '.') codec = codec.substr(1);

					if ((codec == "dxt1") || (codec == "dxt5"))
					{
						FILE *f = fopen(string(string(_tileDir) + string("/") + string (tilestring) + string("/") + file).c_str(), "rb");
						unsigned long fileSize;
						fseek(f , 0 , SEEK_END);
						fileSize = ftell(f) - 8;
						if (codec == "dxt1")
							fileSize *= 2;

						*_pageDimension = (uint16_t) sqrtf((float)fileSize);
					}
					else
					{
						*_pageDimension = 0;
						void *image = vtuDecompressImageFile(string(string(_tileDir) + string("/") + string (tilestring) + string("/") + file).c_str(), _pageDimension);
						free(image);
					}
					success = true;
					break;
				}
			}
			closedir(dp);
		}
	}

	_pageExtension[0] = codec[0];
	_pageExtension[1] = codec[1];
	_pageExtension[2] = codec[2];
	_pageExtension[3] = codec[3];

	return success;
}

void vtPrepare(const GLuint readbackShader, const GLuint renderVTShader)
{
	GLint max_texture_size, max_texture_units;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &max_texture_units);
	assert((PHYS_TEX_DIMENSION >= 2048) && (PHYS_TEX_DIMENSION <= max_texture_size) && (c.physTexDimensionPages * c.pageDimension == PHYS_TEX_DIMENSION));
	assert((TEXUNIT_FOR_PAGETABLE >= 0) && (TEXUNIT_FOR_PAGETABLE < max_texture_units));
	assert((TEXUNIT_FOR_PHYSTEX >= 0) && (TEXUNIT_FOR_PHYSTEX < max_texture_units));


	if (renderVTShader)
	{
		glUseProgram(renderVTShader);
			glUniform1i(glGetUniformLocation(renderVTShader, "pageTableTexture"), TEXUNIT_FOR_PAGETABLE);
			glUniform1i(glGetUniformLocation(renderVTShader, "physicalTexture"), TEXUNIT_FOR_PHYSTEX);
		glUseProgram(0);
	}
	if (readbackShader)
	{
		glUseProgram(readbackShader);
			glUniform1i(glGetUniformLocation(readbackShader, "mipcalcTexture"), TEXUNIT_FOR_MIPCALC);
		glUseProgram(0);
	}


	glGenTextures(1, &vt.physicalTexture);
	glActiveTexture(GL_TEXTURE0 + TEXUNIT_FOR_PHYSTEX);
	glBindTexture(GL_TEXTURE_2D, vt.physicalTexture);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, VT_MAG_FILTER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, VT_MIN_FILTER);

#if ANISOTROPY
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ANISOTROPY);
#endif


	if (c.pageDXTCompression)
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, c.pageDXTCompression, PHYS_TEX_DIMENSION, PHYS_TEX_DIMENSION, 0, c.pageMemsize * c.physTexDimensionPages * c.physTexDimensionPages, NULL);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, c.pageDataFormat == GL_RGB ? GL_RGB : GL_RGBA, PHYS_TEX_DIMENSION, PHYS_TEX_DIMENSION, 0, c.pageDataFormat, c.pageDataType, NULL);



	if (MIPPED_PHYSTEX)
	{
#if !GL_ES_VERSION_2_0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
#endif
		glTexImage2D(GL_TEXTURE_2D, 1, c.pageDataFormat == GL_RGB ? GL_RGB : GL_RGBA, PHYS_TEX_DIMENSION / 2, PHYS_TEX_DIMENSION / 2, 0, c.pageDataFormat, c.pageDataType, NULL);
	}



	glGenTextures(1, &vt.pageTableTexture);
	glActiveTexture(GL_TEXTURE0 + TEXUNIT_FOR_PAGETABLE);
	glBindTexture(GL_TEXTURE_2D, vt.pageTableTexture);
#if !GL_ES_VERSION_2_0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, c.mipChainLength - 1);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	for (uint8_t i = 0; i < c.mipChainLength; i++)
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, c.virtTexDimensionPages >> i, c.virtTexDimensionPages >> i, 0, GL_RGBA, GL_UNSIGNED_BYTE, vt.pageTables[i]); // {GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV} doesn't seem to be faster


	if (USE_MIPCALC_TEXTURE)
	{
		glGenTextures(1, &vt.mipcalcTexture);
		glActiveTexture(GL_TEXTURE0 + TEXUNIT_FOR_MIPCALC);
		glBindTexture(GL_TEXTURE_2D, vt.mipcalcTexture);
#if !GL_ES_VERSION_2_0
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, c.mipChainLength - 1);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c.virtTexDimensionPages, c.virtTexDimensionPages, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // {GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV} doesn't seem to be faster

		uint32_t **mipcalcTables = (uint32_t **) malloc(sizeof(uint32_t *) * c.mipChainLength);
		for (uint8_t i = 0; i < c.mipChainLength; i++)
		{
			mipcalcTables[i] = (uint32_t *) malloc(4 * (c.virtTexDimensionPages >> i) * (c.virtTexDimensionPages >> i));

			for (uint16_t x = 0; x < (c.virtTexDimensionPages >> i); x++)
			{
				for (uint16_t y = 0; y < (c.virtTexDimensionPages >> i); y++)
				{
					if (LONG_MIP_CHAIN)
						(mipcalcTables[i][y * (c.virtTexDimensionPages >> i) + x]) = (0xFF << 24) + ((i + ((x & 0xFF00) >> 4) + ((y & 0xFF00) >> 2)) << 16) + (((uint8_t) y) << 8) + ((uint8_t) x); // format: ABGR
					else
						(mipcalcTables[i][y * (c.virtTexDimensionPages >> i) + x]) = (0xFF << 24) + (i << 16) + (y << 8) + x; // format: ABGR
				}
			}

			glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, c.virtTexDimensionPages >> i, c.virtTexDimensionPages >> i, 0, GL_RGBA, GL_UNSIGNED_BYTE, mipcalcTables[i]);
			free(mipcalcTables[i]);
		}
		free(mipcalcTables);
	}


	if (READBACK_MODE_FBO || READBACK_MODE == kBackbufferGetTexImage)
	{
		glGenTextures(1, &vt.fboColorTexture);

		if (READBACK_MODE_FBO)
		{
			glGenTextures(1, &vt.fboDepthTexture);

			glGenFramebuffersEXT(1, &vt.fbo);
		}
	}

	glActiveTexture(GL_TEXTURE0);

#if !GL_ES_VERSION_2_0
	if (USE_PBO_PAGETABLE)
	{
		glGenBuffers(1, &vt.pboPagetable);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vt.pboPagetable);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, (vt.pageTableMipOffsets[c.mipChainLength - 1] + 1) * 4, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}


	if (USE_PBO_PHYSTEX)
	{
		glGenBuffers(1, &vt.pboPhystex);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vt.pboPhystex);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, c.pageMemsize * PBO_PHYSTEX_PAGES, 0, GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	if (USE_PBO_READBACK)
	{
		glGenBuffers(1, &vt.pboReadback);
	}
#endif
}

char * vtGetShaderPrelude()
{
	setlocale( LC_ALL, "C" );

	char *buf = (char *) calloc(1, 1024);
	snprintf(buf, 1024,	"const float phys_tex_dimension_pages = %f;\n"
						"const float page_dimension = %f;\n"
						"const float page_dimension_log2 = %f;\n"
						"const float prepass_resolution_reduction_shift = %f;\n"
						"const float virt_tex_dimension_pages = %f;\n"
						"const float virt_tex_dimension = %f;\n"
						"const float border_width = %f;\n\n"
						"const float max_anisotropy = %f;\n\n"
						"#define USE_MIPCALC_TEXTURE %i\n"
						"#define PAGE_BORDER %i\n"
						"#define MIPPED_PHYSTEX %i\n"
						"#define FALLBACK_ENTRIES %i\n"
						"#define ANISOTROPY %i\n"
						"#define LONG_MIP_CHAIN %i\n"
						"#define TEXUNIT_MIPCALC TEXUNIT%i\n"
						"#define TEXUNIT_PHYSICAL TEXUNIT%i\n"
						"#define TEXUNIT_PAGETABLE TEXUNIT%i\n\n",

						(float)c.physTexDimensionPages, (float)c.pageDimension, log2f(c.pageDimension), (float)PREPASS_RESOLUTION_REDUCTION_SHIFT,
						(float)c.virtTexDimensionPages, (float)(c.virtTexDimensionPages * c.pageDimension), (float)c.pageBorder, float(ANISOTROPY),
						USE_MIPCALC_TEXTURE, c.pageBorder, MIPPED_PHYSTEX, FALLBACK_ENTRIES, ANISOTROPY, LONG_MIP_CHAIN, TEXUNIT_FOR_MIPCALC, TEXUNIT_FOR_PHYSTEX, TEXUNIT_FOR_PAGETABLE);
	return buf;
}

void vtShutdown()
{	
#if ENABLE_MT == 1
	vt.backgroundThread.interrupt();
#elif ENABLE_MT == 2
	vt.backgroundThread.interrupt();
	vt.backgroundThread2.interrupt();
#endif

	free(vt.pageTables[0]);
	free(vt.pageTables);


	if (!USE_PBO_READBACK)
		free(vt.readbackBuffer);

	glDeleteTextures(1, &vt.physicalTexture);
	glDeleteTextures(1, &vt.pageTableTexture);
	if (USE_MIPCALC_TEXTURE)
		glDeleteTextures(1, &vt.mipcalcTexture);

	if (USE_PBO_READBACK)
		glDeleteBuffers(1, &vt.pboReadback);
	if (USE_PBO_PAGETABLE)
		glDeleteBuffers(1, &vt.pboPagetable);
	if (USE_PBO_PHYSTEX)
		glDeleteBuffers(1, &vt.pboPhystex);

	if (READBACK_MODE_FBO || READBACK_MODE == kBackbufferGetTexImage)
	{
		glDeleteTextures(1, &vt.fboDepthTexture);

		if (READBACK_MODE_FBO)
		{
			glDeleteTextures(1, &vt.fboColorTexture);
			glDeleteFramebuffersEXT(1, &vt.fbo);
		}
	}
}

void vtReshape(const uint16_t _w, const uint16_t _h, const float fovInDegrees, const float nearPlane, const float farPlane)
{
	vt.real_w = _w;
	vt.real_h = _h;
	vt.w = _w >> PREPASS_RESOLUTION_REDUCTION_SHIFT;
	vt.h = _h >> PREPASS_RESOLUTION_REDUCTION_SHIFT;
	vt.fovInDegrees = fovInDegrees;


#if !GL_ES_VERSION_2_0
	if (READBACK_MODE_FBO || READBACK_MODE == kBackbufferGetTexImage)
	{
		glEnable(GL_TEXTURE_RECTANGLE_ARB);

		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, vt.fboColorTexture);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, vt.w, vt.h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		if (READBACK_MODE_FBO)
		{
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB, vt.fboDepthTexture);	// TODO: use renderbuffer instead of texture?
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_NONE );
			glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT24, vt.w, vt.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);


			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, vt.fbo);

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, vt.fboColorTexture, 0);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, vt.fboDepthTexture, 0);


			if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
				vt_fatal("Error: couldn't setup FBO %04x\n", (unsigned int)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}

		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
	}

	if (USE_PBO_READBACK)
	{
		glBindBuffer(GL_PIXEL_PACK_BUFFER, vt.pboReadback);
		glBufferData(GL_PIXEL_PACK_BUFFER, vt.w * vt.h * 4, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
#endif

	if (!USE_PBO_READBACK && !READBACK_MODE_NONE && !OPENCL_BUFFERREDUCTION)
	{
		if (vt.readbackBuffer)
			free(vt.readbackBuffer);
		vt.readbackBuffer = (uint32_t *) malloc(vt.w * vt.h * 4);
		assert(vt.readbackBuffer);
	}

	if (PREPASS_RESOLUTION_REDUCTION_SHIFT && fovInDegrees > 0.0)
		vtuPerspective(vt.projectionMatrix, fovInDegrees, (float)vt.w / (float)vt.h, nearPlane, farPlane);


#if OPENCL_BUFFERREDUCTION
	if (READBACK_MODE == kCustomReadback)
	{
		vtReshapeOpenCL(_w >> OPENCL_REDUCTION_SHIFT, _h >> OPENCL_REDUCTION_SHIFT);
		assert(!PREPASS_RESOLUTION_REDUCTION_SHIFT);
	}
	else
	{
		vtReshapeOpenCL(vt.w, vt.h);
		assert(!OPENCL_REDUCTION_SHIFT);
	}
#endif
}

float vtGetBias()
{
	if (MIPPED_PHYSTEX)
		return vt.bias - 0.5f;
	else
		return vt.bias;
}

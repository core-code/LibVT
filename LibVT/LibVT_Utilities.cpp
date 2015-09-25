/*
 *  LibVT_Utilities.cpp
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

extern vtConfig c;


uint32_t * vtuDownsampleImageRGBA(const uint32_t *tex)
{
	uint32_t *smallTex = (uint32_t *)malloc(c.pageDimension * c.pageDimension);
	assert(smallTex);

	for (uint16_t x = 0; x < c.pageDimension / 2; x++)
	{
		for (uint16_t y = 0; y < c.pageDimension / 2; y++)
		{
#ifdef COLOR_CODE_MIPPED_PHYSTEX
			smallTex[y * (c.pageDimension / 2) + x] = (255 << 24) + (0 << 16) + (0 << 8) + 255;
#else
			uint32_t pix1 = tex[(y*2) * c.pageDimension + (x*2)];
			uint32_t pix2 = tex[(y*2+1) * c.pageDimension + (x*2)];
			uint32_t pix3 = tex[(y*2) * c.pageDimension + (x*2+1)];
			uint32_t pix4 = tex[(y*2+1) * c.pageDimension + (x*2+1)];

			uint32_t b1 = BYTE1(pix1) + BYTE1(pix2) + BYTE1(pix3) + BYTE1(pix4);
			uint32_t b2 = BYTE2(pix1) + BYTE2(pix2) + BYTE2(pix3) + BYTE2(pix4);
			uint32_t b3 = BYTE3(pix1) + BYTE3(pix2) + BYTE3(pix3) + BYTE3(pix4);
			uint32_t b4 = BYTE4(pix1) + BYTE4(pix2) + BYTE4(pix3) + BYTE4(pix4);


			smallTex[y * (c.pageDimension / 2) + x] =  ((b4 / 4) << 24) + ((b3 / 4) << 16) + ((b2 / 4) << 8) + (b1 / 4); // ARGB
#endif
		}
	}

	return smallTex;
}

uint32_t * vtuDownsampleImageRGB(const uint32_t *_tex)
{
	uint8_t *tex = (uint8_t *) _tex;
	uint8_t *smallTex = (uint8_t *)malloc((c.pageDimension * c.pageDimension * 3) / 4);
	assert(smallTex);

	for (uint16_t x = 0; x < c.pageDimension / 2; x++)
	{
		for (uint16_t y = 0; y < c.pageDimension / 2; y++)
		{
#ifdef COLOR_CODE_MIPPED_PHYSTEX
			smallTex[y * (c.pageDimension / 2) * 3 + (x*3)] = 200;
			smallTex[y * (c.pageDimension / 2) * 3 + (x*3) + 1] = 10;
			smallTex[y * (c.pageDimension / 2) * 3 + (x*3) + 2] = 70;
#else
			uint8_t pix1 = tex[(y*2) * c.pageDimension * 3 + (x*2*3)];
			uint8_t pix2 = tex[(y*2+1) * c.pageDimension * 3 + (x*2*3)];
			uint8_t pix3 = tex[(y*2) * c.pageDimension * 3 + (x*2*3+3)];
			uint8_t pix4 = tex[(y*2+1) * c.pageDimension * 3 + (x*2*3+3)];

			smallTex[y * (c.pageDimension / 2) * 3 + (x*3)] = (pix1 + pix2 + pix3 + pix4) / 4;

			pix1 = tex[(y*2) * c.pageDimension * 3 + (x*2*3) + 1];
			pix2 = tex[(y*2+1) * c.pageDimension * 3 + (x*2*3) + 1];
			pix3 = tex[(y*2) * c.pageDimension * 3 + (x*2*3+3) + 1];
			pix4 = tex[(y*2+1) * c.pageDimension * 3 + (x*2*3+3) + 1];

			smallTex[y * (c.pageDimension / 2) * 3 + (x*3) + 1] = (pix1 + pix2 + pix3 + pix4) / 4;

			pix1 = tex[(y*2) * c.pageDimension * 3 + (x*2*3) + 2];
			pix2 = tex[(y*2+1) * c.pageDimension * 3 + (x*2*3) + 2];
			pix3 = tex[(y*2) * c.pageDimension * 3 + (x*2*3+3) + 2];
			pix4 = tex[(y*2+1) * c.pageDimension * 3 + (x*2*3+3) + 2];

			smallTex[y * (c.pageDimension / 2) * 3 + (x*3) + 2] = (pix1 + pix2 + pix3 + pix4) / 4;
#endif
		}
	}

	return (uint32_t *)smallTex;
}

void vtuPerspective(double m[4][4], double fovy, double aspect,	double zNear, double zFar)
{
	double sine, cotangent, deltaZ;
	double radians = fovy / 2.0 * 3.14159265358979323846 / 180.0;

	deltaZ = zFar - zNear;
	sine = sin(radians);

	if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
		vt_fatal("Error: perspectve matrix is degenerate");

	cotangent = cos(radians) / sine;

	m[0][0] = cotangent / aspect;
	m[1][1] = cotangent;
	m[2][2] = -(zFar + zNear) / deltaZ;
	m[2][3] = -1.0;
	m[3][2] = -2.0 * zNear * zFar / deltaZ;
}

char vtuFileExists(char *path)
{
	FILE *f;

	f = fopen(path, "r");
	if (f)
	{
		fclose(f);
		return 1;
	}
	else
		return 0;
}

void * vtuLoadFile(const char *filePath, const uint32_t offset, uint32_t *file_size)
{
	uint32_t fs = 0;
	uint32_t *fsp = &fs;
	char *fileData;
	FILE *f;

	f = fopen(filePath, "rb");
	if (!f)
	{
		printf("Error: tried to load nonexisting file");
		return NULL;
	}
#if defined(__APPLE__) && (!(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE))
	fcntl(f->_file, F_GLOBAL_NOCACHE, 1); // prevent the OS from caching this file in RAM
#endif
	assert(f);

	size_t result;

	if (file_size != NULL)
		fsp = file_size;

	if (*fsp != 0)
		*fsp = *fsp - offset;
	else
	{
		fseek(f , 0 , SEEK_END);
		*fsp = ftell(f) - offset;
	}


	fseek(f, offset, SEEK_SET);


	fileData = (char *) malloc(*fsp);
	assert(fileData);

	result = fread(fileData, 1, *fsp, f);

	assert(result == *fsp);

	fclose (f);

	return fileData;
}

/*
 *  LibVT_Readback.cpp
 *
 *
 *  Created by Julian Mayer on 05.03.10.
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

extern vtData vt;
extern vtConfig c;


void vtPrepareReadback()
{
	if (READBACK_MODE_FBO)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, vt.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if (PREPASS_RESOLUTION_REDUCTION_SHIFT)
	{
		glViewport(0, 0, vt.w, vt.h);

		if (vt.fovInDegrees > 0.0)
		{
#if !GL_ES_VERSION_2_0
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadMatrixd((double *)vt.projectionMatrix);
			glMatrixMode(GL_MODELVIEW);
#endif
		}
	}
}

void vtPerformReadback()
{
	uint32_t *buffer = 0;


#if !GL_ES_VERSION_2_0
	if (USE_PBO_READBACK)
		glBindBuffer(GL_PIXEL_PACK_BUFFER, vt.pboReadback);
	else
#endif
		buffer = vt.readbackBuffer;


#if !GL_ES_VERSION_2_0
	if (READBACK_MODE_GET_TEX_IMAGE)
	{
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, vt.fboColorTexture);

		if (READBACK_MODE == kBackbufferGetTexImage)
			glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, 0, 0, vt.w, vt.h);

		if (!OPENCL_BUFFERREDUCTION)
			glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
	}
	else if (READBACK_MODE_READ_PIXELS)
		glReadPixels(0, 0, vt.w, vt.h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
#else
	glReadPixels(0, 0, vt.w, vt.h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
	//glReadPixels(0, 0, vt.w, vt.h, GL_RGBA, GL_UNSIGNED_BYTE, (GLubyte *)buffer);
#endif



	if (READBACK_MODE_FBO)
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


#if !GL_ES_VERSION_2_0
	if (READBACK_MODE_GET_TEX_IMAGE)
	{
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
#endif


	if (READBACK_MODE_BACKBUFFER)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


#if !GL_ES_VERSION_2_0
	if (USE_PBO_READBACK)
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
#endif


	if (PREPASS_RESOLUTION_REDUCTION_SHIFT)
	{
		if (vt.fovInDegrees > 0.0)
		{
#if !GL_ES_VERSION_2_0
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
#endif
		}
		glViewport(0, 0, vt.real_w, vt.real_h);
	}
}

void vtExtractNeededPages(const uint32_t *ext_buffer_BGRA)
{
	const uint32_t width = vt.w;
	const clock_t clocks = vt.thisFrameClock = clock();
	queue<uint32_t>	tmpPages;

	map<uint32_t, uint16_t> tmpPages1;
	multimap<uint16_t, uint32_t> tmpPages2;

	const uint32_t *buffer;

	vt.necessaryPageCount = 0;

	assert(!OPENCL_BUFFERREDUCTION);

	if (READBACK_MODE_NONE)
		buffer = ext_buffer_BGRA;
	else
	{
#if !GL_ES_VERSION_2_0
		if (USE_PBO_READBACK)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, vt.pboReadback);
			buffer = (uint32_t *)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		}
		else
#endif
			buffer = vt.readbackBuffer;
	}

	// erase pages that were requested in previous frames in the pagetable. if they are still necessary they will be readded
	{	// lock
		LOCK(vt.neededPagesMutex)

		for (uint32_t i = 0; i < vt.neededPages.size(); i++)
		{
			uint32_t pageInfo = vt.neededPages[i];

			uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
			uint8_t mip = EXTRACT_MIP(pageInfo);
			*((uint8_t *)&PAGE_TABLE(mip, x_coord, y_coord)) = kTableFree;
		}
	}	// unlock

	assert(buffer);
///*2*/	list<uint32_t> l;				// testcode for determining how many pages are visible each frame
///*1*/	map<uint32_t, uint16_t> bla;	// testcode for producing a reference list of pages with pixel coverage information for quality tests. make sure not to use prepass resolutin reuduction. make sure each frame is rendered at a specific walkthrough position because the code can slow down very much. see also corresponding testcode in vtMapNewPages()

	for (uint32_t y = 0; y < vt.h; y++)
	{
		const uint32_t rows = y * width;

		for (uint32_t x = 0; x < vt.w; x++)
		{
			const uint32_t pixel = *(buffer + rows + x);	// format: BGRA		mip, x, y, STATUS
			const uint8_t mip = (LONG_MIP_CHAIN) ? BYTE1(pixel) & 0x0F : BYTE1(pixel);
			const uint8_t shift = (USE_MIPCALC_TEXTURE ? 0 : mip);
			const uint16_t y_coord = (LONG_MIP_CHAIN) ? ((BYTE2(pixel) | ((BYTE1(pixel) & 0xC0)) << 2) >> shift) : (BYTE2(pixel) >> shift);
			const uint16_t x_coord = (LONG_MIP_CHAIN) ? ((BYTE3(pixel) | ((BYTE1(pixel) & 0x30)) << 4) >> shift) : (BYTE3(pixel) >> shift);

			if ((BYTE4(pixel) == 255) && (mip < c.mipChainLength) && (y_coord < (c.virtTexDimensionPages >> mip)) && (x_coord < (c.virtTexDimensionPages >> mip)))
			{
///*2*/			l.push_back(MAKE_PAGE_INFO(mip, x_coord, y_coord));

				const uint32_t pageEntry = PAGE_TABLE(mip, x_coord, y_coord);

				if ((uint8_t) pageEntry == kTableFree) // if page is not mapped, add it to the download list and make sure we don't handle it again this frame
				{
					const uint32_t pageInfo = MAKE_PAGE_INFO(mip, x_coord, y_coord);

					tmpPages1[pageInfo] = 1;
///*1*/				bla[pageInfo] = 1;

#if DEBUG_LOG > 0
					printf("Requesting page: Mip:%u %u/%u\n", mip, x_coord, y_coord);
#endif

					// we just want to set the alpha channel, luckly this byte is right there on little endian
					// setting just the lowest byte matters for the fallback-entry-mode, else a non-mapped page is empty anyway
					*((uint8_t *)&PAGE_TABLE(mip, x_coord, y_coord)) = kTableMappingInProgress;

					vtcTouchCachedPage(pageInfo);

					vt.necessaryPageCount++;
				}
				else if ((uint8_t) pageEntry == kTableMapped)	// if the page is mapped we need to mark it used
				{
					const uint8_t yInTexture = BYTE2(pageEntry), xInTexture = BYTE3(pageEntry);

					fast_assert((xInTexture < c.physTexDimensionPages) && (yInTexture < c.physTexDimensionPages));

					if (vt.textureStorageInfo[xInTexture][yInTexture].clockUsed != clocks)
					{
						vt.textureStorageInfo[xInTexture][yInTexture].clockUsed = clocks;	// touch page in physical texture

						vtcTouchCachedPage(MAKE_PAGE_INFO(mip, x_coord, y_coord));			// touch page in RAM cache

						vt.necessaryPageCount++;
					}

///*1*/				const uint32_t pageInfo = MAKE_PAGE_INFO(mip, x_coord, y_coord);
///*1*/				if (bla.count(pageInfo)) bla[pageInfo] = bla[pageInfo] + 1;
///*1*/				else	bla[pageInfo] = 1;
				}
				else if ((uint8_t) pageEntry == kTableMappingInProgress)
				{
					const uint32_t pageInfo = MAKE_PAGE_INFO(mip, x_coord, y_coord);

					if (tmpPages1.count(pageInfo))
						tmpPages1[pageInfo] = tmpPages1[pageInfo] + 1;

///*1*/				if (bla.count(pageInfo)) bla[pageInfo] = bla[pageInfo] + 1;
///*1*/				else bla[pageInfo] = 1;
				}
			}
		}
	}
	
///*2*/		l.sort();
///*2*/		l.unique();
///*2*/		printf("%i \n", (int)l.size());

	map<uint32_t, uint16_t>::iterator tmpPagesIter1;
	multimap<uint16_t, uint32_t>::reverse_iterator tmpPagesIter2;
	
///*1*/	printf("NEWFRAME\n\n");
///*1*/	for(tmpPagesIter1 = bla.begin(); tmpPagesIter1 != bla.end(); ++tmpPagesIter1)
///*1*/	printf("PAGE: %i %i\n", tmpPagesIter1->first, tmpPagesIter1->second);

	for(tmpPagesIter1 = tmpPages1.begin(); tmpPagesIter1 != tmpPages1.end(); ++tmpPagesIter1) // pages sorting by importance
		tmpPages2.insert(pair<uint16_t, uint32_t>(tmpPagesIter1->second, tmpPagesIter1->first));
	for(tmpPagesIter2 = tmpPages2.rbegin(); tmpPagesIter2 != tmpPages2.rend(); ++tmpPagesIter2)
		tmpPages.push(tmpPagesIter2->second);


#if !GL_ES_VERSION_2_0
	if (USE_PBO_READBACK)
	{
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
#endif

	queue<uint32_t>	cachedPages;
	queue<uint32_t>	nonCachedPages;

	vtcSplitPagelistIntoCachedAndNoncachedLOCK(&tmpPages, &cachedPages, &nonCachedPages);


	{	// lock
		LOCK(vt.neededPagesMutex)

		vt.neededPages.clear(); // erase old pages we already unmarked them them in the pagetable above. we erase old requests here because we don't want the loading thread stalled while we do our calculations

		if (!nonCachedPages.empty())
		{
			while(!nonCachedPages.empty())
			{
				const uint32_t pageInfo = nonCachedPages.front();

				vt.neededPages.push_back(pageInfo);nonCachedPages.pop();

#if DEBUG_LOG > 0
				const uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
				const uint8_t mip = EXTRACT_MIP(pageInfo);
				printf("Requesting page for loading from disk: Mip:%u %u/%u (%i)\n", mip, x_coord, y_coord, pageInfo);
#endif
			}

#if ENABLE_MT
			vt.neededPagesAvailableCondition.notify_one(); // wake up page loading thread if it is sleeping
#endif
		}
	}	// unlock

	if (!cachedPages.empty()) // pass needed pages that are cached right to newPages so they don't have to roundtrip to another possibly busy thread. this is a optimization just for the MT path.
	{	// lock
		LOCK(vt.newPagesMutex)

		while(!cachedPages.empty())
		{
			const uint32_t pageInfo = cachedPages.front();

			vt.newPages.push(pageInfo);cachedPages.pop();

#if DEBUG_LOG > 0
			const uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
			const uint8_t mip = EXTRACT_MIP(pageInfo);
			printf("Loading page from RAM-cache: Mip:%u %u/%u (%i)\n", mip, x_coord, y_coord, pageInfo);
#endif
		}
	}	// unlock
}

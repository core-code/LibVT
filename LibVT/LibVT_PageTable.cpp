/*
 *  LibVT_PageTable.cpp
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

void _mapPageFallbackEntries(int m, int x_coord, int y_coord, int mip, int x, int y);
void _unmapPageFallbackEntries(int m, int x_coord, int y_coord, int x_search, int y_search, int mip_repl, int x_repl, int y_repl);


//void __debugEraseCachedPages();
//#define DEBUG_ERASE_CACHED_PAGES_EVERY_FRAME




void vtMapNewPages()
{
	queue<uint32_t>	newPages, zero;


#if !ENABLE_MT
	vtLoadNeededPages();
#endif


	{	// lock
		LOCK(vt.newPagesMutex)

		vt.missingPageCount = vt.neededPages.size(); // just stats keeping

		if (USE_PBO_PHYSTEX)
		{
			uint8_t i = 0;
			vt.newPageCount = 0;
			while (i < PBO_PHYSTEX_PAGES && !vt.newPages.empty())
			{
				newPages.push(vt.newPages.front());vt.newPages.pop();
				i++;
				vt.newPageCount++;  // just stats keeping
			}
		}
		else
		{
			newPages = vt.newPages;
			vt.newPageCount = newPages.size();  // just stats keeping
			vt.newPages = zero;
		}
	}	// unlock

	// we do this here instead of in vtLoadNeededPages() when new pages are actually mapped so it runs on the mainThread and the cachedPagesAccessTimes structure doesn't need to be locked
	vtcReduceCacheIfNecessaryLOCK(vt.thisFrameClock);

	if (!newPages.empty())
	{
		bool foundSlot = true;
		const void *image_data;

		for (uint8_t i = 0; i < c.mipChainLength; i++)
		{
#ifdef DEBUG_ERASE_CACHED_PAGES_EVERY_FRAME
			vt.mipLevelTouched[i] = true;
			vt.mipLevelMinrow[i] = 0;
			vt.mipLevelMaxrow[i] = (c.virtTexDimensionPages >> i) - 1;
#else
			vt.mipLevelTouched[i] = false;
			vt.mipLevelMinrow[i] = (uint16_t) c.virtTexDimensionPages >> i;
			vt.mipLevelMaxrow[i] = 0;
#endif
		}



#if USE_PBO_PHYSTEX
		uint8_t xCoordinatesForPageMapping[PBO_PHYSTEX_PAGES];
		uint8_t yCoordinatesForPageMapping[PBO_PHYSTEX_PAGES];
		uint8_t newPageCount = 0;

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vt.pboPhystex);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, c.pageMemsize * PBO_PHYSTEX_PAGES, 0, GL_STREAM_DRAW);

		uint8_t *phys_buffer = (uint8_t *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		assert(phys_buffer);
#endif


		glActiveTexture(GL_TEXTURE0 + TEXUNIT_FOR_PHYSTEX);

		while(!newPages.empty() && foundSlot)
		{
			const uint32_t pageInfo = newPages.front();newPages.pop();
			const uint16_t y_coord = EXTRACT_Y(pageInfo), x_coord = EXTRACT_X(pageInfo);
			const uint8_t mip = EXTRACT_MIP(pageInfo);

			image_data = vtcRetrieveCachedPageLOCK(pageInfo);

			// find slot
			bool foundFree = false;
			uint8_t x, y, storedX = 0, storedY = 0;
			clock_t lowestClock = vt.thisFrameClock;

			foundSlot = false;

			// find least recently used or free page
			for (x = 0; x < c.physTexDimensionPages; x++)
			{
				for (y = 0; y < c.physTexDimensionPages; y++)
				{
					if ((vt.textureStorageInfo[x][y].clockUsed < lowestClock) && (vt.textureStorageInfo[x][y].mip < c.mipChainLength - HIGHEST_MIP_LEVELS_TO_KEEP))
					{
						lowestClock = vt.textureStorageInfo[x][y].clockUsed;
						storedX = x;
						storedY = y;
						foundSlot = true;

						if (lowestClock == 0)
						{
							foundFree = true;
							break;
						}
					}
				}
				if (foundFree)
					break;
			}


			if (foundSlot)
			{
				x = storedX;
				y = storedY;

				if (!foundFree)
				{
					// unmap page
#if DEBUG_LOG > 0
					printf("Unloading page from VRAM: Mip:%u %u/%u from %u/%u lastUsed: %llu\n", vt.textureStorageInfo[x][y].mip, vt.textureStorageInfo[x][y].x, vt.textureStorageInfo[x][y].y, x, y, (long long unsigned int)lowestClock);
#endif

					vtUnmapPage(vt.textureStorageInfo[x][y].mip, vt.textureStorageInfo[x][y].x, vt.textureStorageInfo[x][y].y, x, y); // dont need complete version cause we map a new page at the same location
				}

				assert((x < c.physTexDimensionPages) && (y < c.physTexDimensionPages));


				// map page
				//vt.textureStorageInfo[x][y].active = true;
				vt.textureStorageInfo[x][y].x = x_coord;
				vt.textureStorageInfo[x][y].y = y_coord;
				vt.textureStorageInfo[x][y].mip = mip;
				vt.textureStorageInfo[x][y].clockUsed = vt.thisFrameClock;



				PAGE_TABLE(mip, x_coord, y_coord) = (MIP_INFO(mip) << 24) + (x << 16) + (y << 8) + kTableMapped;


				touchMipRow(mip, y_coord)

				if (FALLBACK_ENTRIES)
				{
					if (mip >= 1)
					{
						_mapPageFallbackEntries(mip - 1, x_coord * 2, y_coord * 2, mip, x, y);
						_mapPageFallbackEntries(mip - 1, x_coord * 2, y_coord * 2 + 1, mip, x, y);
						_mapPageFallbackEntries(mip - 1, x_coord * 2 + 1, y_coord * 2, mip, x, y);
						_mapPageFallbackEntries(mip - 1, x_coord * 2 + 1, y_coord * 2 + 1, mip, x, y);
					}
				}


#if USE_PBO_PHYSTEX
				memcpy(phys_buffer + c.pageMemsize * newPageCount, image_data, c.pageMemsize);
				xCoordinatesForPageMapping[newPageCount] = x;
				yCoordinatesForPageMapping[newPageCount] = y;

				newPageCount ++;
#else


				if (c.pageDXTCompression)
					glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, x * c.pageDimension, y * c.pageDimension, c.pageDimension, c.pageDimension, c.pageDXTCompression, c.pageMemsize, image_data);
				else
					glTexSubImage2D(GL_TEXTURE_2D, 0, x * c.pageDimension, y * c.pageDimension, c.pageDimension, c.pageDimension, c.pageDataFormat, c.pageDataType, image_data);

#if MIPPED_PHYSTEX
				uint32_t *mippedData;

				if (IMAGE_DECOMPRESSION_LIBRARY == DecompressionMac) // TODO: assert away other option
					mippedData = vtuDownsampleImageRGBA((const uint32_t *)image_data);
				else
					mippedData = vtuDownsampleImageRGB((const uint32_t *)image_data);

				glTexSubImage2D(GL_TEXTURE_2D, 1, x * (c.pageDimension / 2), y * (c.pageDimension / 2), (c.pageDimension / 2), (c.pageDimension / 2), c.pageDataFormat, c.pageDataType, mippedData);
				free(mippedData);
#endif
#if DEBUG_LOG > 0
				printf("Loading page to VRAM: Mip:%u %u/%u to %u/%u\n", mip, x_coord, y_coord, x, y);
#endif
#endif
			}
			else
			{	// lock
				LOCK(vt.newPagesMutex)

				printf("WARNING: skipping page loading because there are no free slots %i %i \n", vt.necessaryPageCount, c.physTexDimensionPages * c.physTexDimensionPages);

				vt.newPages.push(pageInfo);

				while (!newPages.empty())
				{
					vt.newPages.push(newPages.front());newPages.pop();
				}
			}	// unlock
		}

#if USE_PBO_PHYSTEX
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		for (uint8_t i = 0; i < newPageCount; i++)
		{
			if (c.pageDXTCompression)
				glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, xCoordinatesForPageMapping[i] * c.pageDimension, yCoordinatesForPageMapping[i] * c.pageDimension, c.pageDimension, c.pageDimension, c.pageDXTCompression, c.pageMemsize, (uint8_t *) NULL + (i * c.pageMemsize));
			else
				glTexSubImage2D(GL_TEXTURE_2D, 0, xCoordinatesForPageMapping[i] * c.pageDimension, yCoordinatesForPageMapping[i] * c.pageDimension, c.pageDimension, c.pageDimension, c.pageDataFormat, c.pageDataType,  (uint8_t *) NULL + (i * c.pageMemsize));
		}

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#endif

		glActiveTexture(GL_TEXTURE0 + TEXUNIT_FOR_PAGETABLE);


#if USE_PBO_PAGETABLE
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vt.pboPagetable);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, (vt.pageTableMipOffsets[c.mipChainLength - 1] + 1) * 4, 0, GL_STREAM_DRAW);

		uint32_t *table_buffer = (uint32_t *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		assert(table_buffer);
		memcpy(table_buffer, vt.pageTables[0], (vt.pageTableMipOffsets[c.mipChainLength - 1] + 1) * 4);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
#endif


		// done, upload pageTable
		for (uint8_t i = 0; i < c.mipChainLength; i++)
		{
			if (vt.mipLevelTouched[i] == true) // the whole touched, minrow, maxrow mess is there so we update only between the lowest and highest modified row, or nothing at all if no pixels are touched. ideally the page table updates should be much more finely grained than that.
			{
#if USE_PBO_PAGETABLE
				glTexSubImage2D(GL_TEXTURE_2D, i, 0, vt.mipLevelMinrow[i], c.virtTexDimensionPages >> i, vt.mipLevelMaxrow[i] + 1 - vt.mipLevelMinrow[i], GL_RGBA, GL_UNSIGNED_BYTE, (uint32_t *) NULL + (vt.pageTableMipOffsets[i] + (c.virtTexDimensionPages >> i) * vt.mipLevelMinrow[i]));
#else
				glTexSubImage2D(GL_TEXTURE_2D, i, 0, vt.mipLevelMinrow[i], c.virtTexDimensionPages >> i, vt.mipLevelMaxrow[i] + 1 - vt.mipLevelMinrow[i], GL_RGBA, GL_UNSIGNED_BYTE, vt.pageTables[i] + (c.virtTexDimensionPages >> i) * vt.mipLevelMinrow[i]);
#endif
			}
		}


#if USE_PBO_PAGETABLE
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#endif

		glActiveTexture(GL_TEXTURE0);
	}

	if (DYNAMIC_LOD_ADJUSTMENT)
	{	// automatic LoD bias adjustment
		float std; // TODO: doing this here is a BUG

		if (MIPPED_PHYSTEX)
			std = -0.5;
		else
			std = 0.;

		int pageOverflow =	vt.necessaryPageCount + c.residentPages - c.physTexDimensionPages * c.physTexDimensionPages;

		if (pageOverflow > -2.0f)
			vt.bias += 0.1f;

		if (pageOverflow < -7.0f && vt.bias > 0.0f)
			vt.bias -= 0.1f;
	}

#ifdef DEBUG_ERASE_CACHED_PAGES_EVERY_FRAME
	__debugEraseCachedPages();
#endif

	
//	// testcode for performing quality tests. it spews out a list of loaded pages every frame. this can be compared against a reference list with pixel coverage information (produced by commented code in vtExtractNeededPages()). make sure the simulation runs at 60FPS and is at a specific walthrough position each frame. 
//	for (int x = 0; x < c.physTexDimensionPages; x++)
//	{
//		for (int y = 0; y < c.physTexDimensionPages; y++)
//		{
//			if ((vt.textureStorageInfo[x][y].clockUsed == vt.thisFrameClock))
//			{
//				printf("PAGE: %i ", MAKE_PAGE_INFO(vt.textureStorageInfo[x][y].mip, vt.textureStorageInfo[x][y].x, vt.textureStorageInfo[x][y].y));
//			}
//		}
//	}
//	printf("\n\nNEWFRAME\n\n");
}


void _mapPageFallbackEntries(int m, int x_coord, int y_coord, int mip, int x, int y) // TODO: test long mip chain
{
	const uint32_t pageEntry = PAGE_TABLE(m, x_coord, y_coord);

	if ((uint8_t) pageEntry != kTableMapped)
	{
		PAGE_TABLE(m, x_coord, y_coord) = (MIP_INFO(mip) << 24) + (x << 16) + (y << 8) + ((uint8_t) pageEntry);
		touchMipRow(m, y_coord)

		if (m >= 1)
		{
			_mapPageFallbackEntries(m - 1, x_coord * 2, y_coord * 2, mip, x, y);
			_mapPageFallbackEntries(m - 1, x_coord * 2, y_coord * 2 + 1, mip, x, y);
			_mapPageFallbackEntries(m - 1, x_coord * 2 + 1, y_coord * 2, mip, x, y);
			_mapPageFallbackEntries(m - 1, x_coord * 2 + 1, y_coord * 2 + 1, mip, x, y);
		}
	}
}

void _unmapPageFallbackEntries(int m, int x_coord, int y_coord, int x_search, int y_search, int mip_repl, int x_repl, int y_repl)
{
	const uint32_t pageEntry = PAGE_TABLE(m, x_coord, y_coord);

	if ((BYTE3(pageEntry) == x_search) && (BYTE2(pageEntry) == y_search))
	{
		PAGE_TABLE(m, x_coord, y_coord) = (mip_repl << 24) + (x_repl << 16) + (y_repl << 8) + ((uint8_t) pageEntry);
		touchMipRow(m, y_coord)

		if (m >= 1)
		{
			_unmapPageFallbackEntries(m - 1, x_coord * 2, y_coord * 2, x_search, y_search, mip_repl, x_repl, y_repl);
			_unmapPageFallbackEntries(m - 1, x_coord * 2, y_coord * 2 + 1, x_search, y_search, mip_repl, x_repl, y_repl);
			_unmapPageFallbackEntries(m - 1, x_coord * 2 + 1, y_coord * 2, x_search, y_search, mip_repl, x_repl, y_repl);
			_unmapPageFallbackEntries(m - 1, x_coord * 2 + 1, y_coord * 2 + 1, x_search, y_search, mip_repl, x_repl, y_repl);
		}
	}
}

void vtUnmapPage(int mipmap_level, int x_coord, int y_coord, int x_storage_location, int y_storage_location)
{
	if (FALLBACK_ENTRIES)
	{
		const uint32_t pageEntry = PAGE_TABLE(mipmap_level + 1, x_coord / 2, y_coord / 2);
		*((uint8_t *)&PAGE_TABLE(mipmap_level, x_coord, y_coord)) = kTableFree;
		
		_unmapPageFallbackEntries(mipmap_level, x_coord, y_coord, x_storage_location, y_storage_location, BYTE4(pageEntry), BYTE3(pageEntry), BYTE2(pageEntry));
	}
	else
	{
		PAGE_TABLE(mipmap_level, x_coord, y_coord) = kTableFree;
		touchMipRow(mipmap_level, y_coord)
	}
}
				  
void vtUnmapPageCompleteley(int mipmap_level, int x_coord, int y_coord, int x_storage_location, int y_storage_location)
{
	vtUnmapPage(mipmap_level, x_coord, y_coord, x_storage_location, y_storage_location);

	vt.textureStorageInfo[x_storage_location][y_storage_location].x = 0;
	vt.textureStorageInfo[x_storage_location][y_storage_location].y = 0;
	vt.textureStorageInfo[x_storage_location][y_storage_location].mip = 0;
	vt.textureStorageInfo[x_storage_location][y_storage_location].clockUsed = 0;
}



void __debugEraseCachedPages()
{
#ifdef DEBUG_ERASE_CACHED_PAGES_EVERY_FRAME
	for (uint8_t i = 0; i < c.mipChainLength; i++)
		for (uint16_t x = 0; x < (c.virtTexDimensionPages >> i); x++)
			for (uint16_t y = 0; y < (c.virtTexDimensionPages >> i); y++)
				PAGE_TABLE(i, x, y) = kTableFree;


	for (int x = 0; x < c.physTexDimensionPages; x++)
	{
		for (int y = 0; y < c.physTexDimensionPages; y++)
		{
			vt.textureStorageInfo[x][y].x = 0;
			vt.textureStorageInfo[x][y].y = 0;
			vt.textureStorageInfo[x][y].mip = 0;
			vt.textureStorageInfo[x][y].clockUsed = 0;
		}
	}
#endif
}

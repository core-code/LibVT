/*
 *  LibVT_Cache.cpp
 *
 *
 *  Created by Julian Mayer on 26.11.09.
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

void vtcRemoveCachedPageLOCK(uint32_t pageInfo)
{
	LOCK(vt.cachedPagesMutex)
	
	return _vtcRemoveCachedPage(pageInfo);
}

void vtcTouchCachedPage(uint32_t pageInfo)
{
	vt.cachedPagesAccessTimes[pageInfo] = vt.thisFrameClock;
}

void vtcSplitPagelistIntoCachedAndNoncachedLOCK(queue<uint32_t> *s, queue<uint32_t> *cached, queue<uint32_t> *nonCached)
{
	LOCK(vt.cachedPagesMutex)

	while(!s->empty())
	{
		uint32_t page = s->front();

		if (vt.cachedPages.count(page))
			cached->push(page);
		else
			nonCached->push(page);

		s->pop();
	}
}

bool vtcIsPageInCacheLOCK(uint32_t pageInfo)
{
	LOCK(vt.cachedPagesMutex)

	return (vt.cachedPages.count(pageInfo) > 0);
}

void vtcInsertPageIntoCacheLOCK(uint32_t pageInfo, void * image_data)
{
	LOCK(vt.cachedPagesMutex)

	vt.cachedPages.insert(pair<uint32_t, void *>(pageInfo, image_data));
}

void * vtcRetrieveCachedPageLOCK(uint32_t pageInfo)
{
	LOCK(vt.cachedPagesMutex)

	assert(vt.cachedPages.count(pageInfo));

	return vt.cachedPages.find(pageInfo)->second;
}

void vtcReduceCacheIfNecessaryLOCK(clock_t currentTime)
{
	LOCK(vt.cachedPagesMutex)

	uint32_t size = (uint32_t)vt.cachedPages.size();

	if (size > c.maxCachedPages)
	{
		uint32_t pagesToErase = (size - c.maxCachedPages) * 4;
		if (pagesToErase > (c.maxCachedPages / 10)) pagesToErase = c.maxCachedPages / 10;
		multimap<clock_t, uint32_t> oldestPages;

#if DEBUG_LOG > 0
		printf("RAM-cache has %i too many pages - erasing the %i least recently touched pages!\n", (size - c.maxCachedPages), pagesToErase);
#endif

		for (uint32_t i = 0; i < pagesToErase; i++)
			oldestPages.insert(pair<clock_t, uint32_t>(currentTime+1+i, i));

		map<uint32_t, clock_t>::iterator cachedIter;
		for(cachedIter = vt.cachedPagesAccessTimes.begin(); cachedIter != vt.cachedPagesAccessTimes.end(); ++cachedIter)
		{
			if (cachedIter->second < oldestPages.rbegin()->first)
			{
				oldestPages.insert(pair<clock_t, uint32_t>(cachedIter->second, cachedIter->first));

				oldestPages.erase(--oldestPages.rbegin().base()); // this really is the easiest way to just erase the last element - C++ sucks
			}
		}

		assert(oldestPages.size() == pagesToErase);

		multimap<clock_t, uint32_t>::iterator oldestIter;
		for(oldestIter = oldestPages.begin(); oldestIter != oldestPages.end(); ++oldestIter)
		{
			uint32_t pageInfo = oldestIter->second;
			_vtcRemoveCachedPage(pageInfo);

#if DEBUG_LOG > 1
			printf("Un-loading page from RAM-cache: Mip:%u %u/%u\n", EXTRACT_MIP(pageInfo), EXTRACT_X(pageInfo), EXTRACT_Y(pageInfo));
#endif
		}
	}
}

void _vtcRemoveCachedPage(uint32_t pageInfo)
{
	void *data = vt.cachedPages[pageInfo];
	free(data);
	vt.cachedPages.erase(pageInfo);
	vt.cachedPagesAccessTimes.erase(pageInfo);
}
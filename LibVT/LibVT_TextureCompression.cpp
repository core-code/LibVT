/*
 *  LibVT_TextureCompression.cpp
 *
 *
 *  Created by Julian Mayer on 20.04.10.
 *  Copyright 2010 A. Julian Mayer. 
 *
 */

/*
 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "LibVT_Internal.h"
#include "LibVT.h"

#include "dxt.h"

extern vtConfig c;



void * vtuCompressRGBA_DXT1(void *rgba)
{
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
	return NULL;
#else

	int out_bytes = 0;
	uint8_t *out = (uint8_t *)malloc((c.pageDimension+3)*(c.pageDimension+3)/16*8);

	CompressImageDXT1((const byte*)rgba, out, c.pageDimension, c.pageDimension, out_bytes);

	return out;
#endif
}

void * vtuCompressRGBA_DXT5(void *rgba)
{
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
	return NULL;
#else
	int out_bytes = 0;
	uint8_t *out = (uint8_t *)malloc((c.pageDimension+3)*(c.pageDimension+3)/16*16);

	CompressImageDXT5((const byte*)rgba, out, c.pageDimension, c.pageDimension, out_bytes);

	return out;
#endif
}

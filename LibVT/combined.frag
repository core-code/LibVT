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
	#ifdef GL_OES_standard_derivatives
	   #extension GL_OES_standard_derivatives: require
	#endif
	varying vec2 texcoord;
#else
	#define texcoord gl_TexCoord[0]
#endif


#if MIPPED_PHYSTEX || ANISOTROPY || (!FALLBACK_ENTRIES && USE_MIPCALC_TEXTURE)
	#if GL_EXT_gpu_shader4
		#extension GL_EXT_gpu_shader4: require
		#if MIPPED_PHYSTEX || ANISOTROPY
			#define VTEX_GRAD 1
		#endif
	#else
		#error "a mipped physical texture, anisotropy or no fallback entries are used, but GL_EXT_gpu_shader4 is not supported. you can disable this error, but there will be artifacts"
	#endif
#endif

struct vtexcoord
{
   vec2 coord;
#ifdef VTEX_GRAD
   vec4 grad;
#endif
};

uniform sampler2D pageTableTexture;
uniform sampler2D physicalTexture;
uniform sampler2D mipcalcTexture;
uniform float mip_bias;


const float mip_trilerp_bias = 0.0;




#if !USE_MIPCALC_TEXTURE
#if ANISOTROPY
float mipmapLevel(vec2 uv, float textureSize)
{
	vec2 dx = dFdx(uv * textureSize);
	vec2 dy = dFdy(uv * textureSize);
	float Pmax = max(dot(dx, dx), dot(dy, dy));
	float Pmin = min(dot(dx, dx), dot(dy, dy));

	return 0.5 * log2(Pmax / min(ceil(Pmax/Pmin), max_anisotropy * max_anisotropy)) + mip_bias - prepass_resolution_reduction_shift;
}
#else
float mipmapLevel(vec2 uv, float textureSize)
{
	vec2 dx = dFdx(uv * textureSize);
	vec2 dy = dFdy(uv * textureSize);
	float d = max(dot(dx, dx), dot(dy, dy));

	return 0.5 * log2(d) + mip_bias - prepass_resolution_reduction_shift;
}
#endif
#endif

vec4 calculatePageRequest(vec2 uv)
{
	vec4 result;

#if USE_MIPCALC_TEXTURE
	result = texture2D(mipcalcTexture, texcoord.xy, page_dimension_log2 - prepass_resolution_reduction_shift + mip_bias);

	#if ANISOTROPY
		#error ANISOTROPY not supported in USE_MIPCALC_TEXTURE mode
	#endif
#else

	float mip = max(mipmapLevel(texcoord.xy, virt_tex_dimension), 0.0);
	vec2 page = floor(texcoord.xy * virt_tex_dimension_pages) / 255.0;

#if LONG_MIP_CHAIN
	result.rg = fract(page);
	vec2 pagefloor = floor(page);
	result.b = (pagefloor.y * 64.0 + pagefloor.x * 16.0 + mip) / 255.0; // 2 bit y  2 bit x  4 bit mip
#else
	result.rg = page;
	result.b = mip / 255.0;
#endif

	result.a = 1.0;		// BGRA		mip, x, y, 255
#endif

	return result;
}


vec4 samplePagetableOnce(vec2 coordinates)
{
#if USE_MIPCALC_TEXTURE
		return texture2D(pageTableTexture, coordinates, page_dimension_log2 + mip_bias) * 255.0;
#else
		return texture2DLod(pageTableTexture, coordinates, mipmapLevel(coordinates, virt_tex_dimension) + mip_bias) * 255.0;
#endif
}

#if !FALLBACK_ENTRIES
vec4 samplePagetableMultiple(vec2 coordinates, float bias, vec4 info)
{
#if USE_MIPCALC_TEXTURE
	float scale = exp2(-(page_dimension_log2 + bias)); // apply bias to gradients
	info = info / scale;
	return texture2DGrad(pageTableTexture, coordinates, info.xy, info.zw) * 255.0; // sampling inside a loop only works correctly with explicit lod or gradients
#else
	return texture2DLod(pageTableTexture, coordinates, info.x + bias) * 255.0;
#endif
}
#endif

#if !FALLBACK_ENTRIES
vtexcoord calulateCoordinatesFromSample(vec4 pageTableEntry, vec4 gradient)
#else
vtexcoord calulateCoordinatesFromSample(vec4 pageTableEntry)
#endif
{
#if LONG_MIP_CHAIN
	float mipExp = exp2(pageTableEntry.a);
#else
	float mipExp = pageTableEntry.a + 1.0;
#endif
	vec2 pageCoord = pageTableEntry.bg;
	vec2 withinPageCoord = fract(texcoord.xy * mipExp);

#if PAGE_BORDER
	withinPageCoord = withinPageCoord * (page_dimension - border_width * 2.0)/page_dimension + border_width/page_dimension;
//	withinPageCoord = withinPageCoord * (page_dimension - border_width)/page_dimension + 0.5/page_dimension; // one side border test code
#endif

   	vtexcoord result;
	result.coord = ((pageCoord + withinPageCoord) / phys_tex_dimension_pages);
#ifdef VTEX_GRAD
	float page_unit_to_phys = ((page_dimension - border_width * 2.0) /page_dimension) / phys_tex_dimension_pages;
	float gradient_scale = exp2(mip_bias + mip_trilerp_bias) * page_unit_to_phys * mipExp;

#if !FALLBACK_ENTRIES && USE_MIPCALC_TEXTURE
	gradient *= gradient_scale; // we already calculated a gradient earlier
	result.grad = gradient;
#else
	result.grad = vec4(dFdx(texcoord.xy) * gradient_scale, dFdy(texcoord.xy) * gradient_scale);
#endif
#endif

	return result;
}

vtexcoord calulateVirtualTextureCoordinates()
{
#if FALLBACK_ENTRIES
	vec4 pageTableEntry = samplePagetableOnce(texcoord.xy);

	return calulateCoordinatesFromSample(pageTableEntry);
#else
#if USE_MIPCALC_TEXTURE
	vec4 info;
	info.xy = dFdx(texcoord.xy); // calc gradients only once
	info.zw = dFdy(texcoord.xy);
#else
	vec4 info;
	info.x = mipmapLevel(texcoord.xy, virt_tex_dimension); // calc mipmap level only once
#endif

	vec4 pageTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
	float bias = 0.0;

	while (pageTableEntry.r < 200.0 && bias < 8.0)
	{
		pageTableEntry = samplePagetableMultiple(texcoord.xy, mip_bias + bias, info);
		bias += 1.0;
	}

	if (pageTableEntry.r > 200.0)
	{
		return calulateCoordinatesFromSample(pageTableEntry, info);
	}
	else
	{
		pageTableEntry = samplePagetableMultiple(texcoord.xy, mip_bias - 1.0, info);

		if (pageTableEntry.r > 200.0)
			return calulateCoordinatesFromSample(pageTableEntry, info);
		else
		{
		   	vtexcoord result;
			result.coord = vec2(2.0, 2.0);
			return result;
		}
	}
#endif
}

vec4 sampleVirtualTexture(vtexcoord coordinates)
{
#ifdef VTEX_GRAD
		return texture2DGrad(physicalTexture, coordinates.coord, coordinates.grad.xy, coordinates.grad.zw);
#else
		return texture2D(physicalTexture, coordinates.coord);
#endif
}


void main()
{
	gl_FragData[0] = sampleVirtualTexture(calulateVirtualTextureCoordinates());
	gl_FragData[1] = calculatePageRequest(texcoord.xy);
}

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
uniform float mip_bias;

const float mip_trilerp_bias = 0.0;

#ifdef DEBUG
uniform sampler2D normalTexture;
uniform sampler2D mipcalcTexture;
uniform int debugMode;
#endif

#if (!USE_MIPCALC_TEXTURE || defined(DEBUG)) && !defined(GL_ES)
#if ANISOTROPY
float mipmapLevel(vec2 uv, float textureSize)
{
	vec2 dx = dFdx(uv * textureSize);
	vec2 dy = dFdy(uv * textureSize);
	float Pmax = max(dot(dx, dx), dot(dy, dy));
	float Pmin = min(dot(dx, dx), dot(dy, dy));
	float N = min(ceil(Pmax/Pmin), max_anisotropy * max_anisotropy);

	return 0.5 * log2(Pmax / N) + mip_bias;
}
#else
float mipmapLevel(vec2 uv, float textureSize)
{
	vec2 dx = dFdx(uv * textureSize);
	vec2 dy = dFdy(uv * textureSize);
	float d = max(dot(dx, dx), dot(dy, dy));

	return 0.5 * log2(d) + mip_bias;
}
#endif
#endif


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

	while (pageTableEntry.r < 200.0 && bias < 10.0)
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





//	varying vec3 v;
//	varying vec3 lightvec;
//	varying vec3 normal;
//	varying vec4 FrontColor;

void main(void)
{
	vtexcoord coord = calulateVirtualTextureCoordinates();
	vec4 vtex = sampleVirtualTexture(coord);


#ifdef GL_ES
		gl_FragColor	= vtex;
#else
//		vec3 Eye		= normalize(-v);
//		vec3 Reflected	= normalize(reflect( -lightvec, normal));
//
//		vec4 IAmbient	= gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
//		vec4 IDiffuse	= gl_LightSource[0].diffuse * max(dot(normal, lightvec), 0.0) * gl_FrontMaterial.diffuse;
//		vec4 ISpecular	= gl_LightSource[0].specular * pow(max(dot(Reflected, Eye), 0.0), gl_FrontMaterial.shininess) * gl_FrontMaterial.specular;
//
//		vec4 color		= vec4((gl_FrontLightModelProduct.sceneColor + IAmbient + IDiffuse) * vtex + ISpecular);
//		gl_FragColor	= color;
		gl_FragColor	= vtex;
#endif









//		gl_FragColor	= vec4(coord.grad.x, coord.grad.y, coord.grad.z, 1.0);
//		gl_FragColor    = vec4(mipExp / 255.0, mipExp / 255.0, mipExp / 255.0, 1.0);
//		gl_FragColor    = vec4(mipExp / 100.0, mipExp / 100.0, mipExp / 100.0, 1.0);
//		gl_FragColor    = vec4((withinPageCoord / phys_tex_dimension_pages).x * 10.0, (withinPageCoord / phys_tex_dimension_pages).y * 10.0, 0.0, 1.0);
//


#ifdef DEBUG
	if (debugMode == 1)
	{
		vec2 page = fract(floor(texcoord.xy * (vec2(virt_tex_dimension_pages))) / 256.0);
		float m = mipmapLevel(texcoord.xy, virt_tex_dimension);
		float i;
		vec4 result;

//		for (i = 0.0; i < m - 0.5; i += 1.0)
//			page /= 2.0;
		result.rg = page / 255.0;
		result.b = float(int(m)) / 255.0;
		result.a = 1.0;

		gl_FragColor = result * 1.0;

	//	gl_FragColor = vec4(result.b, result.b, result.b, 0.1)  * 10.0;
	}
	else if (debugMode == 2)
	{
		vec4 tex = texture2D(mipcalcTexture, texcoord.xy, page_dimension_log2 - 0.5 - prepass_resolution_reduction_shift + mip_bias);

		int bla = int(tex.b * 255.0);
		float blente = float(bla) / 255.0;

		gl_FragColor = tex * 1.0;
		//gl_FragColor = vec4(blente, blente, blente, 0.1) * 10.0;
	}

	else if (debugMode == 3)
	{
		vec2 page = fract(floor(texcoord.xy * (vec2(virt_tex_dimension_pages))) / 256.0);
		float m = mipmapLevel(texcoord.xy, virt_tex_dimension);
		float i;
		vec4 result;

		for (i = 0.0; i < m - 0.5; i += 1.0)
			page /= 2.0;
		result.rg = page / 255.0;
		result.rg = floor(result.rg * 256.0 + 0.5) / 255.0;

		vec4 tex = texture2D(mipcalcTexture, texcoord.xy, page_dimension_log2 - 0.5 - prepass_resolution_reduction_shift  + mip_bias);

		gl_FragColor = vec4(0.5 + tex.r - result.r, 0.5 + tex.g - result.g, 0.0, 1.0);
		if (((tex.r - result.r) == 0.0) && ((tex.g - result.g) == 0.0))
			gl_FragColor	= vec4(0.1, 0.9, 0.1, 1.0);
	}

	else if (debugMode == 4) // VT without fallback
	{
		vec4 pageTableEntry = vec4(0.0, 0.0, 0.0, 0.0);

		pageTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias) * 255.0;

		if (pageTableEntry.r > 200.0)
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
	#endif
			vec2 finalCoord = (pageCoord + withinPageCoord) / phys_tex_dimension_pages;

			gl_FragColor	= texture2D(physicalTexture, finalCoord);
		}
		else
		{
//			float plus_bias = 0.0;
//			float minus_bias = 0.0;
//			vec4 plusTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
//			vec4 minusTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
//
//			while (plusTableEntry.r < 200.0 && plus_bias < 8.0)
//			{
//				plusTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + plus_bias) * 255.0;
//				plus_bias += 0.1;
//			}
//			while (minusTableEntry.r < 200.0 && minus_bias < 8.0)
//			{
//				minusTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias - minus_bias) * 255.0;
//				minus_bias += 0.1;
//			}
//
//			if ((plusTableEntry.r > 200.0 ) && (minusTableEntry.r > 200.0))
//				gl_FragColor	= vec4(0.0, 0.9, 0.0, 1.0);
//			else if (plusTableEntry.r > 200.0)
//				gl_FragColor	= vec4(plus_bias / 5.0, 0.3, 0.0, 1.0);
//			else if (minusTableEntry.r > 200.0)
//				gl_FragColor	= vec4(minus_bias / 5.0, 0.0, 0.3, 1.0);
//			else
				gl_FragColor	= vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	else if (debugMode == 5)
	{
//		if (pageTableEntry.r > 200.0)
//		{
//#if LONG_MIP_CHAIN
//		float mipExp = exp2(pageTableEntry.a);
//#else
//		float mipExp = pageTableEntry.a + 1.0;
//#endif
//			vec2 pageCoord = pageTableEntry.bg;
//			vec2 withinPageCoord = fract(texcoord.xy * mipExp);
//#if PAGE_BORDER
//			withinPageCoord = withinPageCoord * (page_dimension - border_width * 2.0)/page_dimension + border_width/page_dimension;
//#endif
//			vec2 finalCoord = (pageCoord + withinPageCoord) / phys_tex_dimension_pages;
//
//
//			vec3 Eye		= normalize(-v);
//			vec3 Reflected	= normalize(reflect( -lightvec, normal));
//
//			vec4 IAmbient	= gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
//			vec4 IDiffuse	= gl_LightSource[0].diffuse * max(dot(normal, lightvec), 0.0) * gl_FrontMaterial.diffuse;
//			vec4 ISpecular	= gl_LightSource[0].specular * pow(max(dot(Reflected, Eye), 0.0), gl_FrontMaterial.shininess) * gl_FrontMaterial.specular;
//
//			vec4 color		= vec4((gl_FrontLightModelProduct.sceneColor + IAmbient + IDiffuse) * texture2D(physicalTexture, finalCoord) + ISpecular);
//			gl_FragColor	= color;
//		}
//		else
//		{
		vec4 pageTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
		float bias = 0.0;

		while (pageTableEntry.r < 200.0 && bias < 5.0)
		{
			pageTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + bias) * 255.0;
			bias += 1.0;
		}

		if (pageTableEntry.r > 200.0)
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
	#endif
			vec2 finalCoord = (pageCoord + withinPageCoord) / phys_tex_dimension_pages;

			gl_FragColor	= texture2D(physicalTexture, finalCoord);
		}
		else
		{
			float plus_bias = 0.0;
			float minus_bias = 0.0;
			vec4 plusTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 minusTableEntry = vec4(0.0, 0.0, 0.0, 0.0);

			while (plusTableEntry.r < 200.0 && plus_bias < 8.0)
			{
				plusTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + plus_bias) * 255.0;
				plus_bias += 0.1;
			}
			while (minusTableEntry.r < 200.0 && minus_bias < 8.0)
			{
				minusTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias - minus_bias) * 255.0;
				minus_bias += 0.1;
			}

			if ((plusTableEntry.r > 200.0 ) && (minusTableEntry.r > 200.0))
				gl_FragColor	= vec4(0.0, 0.9, 0.0, 1.0);
			else if (plusTableEntry.r > 200.0)
				gl_FragColor	= vec4(plus_bias / 5.0, 0.3, 0.0, 1.0);
			else if (minusTableEntry.r > 200.0)
				gl_FragColor	= vec4(minus_bias / 5.0, 0.0, 0.3, 1.0);
			else
				gl_FragColor	= vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	else if (debugMode == 6)
	{
//		if (pageTableEntry.r > 200.0)
//		{
//#if LONG_MIP_CHAIN
//		float mipExp = exp2(pageTableEntry.a);
//#else
//		float mipExp = pageTableEntry.a + 1.0;
//#endif
//			vec2 pageCoord = pageTableEntry.bg;
//			vec2 withinPageCoord = fract(texcoord.xy * mipExp);
//#if PAGE_BORDER
//			withinPageCoord = withinPageCoord * (page_dimension - border_width * 2.0)/page_dimension + border_width/page_dimension;
//#endif
//			vec2 finalCoord = (pageCoord + withinPageCoord) / phys_tex_dimension_pages;
//
//
//			vec3 Eye		= normalize(-v);
//			vec3 Reflected	= normalize(reflect( -lightvec, normal));
//
//			vec4 IAmbient	= gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
//			vec4 IDiffuse	= gl_LightSource[0].diffuse * max(dot(normal, lightvec), 0.0) * gl_FrontMaterial.diffuse;
//			vec4 ISpecular	= gl_LightSource[0].specular * pow(max(dot(Reflected, Eye), 0.0), gl_FrontMaterial.shininess) * gl_FrontMaterial.specular;
//
//			vec4 color		= vec4((gl_FrontLightModelProduct.sceneColor + IAmbient + IDiffuse) * texture2D(physicalTexture, finalCoord) + ISpecular);
//			gl_FragColor	= color;
//		}
//		else
//		{
		vec4 pageTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
		float bias = 0.0;

		while (pageTableEntry.r < 200.0 && bias < 5.0)
		{
			pageTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + bias) * 255.0;
			bias += 1.0;
		}

		if (pageTableEntry.r > 200.0)
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
	#endif
			vec2 finalCoord = (pageCoord + withinPageCoord) / phys_tex_dimension_pages;

			gl_FragColor	= texture2D(physicalTexture, finalCoord);
			gl_FragColor	= vec4(0.0, 0.9, 0.0, 1.0);
		}
		else
		{
			float plus_bias = 0.0;
			float minus_bias = 0.0;
			vec4 plusTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
			vec4 minusTableEntry = vec4(0.0, 0.0, 0.0, 0.0);

			while (plusTableEntry.r < 200.0 && plus_bias < 8.0)
			{
				plusTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + plus_bias) * 255.0;
				plus_bias += 0.1;
			}
			while (minusTableEntry.r < 200.0 && minus_bias < 8.0)
			{
				minusTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias - minus_bias) * 255.0;
				minus_bias += 0.1;
			}

			if ((plusTableEntry.r > 200.0 ) && (minusTableEntry.r > 200.0))
				gl_FragColor	= vec4(0.0, 0.9, 0.0, 1.0);
			else if (plusTableEntry.r > 200.0)
				gl_FragColor	= vec4(plus_bias / 5.0, 0.3, 0.0, 1.0);
			else if (minusTableEntry.r > 200.0)
				gl_FragColor	= vec4(minus_bias / 5.0, 0.0, 0.3, 1.0);
			else
				gl_FragColor	= vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	else if (debugMode == 7) // phystex
	{
		vec2 coord = gl_FragCoord.xy / 1000.0;
		if ((coord.x < 1.0) && (coord.y < 1.0))
			gl_FragColor = texture2D(physicalTexture, coord);
	}
	else if ((debugMode == 8) || (debugMode == 9)) // non vt tex
	{
//		vec3 Eye		= normalize(-v);
//		vec3 Reflected	= normalize(reflect( -lightvec, normal));
//
//		vec4 IAmbient	= gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
//		vec4 IDiffuse	= gl_LightSource[0].diffuse * max(dot(normal, lightvec), 0.0) * gl_FrontMaterial.diffuse;
//		vec4 ISpecular	= gl_LightSource[0].specular * pow(max(dot(Reflected, Eye), 0.0), gl_FrontMaterial.shininess) * gl_FrontMaterial.specular;
//
//		vec4 color		= vec4((gl_FrontLightModelProduct.sceneColor + IAmbient + IDiffuse) * texture2D(normalTexture, texcoord.xy) + ISpecular);
//		gl_FragColor	= color;
		gl_FragColor	= texture2D(normalTexture, texcoord.xy);
	}
	else if (debugMode == 10)
	{
		float m = mipmapLevel(texcoord.xy, virt_tex_dimension);
		gl_FragColor	= vec4(float(int(m)) / 10.0, float(int(m)) / 10.0, float(int(m)) / 10.0, 1.0);

	}
	else if (debugMode == 11)
	{
		float m = texture2D(mipcalcTexture, texcoord.xy, page_dimension_log2 - 0.5 - prepass_resolution_reduction_shift + mip_bias).b * 255.0;
		gl_FragColor	= vec4(m / 10.0, m / 10.0, m / 10.0, 1.0);

	}
	else if (debugMode == 12)
	{
//	vec4 pageTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias) * 255.0;
		vec4 pageTableEntryDB = texture2D(mipcalcTexture, texcoord.xy, page_dimension_log2 - 0.5 - prepass_resolution_reduction_shift) * 255.0;

//		if (pageTableEntry.r > 200.0)
		if (pageTableEntryDB.r > -10.0)
		{
//			vec4 sample = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias) * 255.0;
				vec4 sampleDB = texture2D(mipcalcTexture, texcoord.xy, page_dimension_log2 - 0.5 - prepass_resolution_reduction_shift ) * 255.0;

//				if (pageTableEntryDB != sampleDB)
//				{
//					vec4 sample2 = vec4(0.0, 0.0, 0.0, 0.0);
//					sample2 = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias) * 255.0;
//
//					if (mip_bias < -0.1)
//						gl_FragColor	= vec4(pageTableEntryDB.b, pageTableEntryDB.g, pageTableEntryDB.a, 1.0) / 255.0;
//					else if (mip_bias > 0.05)
//						gl_FragColor	= vec4(sampleDB.b, sampleDB.g, sampleDB.a, 1.0) / 255.0;
//					else
//						gl_FragColor	= sample / 255.0;
//				}
//				else
//					gl_FragColor	= vec4(0.0, 1.0, 0.0, 1.0);
//
				//gl_FragColor	= vec4(sampleDB.b, sampleDB.g, sampleDB.a, 1.0) / 255.0;
				//gl_FragColor	= vec4(pageTableEntryDB.b, pageTableEntryDB.g, pageTableEntryDB.a, 1.0) / 255.0;
			gl_FragColor = abs(pageTableEntryDB - sampleDB);
		}
		else
		{
			gl_FragColor	= vec4(1.0, 1.0, 1.0, 1.0);
		}
	}
	else if (debugMode == 13)
	{
 #ifdef TEX_QUERY
		float mip = textureQueryLOD(texcoord.xy).y / 255.0;

		gl_FragColor = vec4(mip, mip, mip, 0.1) * 10.0;
	#endif
	}
	else if (debugMode == 14)  // NVIDIA sample "bug" test
	{
		vec4 pageTableEntry = vec4(0.0, 0.0, 0.0, 0.0);
		float bias = -1.0;

		while (pageTableEntry.r < 200.0 && bias < 5.0)
		{
			bias += 1.0;
			pageTableEntry = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + bias) * 255.0;
		}

		if (pageTableEntry.r > 200.0)
		{
			vec4 sample = texture2D(pageTableTexture, texcoord.xy, page_dimension_log2 - 0.5 + mip_bias + bias) * 255.0;
			gl_FragColor	=  vec4(sample.r, pageTableEntry.r, sample.g, 1.0) / 255.0;
		}
		else
		{
			gl_FragColor	= vec4(1.0, 1.0, 1.0, 1.0);
		}
	}
	else if (debugMode == 15) // test tile determination by texture sampling at mip level 0
	{
		vec4 tex = texture2DLod(mipcalcTexture, texcoord.xy, 0.0);

//	result.b = (pagefloor.y * 64.0 + pagefloor.x * 16.0 + mip) / 255.0; // 2 bit y  2 bit x  4 bit mip
#if LONG_MIP_CHAIN
		int xy = int(tex.b * 255.0);
		xy /= 16; // shift away 4 bit mip
		int x = xy / 4;
		int y = int(fract(float(xy) / 4.0) * 4.0);
		tex.g += float(x);
		tex.r += float(y);

	//	tex /= 4.0;

		tex.b = 0.0;
#else
#endif
		gl_FragColor = tex;

	}
	else if (debugMode == 16)  // test calculated tile determination at mip level 0
	{
		vec4 result;

		result.rg = floor(texcoord.xy * (vec2(virt_tex_dimension_pages))) / 255.0;
	//	result.rg *= 254.75/255.0;
		result.b = (mipmapLevel(texcoord.xy, virt_tex_dimension)) / 255.0;
		result.a = 1.0;
	//	result /= 4.0;
		result.b = 0.0;
		gl_FragColor = result;
	}
	else if (debugMode == 17) // compare calculated and sampled tile determination at mip level 0
	{
		vec4 tex = texture2DLod(mipcalcTexture, texcoord.xy, 0.0);
		vec4 result;

#if LONG_MIP_CHAIN
		int xy = int(tex.b * 255.0);
		xy /= 16; // shift away 4 bit mip
		int x = xy / 4;
		int y = int(fract(float(xy) / 4.0) * 4.0);
		tex.g += float(x);
		tex.r += float(y);

#else
#endif

	//	result.rg = fract( (floor(texcoord.xy * (vec2(virt_tex_dimension_pages))) / 255.0));
		result.rg = (floor(texcoord.xy * (vec2(virt_tex_dimension_pages))) / 255.0);// * (1.0 - 3.0 * 1.0 / 1020.0);
		//result.rg = floor(result.rg * 256.0 + 0.5) / 255.0;

//		gl_FragColor = vec4(0.5 + (result.r - tex.r) * 100.0, 0.5 + (result.g - tex.g) * 100.0, 0.0, 1.0);
//		if ((gl_FragColor.r == 0.5) && (gl_FragColor.g == 0.5))
//			gl_FragColor	= vec4(0.1, 0.9, 0.1, 1.0);
		if ((int(result.r * 255.0) != int(tex.r * 255.0)) || (int(result.g * 255.0) != int(tex.g * 255.0)))
		{
			gl_FragColor =  vec4((result.r - tex.r) * 10.0, (result.g - tex.g) * 10.0, 0.0, 1.0);
		}
		else
			gl_FragColor	= vec4(0.0, 1.0, 0.0, 1.0);
	}
#endif
}

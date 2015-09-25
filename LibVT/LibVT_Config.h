/*!
 * @file	LibVT_Config.h
 * @brief	The Config-file for LibVT.
 */



/*!
 * @def		TEXUNIT_FOR_PAGETABLE
 * @brief	The texunit to use for the page table texture, can't be used otherwise in the client app <br>
 * Note:	0 - 15, depending on the hardware
 */
#define TEXUNIT_FOR_PAGETABLE		1

/*!
 * @def		TEXUNIT_FOR_PHYSTEX
 * @brief	The texunit to use for the physical texture, can't be used otherwise in the client app <br>
 * Note:	0 - 15, depending on the hardware
 */
#define TEXUNIT_FOR_PHYSTEX			2

/*!
 * @def		TEXUNIT_FOR_MIPCALC
 * @brief	The texunit to use for the tile calculation texture, can't be used otherwise in the client app, only takes effect if USE_MIPCALC_TEXTURE is 1 <br>
 * Note:	0 - 15, depending on the hardware
 */
#define TEXUNIT_FOR_MIPCALC			3


// ********** PAGE STORE OPTIONS **********

/*!
 * @def		LONG_MIP_CHAIN
 * @brief	Controls wheather a mipChainLength below 9 or above 9 is allowed <br>
 * Note:	Set to zero for mipChainLength <= 9, 1 for mipChainLength > 9 <br>
 * Values:	0, 1
 */
#define	LONG_MIP_CHAIN				0

/*!
 * @def		ANISOTROPY
 * @brief	Controls the max. anisotropy to use (for filtering the virtual texture) <br>
 * Values:	0 (to turn off), 2, 4, 6, 8, 10, 12, 14, 16
 */
#define ANISOTROPY					0

/*!
 * @def		VT_MAG_FILTER
 * @brief	Controls the magnification filter (for filtering the virtual texture) <br>
 * Note:	To use GL_LINEAR, the stored page tile must be generated with the "bilinear" option which generates a border <br>
 * Values:	GL_NEAREST, GL_LINEAR
 */
#define VT_MAG_FILTER				GL_LINEAR

/*!
 * @def		VT_MIN_FILTER
 * @brief	Controls the minification filter (for filtering the virtual texture) <br>
 * Note:	To use GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST or GL_LINEAR_MIPMAP_LINEAR the stored page tile must be generated with the "bilinear" option which generates a border. If GL_*_MIPMAP_* is used, the physical texture will have two instead of one levels, taking 1/4 more memory and necessitating a more complex shader with gradient calculation. If GL_EXT_gpu_shader4 is not available there will probably be artifacts. <br>
 * Values:	GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST (probably dumb), GL_LINEAR_MIPMAP_NEAREST (probably dumb), GL_NEAREST_MIPMAP_LINEAR (probably dumb), GL_LINEAR_MIPMAP_LINEAR
 */
#define VT_MIN_FILTER				GL_LINEAR


// ********** PERFORMANCE OPTIONS **********

/*!
 * @def		PHYS_TEX_DIMENSION
 * @brief	This is is the width/height of the physical texture measured in pixels <br>
 * Note:	Affects VRAM usage / performance, setting this too high may cause silent failures => everything black <br>
 * Values:	2048, 4096, 8192 <br>
 * Info:	Testing suggests minimum ratios of phystex to viewport pixels of 2.1, 2.9, 3.9 for pagesizes of 64, 128, 256 for terrain, other scenes are worse.
 */
#define PHYS_TEX_DIMENSION			8192

/*!
 * @def		MAX_RAMCACHE_MB
 * @brief	The maximal amount of megabytes to use for RAM caching of pages <br>
 * Note:	Affects RAM usage / performance <br>
 * Values:	50 - FREE_RAM_IN_SYSTEM
 */
#define MAX_RAMCACHE_MB				2000

/*!
 * @def		PREPASS_RESOLUTION_REDUCTION_SHIFT
 * @brief	Perform readback on full screen resolution or shifted right x times <br>
 * Note:	Affects correctness / performance. 0 is very slow, 3 is probably to inaccurate for anything but a terrain scene. Must obviously be 0 in a single-pass solution. <br>
 * Values:	0 - 4
 */
#define PREPASS_RESOLUTION_REDUCTION_SHIFT	2

/*!
 * @def		HIGHEST_MIP_LEVELS_TO_KEEP
 * @brief	Keep the X highest mip levels in VRAM all the time <br>
 * Note:	Affects VRAM usage / performance <br>
 * Values:	0 - 5
 */
#define HIGHEST_MIP_LEVELS_TO_KEEP	1

/*!
 * @def		HIGHEST_MIP_LEVELS_TO_PRECACHE
 * @brief	Load the X highest mip levels to the RAM cache before starting <br>
 * Note:	Affects startupTime / runtimePerformance <br>
 * Values:	0 - 7 (must be lower than the mipchain length)
 */
#define HIGHEST_MIP_LEVELS_TO_PRECACHE	0

/*!
 * @def		READBACK_MODE
 * @brief	Controls whether the prepass is rendered into a FBO or the normal backbuffer and whether readback is done using glGetTexImage() or glReadPixels() <br>
 * Note:	Affects VRAM usage / performance <br>
 * Values:	kBackbufferReadPixels (worst), kBackbufferGetTexImage (bad), kFBOReadPixels (best), kFBOGetTexImage (good), kCustomReadback (use if you don't call vtPrepareReadback() and vtPerformReadback() but do the readback yourself and provide the buffer to vtExtractNeededPages(). e.g. when doing OSG integration; or when having the OpenCL integration on and doing a single renderpass for both rendering and tile determination)
 */
#define READBACK_MODE				kFBOGetTexImage

/*!
 * @def		USE_PBO_READBACK
 * @brief	Perform asynchronous prepass readback with the help of a Pixel Buffer Object. Currently this means that vtPerformReadback() will return immeadetly because it only triggers the asynchronous readback. The buffer will then be mapped at the beginning of vtExtractNeededPages(). Thereforce you can do CPU work between vtPerformReadback() and vtExtractNeededPages() that would otherwise be wasted waiting for the GPU result. Must be on if you want to delay the readback until the next frame. <br>
 * Note:	Affects VRAM usage / performance <br>
 * Values:	0 - 1
 */
#define USE_PBO_READBACK			0

/*!
 * @def		USE_PBO_PAGETABLE
 * @brief	Perform asynchronous pagetable texture uploading with the help of a Pixel Buffer Object.
 * Note:	Affects VRAM usage / performance <br>
 * Values:	0 - 1
 */
#define USE_PBO_PAGETABLE			0

/*!
 * @def		USE_PBO_PHYSTEX
 * @brief	Perform asynchronous physical texture uploading with the help of a Pixel Buffer Object.
 * Note:	Affects VRAM usage / performance <br>
 * Values:	0 - 1
 */
#define USE_PBO_PHYSTEX				0
/*!
 * @def		PBO_PHYSTEX_PAGES
 * @brief	The size of the PBO for phystex updates in pages. Limits the amount of new pages that can be sent to the GPU in one frame.
 * Note:	Affects VRAM usage / performance <br>
 * Values:	1 - 30
 */
#define PBO_PHYSTEX_PAGES			15

/*!
 * @def		USE_MIPCALC_TEXTURE
 * @brief	In the tile calculation pre-pass determine tiles by sampling from an additional texture instead of calculating it. <br>
 * Note:	Affects performance, correctness and VRAM usage <br>
 * Values:	0 - 1 <br>
 * Info:	This a bit faster on some hardware but uses more texture memory (the texture is the same size as the page-table texture) and may be slower on other hardware. Currenly must be in in LONG_MIPCHAIN mode because the manual calculation is a bit inaccurate, perhaps precision limitations?
 */
#define USE_MIPCALC_TEXTURE			1

/*!
 * @def		REALTIME_DXT_COMPRESSION
 * @brief	Compress tiles to DXT1 before sending them to the GPU. <br>
 * Note:	Affects performance, correctness and VRAM usage <br>
 * Values:	0 - 1 <br>
 * Info:	This currently only works with DecompressionLibJPEGTurbo or DecompressionMac. Reduces GPU memory requirements to just 1/8.
 */
#define REALTIME_DXT_COMPRESSION	0

/*!
 * @def		DYNAMIC_LOD_ADJUSTMENT
 * @brief	Turn this on to make LibVT dynamically adjust the mipmap lod bias to make the required tiles fit the physical page.  <br>
 * Note:	Affects quality <br>
 * Info:	Can be off if your scene fits anyway, turn it on to provide smooth degradation instead if dropping tiles randomly. Algorithm is a bit unstable. Requires that you pass vtGetBias() as value for the uniform "mip_bias" to the shaders (both!). <br>
 * Values:	0 - 1
 */
#define DYNAMIC_LOD_ADJUSTMENT		0

/*!
 * @def		OPENCL_BUFFERREDUCTION
 * @brief	Turn this on if you want to use the OpenCL buffer compression kernels which can 1.) provide speedups in 2 pass mode because the readback is on a few bytes instead of a large buffer 2.) enable a single pass solution <br>
 * Values:	0 - 1
 */
#define OPENCL_BUFFERREDUCTION		0

/*!
 * @def		OPENCL_REDUCTION_SHIFT
 * @brief	Tells the OpenCL reduction kernels to analyze the information buffer at a lower resolution. <br>
 * Note:	Affects performance, correctness <br>
 * Info:	Is similar to PREPASS_RESOLUTION_REDUCTION_SHIFT, which can not be used in a single pass solution. If you are using OpenCL, but a dual pass solution, use PREPASS_RESOLUTION_REDUCTION_SHIFT instead. <br>
 * Values:	0 - 4
 */
#define OPENCL_REDUCTION_SHIFT		0

/*!
 * @def		DEBUG_LOG
 * @brief	Sets the level of information about requested, loaded and unloaded pages outputted via printf() <br>
 * Info:	Should be set to 0 unless a problem is being debugged <br>
 * Values:	0 - 2
 */
#define DEBUG_LOG					0

/*!
 * @def		ENABLE_MT
 * @brief	Enable asynchronous page loading using 1 or 2 additional threads <br>
 * Note:	Affects performance <br>
 * Values:	0 - 2 (0 = no asynchronous page loading, 1 = asynchronous page loading using 1 additional thread, 2 = asynchronous page loading using 2 additional threads, one for loading and decompression respectively)
 */
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
	#define ENABLE_MT				0
#elif defined(WIN32)
	#define ENABLE_MT				2
#else
	#define ENABLE_MT				2
#endif

/*!
 * @def		FALLBACK_ENTRIES
 * @brief	Writes fallback entries in the page table so no looping in the pixel shader is necessary. Trades pixel shader cost with page table management & traffic cost. <br>
 * Note:	Affects performance, turning this off is SLOWER <br>
 * Values:	0 - 1
 */
#define FALLBACK_ENTRIES			1

/*!
 * @def		IMAGE_DECOMPRESSION_LIBRARY
 * @brief	Sets the image decompression library that will be used <br>
 * Note:	When setting to a library which doesn't handle all formats, this restricts the possible image formats at runtime <br>
 * Values:	DecompressionLibPNG, DecompressionSTBIPNG, DecompressionLibJPEG, DecompressionLibJPEGTurbo, DecompressionSTBIJPEG, DecompressionMac, DecompressionDevil
 */
#define IMAGE_DECOMPRESSION_LIBRARY	DecompressionSTBIJPEG



#if (FALLBACK_ENTRIES == 1) && (HIGHEST_MIP_LEVELS_TO_KEEP == 0)
	#error FALLBACK_ENTRIES requires HIGHEST_MIP_LEVELS_TO_KEEP >= 1
#endif

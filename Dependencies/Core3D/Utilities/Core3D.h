// TODO: display link, evaluate globals, tmpMsg shit, move events to responder model, instantiate Simulation from NIB file (really??), fonts, kill SDL (http://developer.apple.com/qa/qa2004/qa1385.html), interleaved arrays

#ifndef CORE3D_HEADER
#define CORE3D_HEADER 1

#if !defined(__cplusplus) || !defined(__OBJC__)
	#error CORE3D_NEEDS_OBJC++
#endif


typedef struct _TriangleIntersectionInfo {
	BOOL intersects;
//	vector3f v1, v2, v3;
//	vector3f o1, o2, o3;
	vector3f normal;
	float depth;
} TriangleIntersectionInfo;

enum {
	kIntersecting = -1,
	kOutside,
	kInside
};

typedef enum {
	kNeedEnabled = 0,
	kNeedDisabled,
} requirementEnum;

typedef enum {
	kXAxis = 0,
	kYAxis = 1,
	kZAxis = 2,
	kDisabledAxis = 3,
	kYXZRotation = 33,
	kXYZRotation = 36,
	kDisabledRotation = 63
} axisConfigurationEnum;

typedef enum {
	kNoVendor = 0,
	kATI,
	kNVIDIA
} gpuVendorEnum;

typedef enum {
	kRenderPassSetMaterial = 1,
	kRenderPassUseTexture = 2,
	kRenderPassUpdateVFC = 4,
	kRenderPassUpdateVFCShadow = 8,
	kRenderPassDrawAdditions = 16,

	kMainRenderPass = 23,
	kAdditionalRenderPass = 0

} renderPassEnum;

typedef enum {
	kNoFiltering = 0,
	kPCFNvidia,
	kPCF4Random,
	kPCF16
} shadowFilteringEnum;

typedef enum {
	kNoShadow = 0,
	kShipOnly,
	kEverythingSmall,
	kEverythingMedium,
	kEverythingLarge
} shadowModeEnum;

typedef struct _Settings {
	BOOL disableTextureCompression;
	BOOL enablePostprocessing;
	BOOL disableVFC;
	BOOL disableTex;
	BOOL disableVBLSync;
	BOOL doWireframe;
	BOOL doBenchmark;

	BOOL displayFPS;
	BOOL displayOctree;
	BOOL displayNormals;
	shadowModeEnum shadowMode;
	shadowFilteringEnum shadowFiltering;
} Settings;

typedef struct _Info {
	gpuVendorEnum gpuVendor;
	float width;
	float height;
	float fps;

	uint64_t frame;
	renderPassEnum	renderpass;
	uint32_t renderedFaces;
	uint32_t visitedNodes;
	uint32_t drawCalls;

	matrix44f_c lightProjectionMatrix;
} Info;

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE

	#define IMG_EXTENSIONS [NSArray arrayWithObjects:@"png.pvrtc", @"jpg.pvrtc", @"pvrtc", @"png", @"jpg", nil]
	extern UIAccelerationValue		accelerometerGravity[3];
	#define SCENE_CONFIG_IPHONE 1
	#define USE_BMFONT 1
	#define SOUND_TYPE SystemSoundID

#elif defined(__APPLE__) || defined(WIN32)
	#define IMG_EXTENSIONS [NSArray arrayWithObjects:@"png", @"jpg", nil]
	//#define USE_BMFONT 1
	#define SOUND_TYPE NSSound*

#else
	#define IMG_EXTENSIONS [NSArray arrayWithObjects:@"png", @"jpg", nil]
	#define SOUND_TYPE int
#endif

#import "Utilities.h"
#import "StateUtilities.h"
#import "SceneNode.h"
#import "Camera.h"
#import "Mesh.h"


#import "Scene.h"

#define AXIS_CONFIGURATION(x,y,z) ((axisConfigurationEnum)(x | y << 2 | z << 4))


extern Info globalInfo;
extern Settings globalSettings;
extern NSMutableArray *pressedKeys;
extern NSMutableArray *activeTouches;
extern BOOL wasShaking;
extern Scene *scene;



const uint8_t debugRenderShadowmap = 0;
const uint8_t printDetailedOctreeInfo = 0;
const uint8_t drawObjectCenters = 0;
const uint8_t disableFBO = 0;
#endif

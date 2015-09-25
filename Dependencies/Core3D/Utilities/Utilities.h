#import "Core3D.h"

GLuint LoadShaders(NSString *shadername, NSString *preprocessorDefines);
GLuint LoadTexture(NSString *imagePath, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy);
GLuint LoadTextureNamed(NSString *name, GLint minFilter, GLint magFilter, GLint mipmap, GLfloat anisontropy);
void DrawFullscreenQuad(short screenWidth, short screenHeight, short textureWidth, short textureHeight);
char AABoxInFrustum(const float frustum[6][4], float x, float y, float z, float ex, float ey, float ez);
void RenderAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
void RenderTexture(GLuint size);
void NanosecondsInit();
uint64_t GetNanoseconds();
BOOL PreCheckOpenGL();
SOUND_TYPE LoadSound(NSString *name);
void PlaySound(SOUND_TYPE soundID);
void UnloadSound(SOUND_TYPE soundID);

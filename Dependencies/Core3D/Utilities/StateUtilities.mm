#import "Core3D.h"

#ifdef WIN32
// TODO: this really shouldn't be here
@implementation NSData (Bzip2)
- (NSData *) bunzip2{	return nil; }
@end
#endif

void myColor(const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha)
#ifdef GL_ES_VERSION_2_0
{}
#else
{
	static GLfloat storedRed = 1.0f;
	static GLfloat storedGreen = 1.0f;
	static GLfloat storedBlue = 1.0f;
	static GLfloat storedAlpha = 1.0f;

	if ((storedRed != red) || (storedGreen != green) || (storedBlue != blue) || (storedAlpha != alpha))
	{
		glColor4f(red, green, blue, alpha);

		storedRed = red;
		storedGreen = green;
		storedBlue = blue;
		storedAlpha = alpha;
	}
}
#endif

void myMaterialSpecular(const GLfloat params[4])
#ifdef GL_ES_VERSION_2_0
{}
#else
{
	static GLfloat storedParams[4];
	uint8_t i;

	for (i = 0; i < 4; i++)
	{
		if (storedParams[i] != params[i])
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, params);
			for (i = 0; i < 4; i++)
				storedParams[i] = params[i];

			return;
		}
	}
}
#endif

void myMaterialShininess(const GLfloat shininess)
#ifdef GL_ES_VERSION_2_0
{}
#else
{
	static GLfloat storedParam = FLT_MIN;

	if (storedParam != shininess)
	{
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
		storedParam = shininess;
	}
}
#endif

void myPointSize(const GLfloat size)
#ifdef GL_ES_VERSION_2_0
{}
#else
{
	static GLfloat storedParam = FLT_MIN;

	if (storedParam != size)
	{
		glPointSize(size);
		storedParam = size;
	}
}
#endif

void myBlendFunc(const GLenum s, const GLenum d)
{
	static GLenum storedS = 0xffffffff;
	static GLenum storedD = 0xffffffff;

	if ((storedS != s) || (storedD != d))
	{
		glBlendFunc(s, d);
		storedS = s;
		storedD = d;
	}
}

void myClientStateVTN(const requirementEnum vertexneed, const requirementEnum textureneed, const requirementEnum normalneed)
#ifdef GL_ES_VERSION_2_0
{}
#else
{
	static requirementEnum stored[3] = {kNeedDisabled, kNeedDisabled, kNeedDisabled};

	if (stored[0] != vertexneed)
	{
		stored[0] = vertexneed;

		if (vertexneed == kNeedEnabled)	{	glEnableClientState(GL_VERTEX_ARRAY); }
		else							{	glDisableClientState(GL_VERTEX_ARRAY); }
	}

	if (stored[1] != textureneed)
	{
		stored[1] = textureneed;

		if (textureneed == kNeedEnabled)	{	glEnableClientState(GL_TEXTURE_COORD_ARRAY); }
		else								{	glDisableClientState(GL_TEXTURE_COORD_ARRAY); }
	}

	if (stored[2] != normalneed)
	{
		stored[2] = normalneed;

		if (normalneed == kNeedEnabled)	{	glEnableClientState(GL_NORMAL_ARRAY); }
		else							{	glDisableClientState(GL_NORMAL_ARRAY); }
	}
}
#endif

//void myNormalPointer(const GLenum type, const GLsizei stride, const GLvoid *pointer)
//{
//	static GLenum storedType;
//	static GLsizei storedStride;
//	static const GLvoid *storedPointer;
//
//	if ((storedType != type) || (storedStride != stride) || (storedPointer != pointer))
//	{
//		glNormalPointer(type, stride, pointer);
//		storedType = type;
//		storedStride = stride;
//		storedPointer = pointer;
//	}
////	else
////		glNormalPointer(storedType, storedStride, storedPointer); // TODO: why do we have to do redundant changes here??
//
//#ifdef DEBUG_ALIGNMENT
//	if (type == GL_SHORT)
//	{
//		if ((int)pointer % 2 != 0)
//			fatal("sucky alignment");
//	}
//	else if ((type == GL_FLOAT) || (type == GL_INT))
//	{
//		if ((int)pointer % 4 != 0)
//			fatal("sucky alignment");
//	}
//	else if (type == GL_DOUBLE)
//	{
//		if ((int)pointer % 8 != 0)
//			fatal("sucky alignment");
//	}
//	else
//		fatal("invalid type");
//#endif
//}
//
//void myVertexPointer(const GLint size, const GLenum type, const GLsizei stride, const GLvoid *pointer)
//{
//	static GLint storedSize;
//	static GLenum storedType;
//	static GLsizei storedStride;
//	static const GLvoid *storedPointer;
//
//	if ((storedSize != size) || (storedType != type) || (storedStride != stride) || (storedPointer != pointer))
//	{
//		glVertexPointer(size, type, stride, pointer);
//		storedSize = size;
//		storedType = type;
//		storedStride = stride;
//		storedPointer = pointer;
//
////		printf(" just set");
////		GLuint bla;
////		GLvoid *ente;
////		glGetIntegerv(GL_VERTEX_ARRAY_SIZE, (GLint *) &bla);
////		printf(" GL_VERTEX_ARRAY_SIZE %i ", bla);
////		glGetIntegerv(GL_VERTEX_ARRAY_TYPE, (GLint *) &bla);
////		printf(" GL_VERTEX_ARRAY_TYPE %i ", bla);
////		glGetIntegerv(GL_VERTEX_ARRAY_STRIDE, (GLint *) &bla);
////		printf(" GL_VERTEX_ARRAY_STRIDE %i", bla);
////		glGetPointerv(GL_VERTEX_ARRAY_POINTER, &ente);
////		printf(" GL_VERTEX_ARRAY_POINTER %p %p \n\n", ente, pointer);
//	}
//	else
//	{
////		printf(" should be same!:" );
////
////		GLint bla;
////		GLvoid *ente;
////		glGetIntegerv(GL_VERTEX_ARRAY_SIZE, (GLint *) &bla);
//////		if (bla != size)
////			printf(" GL_VERTEX_ARRAY_SIZE %i ", bla);
////		glGetIntegerv(GL_VERTEX_ARRAY_TYPE, (GLint *) &bla);
//////		if (bla != type)
////			printf(" GL_VERTEX_ARRAY_TYPE %i ", bla);
////		glGetIntegerv(GL_VERTEX_ARRAY_STRIDE, (GLint *) &bla);
//////		if (bla != stride)
////			printf(" GL_VERTEX_ARRAY_STRIDE %i ", bla);
////		glGetPointerv(GL_VERTEX_ARRAY_POINTER, &ente);
//////		if (ente != pointer)
////
////			printf(" GL_VERTEX_ARRAY_POINTER %p %p \n\n", ente, pointer);
//
//		//glVertexPointer(storedSize, storedType, storedStride, storedPointer);
//	}
//
//#ifdef DEBUG_ALIGNMENT
//	if (type == GL_SHORT)
//	{
//		if ((int)pointer % 2 != 0)
//			fatal("sucky alignment");
//	}
//	else if ((type == GL_FLOAT) || (type == GL_INT))
//	{
//		if ((int)pointer % 4 != 0)
//			fatal("sucky alignment")
//	}
//	else if (type == GL_DOUBLE)
//	{
//		if ((int)pointer % 8 != 0)
//			fatal("sucky alignment")
//	}
//	else
//		fatal("invalid type");
//#endif
//}
//
//void myTexCoordPointer(const GLint size, const GLenum type, const GLsizei stride, const GLvoid *pointer)
//{
//	static GLint storedSize;
//	static GLenum storedType;
//	static GLsizei storedStride;
//	static GLvoid *storedPointer;
//
//	if ((storedSize != size) || (storedType != type) || (storedStride != stride) || (storedPointer != pointer))
//	{
//		glTexCoordPointer(size, type, stride, pointer);
//		storedSize = size;
//		storedType = type;
//		storedStride = stride;
//		storedPointer = (GLvoid *) pointer;
//	}
////	else
//	//	glTexCoordPointer(size, type, stride, pointer);
//
//#ifdef DEBUG_ALIGNMENT
//	if (type == GL_SHORT)
//	{
//		if ((int)pointer % 2 != 0)
//			fatal("sucky alignment");
//	}
//	else if ((type == GL_FLOAT) || (type == GL_INT))
//	{
//		if ((int)pointer % 4 != 0)
//			fatal("sucky alignment")
//	}
//	else if (type == GL_DOUBLE)
//	{
//		if ((int)pointer % 8 != 0)
//			fatal("sucky alignment")
//	}
//	else
//		fatal("invalid type");
//#endif
//}

//
//  Mesh.m
//  Core3D
//
//  Created by Julian Mayer on 16.11.07.
//  Copyright 2007 - 2010 A. Julian Mayer.
//
/*
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#import "Core3D.h"

#if defined(TARGET_OS_IPHONE) || defined(WIN32) || defined(linux) // header missing in iphone sdk
typedef  struct {char *next_in;	unsigned int avail_in; unsigned int total_in_lo32; unsigned int total_in_hi32; char *next_out; unsigned int avail_out; unsigned int total_out_lo32;	unsigned int total_out_hi32; void *state; void *(*bzalloc)(void *,int,int);	void (*bzfree)(void *,void *); void *opaque; } bz_stream;
#define BZ_OK                0
#define BZ_STREAM_END        4
#define BZ_API(func) func
#define BZ_EXTERN extern
typedef void BZFILE;
BZ_EXTERN int BZ_API(BZ2_bzDecompressInit) (bz_stream *strm, int verbosity,	int _small);
BZ_EXTERN int BZ_API(BZ2_bzDecompress) (bz_stream* strm);
BZ_EXTERN int BZ_API(BZ2_bzCompressEnd) ( bz_stream* strm);
//BZ_EXTERN BZFILE* BZ_API(BZ2_bzReadOpen) (int* bzerror, FILE* f, int verbosity, int small, void* unused, int nUnused);
BZ_EXTERN void BZ_API(BZ2_bzReadClose) (int* bzerror, BZFILE* b);
BZ_EXTERN int BZ_API(BZ2_bzRead) (int* bzerror, BZFILE* b, void* buf, int len );
#else
#import "bzlib.h"
#endif


#define RECURSION_THRESHOLD 1000

GLfloat frustum[6][4];
uint16_t _visibleNodeStackTop;

static void vfcTestOctreeNode(struct octree_struct *octree, uint16_t *visibleNodeStack, uint32_t nodeNum);

@implementation Mesh

@synthesize octree, color, specularColor, visibleNodeStack, visibleNodeStackTop, shininess, name, doubleSided;

+ (struct octree_struct *)_loadOctreeFromFile:(NSURL *)file
{
	octree_struct *_octree;
	FILE *f;
	NSString *p = [file path];

#ifdef WIN32
	if ([p hasPrefix:@"/"]) p = [p substringFromIndex:1];
#endif
	f = fopen([p UTF8String], "rb");

	assert(f);

	if (![[[file path] pathExtension] isEqualToString:@"bz2"])
	{
		unsigned long fileSize;
		size_t result;


		fseek(f , 0 , SEEK_END);
		fileSize = ftell(f);
		rewind(f);


		_octree = (octree_struct *) malloc(fileSize);
		assert(_octree);

		result = fread (_octree, 1, fileSize, f);
		assert(result == fileSize);
	}
	else
	{
#if defined(WIN32) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) || defined(linux)
		fatal("Error: compressed mesh loading disabled under Win32/Linux\n");
#else
		BZFILE* b;
		int     numRead, bzerror;
		octree_struct test;


		b = BZ2_bzReadOpen ( &bzerror, f, 0, 0, NULL, 0 );
		assert(bzerror == BZ_OK);

		numRead = BZ2_bzRead ( &bzerror, b, (char *)&test, sizeof(octree_struct));
		assert ((numRead == sizeof(octree_struct)) && (bzerror == BZ_OK));

		uint32_t size = (sizeof(struct octree_struct) + (test.nodeCount - 1) * sizeof(struct octree_node)) +
							test.vertexCount * ((test.magicWord == 0x6D616C62) ? 6 : 8) * sizeof(float) +
							test.rootnode.faceCount * 3 * (test.vertexCount > 0xFFFF ? sizeof(uint32_t) : sizeof(uint16_t)), read = sizeof(octree_struct);
		_octree = (octree_struct *) malloc(size);
		assert(_octree);

		memcpy(_octree, &test, sizeof(octree_struct)); // TODO: realloc instead

		while (bzerror == BZ_OK)
		{
			numRead = BZ2_bzRead(&bzerror, b, ((char *)_octree) + read, size - read);

			if (bzerror == BZ_OK)
				read += numRead;
		}

		assert((bzerror == BZ_STREAM_END) && ((read + numRead) == size));

		BZ2_bzReadClose(&bzerror, b);
#endif
	}

	fclose (f);

	if ((_octree->magicWord != 0x6D616C62) && (_octree->magicWord != 0xDEADBEEF))
		fatal("Error: the file named %s doesn't seem to be a valid octree", [[file absoluteString] UTF8String]);

	return _octree;
}

- (id)init
{
	[self doesNotRecognizeSelector:_cmd];
	return nil;
}

- (id)initWithOctreeNamed:(NSString *)_name
{
	NSString *octreeURL = [[NSBundle mainBundle] pathForResource:_name ofType:@"octree"];
	NSString *bz2URL = [[NSBundle mainBundle] pathForResource:_name ofType:@"octree.bz2"];

	if (!octreeURL && !bz2URL)
	{	fatal("Error: there is no octree named: %s", [_name UTF8String]); }

	return [self initWithOctree:(octreeURL ? [NSURL fileURLWithPath:octreeURL] : [NSURL fileURLWithPath:bz2URL]) andName:_name];
}

- (id)initWithOctree:(NSURL *)file andName:(NSString *)_name
{
	if ((self = [super init]))
	{
		shininess = 30.0;
		doubleSided = FALSE;
		name = [[NSString alloc] initWithString:_name];

		[self setColor:vector4f(0.8, 0.8, 0.8, 1.0)];
		[self setSpecularColor:vector4f(0.3, 0.3, 0.3, 1.0)];

		octree = [Mesh _loadOctreeFromFile:file];

#ifdef TARGET_OS_IPHONE
		if (octree->vertexCount > 0xFFFF)
			fatal("Error: only 0xFFFF vertices per object supported on the iPhone");
#endif

		glGenBuffers(1, &vertexVBOName);
		glGenBuffers(1, &indexVBOName);

		glBindBuffer(GL_ARRAY_BUFFER, vertexVBOName);
		glBufferData(GL_ARRAY_BUFFER, octree->vertexCount * ((octree->magicWord == 0x6D616C62) ? 6 : 8) * sizeof(float), OFFSET_VERTICES, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBOName);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, octree->rootnode.faceCount * 3 * (octree->vertexCount > 0xFFFF ? sizeof(uint32_t) : sizeof(uint16_t)), OFFSET_FACES, GL_STATIC_DRAW);


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		if (octree->magicWord != 0x6D616C62)
			texName = LoadTextureNamed(name, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 4.0);

		visibleNodeStack = (uint16_t *) calloc(1, octree->nodeCount * sizeof(uint16_t));

		[self cleanup];
	}

	return self;
}

- (vector3f)center
{
	struct octree_node *n1 = (struct octree_node *) NODE_NUM(0);
	vector3f extent = vector3f(n1->aabbExtentX, n1->aabbExtentY, n1->aabbExtentZ);
	vector3f origin = vector3f(n1->aabbOriginX, n1->aabbOriginY, n1->aabbOriginZ);

	return vector3f(origin + extent / 2.0);
}

- (float)radius
{
	struct octree_node *n1 = (struct octree_node *) NODE_NUM(0);
	vector3f extent = vector3f(n1->aabbExtentX, n1->aabbExtentY, n1->aabbExtentZ);
	return extent.length() / 2.0;
}

- (void)cleanup
{
#ifndef DEBUG
	octree = (octree_struct *) realloc(octree, (sizeof(struct octree_struct) + (octree->nodeCount - 1) * sizeof(struct octree_node)));
#endif
}

- (id)copyWithZone:(NSZone *)zone
{
	Mesh *octreeCopy = NSCopyObject(self, 0, zone);

	octreeCopy->name = [[NSString alloc] initWithString:name];
	octreeCopy->visibleNodeStack = (uint16_t *) calloc(1, octree->nodeCount * sizeof(uint16_t));

	return octreeCopy;
}

- (NSString *)description
{
	NSMutableString *desc = [NSMutableString stringWithString:@"Mesh: "];

	[desc appendFormat:@"%@\n Nodes/Vertices/Faces: %i / %i / %i\n", name, octree->nodeCount, octree->vertexCount, octree->rootnode.faceCount];
	struct octree_node *n = (struct octree_node *) NODE_NUM(0);
	[desc appendFormat:@" RootNode: firstFace: %i faceCount:%i\n  origin:%f %f %f\n  extent: %f %f %f\n  children: %i %i %i %i %i %i %i %i\n", n->firstFace, n->faceCount, n->aabbOriginX, n->aabbOriginY, n->aabbOriginZ, n->aabbExtentX, n->aabbExtentY, n->aabbExtentZ, n->childIndex1, n->childIndex2, n->childIndex3, n->childIndex4, n->childIndex5, n->childIndex6, n->childIndex7, n->childIndex8];


	if (printDetailedOctreeInfo)
	{
		[desc appendFormat:@"NodeOffset:%p\n", OFFSET_NODES];
		[desc appendFormat:@"VertexOffset:%p\n", OFFSET_VERTICES];
		[desc appendFormat:@"FaceOffset:%p\n", OFFSET_FACES];

		uint32_t i;
		[desc appendString:@"Nodes:\n"];
		for (i = 0; i < octree->nodeCount; i++)
		{
			struct octree_node *n = (struct octree_node *) NODE_NUM(i);
			[desc appendFormat:@"%i: firstFace: %i faceCount:%i origin:%f %f %f extent: %f %f %f children: %i %i %i %i %i %i %i %i\n", i, n->firstFace, n->faceCount, n->aabbOriginX, n->aabbOriginY, n->aabbOriginZ, n->aabbExtentX, n->aabbExtentY, n->aabbExtentZ, n->childIndex1, n->childIndex2, n->childIndex3, n->childIndex4, n->childIndex5, n->childIndex6, n->childIndex7, n->childIndex8];
		}
#ifdef DEBUG
		[desc appendString:@"\nVertices:\n"];
		for (i = 0; i < octree->vertexCount; i++)
		{
			float *v = (float *) VERTEX_NUM(i);
			if (octree->magicWord == 0x6D616C62)
				[desc appendFormat:@"%i: x: %f y: %f z: %f nx: %f ny: %f nz: %f\n", i, *v, *(v+1), *(v+2), *(v+3), *(v+4), *(v+5)];
			else
				[desc appendFormat:@"%i: x: %f y: %f z: %f nx: %f ny: %f nz: %f  tx: %f ty: %f tz: %f\n", i, *v, *(v+1), *(v+2), *(v+3), *(v+4), *(v+5), *(v+6), *(v+7), *(v+8)];
		}
		[desc appendString:@"\nFaces:\n"];
		for (i = 0; i < octree->rootnode.faceCount; i++)
		{
			uint16_t *f = (uint16_t *) FACE_NUM(i);

			[desc appendFormat:@"%i: v1: %u v2: %u v3: %u\n", i, *f, *(f+1), *(f+2)];
		}
#endif
	}

	//return [NSString stringWithString:[[super description] stringByAppendingString:desc]];
	return [NSString stringWithString:desc];
}


#ifdef DEBUG
- (void)renderOctree
{
#ifndef WIN32
#ifndef TARGET_OS_IPHONE
	GLint prog;
	myColor(0.4, 0, 0, 1.0);
	glDisable(GL_LIGHTING);
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
	glUseProgram(0);

	glBegin(GL_LINES);
	uint32_t j;
	for (j = 0; j < octree->nodeCount; j++)
	{
		struct octree_node *n = (struct octree_node *) NODE_NUM(j);

		RenderAABB(n->aabbOriginX, n->aabbOriginY, n->aabbOriginZ, n->aabbOriginX + n->aabbExtentX, n->aabbOriginY + n->aabbExtentY, n->aabbOriginZ + n->aabbExtentZ);
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glUseProgram(prog);
#endif
#endif
}

- (void)renderNormals
{
#ifndef WIN32
#ifndef TARGET_OS_IPHONE
	GLint prog;
	myColor(0.4, 0.0, 0.4, 1.0);
	myClientStateVTN(kNeedDisabled, kNeedDisabled, kNeedDisabled);
	glDisable(GL_LIGHTING);
	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
	glUseProgram(0);

	glBegin(GL_LINES);
	uint16_t i;
	for (i = 0; i < octree->rootnode.faceCount; i++)
	{
		const float normalscale = 0.2;
		uint16_t *f = (uint16_t *) FACE_NUM(i); // TODO: displaying normals is broken for octrees with len(vertices) > 0xFFFF
		float *v1 = (float *) VERTEX_NUM(*f);
		float *v2 = (float *) VERTEX_NUM(*(f+1));
		float *v3 = (float *) VERTEX_NUM(*(f+2));

		glVertex3f(*(v1), *(v1+1), *(v1+2));
		glVertex3f(*(v1)+(*(v1+3)/normalscale), *(v1+1)+(*(v1+4)/normalscale), *(v1+2)+(*(v1+5)/normalscale));

		glVertex3f(*(v2), *(v2+1), *(v2+2));
		glVertex3f(*(v2)+(*(v2+3)/normalscale), *(v2+1)+(*(v2+4)/normalscale), *(v2+2)+(*(v2+5)/normalscale));

		glVertex3f(*(v3), *(v3+1), *(v3+2));
		glVertex3f(*(v3)+(*(v3+3)/normalscale), *(v3+1)+(*(v3+4)/normalscale), *(v3+2)+(*(v3+5)/normalscale));
	}
	glEnd();

	glEnable(GL_LIGHTING);
	glUseProgram(prog);

	globalInfo.drawCalls++;
#endif
#endif
}
#endif

- (void)renderNode
{

#ifndef TARGET_OS_IPHONE
	if (globalSettings.doWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif


	if (globalInfo.renderpass & kRenderPassSetMaterial)
	{
		myColor(color[0], color[1], color[2], color[3]);
		myMaterialSpecular(specularColor.data());
		myMaterialShininess(shininess);
	}


	if (doubleSided)
		glDisable(GL_CULL_FACE);



	if ((octree->magicWord != 0x6D616C62) && (globalInfo.renderpass & kRenderPassUseTexture) && (!globalSettings.disableTex))
	{
		myClientStateVTN(kNeedEnabled, kNeedEnabled, kNeedEnabled);



		glBindBuffer(GL_ARRAY_BUFFER, vertexVBOName);	// must be bevore glTexCoordPointer


#ifndef GL_ES_VERSION_2_0
		glEnable(GL_TEXTURE_2D);
#endif
		if (texName)
			glBindTexture(GL_TEXTURE_2D, texName);
#ifdef GL_ES_VERSION_2_0

		glEnableVertexAttribArray(TEXCOORD_ARRAY);

		glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const GLfloat *) (sizeof(float) * 6));

#else
		glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 8, (const GLfloat *) (sizeof(float) * 6));
#endif
	}
	else
	{
		myClientStateVTN(kNeedEnabled, kNeedDisabled, kNeedEnabled);

		glBindBuffer(GL_ARRAY_BUFFER, vertexVBOName);
	}


#ifdef GL_ES_VERSION_2_0
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(NORMAL_ARRAY, 3, GL_FLOAT, GL_FALSE, (octree->magicWord == 0x6D616C62) ? sizeof(float) * 6 : sizeof(float) * 8, (const GLfloat *) (sizeof(float) * 3));
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, (octree->magicWord == 0x6D616C62) ? sizeof(float) * 6 : sizeof(float) * 8, (const GLfloat *) 0);
#else
	glNormalPointer(GL_FLOAT, (octree->magicWord == 0x6D616C62) ? sizeof(float) * 6 : sizeof(float) * 8, (const GLfloat *) (sizeof(float) * 3));
	glVertexPointer(3, GL_FLOAT, (octree->magicWord == 0x6D616C62) ? sizeof(float) * 6 : sizeof(float) * 8, (const GLfloat *) 0);
#endif



	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBOName);

	if (globalSettings.disableVFC) // no view frustum culling, render everthing with a single call
	{
#ifndef TARGET_OS_IPHONE
		if 	(octree->vertexCount > 0xFFFF)
		{	glDrawElements(GL_TRIANGLES, octree->rootnode.faceCount * 3, GL_UNSIGNED_INT, (const GLuint *) 0); }
		else
#endif
		{	glDrawElements(GL_TRIANGLES, octree->rootnode.faceCount * 3, GL_UNSIGNED_SHORT, (const GLushort *) 0); }

		globalInfo.renderedFaces += octree->rootnode.faceCount;
		globalInfo.visitedNodes ++;
		globalInfo.drawCalls++;
	}
	else
	{
		if ((globalInfo.renderpass & kRenderPassUpdateVFC) || (globalInfo.renderpass & kRenderPassUpdateVFCShadow))
		{
			_visibleNodeStackTop = 0;

			matrix44f_c modelview = [[scene camera] modelViewMatrix];

			extract_frustum_planes(modelview, globalInfo.renderpass == (globalInfo.renderpass & kRenderPassUpdateVFCShadow) ? globalInfo.lightProjectionMatrix :  [[scene camera] projectionMatrix], frustum, cml::z_clip_neg_one, false);

			vfcTestOctreeNode(octree, visibleNodeStack, 0);

			visibleNodeStackTop = _visibleNodeStackTop;
		}

		uint16_t i;
		for (i = 0; i < visibleNodeStackTop;)
		{
			struct octree_node *n = (struct octree_node *) NODE_NUM(visibleNodeStack[i]);
			uint32_t fc = n->faceCount;
			uint32_t ff = n->firstFace;
			uint16_t v = i+1;
			while (v < visibleNodeStackTop)
			{
				struct octree_node *nn = (struct octree_node *) NODE_NUM(visibleNodeStack[v]);

				if (nn->firstFace != n->firstFace + n->faceCount)	// TODO: allow for some draw call reduction at the expense of drawing invisible stuff
					break;


				fc += nn->faceCount;
				n = nn;
				v++;
			}

			i = v;
#ifndef TARGET_OS_IPHONE
			if 	(octree->vertexCount > 0xFFFF)
			{	glDrawElements(GL_TRIANGLES, fc * 3, GL_UNSIGNED_INT, (const GLuint *) 0 + (ff * 3)); }
			else
#endif
			{	glDrawElements(GL_TRIANGLES, fc * 3, GL_UNSIGNED_SHORT, (const GLushort *) 0 + (ff * 3)); }

			globalInfo.drawCalls++;
			globalInfo.renderedFaces += n->faceCount;
		}
	}


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



	if ((octree->magicWord != 0x6D616C62) && (globalInfo.renderpass & kRenderPassUseTexture))
	{
#ifndef GL_ES_VERSION_2_0
		glDisable(GL_TEXTURE_2D);
#endif
	}


	if (globalInfo.renderpass & kRenderPassDrawAdditions)
	{

		#ifndef TARGET_OS_IPHONE
		if (drawObjectCenters)
			[self renderCenter];

		#ifdef DEBUG
		if (globalSettings.displayOctree)
			[self renderOctree];

		if (globalSettings.displayNormals)
			[self renderNormals];
		#endif
		#endif
	}

	if (doubleSided)
		glEnable(GL_CULL_FACE);

#ifndef TARGET_OS_IPHONE
	if (globalSettings.doWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif


}


- (void)dealloc
{
	free(octree);
	free(visibleNodeStack);
	[name release];

	glDeleteBuffers(1, &vertexVBOName);		// TODO: this surely makes everything go nuts when the object is copied
	glDeleteBuffers(1, &indexVBOName);
	glDeleteTextures(1, &texName);

	[super dealloc];
}
@end

static void vfcTestOctreeNode(struct octree_struct *octree, uint16_t *visibleNodeStack, uint32_t nodeNum) // TODO: optimization: VFC coherence (http://www.cescg.org/CESCG-2002/DSykoraJJelinek/index.html)
{
	struct octree_node *n = (struct octree_node *) NODE_NUM(nodeNum);
	char result;

	globalInfo.visitedNodes++;

	if (n->faceCount == 0)
		return;

	result = AABoxInFrustum((const float (*)[4])frustum, n->aabbOriginX, n->aabbOriginY, n->aabbOriginZ, n->aabbExtentX, n->aabbExtentY, n->aabbExtentZ);
	if (result == kIntersecting)
	{
		if ((n->childIndex1 == 0xFFFF) || (n->faceCount < RECURSION_THRESHOLD))
			visibleNodeStack[_visibleNodeStackTop++] = nodeNum;
		else
		{
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex1);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex2);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex3);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex4);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex5);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex6);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex7);
			vfcTestOctreeNode(octree, visibleNodeStack, n->childIndex8);
		}
	}
	else if (result == kInside)
		visibleNodeStack[_visibleNodeStackTop++] = nodeNum;
}

//
//  Mesh.h
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

#define VERTEX_ARRAY	0
#define TEXCOORD_ARRAY	1
#define NORMAL_ARRAY	2

struct octree_node
{
	uint32_t firstFace;
	uint32_t faceCount;
	float aabbOriginX, aabbOriginY, aabbOriginZ;
	float aabbExtentX, aabbExtentY, aabbExtentZ;
	uint16_t childIndex1, childIndex2, childIndex3, childIndex4, childIndex5, childIndex6, childIndex7, childIndex8;
};

struct octree_struct // TODO: optimization: add optimized prefetch indices for glDrawRangeElements, convert to aabbCenter
{
	uint32_t magicWord;
	uint32_t nodeCount;
	uint32_t vertexCount;
	struct octree_node rootnode;
};

#define _OFFSET_NODES(oct)		((char *)oct + sizeof(struct octree_struct) - sizeof(struct octree_node))
#define _OFFSET_VERTICES(oct)	(_OFFSET_NODES(oct) + oct->nodeCount * sizeof(struct octree_node))
#define _OFFSET_FACES(oct)		(_OFFSET_VERTICES(oct) + oct->vertexCount * ((oct->magicWord == 0x6D616C62) ? 6 : 8) * sizeof(float))
#define _NODE_NUM(oct, x)		(_OFFSET_NODES(oct) + (x) * sizeof(struct octree_node))
#define _VERTEX_NUM(oct, x)		(_OFFSET_VERTICES(oct) + (x) * ((oct->magicWord == 0x6D616C62) ? 6 : 8) * sizeof(float))
#define _FACE_NUM(oct, x)		(_OFFSET_FACES(oct) + (x) * 3 * sizeof(uint16_t))

#define OFFSET_NODES	(_OFFSET_NODES(octree))
#define OFFSET_VERTICES	(_OFFSET_VERTICES(octree))
#define OFFSET_FACES	(_OFFSET_FACES(octree))
#define NODE_NUM(x)		(_NODE_NUM(octree, x))
#define VERTEX_NUM(x)	(_VERTEX_NUM(octree, x))
#define FACE_NUM(x)		(_FACE_NUM(octree, x))


@interface Mesh : SceneNode
{
	struct octree_struct	*octree;

	vector4f				color, specularColor;
	float					shininess;
	NSString				*name;
	BOOL					doubleSided;


	uint16_t				*visibleNodeStack;
	uint16_t				visibleNodeStackTop;

	@public
	GLuint					texName;

@public
	GLuint					vertexVBOName, indexVBOName;
}

@property (readonly, nonatomic) NSString *name;
@property (readonly, nonatomic) struct octree_struct *octree;
@property (assign, nonatomic) vector4f color;
@property (assign, nonatomic) vector4f specularColor;
@property (assign, nonatomic) BOOL doubleSided;
@property (assign, nonatomic) float shininess;
@property (readonly, nonatomic) uint16_t *visibleNodeStack;
@property (readonly, nonatomic) uint16_t visibleNodeStackTop;

+ (struct octree_struct *)_loadOctreeFromFile:(NSURL *)file;
- (id)initWithOctreeNamed:(NSString *)_name;
- (id)initWithOctree:(NSURL *)file andName:(NSString *)_name;
- (void)cleanup;

- (vector3f)center;
- (float)radius;
@end
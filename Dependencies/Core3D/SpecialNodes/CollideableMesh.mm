//
//  CollideableMesh.m
//  Core3D
//
//  Created by Julian Mayer on 14.05.08.
//  Copyright 2008 - 2010 A. Julian Mayer.
//
/*
 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#import "Core3D.h"
#import "CollideableMesh.h"



int tri_tri_overlap_test_3d(float p1[3], float q1[3], float r1[3], float p2[3], float q2[3], float r2[3]);
int intersect_triangle(float orig[3], float dir[3], float vert0[3], float vert1[3], float vert2[3], float *t, float *u, float *v);
bool SegmentIntersectsAABB(const vector3f segmentStart, const vector3f segmentEnd, const vector3f aabbCenter, const vector3f aabbExtent);
//BOOL AABBIntersectsAABB(const vector3f aabbCenter1, const vector3f aabbExtent1, const vector3f aabbCenter2, const vector3f aabbExtent2);
bool OBBIntersectsOBB (const vector3f obb1Center, const vector3f obb1Extent, const matrix33f_c obb1OrthogonalBase, const vector3f obb2Center, const vector3f obb2Extent, const matrix33f_c obb2OrthogonalBase);



BOOL intersectOctreeNodeWithLine(struct octree_struct *octree, int nodeNum, const vector3f startPoint, const vector3f endPoint, float intersectionPoint[3]);
BOOL intersectOctreeNodeWithOctreeNode(struct octree_struct *octree, int nodeNum, const vector3f position, const matrix33f_c orthogonalBase, struct octree_struct *otherOctree, int otherNodeNum, const vector3f otherPosition,  const matrix33f_c otherOrthogonalBase, TriangleIntersectionInfo *intersectionInfo);

@implementation CollideableMesh

#ifndef DISABLE_COLLISION

#error THIS CODE DOES NOT COMPILE
- (id)initWithOctree:(NSURL *)file andName:(NSString *)_name
{
	if ((self = [super initWithOctree:data andName:_name]))
	{
		NSData *octreeData = [NSData dataWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[_name stringByAppendingString:@"_collision"] ofType:@"octree"]];

		if (!octreeData)
			octreeData = [[NSData dataWithContentsOfFile:[[NSBundle mainBundle] pathForResource:[_name stringByAppendingString:@"_collision"] ofType:@"octree.bz2"]] bunzip2];

		if (!octreeData)
		{
			NSLog(@"Warning: there is no collision octree for collidable object: %@", _name);

			octree_collision = octree;
		}
		else
		{
			octree_collision = (octree_struct *) malloc([octreeData length]);
			[octreeData getBytes:octree_collision];

			if (octree_collision->magicWord != 0x6D616C62)
				NSLog(@"Notice: texcoords are superfluous for collision octree: %@", _name);

			[super cleanup];
		}
	}

	return self;
}

- (void)cleanup
{
}

- (void)dealloc
{
	if (octree != octree_collision)
		free(octree_collision);

	[super dealloc];
}

#ifdef RENDER_COLLISION_OCTREES
- (void)renderNode
{
	if (globalInfo.renderpass & kRenderPassSetMaterial)
	{
		myColor(color[0], color[1], color[2], color[3]);
		myMaterialSpecular(specularColor.data());
		myMaterialShininess(shininess);
	}

	myClientStateVTN(kNeedDisabled, kNeedDisabled, kNeedDisabled);

	glBegin(GL_TRIANGLES);
	uint32_t i;
	for (i = 0; i < octree_collision->rootnode.faceCount; i++)
	{
		uint16_t *f = (uint16_t *) _FACE_NUM(octree_collision, i);
		float *v1 = (float *) _VERTEX_NUM(octree_collision, *f);
		float *v2 = (float *) _VERTEX_NUM(octree_collision, *(f+1));
		float *v3 = (float *) _VERTEX_NUM(octree_collision, *(f+2));

		glNormal3f(*(v1+3), *(v1+4), *(v1+5));
		glVertex3f(*v1, *(v1+1), *(v1+2));

		glNormal3f(*(v2+3), *(v2+4), *(v2+5));
		glVertex3f(*v2, *(v2+1), *(v2+2));

		glNormal3f(*(v3+3), *(v3+4), *(v3+5));
		glVertex3f(*v3, *(v3+1), *(v3+2));
	}
	glEnd();

	globalInfo.drawCalls++;
}
#endif

- (vector3f)intersectWithLineStart:(vector3f)startPoint end:(vector3f)endPoint // TODO: this is completely buggy, but i suspect the tri-ray code is faulty
{
	vector3f ip;
	BOOL intersects;

	if (rotation[0] || rotation[1] || rotation[2])
	{
		matrix33f_c rm;

		matrix_rotation_euler(rm, rad(-rotation[0]), rad(-rotation[1]), rad(-rotation[2]), euler_order_xyz);

		intersects = intersectOctreeNodeWithLine(octree_collision, 0, transform_vector(rm, startPoint) + position, transform_vector(rm, endPoint) + position, ip.data());
	}
	else
		intersects = intersectOctreeNodeWithLine(octree_collision, 0, startPoint + position, endPoint + position, ip.data());

	if (intersects)
		return ip;
	else
		return vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
}

//- (BOOL)OBBIntersectsWithLineStart:(vector3f)startPoint end:(vector3f)endPoint
//{
//	matrix33f_c rm;
//	vector3f aabbExtent = vector3f((octree_collision->rootnode.aabbExtentX/2), (octree_collision->rootnode.aabbExtentY/2), (octree_collision->rootnode.aabbExtentZ/2));
//	vector3f aabbOrigin = vector3f(octree_collision->rootnode.aabbOriginX, octree_collision->rootnode.aabbOriginY, octree_collision->rootnode.aabbOriginZ) + aabbExtent;
//
//	matrix_rotation_euler(rm, rad(-rotation[0]), rad(-rotation[1]), rad(-rotation[2]), euler_order_xyz);
//
//	return SegmentIntersectsAABB(transform_vector(rm, startPoint) + position, transform_vector(rm, endPoint) + position, aabbOrigin, aabbExtent);
//}

- (TriangleIntersectionInfo)intersectWithMesh:(CollideableMesh *)otherMesh;
{
	matrix33f_c rm, orm;
	TriangleIntersectionInfo tif;

	matrix_rotation_euler(rm, rad(rotation[0]), rad(rotation[1]), rad(rotation[2]), euler_order_xyz);
	matrix_rotation_euler(orm, rad([otherMesh rotation][0]), rad([otherMesh rotation][1]), rad([otherMesh rotation][2]), euler_order_xyz);

	tif.intersects = intersectOctreeNodeWithOctreeNode(octree_collision, 0, position, rm, otherMesh->octree_collision, 0, [otherMesh position], orm, &tif);

	return tif;
}
#else
- (TriangleIntersectionInfo)intersectWithMesh:(CollideableMesh *)otherMesh
{
	TriangleIntersectionInfo tif;
	tif.intersects = NO;
	return tif;
}
- (vector3f)intersectWithLineStart:(vector3f)startPoint end:(vector3f)endPoint
{
	return vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
}
#endif
@end

BOOL intersectOctreeNodeWithOctreeNode(struct octree_struct *thisOctree, int nodeNum, const vector3f position, const matrix33f_c orthogonalBase, struct octree_struct *otherOctree, int otherNodeNum, const vector3f otherPosition,  const matrix33f_c otherOrthogonalBase, TriangleIntersectionInfo *intersectionInfo) // TODO: this is broken for octrees with len(vertices) > 0xFFFF
{
	struct octree_node *n1 = (struct octree_node *) _NODE_NUM(thisOctree, nodeNum);
	struct octree_node *n2 = (struct octree_node *) _NODE_NUM(otherOctree, otherNodeNum);

	if (n1->faceCount && n2->faceCount )
	{
		vector3f aabbExtent1 = vector3f(n1->aabbExtentX, n1->aabbExtentY, n1->aabbExtentZ) / 2.0;
		vector3f aabbOrigin1 = vector3f(n1->aabbOriginX, n1->aabbOriginY, n1->aabbOriginZ) + aabbExtent1 + position;
		vector3f aabbExtent2 = vector3f(n2->aabbExtentX, n2->aabbExtentY, n2->aabbExtentZ) / 2.0;
		vector3f aabbOrigin2 = vector3f(n2->aabbOriginX, n2->aabbOriginY, n2->aabbOriginZ) + aabbExtent2 + otherPosition;
		BOOL intersects = OBBIntersectsOBB(aabbOrigin1, aabbExtent1, orthogonalBase, aabbOrigin2, aabbExtent2, otherOrthogonalBase);

		if (intersects)
		{
			if ((n1->childIndex1 == 0xFFFF) && (n2->childIndex1 == 0xFFFF)) // we are both leafes
			{	// TODO: we should really first test all triangles against the obb first
				uint32_t i;
				for (i = n1->firstFace; i < n1->firstFace + n1->faceCount; i++)
				{
					uint32_t v;
					for (v = n2->firstFace; v < n2->firstFace + n2->faceCount; v++)
					{
						uint16_t *f1 = (uint16_t *) _FACE_NUM(thisOctree, i);
						float *v11 = (float *) _VERTEX_NUM(thisOctree, *f1);
						float *v12 = (float *) _VERTEX_NUM(thisOctree, *(f1+1));
						float *v13 = (float *) _VERTEX_NUM(thisOctree, *(f1+2));

						uint16_t *f2 = (uint16_t *) _FACE_NUM(otherOctree, v);
						float *v21 = (float *) _VERTEX_NUM(otherOctree, *f2);
						float *v22 = (float *) _VERTEX_NUM(otherOctree, *(f2+1));
						float *v23 = (float *) _VERTEX_NUM(otherOctree, *(f2+2));

						vector3f p1 = transform_vector(orthogonalBase, vector3f(v11[0], v11[1], v11[2])) + position;
						vector3f q1 = transform_vector(orthogonalBase, vector3f(v12[0], v12[1], v12[2])) + position;
						vector3f r1 = transform_vector(orthogonalBase, vector3f(v13[0], v13[1], v13[2])) + position;
						vector3f p2 = transform_vector(otherOrthogonalBase, vector3f(v21[0], v21[1], v21[2])) + otherPosition;
						vector3f q2 = transform_vector(otherOrthogonalBase, vector3f(v22[0], v22[1], v22[2])) + otherPosition;
						vector3f r2 = transform_vector(otherOrthogonalBase, vector3f(v23[0], v23[1], v23[2])) + otherPosition;

						if (tri_tri_overlap_test_3d(p1.data(), q1.data(), r1.data(), p2.data(), q2.data(), r2.data()))
						{
//							intersectionInfo->v1 = vector3f(v11[0], v11[1], v11[2]);
//							intersectionInfo->v2 = vector3f(v12[0], v12[1], v12[2]);
//							intersectionInfo->v3 = vector3f(v13[0], v13[1], v13[2]);
//							intersectionInfo->o1 = vector3f(v21[0], v21[1], v21[2]);
//							intersectionInfo->o2 = vector3f(v22[0], v22[1], v22[2]);
//							intersectionInfo->o3 = vector3f(v23[0], v23[1], v23[2]);
							intersectionInfo->normal = unit_cross(vector3f(v11[0], v11[1], v11[2]) - vector3f(v12[0], v12[1], v12[2]),  vector3f(v11[0], v11[1], v11[2]) - vector3f(v13[0], v13[1], v13[2]));

							return YES;
						}
					}
				}
			}
			else if ((n1->childIndex1 == 0xFFFF) && (n2->childIndex1 != 0xFFFF)) // we are a leaf, other is not
			{
				int indices[8] = {n2->childIndex1, n2->childIndex2, n2->childIndex3, n2->childIndex4, n2->childIndex5, n2->childIndex6, n2->childIndex7, n2->childIndex8};
				int i;

				for(i = 0; i < 8; i++)
					if (intersectOctreeNodeWithOctreeNode(thisOctree, nodeNum, position, orthogonalBase, otherOctree, indices[i], otherPosition, otherOrthogonalBase, intersectionInfo))
						return YES;
			}
			else // we are not a leaf, other may be or not
			{
				int indices[8] = {n1->childIndex1, n1->childIndex2, n1->childIndex3, n1->childIndex4, n1->childIndex5, n1->childIndex6, n1->childIndex7, n1->childIndex8};
				int i;

				for(i = 0; i < 8; i++)
					if (intersectOctreeNodeWithOctreeNode(thisOctree, indices[i], position, orthogonalBase, otherOctree, otherNodeNum, otherPosition, otherOrthogonalBase, intersectionInfo))
						return YES;
			}
		}
	}

	return NO;
}

BOOL intersectOctreeNodeWithLine(struct octree_struct *thisOctree, int nodeNum, const vector3f startPoint, const vector3f endPoint, float intersectionPoint[3]) // TODO: this is broken for octrees with len(vertices) > 0xFFFF
{
	struct octree_node *n = (struct octree_node *) _NODE_NUM(thisOctree, nodeNum);

	if (n->faceCount)
	{
		float aabbOrigin[3] = {n->aabbOriginX + (n->aabbExtentX/2), n->aabbOriginY + (n->aabbExtentY/2), n->aabbOriginZ + (n->aabbExtentZ/2)};
		float aabbExtent[3] = {(n->aabbExtentX/2), (n->aabbExtentY/2), (n->aabbExtentZ/2)};
		BOOL intersects = SegmentIntersectsAABB((float *)&startPoint, (float *)&endPoint, aabbOrigin, aabbExtent);

		if (intersects)
		{
			uint32_t i;

			if (n->childIndex1 == 0xFFFF) // we are a leaf
			{

				for (i = n->firstFace; i < n->firstFace + n->faceCount; i++)
				{
					uint16_t *f = (uint16_t *) _FACE_NUM(thisOctree, i);
					float *v1 = (float *) _VERTEX_NUM(thisOctree, *f);
					float *v2 = (float *) _VERTEX_NUM(thisOctree, *(f+1));		// nana this could be prettified
					float *v3 = (float *) _VERTEX_NUM(thisOctree, *(f+2));

					float t, u, v;
					if(intersect_triangle((float *)&startPoint, (float *)&endPoint, v1, v2, v3, &t, &u, &v) && (v <= 1.0f))
					{
						vector3f ip = (1-u-v) * vector3f(v1[0],v1[1],v1[2]) + u * vector3f(v2[0],v2[1],v2[2]) + v * vector3f(v3[0],v3[1],v3[2]);
						intersectionPoint[0] = ip[0];
						intersectionPoint[1] = ip[1];
						intersectionPoint[2] = ip[2];

						return YES;
					}
				}
			}
			else
			{
				int indices[8] = {n->childIndex1, n->childIndex2, n->childIndex3, n->childIndex4, n->childIndex5, n->childIndex6, n->childIndex7, n->childIndex8};

				for(i = 0; i < 8; i++)
					if (intersectOctreeNodeWithLine(thisOctree, indices[i], startPoint, endPoint, intersectionPoint))
						return YES;
			}
		}
	}

	return NO;
}


#pragma mark -
#pragma mark -
#pragma mark -


bool SegmentIntersectsAABB(const vector3f segmentStart, const vector3f segmentEnd, const vector3f aabbCenter, const vector3f aabbExtent) // seperating axis theorem, code found in opcode, adapted
{	const vector3f d = (segmentEnd - segmentStart) * 0.5f;
	const vector3f ad = vector3f(fabsf(d[0]), fabsf(d[1]), fabsf(d[2]));
	const vector3f c = (segmentStart + d) - aabbCenter;

    if (fabsf(c[0]) > aabbExtent[0] + ad[0]) return FALSE;
    if (fabsf(c[1]) > aabbExtent[1] + ad[1]) return FALSE;
    if (fabsf(c[2]) > aabbExtent[2] + ad[2]) return FALSE;

    if (fabsf(d[1] * c[2] - d[2] * c[1]) > (aabbExtent[1] * ad[2]) + (aabbExtent[2] * ad[1]))  return FALSE;
    if (fabsf(d[2] * c[0] - d[0] * c[2]) > (aabbExtent[2] * ad[0]) + (aabbExtent[0] * ad[2]))  return FALSE;
    if (fabsf(d[0] * c[1] - d[1] * c[0]) > (aabbExtent[0] * ad[1]) + (aabbExtent[1] * ad[0]))  return FALSE;

    return TRUE;
}

/*bool AABBIntersectsAABB(const vector3f aabbCenter1, const vector3f aabbExtent1, const vector3f aabbCenter2, const vector3f aabbExtent2) // seperating axis theorem, code found on some board, adapted
{
	vector3f T1 = aabbCenter2 - aabbCenter1;
	vector3f T2 = aabbExtent1 + aabbExtent2;

	return (fabsf(T1[0]) <= T2[0]) && (fabsf(T1[1]) <= T2[1]) && (fabsf(T1[2]) <= T2[2]);
}*/

bool OBBIntersectsOBB (const vector3f obb1Center, const vector3f obb1Extent, const matrix33f_c obb1OrthogonalBase, const vector3f obb2Center, const vector3f obb2Extent, const matrix33f_c obb2OrthogonalBase) // code from http://www.gamasutra.com/features/19991018/Gomez_5.htm
{	// TODO: replace by more optimized code, like from OPCODE
	vector3f v = obb2Center - obb1Center; 	//translation, in parent frame

	//translation, in A's frame
	float T[3] = {dot(v, matrix_get_x_basis_vector(obb1OrthogonalBase)), dot(v, matrix_get_y_basis_vector(obb1OrthogonalBase)), dot(v, matrix_get_z_basis_vector(obb1OrthogonalBase))};

	//B's basis with respect to A's local frame
	float R[3][3];
	float ra, rb, t;
	long i, k;

	//calculate rotation matrix
	for( i=0 ; i<3 ; i++ )
		for( k=0 ; k<3 ; k++ )
			R[i][k] = dot(matrix_get_basis_vector(obb1OrthogonalBase,i), matrix_get_basis_vector(obb2OrthogonalBase,k));

	/*ALGORITHM: Use the separating axis test for all 15 potential
	 separating axes. If a separating axis could not be found, the two
	 boxes overlap. */

	//A's basis vectors
	for( i=0 ; i<3 ; i++ )
	{
		ra = obb1Extent[i];

		rb = obb2Extent[0]*fabsf(R[i][0]) + obb2Extent[1]*fabsf(R[i][1]) + obb2Extent[2]*fabsf(R[i][2]);

		t = fabsf( T[i] );

		if( t > ra + rb )
			return FALSE;
	}

	//B's basis vectors
	for( k=0 ; k<3 ; k++ )
	{
		ra = obb1Extent[0]*fabsf(R[0][k]) + obb1Extent[1]*fabsf(R[1][k]) + obb1Extent[2]*fabsf(R[2][k]);
		rb = obb2Extent[k];
		t = fabsf( T[0]*R[0][k] + T[1]*R[1][k] +  T[2]*R[2][k] );

		if( t > ra + rb )
			return FALSE;
	}

	//9 cross products

	//L = A0 x B0
	ra = obb1Extent[1]*fabsf(R[2][0]) + obb1Extent[2]*fabsf(R[1][0]);
	rb = obb2Extent[1]*fabsf(R[0][2]) + obb2Extent[2]*fabsf(R[0][1]);
	t = fabsf( T[2]*R[1][0] - T[1]*R[2][0] );

	if( t > ra + rb )
		return FALSE;

	//L = A0 x B1
	ra = obb1Extent[1]*fabsf(R[2][1]) + obb1Extent[2]*fabsf(R[1][1]);
	rb = obb2Extent[0]*fabsf(R[0][2]) + obb2Extent[2]*fabsf(R[0][0]);
	t = fabsf( T[2]*R[1][1] - T[1]*R[2][1] );

	if( t > ra + rb )
		return FALSE;

	//L = A0 x B2
	ra = obb1Extent[1]*fabsf(R[2][2]) + obb1Extent[2]*fabsf(R[1][2]);
	rb = obb2Extent[0]*fabsf(R[0][1]) + obb2Extent[1]*fabsf(R[0][0]);
	t = fabsf( T[2]*R[1][2] -  T[1]*R[2][2] );

	if( t > ra + rb )
		return FALSE;

	//L = A1 x B0
	ra = obb1Extent[0]*fabsf(R[2][0]) + obb1Extent[2]*fabsf(R[0][0]);
	rb = obb2Extent[1]*fabsf(R[1][2]) + obb2Extent[2]*fabsf(R[1][1]);
	t = fabsf( T[0]*R[2][0] - T[2]*R[0][0] );

	if( t > ra + rb )
		return FALSE;

	//L = A1 x B1
	ra = obb1Extent[0]*fabsf(R[2][1]) + obb1Extent[2]*fabsf(R[0][1]);
	rb = obb2Extent[0]*fabsf(R[1][2]) + obb2Extent[2]*fabsf(R[1][0]);
	t = fabsf( T[0]*R[2][1] -  T[2]*R[0][1] );

	if( t > ra + rb )
		return FALSE;

	//L = A1 x B2
	ra = obb1Extent[0]*fabsf(R[2][2]) + obb1Extent[2]*fabsf(R[0][2]);
	rb = obb2Extent[0]*fabsf(R[1][1]) + obb2Extent[1]*fabsf(R[1][0]);
	t = fabsf( T[0]*R[2][2] - T[2]*R[0][2] );

	if( t > ra + rb )
		return FALSE;

	//L = A2 x B0
	ra = obb1Extent[0]*fabsf(R[1][0]) + obb1Extent[1]*fabsf(R[0][0]);
	rb = obb2Extent[1]*fabsf(R[2][2]) + obb2Extent[2]*fabsf(R[2][1]);
	t = fabsf( T[1]*R[0][0] - T[0]*R[1][0] );

	if( t > ra + rb )
		return FALSE;

	//L = A2 x B1
	ra = obb1Extent[0]*fabsf(R[1][1]) + obb1Extent[1]*fabsf(R[0][1]);
	rb = obb2Extent[0]*fabsf(R[2][2]) + obb2Extent[2]*fabsf(R[2][0]);
	t = fabsf( T[1]*R[0][1] -  T[0]*R[1][1] );

	if( t > ra + rb )
		return FALSE;

	//L = A2 x B2
	ra = obb1Extent[0]*fabsf(R[1][2]) + obb1Extent[1]*fabsf(R[0][2]);
	rb = obb2Extent[0]*fabsf(R[2][1]) + obb2Extent[1]*fabsf(R[2][0]);
	t = fabsf( T[1]*R[0][2] - T[0]*R[1][2] );

	if( t > ra + rb )
		return FALSE;

	return TRUE; /*no separating axis found, the two boxes overlap */
}

/*
*
*  Triangle-Triangle Overlap Test Routines
*  July, 2002
*  Updated December 2003
*
*  This file contains C implementation of algorithms for
*  performing two and three-dimensional triangle-triangle intersection test
*  The algorithms and underlying theory are described in
*
* "Fast and Robust Triangle-Triangle Overlap Test
*  Using Orientation Predicates"  P. Guigue - O. Devillers
*
*  Journal of Graphics Tools, 8(1), 2003
*
*  Several geometric predicates are defined.  Their parameters are all
*  points.  Each point is an array of two or three float precision
*  floating point numbers. The geometric predicates implemented in
*  this file are:
*
*    int tri_tri_overlap_test_3d(p1,q1,r1,p2,q2,r2)
*    int tri_tri_overlap_test_2d(p1,q1,r1,p2,q2,r2)
*
*    int tri_tri_intersection_test_3d(p1,q1,r1,p2,q2,r2,
*                                     coplanar,source,target)
*
*       is a version that computes the segment of intersection when
*       the triangles overlap (and are not coplanar)
*
*    each function returns 1 if the triangles (including their
*    boundary) intersect, otherwise 0
*
*
*  Other information are available from the Web page
*  http://www.acm.org/jgt/papers/GuigueDevillers03/
*
*/



/* function prototype */

//int tri_tri_overlap_test_3d(float p1[3], float q1[3], float r1[3], 			    float p2[3], float q2[3], float r2[3]);


int coplanar_tri_tri3d(float  p1[3], float  q1[3], float  r1[3],
		       float  p2[3], float  q2[3], float  r2[3],
		       float  N1[3], float  N2[3]);


int tri_tri_overlap_test_2d(float p1[2], float q1[2], float r1[2],
			    float p2[2], float q2[2], float r2[2]);


int tri_tri_intersection_test_3d(float p1[3], float q1[3], float r1[3],
				 float p2[3], float q2[3], float r2[3],
				 int * coplanar,
				 float source[3],float target[3]);

/* coplanar returns whether the triangles are coplanar
*  source and target are the endpoints of the segment of
*  intersection if it exists)
*/


/* some 3D macros */

#define CROSS(dest,v1,v2)                       \
               dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
               dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
               dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])



#define SUB(dest,v1,v2) dest[0]=v1[0]-v2[0]; \
                        dest[1]=v1[1]-v2[1]; \
                        dest[2]=v1[2]-v2[2];


#define SCALAR(dest,alpha,v) dest[0] = alpha * v[0]; \
                             dest[1] = alpha * v[1]; \
                             dest[2] = alpha * v[2];



#define CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) {\
  SUB(v1,p2,q1)\
  SUB(v2,p1,q1)\
  CROSS(N1,v1,v2)\
  SUB(v1,q2,q1)\
  if (DOT(v1,N1) > 0.0f) return 0;\
  SUB(v1,p2,p1)\
  SUB(v2,r1,p1)\
  CROSS(N1,v1,v2)\
  SUB(v1,r2,p1) \
  if (DOT(v1,N1) > 0.0f) return 0;\
  else return 1; }



/* Permutation in a canonical form of T2's vertices */

#define TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) { \
  if (dp2 > 0.0f) { \
     if (dq2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2) \
     else if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)\
     else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) }\
  else if (dp2 < 0.0f) { \
    if (dq2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)\
    else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)\
    else CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)\
  } else { \
    if (dq2 < 0.0f) { \
      if (dr2 >= 0.0f)  CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)\
      else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2)\
    } \
    else if (dq2 > 0.0f) { \
      if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)\
      else  CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)\
    } \
    else  { \
      if (dr2 > 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)\
      else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2)\
      else return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);\
     }}}



/*
*
*  Three-dimensional Triangle-Triangle Overlap Test
*
*/


int tri_tri_overlap_test_3d(float p1[3], float q1[3], float r1[3],

			    float p2[3], float q2[3], float r2[3])
{
  float dp1, dq1, dr1, dp2, dq2, dr2;
  float v1[3], v2[3];
  float N1[3], N2[3];

  /* Compute distance signs  of p1, q1 and r1 to the plane of
     triangle(p2,q2,r2) */


  SUB(v1,p2,r2)
  SUB(v2,q2,r2)
  CROSS(N2,v1,v2)

  SUB(v1,p1,r2)
  dp1 = DOT(v1,N2);
  SUB(v1,q1,r2)
  dq1 = DOT(v1,N2);
  SUB(v1,r1,r2)
  dr1 = DOT(v1,N2);

  if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0;

  /* Compute distance signs  of p2, q2 and r2 to the plane of
     triangle(p1,q1,r1) */


  SUB(v1,q1,p1)
  SUB(v2,r1,p1)
  CROSS(N1,v1,v2)

  SUB(v1,p2,r1)
  dp2 = DOT(v1,N1);
  SUB(v1,q2,r1)
  dq2 = DOT(v1,N1);
  SUB(v1,r2,r1)
  dr2 = DOT(v1,N1);

  if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;

  /* Permutation in a canonical form of T1's vertices */


  if (dp1 > 0.0f) {
    if (dq1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
    else if (dr1 > 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
    else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
  } else if (dp1 < 0.0f) {
    if (dq1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
    else if (dr1 < 0.0f) TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    else TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
  } else {
    if (dq1 < 0.0f) {
      if (dr1 >= 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
    }
    else if (dq1 > 0.0f) {
      if (dr1 > 0.0f) TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    }
    else  {
      if (dr1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
      else if (dr1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
      else return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);
    }
  }
};



int coplanar_tri_tri3d(float p1[3], float q1[3], float r1[3],
		       float p2[3], float q2[3], float r2[3],
		       float normal_1[3], float normal_2[3]){

  float P1[2],Q1[2],R1[2];
  float P2[2],Q2[2],R2[2];

  float n_x, n_y, n_z;

  n_x = ((normal_1[0]<0)?-normal_1[0]:normal_1[0]);
  n_y = ((normal_1[1]<0)?-normal_1[1]:normal_1[1]);
  n_z = ((normal_1[2]<0)?-normal_1[2]:normal_1[2]);


  /* Projection of the triangles in 3D onto 2D such that the area of
     the projection is maximized. */


  if (( n_x > n_z ) && ( n_x >= n_y )) {
    // Project onto plane YZ

      P1[0] = q1[2]; P1[1] = q1[1];
      Q1[0] = p1[2]; Q1[1] = p1[1];
      R1[0] = r1[2]; R1[1] = r1[1];

      P2[0] = q2[2]; P2[1] = q2[1];
      Q2[0] = p2[2]; Q2[1] = p2[1];
      R2[0] = r2[2]; R2[1] = r2[1];

  } else if (( n_y > n_z ) && ( n_y >= n_x )) {
    // Project onto plane XZ

    P1[0] = q1[0]; P1[1] = q1[2];
    Q1[0] = p1[0]; Q1[1] = p1[2];
    R1[0] = r1[0]; R1[1] = r1[2];

    P2[0] = q2[0]; P2[1] = q2[2];
    Q2[0] = p2[0]; Q2[1] = p2[2];
    R2[0] = r2[0]; R2[1] = r2[2];

  } else {
    // Project onto plane XY

    P1[0] = p1[0]; P1[1] = p1[1];
    Q1[0] = q1[0]; Q1[1] = q1[1];
    R1[0] = r1[0]; R1[1] = r1[1];

    P2[0] = p2[0]; P2[1] = p2[1];
    Q2[0] = q2[0]; Q2[1] = q2[1];
    R2[0] = r2[0]; R2[1] = r2[1];
  }

  return tri_tri_overlap_test_2d(P1,Q1,R1,P2,Q2,R2);

};


/*
*
*  Three-dimensional Triangle-Triangle Intersection
*
*/

/*
   This macro is called when the triangles surely intersect
   It constructs the segment of intersection of the two triangles
   if they are not coplanar.
*/
/*
#define CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) { \
  SUB(v1,q1,p1) \
  SUB(v2,r2,p1) \
  CROSS(N,v1,v2) \
  SUB(v,p2,p1) \
  if (DOT(v,N) > 0.0f) {\
    SUB(v1,r1,p1) \
    CROSS(N,v1,v2) \
    if (DOT(v,N) <= 0.0f) { \
      SUB(v2,q2,p1) \
      CROSS(N,v1,v2) \
      if (DOT(v,N) > 0.0f) { \
	SUB(v1,p1,p2) \
	SUB(v2,p1,r1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p1,v1) \
	SUB(v1,p2,p1) \
	SUB(v2,p2,r2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p2,v1) \
	return 1; \
      } else { \
	SUB(v1,p2,p1) \
	SUB(v2,p2,q2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p2,v1) \
	SUB(v1,p2,p1) \
	SUB(v2,p2,r2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p2,v1) \
	return 1; \
      } \
    } else { \
      return 0; \
    } \
  } else { \
    SUB(v2,q2,p1) \
    CROSS(N,v1,v2) \
    if (DOT(v,N) < 0.0f) { \
      return 0; \
    } else { \
      SUB(v1,r1,p1) \
      CROSS(N,v1,v2) \
      if (DOT(v,N) >= 0.0f) { \
	SUB(v1,p1,p2) \
	SUB(v2,p1,r1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p1,v1) \
	SUB(v1,p1,p2) \
	SUB(v2,p1,q1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p1,v1) \
	return 1; \
      } else { \
	SUB(v1,p2,p1) \
	SUB(v2,p2,q2) \
	alpha = DOT(v1,N1) / DOT(v2,N1); \
	SCALAR(v1,alpha,v2) \
	SUB(source,p2,v1) \
	SUB(v1,p1,p2) \
	SUB(v2,p1,q1) \
	alpha = DOT(v1,N2) / DOT(v2,N2); \
	SCALAR(v1,alpha,v2) \
	SUB(target,p1,v1) \
	return 1; \
      }}}}



#define TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) { \
  if (dp2 > 0.0f) { \
     if (dq2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2) \
     else if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)\
     else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) }\
  else if (dp2 < 0.0f) { \
    if (dq2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)\
    else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)\
    else CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)\
  } else { \
    if (dq2 < 0.0f) { \
      if (dr2 >= 0.0f)  CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)\
      else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2)\
    } \
    else if (dq2 > 0.0f) { \
      if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)\
      else  CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)\
    } \
    else  { \
      if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)\
      else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2)\
      else { \
       	*coplanar = 1; \
	return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);\
     } \
  }} }
  */

/*
   The following version computes the segment of intersection of the
   two triangles if it exists.
   coplanar returns whether the triangles are coplanar
   source and target are the endpoints of the line segment of intersection
*/
/*
int tri_tri_intersection_test_3d(float p1[3], float q1[3], float r1[3],
				 float p2[3], float q2[3], float r2[3],
				 int * coplanar,
				 float source[3], float target[3] )

{
  float dp1, dq1, dr1, dp2, dq2, dr2;
  float v1[3], v2[3], v[3];
  float N1[3], N2[3], N[3];
  float alpha;

  // Compute distance signs  of p1, q1 and r1
  // to the plane of triangle(p2,q2,r2)


  SUB(v1,p2,r2)
  SUB(v2,q2,r2)
  CROSS(N2,v1,v2)

  SUB(v1,p1,r2)
  dp1 = DOT(v1,N2);
  SUB(v1,q1,r2)
  dq1 = DOT(v1,N2);
  SUB(v1,r1,r2)
  dr1 = DOT(v1,N2);

  if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0;

  // Compute distance signs  of p2, q2 and r2
  // to the plane of triangle(p1,q1,r1)


  SUB(v1,q1,p1)
  SUB(v2,r1,p1)
  CROSS(N1,v1,v2)

  SUB(v1,p2,r1)
  dp2 = DOT(v1,N1);
  SUB(v1,q2,r1)
  dq2 = DOT(v1,N1);
  SUB(v1,r2,r1)
  dr2 = DOT(v1,N1);

  if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;

  // Permutation in a canonical form of T1's vertices


  if (dp1 > 0.0f) {
    if (dq1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
    else if (dr1 > 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)

    else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
  } else if (dp1 < 0.0f) {
    if (dq1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
    else if (dr1 < 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    else TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
  } else {
    if (dq1 < 0.0f) {
      if (dr1 >= 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)
    }
    else if (dq1 > 0.0f) {
      if (dr1 > 0.0f) TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)
      else TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)
    }
    else  {
      if (dr1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)
      else if (dr1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)
      else {
	// triangles are co-planar

	*coplanar = 1;
	return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);
      }
    }
  }
};


*/


/*
*
*  Two dimensional Triangle-Triangle Overlap Test
*
*/


/* some 2D macros */

#define ORIENT_2D(a, b, c)  ((a[0]-c[0])*(b[1]-c[1])-(a[1]-c[1])*(b[0]-c[0]))


#define INTERSECTION_TEST_VERTEX(P1, Q1, R1, P2, Q2, R2) {\
  if (ORIENT_2D(R2,P2,Q1) >= 0.0f)\
    if (ORIENT_2D(R2,Q2,Q1) <= 0.0f)\
      if (ORIENT_2D(P1,P2,Q1) > 0.0f) {\
	if (ORIENT_2D(P1,Q2,Q1) <= 0.0f) return 1; \
	else return 0;} else {\
	if (ORIENT_2D(P1,P2,R1) >= 0.0f)\
	  if (ORIENT_2D(Q1,R1,P2) >= 0.0f) return 1; \
	  else return 0;\
	else return 0;}\
    else \
      if (ORIENT_2D(P1,Q2,Q1) <= 0.0f)\
	if (ORIENT_2D(R2,Q2,R1) <= 0.0f)\
	  if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) return 1; \
	  else return 0;\
	else return 0;\
      else return 0;\
  else\
    if (ORIENT_2D(R2,P2,R1) >= 0.0f) \
      if (ORIENT_2D(Q1,R1,R2) >= 0.0f)\
	if (ORIENT_2D(P1,P2,R1) >= 0.0f) return 1;\
	else return 0;\
      else \
	if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) {\
	  if (ORIENT_2D(R2,R1,Q2) >= 0.0f) return 1; \
	  else return 0; }\
	else return 0; \
    else  return 0; \
 };



#define INTERSECTION_TEST_EDGE(P1, Q1, R1, P2, Q2, R2) { \
  if (ORIENT_2D(R2,P2,Q1) >= 0.0f) {\
    if (ORIENT_2D(P1,P2,Q1) >= 0.0f) { \
        if (ORIENT_2D(P1,Q1,R2) >= 0.0f) return 1; \
        else return 0;} else { \
      if (ORIENT_2D(Q1,R1,P2) >= 0.0f){ \
	if (ORIENT_2D(R1,P1,P2) >= 0.0f) return 1; else return 0;} \
      else return 0; } \
  } else {\
    if (ORIENT_2D(R2,P2,R1) >= 0.0f) {\
      if (ORIENT_2D(P1,P2,R1) >= 0.0f) {\
	if (ORIENT_2D(P1,R1,R2) >= 0.0f) return 1;  \
	else {\
	  if (ORIENT_2D(Q1,R1,R2) >= 0.0f) return 1; else return 0;}}\
      else  return 0; }\
    else return 0; }}



int ccw_tri_tri_intersection_2d(float p1[2], float q1[2], float r1[2],
				float p2[2], float q2[2], float r2[2]) {
  if ( ORIENT_2D(p2,q2,p1) >= 0.0f ) {
    if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {
      if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) return 1;
      else INTERSECTION_TEST_EDGE(p1,q1,r1,p2,q2,r2)
    } else {
      if ( ORIENT_2D(r2,p2,p1) >= 0.0f )
	INTERSECTION_TEST_EDGE(p1,q1,r1,r2,p2,q2)
      else INTERSECTION_TEST_VERTEX(p1,q1,r1,p2,q2,r2)}}
  else {
    if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {
      if ( ORIENT_2D(r2,p2,p1) >= 0.0f )
	INTERSECTION_TEST_EDGE(p1,q1,r1,q2,r2,p2)
      else  INTERSECTION_TEST_VERTEX(p1,q1,r1,q2,r2,p2)}
    else INTERSECTION_TEST_VERTEX(p1,q1,r1,r2,p2,q2)}
};


int tri_tri_overlap_test_2d(float p1[2], float q1[2], float r1[2],
			    float p2[2], float q2[2], float r2[2]) {
  if ( ORIENT_2D(p1,q1,r1) < 0.0f )
    if ( ORIENT_2D(p2,q2,r2) < 0.0f )
      return ccw_tri_tri_intersection_2d(p1,r1,q1,p2,r2,q2);
    else
      return ccw_tri_tri_intersection_2d(p1,r1,q1,p2,q2,r2);
  else
    if ( ORIENT_2D(p2,q2,r2) < 0.0f )
      return ccw_tri_tri_intersection_2d(p1,q1,r1,p2,r2,q2);
    else
      return ccw_tri_tri_intersection_2d(p1,q1,r1,p2,q2,r2);

};

#define EPSILON 0.000001

int
intersect_triangle(float orig[3], float dir[3],
                   float vert0[3], float vert1[3], float vert2[3],
                   float *t, float *u, float *v)
{
   float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   float det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
   if (det < EPSILON)
      return 0;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec);
   if (*u < 0.0 || *u > det)
      return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec);
   if (*v < 0.0 || *u + *v > det)
      return 0;

   /* calculate t, scale parameters, ray intersects triangle */
   *t = DOT(edge2, qvec);
   inv_det = 1.0 / det;
   *t *= inv_det;
   *u *= inv_det;
   *v *= inv_det;
#else                    /* the non-culling branch */
   if (det > -EPSILON && det < EPSILON)
     return 0;
   inv_det = 1.0 / det;

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = DOT(tvec, pvec) * inv_det;
   if (*u < 0.0 || *u > 1.0)
     return 0;

   /* prepare to test V parameter */
   CROSS(qvec, tvec, edge1);

   /* calculate V parameter and test bounds */
   *v = DOT(dir, qvec) * inv_det;
   if (*v < 0.0 || *u + *v > 1.0)
     return 0;

   /* calculate t, ray intersects triangle */
   *t = DOT(edge2, qvec) * inv_det;
#endif
   return 1;
}

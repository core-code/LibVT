//
//  CollideableMesh.h
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


@interface CollideableMesh : Mesh
{
	struct octree_struct	*octree_collision;
}


- (vector3f)intersectWithLineStart:(vector3f)startPoint end:(vector3f)endPoint;
- (TriangleIntersectionInfo)intersectWithMesh:(CollideableMesh *)otherMesh;

@end
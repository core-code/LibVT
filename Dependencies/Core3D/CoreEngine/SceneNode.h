//
//  SceneNode.h
//  Core3D
//
//  Created by Julian Mayer on 21.11.07.
//  Copyright 2007 - 2010 A. Julian Mayer.
//
/*
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

@interface SceneNode : NSObject
{
	BOOL					enabled;
	vector3f				position, rotation;
	SceneNode				*relativeModeTarget;
	axisConfigurationEnum	relativeModeAxisConfiguration, axisConfiguration;
	NSMutableArray			*children;
}

@property (assign, nonatomic) BOOL enabled;
@property (assign, nonatomic) axisConfigurationEnum relativeModeAxisConfiguration;
@property (assign, nonatomic) axisConfigurationEnum axisConfiguration;
@property (assign, nonatomic) vector3f position;
@property (assign, nonatomic) vector3f rotation;
@property (retain, nonatomic) SceneNode *relativeModeTarget;
@property (retain, nonatomic) NSMutableArray *children;

- (void)setPositionByMovingForward:(float)amount;
- (void)setRotationFromLookAt:(vector3f)lookAt;
- (vector3f)getLookAt;

- (void)transform;

- (void)reshapeNode:(NSArray *)size;
- (void)renderNode;
- (void)updateNode;

- (void)reshape:(NSArray *)size;
- (void)render;
- (void)update;

- (void)renderCenter;

@end
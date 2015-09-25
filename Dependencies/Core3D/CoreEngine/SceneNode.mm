//
//  SceneNode.m
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

#import "Core3D.h"


@implementation SceneNode

@synthesize position, rotation, relativeModeTarget, relativeModeAxisConfiguration, axisConfiguration, children, enabled;

- (id)init
{
	if ((self = [super init]))
	{
		axisConfiguration = kYXZRotation;
		relativeModeAxisConfiguration = kYXZRotation;
		relativeModeTarget = nil;
		children = [[NSMutableArray alloc] initWithCapacity:5];
		enabled = YES;
	}

	return self;
}

- (void)update
{
	if (enabled)
		[self updateNode];

	[children makeObjectsPerformSelector:@selector(update)];
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"<%@ 0x%08x>\n position: %f %f %f\n rotation: %f %f %f \n children:%@", [self class], self, position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], [children description]];
}

- (void)transform
{
	if (relativeModeTarget != nil)
	{
		[[scene camera] translate:[relativeModeTarget position]];
		[[scene camera] rotate:[relativeModeTarget rotation] withConfig:relativeModeAxisConfiguration];
	}

	[[scene camera] translate:position];
	[[scene camera] rotate:rotation withConfig:axisConfiguration];
}

- (void)render
{
	[[scene camera] push];

	[self transform];

	if (enabled)
		[self renderNode];

	[children makeObjectsPerformSelector:@selector(render)];

	[[scene camera] pop];
}

- (void)reshape:(NSArray *)size
{
	[self reshapeNode:size];
	[children makeObjectsPerformSelector:@selector(reshapeNode:) withObject:size];
}

- (void)updateNode {}
- (void)renderNode {}
- (void)reshapeNode:(NSArray *)size {}

- (void)renderCenter
{
#ifndef TARGET_OS_IPHONE
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	
	myPointSize(10.0);
	myClientStateVTN(kNeedDisabled, kNeedDisabled, kNeedDisabled);
	myColor(0.4, 0.01, 0.01, 1.0);
	glBegin(GL_POINTS);
		glVertex3f(0, 0, 0);
	glEnd();
	
	myPointSize(1.0);
	glBegin(GL_LINES);
		myColor(0.4, 0.4, 0.4, 1.0);
		glVertex3f(0, 0, 0);
			vector3f la = vector3f(0, 0, -1);
	glVertex3fv((const GLfloat *)&la);
	glEnd();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	globalInfo.drawCalls += 2;
#endif
}

- (void)setRotationFromLookAt:(vector3f)lookAt
{
	static const vector3f forward = vector3f(0, 0, -1);
	vector3f direction = lookAt - position;
	vector3f direction_without_y = vector3f(direction[0], 0, direction[2]);

	float yrotdeg = cml::deg(unsigned_angle(forward, direction_without_y));
	float xrotdeg = cml::deg(unsigned_angle(direction_without_y, direction));

	rotation[0] = direction[1] > 0 ? xrotdeg : -xrotdeg;
	rotation[1] = direction[0] > 0 ? -yrotdeg : yrotdeg;
	rotation[2] = 0;
}

- (void)setPositionByMovingForward:(float)amount
{
	position += [self getLookAt] * amount;
}

- (vector3f)getLookAt
{
	matrix33f_c m;
	static const vector3f forward = vector3f(0, 0, -1);
	vector3f rot = rotation;

	if (relativeModeTarget != nil)
	{
		NSLog(@"Warning: getLookAt for target mode probably broken"); // FIXME: fix this
		rot += [relativeModeTarget rotation];
	}

	matrix_rotation_euler(m, cml::rad(rot[0]), cml::rad(rot[1]), cml::rad(rot[2]), cml::euler_order_xyz);

	return transform_vector(m, forward);
}

- (void)dealloc
{
	[children release];

	[super dealloc];
}
@end

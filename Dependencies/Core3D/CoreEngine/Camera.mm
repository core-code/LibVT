//
//  Camera.m
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


@implementation Camera

@synthesize fov, nearPlane, farPlane, projectionMatrix, viewMatrix;

- (id)init
{
	if ((self = [super init]))
	{
		fov = 45.0f;
		nearPlane = 1.0f;
		farPlane = 8000.0f;

		[self addObserver:self forKeyPath:@"fov" options:NSKeyValueObservingOptionNew context:NULL];
		[self addObserver:self forKeyPath:@"nearPlane" options:NSKeyValueObservingOptionNew context:NULL];
		[self addObserver:self forKeyPath:@"farPlane" options:NSKeyValueObservingOptionNew context:NULL];

		modelViewMatrices.push_back(cml::identity_transform<4,4>());
	}

	return self;
}

- (void)reshapeNode:(NSArray *)size
{
	globalInfo.width = [[size objectAtIndex:0] intValue];
	globalInfo.height = [[size objectAtIndex:1] intValue];

	glViewport(0, 0, globalInfo.width, globalInfo.height);

	[self updateProjection];
}

- (void)transform
{
	[[scene camera] rotate:-rotation withConfig:axisConfiguration];
	[[scene camera] translate:-position];

	if (relativeModeTarget != nil)
	{
		[[scene camera] rotate:-[relativeModeTarget rotation] withConfig:relativeModeAxisConfiguration];
		[[scene camera] translate:-[relativeModeTarget position]];
	}

	viewMatrix = modelViewMatrices.back();
}

- (void)updateProjection
{
	matrix_perspective_yfov_RH(projectionMatrix, cml::rad(fov), globalInfo.width / globalInfo.height, nearPlane, farPlane, cml::z_clip_neg_one);
#ifdef TARGET_OS_IPHONE
	matrix_rotate_about_local_z(projectionMatrix, (float) -(M_PI / 2.0));
#endif

#ifndef GL_ES_VERSION_2_0
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projectionMatrix.data());
	glMatrixMode(GL_MODELVIEW);
#endif
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	[self updateProjection];
}

- (matrix44f_c)modelViewMatrix
{
	return modelViewMatrices.back();
}

- (void)identity
{
	cml::identity_transform(modelViewMatrices.back());
}

- (void)translate:(vector3f)tra
{
	matrix44f_c m;
	matrix_translation(m, tra);
	modelViewMatrices.back() *= m;

#ifndef GL_ES_VERSION_2_0
	glLoadMatrixf(modelViewMatrices.back().data());
#endif
}

- (void)rotate:(vector3f)rot withConfig:(axisConfigurationEnum)axisRotation;
{
	for (uint8_t i = 0; i < 3; i++)	// this allows us to configure per-node the rotation order and axis to ignore (which is mostly useful for target mode)
	{
		uint8_t axis = (axisRotation >> (i * 2)) & 3;

		if ((axis != kDisabledAxis) && (rot[axis] != 0))
			matrix_rotate_about_local_axis(modelViewMatrices.back(), axis, cml::rad(rot[axis]));
	}

#ifndef GL_ES_VERSION_2_0
	glLoadMatrixf(modelViewMatrices.back().data());
#endif
}

- (void)push
{
	matrix44f_c m = modelViewMatrices.back();
	modelViewMatrices.push_back(m);
}

- (void)pop
{
	modelViewMatrices.pop_back();

#ifndef GL_ES_VERSION_2_0
	glLoadMatrixf(modelViewMatrices.back().data());
#endif
}
@end

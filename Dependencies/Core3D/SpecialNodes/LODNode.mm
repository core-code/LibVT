//
//  LODNode.mm
//  Core3D
//
//  Created by Julian Mayer on 27.01.10.
//  Copyright 2010 A. Julian Mayer.
//
/*
 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#import "Core3D.h"
#import "LODNode.h"

@implementation LODNode

- (id)initWithOctreesNamed:(NSArray *)_names andFactor:(float)_factor
{
	if ((self = [super init]))
	{
		for (NSString *name in _names)
			[children addObject:[[Mesh alloc] initWithOctreeNamed:name]];

		assert([_names count] == 2);

		factors[0] = _factor;
		factors[1] = 999999;

//		[(Mesh *)[children objectAtIndex:0] setColor:vector4f(1.0, 1.0, 1.0, 1.0)];
//		[(Mesh *)[children objectAtIndex:1] setColor:vector4f(1.0, 0.0, 0.0, 1.0)];
//		[(Mesh *)[children objectAtIndex:2] setColor:vector4f(0.0, 1.0, 0.0, 1.0)];
//		[(Mesh *)[children objectAtIndex:3] setColor:vector4f(0.0, 0.0, 1.0, 1.0)];

		center = [(Mesh *)[children objectAtIndex:0] center];
		radius = [(Mesh *)[children objectAtIndex:0] radius];
	}
	return self;
}

- (void)render // override render instead of implementing renderNode
{
	vector3f ro = center + [(SceneNode *)[children objectAtIndex:0] position];
	vector3f cp = [[scene camera] position];
	float distFactor = vector3f(cp - ro).length() / radius;



	for (uint8_t i = 0; i < 2; i++)
	{
		if (distFactor < factors[i])
		{
			[[children objectAtIndex:i] render];
			break;
		}
	}
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
    SEL aSelector = [invocation selector];

    if ([[children objectAtIndex:0] respondsToSelector:aSelector])
        [invocation invokeWithTarget:[children objectAtIndex:0]];
    else
        [self doesNotRecognizeSelector:aSelector];
}
@end


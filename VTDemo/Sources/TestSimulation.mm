//
//  TestSimulation.m
//  VTDemo
//
//  Created by Julian Mayer on 23.12.09.
/*	Copyright (c) 2010 A. Julian Mayer
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#import "TestSimulation.h"


@implementation TestSimulation

- (void)generateNextPos
{
	lastPos = nextPos;
	if (random_integer(1, 5) == 5)
		nextPos = vector3f(random_real(-5000, 5000), random_real(3000, 10000), random_real(-5000, 5000));
	else
		nextPos = vector3f(random_real(-5000, 5000), random_real(0, 2000), random_real(-5000, 5000));
	
	steps = random_integer(120, 240);
	stepsDone = 0;
	
	if (random_integer(0, 1) == 1)
		[[scene camera] setRotation:vector3f(random_real(-10, -50), 0, 0)];
	else
		[[scene camera] setRotation:vector3f(-90, 0, 0)];

}

- (id)init
{
	if ((self = [super init]))
	{
		nextPos = vector3f(0, 1000, 0);
		[self generateNextPos];

	}
	return self;
}


- (void)update
{
	[[scene camera] setPosition:lastPos + stepsDone * (nextPos - lastPos) / steps];
	
	stepsDone++;
	
	if (stepsDone == steps)
		[self generateNextPos];
}
@end
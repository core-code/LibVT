//
//  Skybox.m
//  Core3D
//
//  Created by Julian Mayer on 09.12.07.
//  Copyright 2007 - 2010 A. Julian Mayer.
//
/*
 This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License along with this library; if not, see <http://www.gnu.org/licenses/> or write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#import "Core3D.h"
#import "Skybox.h"

@implementation Skybox

@synthesize size;

- (id)initWithSurroundTextureNamed:(NSString *)surroundName
{
	return [self initWithSurroundTextureNamed:surroundName andUpTextureNamed:nil andDownTextureNamed:nil];
}

- (id)initWithSurroundTextureNamed:(NSString *)surroundName andDownTextureNamed:(NSString *)downName
{
	return [self initWithSurroundTextureNamed:surroundName andUpTextureNamed:nil andDownTextureNamed:downName];
}

- (id)initWithSurroundTextureNamed:(NSString *)surroundName andUpTextureNamed:(NSString *)upName
{
	return [self initWithSurroundTextureNamed:surroundName andUpTextureNamed:upName andDownTextureNamed:nil];
}

- (id)initWithSurroundTextureNamed:(NSString *)surroundName andUpTextureNamed:(NSString *)upName andDownTextureNamed:(NSString *)downName
{
	if ((self = [super init]))
	{
#define LOADTEX(y, x) { (y) = LoadTextureNamed((x), GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0); \
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); }

		LOADTEX(surroundTexture, surroundName)

		if (upName)		LOADTEX(upTexture, upName)
		if (downName)	LOADTEX(downTexture, downName)

		size = 5000;
	}
	return self;
}

- (void)renderNode
{
	if (globalInfo.renderpass != kMainRenderPass)
		return;

	[[scene camera] push];
	vector3f cp = [[scene camera] position];
	[[scene camera] setPosition:vector3f(0.0, 0.0, 0.0)];
	[[scene camera] identity];
	[[scene camera] transform];
	[[scene camera] setPosition:cp];

	//glDepthMask(GL_FALSE); // this kills us when we are a subnode of the motionblur shader
#ifndef GL_ES_VERSION_2_0
	glDisable(GL_LIGHTING);
#endif
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	//glDisable(GL_DEPTH_TEST);

	const GLshort vertices[] = {	-size, -size,  size,	-size, -size, -size,	-size,  size,  size,	-size,  size, -size,	// Left Face
									size,  size, -size,		size, -size, -size,		size,  size,  size,		size, -size,  size,		// Right face
									-size,  size, -size,	-size, -size, -size,	size,  size, -size,		size, -size, -size,		// Back Face
									size, -size,  size,		-size, -size,  size,	size,  size,  size,		-size,  size,  size,	// Front Face
									size, -size, -size,		-size, -size, -size,	size, -size,  size,		-size, -size,  size,	// Bottom Face
									-size,  size,  size,	-size,  size, -size,	size,  size,  size,		size,  size, -size};	// Top Face

	const GLfloat texCoords[] = {	0.5,0.0,	1.0,0.0,	0.5,0.5,	1.0,0.5,	// Left Face
									0.5,1.0,	0.5,0.5,	1.0,1.0,	1.0,0.5,	// Right face
									0.0,1.0,	0.0,0.5,	0.5,1.0,	0.5,0.5,	// Back Face
									0.0,0.0,	0.5,0.0,	0.0,0.5,	0.5,0.5,	// Front Face
									1,1,		1,0,	0,1,	0,0,			// Bottom Face
									0,1,		1,1,	0,0,	1,0};			// Top Face

	const GLubyte indices[] = {0,1,2, 1,2,3, 4,5,6, 5,6,7, 8,9,10, 9,10,11, 12,13,14, 13,14,15};

	myColor(1.0, 1.0, 1.0, 1.0);
	myClientStateVTN(kNeedEnabled, kNeedEnabled, kNeedDisabled);

#ifdef GL_ES_VERSION_2_0
	glEnableVertexAttribArray(VERTEX_ARRAY);
	glVertexAttribPointer(TEXCOORD_ARRAY, 3, GL_FLOAT, GL_FALSE, 0, (const GLfloat *) texCoords);
	glVertexAttribPointer(VERTEX_ARRAY, 3, GL_SHORT, GL_FALSE, 0, (const GLshort *) vertices);
#else
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	glVertexPointer(3, GL_SHORT, 0, vertices);
#endif
	



	glBindTexture(GL_TEXTURE_2D, surroundTexture);
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_BYTE, indices);

	globalInfo.drawCalls ++;

	if (upTexture)
	{
		const GLubyte indices[] = {20,21,22,21,22,23};
		glBindTexture(GL_TEXTURE_2D, upTexture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		globalInfo.drawCalls ++;
	}
	if (downTexture)
	{
		const GLubyte indices[] = {16,17,18, 17,18,19};
		glBindTexture(GL_TEXTURE_2D, downTexture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
		globalInfo.drawCalls ++;
	}

	//glDepthMask(GL_TRUE);
#ifndef GL_ES_VERSION_2_0
	glEnable(GL_LIGHTING);
#endif
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	[[scene camera] pop];
}
@end
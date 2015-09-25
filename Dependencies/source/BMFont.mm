//
//  BMFont.h
//  originally written for the polyvision game GunocideIIExTurbo
//
//  Created by Alexander Bierbrauer on 23.10.08.
//  Copyright 2008 polyvision.org. All rights reserved.
//
// This software is released under a BSD license. See LICENSE.TXT
// You must accept the license before using this software.
//
// parts of this code is based on the works of legolas558 who wrote a BMFont loader called oglBMFont

// Parts Copyright A. Julian Mayer 2009. 

#import "Core3D.h"
#import "BMFont.h"

GLubyte indices[BMMAXSTRINGLENGTH*6];


@implementation BMFont

@synthesize scale, rotation, infoCommonLineHeight, color;

- (id)initWithFontNamed:(NSString *)fontName andTextureNamed:(NSString *)textureName;
{
	if ((self = [super init]))
	{
		NSString *texPath = [[NSBundle mainBundle] pathForResource:textureName ofType:@"png"];
		if (texPath)
			texName = LoadTexture(texPath, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_TRUE, 0.0);
		else
			NSLog(@"Warning: could not find font texture named: %@", textureName);


		NSString *fntPath = [[NSBundle mainBundle] pathForResource:fontName ofType:@"fnt"];
		if (!fntPath)
			fatal("Error: could not find font file named: %s", [fntPath UTF8String]);

		NSXMLParser *parser = [[NSXMLParser alloc] initWithData:[NSData dataWithContentsOfFile:fntPath]];

		[parser setDelegate:self];
		[parser setShouldProcessNamespaces:NO];
		[parser setShouldReportNamespacePrefixes:NO];
		[parser setShouldResolveExternalEntities:NO];

		[parser parse];

		if ([parser parserError])
			fatal("Error: error parsing font file: %s error: %s", [fntPath UTF8String], [[[parser parserError] localizedDescription] UTF8String]);

		[parser release];

		// initializing other stuff
		spriteVertices[0] = -1.0f;
		spriteVertices[1] = 1.0f; // links oben
		spriteVertices[2] = 1.0f;
		spriteVertices[3] = 1.0f; // rechts oben
		spriteVertices[4] = -1.0f;
		spriteVertices[5] = -1.0f;
		spriteVertices[6] = 1.0f;
		spriteVertices[7] = -1.0f;

		[self setScale:1.0f];
		[self setRotation:0];
		[self setColor:vector4f(0.7, 0.7, 0.7, 1.0)];

		for (int i = 0; i < BMMAXSTRINGLENGTH; i++)
		{
			indices[i*6] = i*4;
			indices[i*6+1] = i*4+1;
			indices[i*6+2] = i*4+2;
			indices[i*6+3] = i*4+2;
			indices[i*6+4] = i*4+1;
			indices[i*6+5] = i*4+3;
		}

		current = 0;
	}

	return self;
}

- (void)render
{
	// we have some implicit preconditions here:	glEnable(GL_BLEND); glEnable(GL_TEXTURE_2D); glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
	glPushMatrix();

	if (rotation)	glRotatef(rotation, 0, 0, 1);
	if (scale)		glScalef(scale, scale, scale);


	myBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, texName);

	myClientStateVTN(kNeedEnabled, kNeedEnabled, kNeedDisabled);

	myColor(color[0], color[1], color[2], color[3]);

	glVertexPointer(2, GL_FLOAT, 0, spriteVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, spriteTexcoords);

	glDrawElements(GL_TRIANGLES, 6*current, GL_UNSIGNED_BYTE, indices);

	glPopMatrix();

	current = 0;

	globalInfo.drawCalls++;
}

- (void)addString:(char *)stringToRender atPosition:(CGPoint)position
{
	uint16_t i, len = current + strlen(stringToRender) >= BMMAXSTRINGLENGTH ? BMMAXSTRINGLENGTH - current: strlen(stringToRender);
	float x = position.x, y = position.y, xoffset = x / scale, yoffset = y / scale;

	for(i = 0; i < len; i++)
	{
		int currentChar = *(stringToRender+i);

		if ((chars[currentChar].y_ofs == 0) && (currentChar == 32))
		{
			xoffset += infoCommonBase;
			NSLog(@"BMFont:warning simulating space, which is missing in font file");
			continue;
		}
		else if(chars[currentChar].y_ofs == 0) { // checking if a bmfontchar could be found for the request ascii char
			NSLog(@"BMFont:print: WARNING no BMFontChar found for ASCII %d", (int)currentChar);
			continue;
		}

		if (chars[currentChar].w > 0) {
			// links oben
			spriteTexcoords[0 + (current*8)] = (float) chars[currentChar].x / (float)infoCommonScaleWidth;
			spriteTexcoords[1 + (current*8)] = (float) 1.0 - ( chars[currentChar].y / (float) infoCommonScaleHeight);
			spriteVertices[0 + (current*8)] = (float) chars[currentChar].x_ofs + xoffset;
			spriteVertices[1 + (current*8)] = (float) (chars[currentChar].h) + yoffset;

			// links unten
			spriteTexcoords[2 + (current*8)] = (float) chars[currentChar].x / (float) infoCommonScaleWidth;
			spriteTexcoords[3 + (current*8)] = (float) 1.0 - ((chars[currentChar].y + chars[currentChar].h) / (float) infoCommonScaleHeight);
			spriteVertices[2 + (current*8)] = (float) chars[currentChar].x_ofs + xoffset;
			spriteVertices[3 + (current*8)] = (float) yoffset;

			// rechts oben
			spriteTexcoords[4 + (current*8)] = (float) (chars[currentChar].x + chars[currentChar].w) / (float) infoCommonScaleWidth;
			spriteTexcoords[5 + (current*8)] =  (float) 1.0 - ( chars[currentChar].y / (float) infoCommonScaleHeight);
			spriteVertices[4 + (current*8)] = (float) chars[currentChar].w + chars[currentChar].x_ofs + xoffset;
			spriteVertices[5 + (current*8)] = (float) chars[currentChar].h + yoffset;

			// rechts unten
			spriteTexcoords[6 + (current*8)] = (float) (chars[currentChar].x + chars[currentChar].w) / (float) infoCommonScaleWidth;
			spriteTexcoords[7 + (current*8)] = (float) 1.0 - ((chars[currentChar].y + chars[currentChar].h) / (float) infoCommonScaleHeight);
			spriteVertices[6 + (current*8)] = (float) chars[currentChar].w + chars[currentChar].x_ofs + xoffset;
			spriteVertices[7 + (current*8)] = (float) yoffset;

			xoffset += (float)(chars[currentChar].x_advance - chars[currentChar].x_ofs);
			current++;
		}
		else  // if char has no width, treat it like a space
			xoffset += (float)(chars[currentChar].x_advance - chars[currentChar].x_ofs);
	}
}

- (float)getLengthOfString:(char *)string
{
	float length = 0;
	uint16_t len = strlen(string);
	uint16_t i;

	for(i = 0; i < len; i++)
	{
		int currentChar = *(string+i);

		if ((chars[currentChar].y_ofs == 0) && (currentChar == 32))
			length += infoCommonBase;
		else if(chars[currentChar].y_ofs == 0)
			continue;
		else
		{
			if (i == (len - 1))
				length += chars[currentChar].w;
			else
				length += (float)(chars[currentChar].x_advance - chars[currentChar].x_ofs);
		}
	}

	return length * scale;
}

- (void)dealloc
{
	[infoFace release];

	[super dealloc];
}


- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict
{
	if ([elementName isEqualToString:@"info"])
	{
		infoFace = [attributeDict valueForKey:@"face"];
		infoFontSize = [[attributeDict valueForKey:@"size"] intValue];

		//NSLog(@"font face:%@ size:%d",infoFace,infoFontSize);
	}

	if ([elementName isEqualToString:@"common"])
	{
		infoCommonLineHeight = [[attributeDict valueForKey:@"lineHeight"] intValue];
		infoCommonBase = [[attributeDict valueForKey:@"base"] intValue];
		infoCommonScaleWidth = [[attributeDict valueForKey:@"scaleW"] intValue];
		infoCommonScaleHeight = [[attributeDict valueForKey:@"scaleH"] intValue];

		//NSLog(@"font common lineheight:%d base:%d scalewidth:%d scaleheight:%d",infoCommonLineHeight,infoCommonBase,infoCommonScaleWidth,infoCommonScaleHeight);
	}

	if ([elementName isEqualToString:@"char"])
	{
		NSString *charID = [attributeDict valueForKey:@"id"];
		int cid = [charID intValue];

		if (cid >= BMHIGHESTPOSSIBLECHAR)
		{
			NSLog(@"Warning: only ASCII supported in the font file");
			return;
		}

		chars[cid].x = [[attributeDict valueForKey:@"x"] intValue];
		chars[cid].y = [[attributeDict valueForKey:@"y"] intValue];
		chars[cid].w = [[attributeDict valueForKey:@"width"] intValue];
		chars[cid].h = [[attributeDict valueForKey:@"height"] intValue];
		chars[cid].x_ofs = [[attributeDict valueForKey:@"xoffset"] intValue];
		chars[cid].y_ofs = [[attributeDict valueForKey:@"yoffset"] intValue];
		chars[cid].x_advance = [[attributeDict valueForKey:@"xadvance"] intValue];

		//NSLog(@"BMFont added char width ID:%@ x:%d y:%d w:%d h:%d xoffset:%d yoffset:%d xadvance:%d",charID,newChar.x,newChar.y,newChar.w,newChar.h,newChar.x_ofs,newChar.y_ofs,newChar.x_advance);
	}
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
}
@end

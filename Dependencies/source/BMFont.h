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

#define BMMAXSTRINGLENGTH 256
#define BMHIGHESTPOSSIBLECHAR 256

typedef struct _BMFontChar {
	GLuint		x,y,w,h;
	GLint		x_ofs,y_ofs;
	GLuint		x_advance;
} BMFontChar;

@protocol NSXMLParserDelegate;


@interface BMFont : NSObject <NSXMLParserDelegate> {
	NSString				*infoFace;
	BMFontChar				chars[BMHIGHESTPOSSIBLECHAR];
	uint32_t				infoFontSize, infoCommonLineHeight,	infoCommonBase, infoCommonScaleWidth, infoCommonScaleHeight;

	float					scale;
	float					rotation;
	vector4f				color;

	uint8_t					current;
	GLuint					texName;

	GLfloat					spriteVertices[8 * BMMAXSTRINGLENGTH];
	GLfloat					spriteTexcoords[8 * BMMAXSTRINGLENGTH];
}

@property (assign, nonatomic) vector4f color;
@property (assign, nonatomic) float scale;
@property (assign, nonatomic) float rotation;
@property (readonly, nonatomic) uint32_t infoCommonLineHeight;

- (id)initWithFontNamed:(NSString *)fontName andTextureNamed:(NSString *)textureName;
- (void)render;
- (void)addString:(char *)stringToRender atPosition:(CGPoint)position;
- (float)getLengthOfString:(char *)string;

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName attributes:(NSDictionary *)attributeDict;
- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName;
- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string;

@end
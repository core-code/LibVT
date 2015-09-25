#!/usr/bin/env python
#
#  mergeVirtualTextureTiles.py
#  VirtualTexturing
#
#  Created by Julian Mayer on 19.01.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 

import sys, os, math, shutil, math

def print_exit(s): print s;	sys.exit()

try:		import Image
except:		print_exit("Error: Python Imaging Library or later not found")

border = 0
levels = 0
output = ""
inputfolders = []


try:
	for i in range(1, len(sys.argv)):
		if sys.argv[i].startswith("-l="):	levels = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-o="):	output = sys.argv[i][3:]
		elif sys.argv[i].startswith("-b="):	border = int(sys.argv[i][3:])
		else:
			inputfolders.append(sys.argv[i])
except e:
	print e
	print_exit("""Usage: %s -o=<output_folder> -l=<levels_the_inputfolders_have> -b=<border> input_folders (4, 16 or 64)""")

if (levels == 0 or output == "" or (len(inputfolders) != 4 and len(inputfolders) != 16 and len(inputfolders) != 64)):
	print_exit("""Usage: %s -o=<output_folder> -l=<levels_the_inputfolders_have> -b=<border> input_folders (4, 16 or 64)""")

if (len(inputfolders) == 4): bla = 2
if (len(inputfolders) == 16): bla = 1
if (len(inputfolders) == 64): bla = 0

sideLen = int(math.sqrt(len(inputfolders)))
maxTiles = 2**(levels - 1)
tilesStr = "/tiles_b" + str(border) + "_level"
tileSize = Image.open(inputfolders[0] + tilesStr + "0/tile_0_0_0.bmp").size[0] - border*2

for level in range(0, levels+1+(2-bla)):
	os.mkdir(output + tilesStr + str(level))

#copy existing files
for x in range(0, sideLen):
	for y in range(0, sideLen):
		for level in range(0, levels):
			for tx in range(0, maxTiles >> level):
				for ty in range(0, maxTiles >> level):
					shutil.copyfile(inputfolders[y * sideLen + x] + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str(tx) + "_" + str(ty) + ".bmp",
									output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str(tx + x * (maxTiles >> level)) + "_" + str(ty + y * (maxTiles >> level))  + ".bmp")

#fixup borders in existing files
maxTiles = 2**(levels + 2)
if border:
	for level in range(0, levels): #even i don't understand this code (anymore)
		for i in range(0, maxTiles >> (level + bla)):
			for rowcell in range(1, sideLen):
				l_im = Image.open(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen - 1) + "_" + str(i)  + ".bmp")
				r_im = Image.open(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen) + "_" + str(i)  + ".bmp")
				l_im.paste(r_im.crop((border, 0, border*2, tileSize + border*2)), (tileSize + border, 0, tileSize + border*2, tileSize + border*2))
				r_im.paste(l_im.crop((tileSize, 0, tileSize + border, tileSize + border*2)), (0, 0, border, tileSize + border*2))
				l_im.save(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen - 1) + "_" + str(i)  + ".bmp")
				r_im.save(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen) + "_" + str(i)  + ".bmp")

				t_im = Image.open(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str(i) + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen - 1)  + ".bmp")
				b_im = Image.open(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str(i) + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen)  + ".bmp")
				t_im.paste(b_im.crop((0, border, tileSize + border*2, border*2,)), (0, tileSize + border, tileSize + border*2, tileSize + border*2))
				b_im.paste(t_im.crop((0, tileSize, tileSize + border*2, tileSize + border)), (0, 0, tileSize + border*2,  border))
				t_im.save(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str(i) + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen - 1)  + ".bmp")
				b_im.save(output + tilesStr + str(level) + "/tile_" + str(level)  + "_" + str(i) + "_" + str((maxTiles >> (level + bla)) * rowcell / sideLen)  + ".bmp")

#generate new levels
size = tileSize*sideLen
im = Image.new(mode='RGB', size=(size, size))
for x in range(0, sideLen):
	for y in range(0, sideLen):
		tile = Image.open(output + tilesStr + str(levels-1) + "/tile_" + str(levels-1)  + "_" + str(x) + "_" + str(y)  + ".bmp")
		im.paste(tile.crop((border, border, tileSize + border, tileSize + border)), (x*tileSize, y*tileSize, (x+1)*tileSize, (y+1)*tileSize))

size /= 2
clevel = levels
while size >= tileSize:
	im = im.resize((size, size), Image.ANTIALIAS)

	baseDir = output + tilesStr + str(clevel)
	len = size / tileSize

	for x in range(len):
		for y in range(len):
			if (not border):
				part = im.crop((x * tileSize , y * tileSize , x * tileSize + tileSize , y * tileSize + tileSize))
			else:
				part = im.crop((x * tileSize - border, y * tileSize - border, x * tileSize + tileSize + border, y * tileSize + tileSize + border))
				if (x == 0):		part.paste(part.crop((border, 0, border*2, tileSize + border*2)), (0, 0, border, tileSize + border*2))
				if (y == 0):		part.paste(part.crop((0, border, tileSize + border*2, border*2)), (0, 0, tileSize + border*2, border))
				if (x == len-1):	part.paste(part.crop((tileSize, 0, tileSize + border, tileSize + border*2)), (tileSize + border, 0, tileSize + border*2, tileSize + border*2))
				if (y == len-1):	part.paste(part.crop((0, tileSize, tileSize + border*2, tileSize + border)), (0, tileSize + border, tileSize + border*2, tileSize + border*2))

			part.save(baseDir + "/" + "tile_" + str(clevel) + "_" + str(x) + "_" + str(y) + ".bmp")
	size /= 2
	clevel += 1

#!/usr/bin/env python
#
#  convertVirtualTextureTiles.py
#  VirtualTexturing
#
#  Created by Julian Mayer on 2.05.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 
import os, sys, subprocess

keepOriginals = 0
formatIn = "bmp"
formatOut = "jpg"
border = 0

def print_exit(s): print s;	sys.exit()

dir = sys.argv[len(sys.argv) - 1]

try:
	if (len(sys.argv)) == 1:					raise Exception('input', 'error')
	for i in range(1, len(sys.argv) - 1):
		if sys.argv[i].startswith("-k"):		keepOriginals = 1
		elif sys.argv[i].startswith("-b="):		border = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-fi="):	formatIn = sys.argv[i][4:]
		elif sys.argv[i].startswith("-fo="):	formatOut = sys.argv[i][4:]
		else:									raise Exception('input', 'error')
	if formatIn == formatOut:					raise Exception('input', 'error')
except:
	print_exit("""Usage: %s [options] <path_to_tile_store>
Options:
  -k\t\t  Keep original files
  -b=<border>\t  The tiles have a border of <border> pixels (Default: 0)
  -fi=<format>\t  The tiles are in format <bmp, png> (Default: bmp)
  -fo=<format>\t  Transcode the files to <bmp, png, jpg> (Default: jpg)""" % (sys.argv[0]))


if (not os.path.isdir(dir + '/tiles_b' + str(border) + '_level0')):
	print_exit("Error: folder doesn't exist: " + dir + "/tiles_b" + str(border) + "_level0")

levels = 0
side = 1

for i in range(11):
	if (os.path.isdir(dir + '/tiles_b' + str(border) + '_level' + str(i))):
		levels += 1

for i in range(levels)[::-1]:
	for x in range(side):
		for y in range(side):
			file = dir + '/tiles_b' + str(border) + '_level' + str(i) + "/" + "tile_" + str(i) + "_" + str(x) + "_" + str(y) + "." + formatIn
			subprocess.call("mogrify -format " + formatOut  + " -type TrueColor " + file, shell=True)
			if (not keepOriginals):
				os.remove(file)
	side *= 2

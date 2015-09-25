#!/usr/bin/env python
#
#  generateTextureAtlas.py
#  VirtualTexturing
#
#  Created by Julian Mayer on 27.01.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
import sys, os, math, subprocess, glob


#TODO: implement the "Original NOT adjusted, atlas adjusted" --halftexel option as described in "Using Coordinates in the Zero-to-One Range" chapter in the NVIDIA Texture Atlas Whitepaper
convertDir = ""
outDir = "."
size = 16384
imageNumSide = 1
inputImages = []

class PackNode(object): # PackNode class by Simon Wittber
    def __init__(self, area):
        if len(area) == 2:
            area = (0,0,area[0],area[1])
        self.area = area

    def __repr__(self):
        return "<%s %s>" % (self.__class__.__name__, str(self.area))

    def required_sort_order(self):
		return "h" #sort input images by 'h'eight, descending

    def get_width(self):
        return self.area[2] - self.area[0]
    width = property(fget=get_width)

    def get_height(self):
        return self.area[3] - self.area[1]
    height = property(fget=get_height)

    def insert(self, area):
        if hasattr(self, 'child'):
            a = self.child[0].insert(area)
            if a is None: return self.child[1].insert(area)
            return a

        area = PackNode(area)
        if area.width <= self.width and area.height <= self.height:
            self.child = [None,None]
            self.child[0] = PackNode((self.area[0]+area.width, self.area[1], self.area[2], self.area[1] + area.height))
            self.child[1] = PackNode((self.area[0], self.area[1]+area.height, self.area[2], self.area[3]))
            return PackNode((self.area[0], self.area[1], self.area[0]+area.width, self.area[1]+area.height))



def print_exit(s): print s;	sys.exit()

def rectIntersectsRect(r1, r2):
	return not(r1[0] > r2[2] or r1[2] < r2[0] or r1[1] > r2[3] or r1[3] < r2[1])

try:		import Image
except:		print_exit("Error: Python Imaging Library or later not found")

try:
	for i in range(1, len(sys.argv)):
		if sys.argv[i].startswith("-d="):	outDir = sys.argv[i][3:]
		elif sys.argv[i].startswith("-c="):
			convertDir = sys.argv[i][3:] + "/"
			if (not os.path.isfile(convertDir + "convert" + (".exe" if os.name == 'nt' else  ""))): print_exit(convertDir + "convert doesn't exist!")
		elif sys.argv[i].startswith("-s="):	size = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-n="):	imageNumSide = 2 ** int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-i="):
			data = sys.argv[i][3:].split(":")
			xn = int(data[0])
			yn = int(data[1])
			imagePaths = data[2:]
			if len(imagePaths) != xn * yn: print_exit("must specify <x> * <y> images in -i= option")
			images = []
			agg_height = 0
			max_row_width = 0
			for x in range(xn):
				height = Image.open(imagePaths[x * xn]).size[1]
				agg_height += height
				row_width = 0
				for y in range(yn):
					im = Image.open(imagePaths[x * xn + y])
					if (im.size[1] != height): print_exit("all images in a row must have the same height for the -i= option")
					row_width += im.size[0]
					images.append({'path': imagePaths[x * xn + y], 'size': im.size, 'pixels': im.size[0] * im.size[1]})
				if row_width > max_row_width: max_row_width = row_width
			inputImages.append({'x': xn, 'y': xn, 'images': images, 'size': (max_row_width, agg_height), 'pixels': max_row_width * agg_height})
		else:
			expanded = glob.glob(sys.argv[i])
			for imagename in expanded:
				im = Image.open(imagename)
				inputImages.append({'path': imagename, 'size': im.size, 'pixels': im.size[0] * im.size[1]})
	if len(inputImages) == 0: raise()
except SystemExit:
	raise
except:
	print_exit("""Usage: %s [options] input_image_1 input_image_2 ...
Options:
  -c=<convert_bin_path>\tPath to the directory where ImageMagick's convert command resides
  -d=<output_dir>\tDirectory to place resulting files (_texture_atlas*)
  -s=<size>\t\tSet the size of the output image
  -n=<number>\t\tProduce 4^<number> output images
  -i=<x>:<y>:<file_x0_y0>:<file_x1_y0>:...\n\t\t\tJuxtapose these images in the atlas in a <x> by <y> grid""" % (sys.argv[0]))


pixel = 0
for image in inputImages:
	pixel += image['pixels']

print "Minimum atlas dimension:"
print math.sqrt(pixel)

tree = PackNode((size, size))
so = tree.required_sort_order()
if so == "h": inputImages.sort(cmp=lambda a, b:cmp(b['size'][1], a['size'][1]))
elif so == "w": inputImages.sort(cmp=lambda a, b:cmp(b['size'][0], a['size'][0]))
elif so == "s": inputImages.sort(cmp=lambda a, b:cmp(b['size'][0] * b['size'][1], a['size'][0] * a['size'][1]))

for image in inputImages:
	uv = tree.insert(image['size'])
	if uv is None:
		raise ValueError('Pack size too small. Faulty item: ' + str(image) + ' ' + str(inputImages.index(image)) + ' of ' + str(len(inputImages)))
	image['area'] = uv.area


for image in inputImages:
	if 'images' in image:
		current_height = 0
		for x in range(image['x']):
			current_width = 0
			for y in range(image['y']):
				current_image = image['images'][x * image['x'] + y]
				tmp = [image['area'][0] + current_width, image['area'][1] + current_height]
				inputImages.append({'path': current_image['path'], 'area': (tmp[0], tmp[1], tmp[0] + current_image['size'][0], tmp[1] + current_image['size'][1])})
				current_width += current_image['size'][0]
			current_height += image['images'][x * image['x']]['size'][1]
		inputImages.remove(image)


for x in range(imageNumSide):
	for y in range(imageNumSide):
		command = "\"" + convertDir + "convert\" -size " + str(size / imageNumSide) + "x" + str(size / imageNumSide) + " xc:white "

		outRect = [x * size / imageNumSide, y * size / imageNumSide, (x+1) * size / imageNumSide, (y+1) * size / imageNumSide]

		for image in inputImages:
			if rectIntersectsRect(image['area'], outRect):
				xcoord = image['area'][0] - outRect[0]
				xstr = ("+" if xcoord >= 0 else "") + str(xcoord)
				ycoord = image['area'][1] - outRect[1]
				ystr = ("+" if ycoord >= 0 else "") + str(ycoord)
				command += "\"" + image['path'] + "\"" + " -geometry " + xstr + ystr + " -composite "

		command += " " + outDir + "/_texture_atlas" + str(x) + str(y) + ".bmp"
		#print "command for image x/y " + str(y) + " " + str(y)
		#print command
		subprocess.call(command, shell=False)

f = open(outDir + "/_texture_atlas_offset.off", 'w')
for image in inputImages:
	xoff = image['area'][0] / float(size)
	yoff = 1.0 - (image['area'][3] / float(size)) #offset calculation is a bit weird since we must convert to lower left coordinates
	xfactor = (image['area'][2] - image['area'][0]) / float(size)
	yfactor = (image['area'][3] - image['area'][1]) / float(size)
	f.write(os.path.basename(image['path']) + " " + str(xoff) + " " + str(yoff) + " " + str(xfactor) + " " + str(yfactor) + "\n")

print "Success!"

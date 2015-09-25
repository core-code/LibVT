#!/usr/bin/env python
#
#  offsetObjTexcoords.py
#  VirtualTexturing
#
#  Created by Julian Mayer on 30.01.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 

import os, sys

outDir = "."
offsets_tex = {}
offsets_mat = {}
out = []
transformed = 0

try:
	if (len(sys.argv)) == 1:					raise Exception('input', 'error')
	for i in range(1, len(sys.argv)):
		if sys.argv[i].startswith("-d="):		outDir = sys.argv[i][3:]
		elif sys.argv[i].startswith("-o="):		objpath = sys.argv[i][3:]
		elif sys.argv[i].startswith("-t="):		t = open(sys.argv[i][3:], 'r')
		else:									raise Exception('input', 'error')
except:
	print "Usage: offsetObjTexcoords -o=<obj_file> -t=<off_file> -d=<output_dir>"
	sys.exit()


o = open(objpath, 'r')
f = open(os.path.join(outDir, os.path.splitext(os.path.basename(objpath))[0] + "_transformed.obj"), 'w')

lines = t.readlines()
for line in lines:
	items = line.strip().split(" ")
	offsets_tex[items[0]] = items[1:]

lines = o.readlines()
for line in lines:
	items = line.strip().split(" ")
	if items[0] == "mtllib":
		m = open(os.path.join(os.path.split(objpath)[0], items[1]), 'r')
		newmtlname = os.path.splitext(items[1])[0] + "_transformed.mtl"
		fmtl = open(os.path.join(outDir, newmtlname), 'w')
		break

mlines = m.readlines()
current_material = ""
for line in mlines:
	items = line.strip().split(" ")
	if items[0] == "newmtl":
		current_material = items[1]
	elif items[0] == "map_Kd":
		if items[1] in offsets_tex:
			offsets_mat[current_material] = offsets_tex[items[1]]
		else:
			print "warning texture in material file which has no offset specified in offset file: " + items[1]
			print objpath

numvts = 0
vts = []
newvts = []
vtshash = {} # the hashes prevent quadratic runtime with catasthropic results even on medium size files
newvtshash = {}
duplicatevts = {}

for line in lines:
	items = line.strip().split(" ")
	if items[0] == "vt":
		numvts += 1

current_material = ""
for line in lines:
	items = line.strip().split(" ")
	if items[0] == "usemtl":
		current_material = items[1]
	elif items[0] == "vt":
		key = (float(items[1]), float(items[2]))
		if (key in vtshash):	#we already have a identical texture coord, this makes problems with our algorithm
			print "warning duplicate texcoord detected, writing dummy value"
			vts.append([666.666, 666.666])
			duplicatevts[len(vts) - 1] = vtshash[key]
		else:
			vts.append([key[0], key[1]])
			vtshash[key] = len(vts) - 1
	elif items[0] == "mtllib":
		out.append("mtllib " + newmtlname + "\n")
	elif items[0] == "f":
		if (current_material in offsets_mat):
			offsets = offsets_mat[current_material]
			out.append("f")
			for item in items[1:]:
				data = item.split("/")
				if len(data[1]) > 0:
					tcnum = int(data[1]) - 1
					if tcnum in duplicatevts:
						tcnum = duplicatevts[tcnum]
					if (len(vts[tcnum]) == 2):
						del vtshash[(vts[tcnum][0], vts[tcnum][1])]
						vtshash[(vts[tcnum][0] * float(offsets[2]) + float(offsets[0]), vts[tcnum][1] * float(offsets[3]) + float(offsets[1]))] = tcnum
						vts[tcnum] = [vts[tcnum][0] * float(offsets[2]) + float(offsets[0]), vts[tcnum][1] * float(offsets[3]) + float(offsets[1]), vts[tcnum][0], vts[tcnum][1]]
						transformed += 1
						out.append(" " + item)
					else:
						key = (vts[tcnum][2] * float(offsets[2]) + float(offsets[0]), vts[tcnum][3] * float(offsets[3]) + float(offsets[1]))
						normal = "/" + str(data[2]) if len(data) > 2 else ""
						if key in vtshash:
							out.append(" " + str(data[0]) + "/" + str(vtshash[key] + 1) + normal)
						else:
							if key in newvtshash:
								out.append(" " + str(data[0]) + "/" + str(numvts + newvtshash[key] + 1) + normal)
							else:
								newvts.append([key[0], key[1]])
								newvtshash[key] = len(newvts) - 1
								transformed += 1
								out.append(" " + str(data[0]) + "/" + str(numvts + len(newvts)) + normal)
				else:
					out.append(" " + item)
			out.append("\n")
		else:
			out.append(line)
	else:
		out.append(line)

vts.extend(newvts)
for vt in vts:
	f.write("vt " + str(vt[0]) + " " + str(vt[1]) + "\n")

f.write("usemtl mat_texture_atlas\n")

for line in out:
	f.write(line)

fmtl.write("newmtl mat_texture_atlas\nKa 0.200000 0.200000 0.200000\nKd 0.498039 0.498039 0.498039\nKs 1.000000 1.000000 1.000000\nmap_Kd _texture_atlas00.bmp\n")

print "SUCCESS: transformed texcoords: " + str(transformed)

#!/usr/bin/env python
#
#  mergeObj.py
#
#  Created by Julian Mayer on 25.01.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 

import os, sys

outobj = []
outmat = []
materials1 = []
materials_repl = {}
vecs = 0
normals = 0
texcoords = 0
im1 = 0
im2 = 0

try:
	if (len(sys.argv)) == 1:						raise Exception('input', 'error')
	of = open(sys.argv[len(sys.argv) - 1], 'w')
	om = open(sys.argv[len(sys.argv) - 1][:sys.argv[len(sys.argv) - 1].rfind(".")] + ".mtl", 'w')
	for i in range(1, len(sys.argv) - 1):
		if sys.argv[i].startswith("-io1="):				io1 = open(sys.argv[i][5:], 'r')
		elif sys.argv[i].startswith("-im1="):			im1 = open(sys.argv[i][5:], 'r')
		elif sys.argv[i].startswith("-io2="):			io2 = open(sys.argv[i][5:], 'r')
		elif sys.argv[i].startswith("-im2="):			im2 = open(sys.argv[i][5:], 'r')
		else:											raise Exception('input', 'error')
except:
	print """Usage: mergeObj [options] output_obj_file
Options:
 -io1=<input_obj_file_1>	Input Obj file #1 (mandatory)
 -im1=<input_mtl_file_1>	Input Material file #1
 -io2=<input_obj_file_2>	Input Obj file #2 (mandatory)
 -im2=<input_mtl_file_2>	Input Material file #2"""
	sys.exit()

if (im1 != 0):
	lines = im1.readlines()
	for line in lines:
		first = line.strip().split(" ")[0]
		if first == "newmtl":
			materials1.append(line.strip().split(" ")[1])
		outmat.append(line)

	outmat.append("\n")

if (im2 != 0):
	lines = im2.readlines()
	for line in lines:
		first = line.strip().split(" ")[0]
		if first == "newmtl" and line.strip().split(" ")[1] in materials_repl:
			outmat.append("newmtl " + materials_repl[line.strip().split(" ")[1]] + "\n")
		elif first == "newmtl" and materials1.count(line.strip().split(" ")[1]) > 0:
			newname = line.strip().split(" ")[1]
			while materials1.count(newname) > 0:
				newname = newname + "_"
			materials_repl[line.strip().split(" ")[1]] = newname
			outmat.append("newmtl " + newname + "\n")
		else:
			outmat.append(line)

outobj.append("\nmtllib ./" + os.path.basename(sys.argv[len(sys.argv) - 1][:sys.argv[len(sys.argv) - 1].rfind(".")]) + ".mtl\n\n")

lines = io1.readlines()
for line in lines:
	i = line.strip().split(" ")[0]
	if i == "v":
		vecs += 1
	elif i == "vn":
		normals += 1
	elif i == "vt":
		texcoords += 1
	if i != "mtllib":
		outobj.append(line)

outobj.append("\n")

lines = io2.readlines()
for line in lines:
	i = line.strip().split(" ")[0]
	c = line[2:].strip().split(" ")
	if i == "f":
		c = line[2:].strip().split(" ")
		if (len(c) > 4):
			print "Error: faces with more than 3 edges not supported in second obj file"
			sys.exit()
		c0s = c[0].split("/")
		if len(c0s) < 3: c0s.append("")
		c1s = c[1].split("/")
		if len(c1s) < 3: c1s.append("")
		c2s = c[2].split("/")
		if len(c2s) < 3: c2s.append("")
		outobj.append("f " + 	str(int(c0s[0]) + vecs) + "/" + ("" if c0s[1] == "" else str(int(c0s[1]) + texcoords)) + ("" if c0s[2] == "" else ("/" + str(int(c0s[2]) + normals))) + " " + \
					str(int(c1s[0]) + vecs) + "/" + ("" if c1s[1] == "" else str(int(c1s[1]) + texcoords)) + ("" if c1s[2] == "" else ("/" + str(int(c1s[2]) + normals))) + " " +
					str(int(c2s[0]) + vecs) + "/" + ("" if c2s[1] == "" else str(int(c2s[1]) + texcoords)) + ("" if c2s[2] == "" else ("/" + str(int(c2s[2]) + normals))) + " " +  "\n")
	elif i == "usemtl" and line.strip().split(" ")[1] in materials_repl:
		outobj.append("usemtl " + materials_repl[line.strip().split(" ")[1]] + "\n")
	elif i != "mtllib":
		outobj.append(line)

for line in outobj:
	of.write(line)

for line in outmat:
	om.write(line)
#!/usr/bin/env python
#
#  transformObj.py
#
#  Created by Julian Mayer on 23.01.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 

import sys, math

tr = [0.0, 0.0, 0.0]
ro = [0.0, 0.0, 0.0]
outlines = []
mat = [[1, 0, 0], [0, 1, 0], [0, 0, 1]]

def add(a,b):
	return [a[0]+b[0], a[1]+b[1], a[2]+b[2]]
def rad(a):
	return a * math.pi / 180.0
def deg(a):
	return a * 180.0 / math.pi
def transform(v, m):
	return [m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2], m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2], m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2]]
def mat_rot_x(angle):
	return [[1, 0, 0], [0, math.cos(angle), -math.sin(angle)], [0, math.sin(angle), math.cos(angle)]]
def mat_rot_y(angle):
	return [[math.cos(angle), 0, math.sin(angle)], [0, 1, 0],[-math.sin(angle), 0, math.cos(angle)]]
def mat_rot_z(angle):
	return [[math.cos(angle), -math.sin(angle), 0], [math.sin(angle), math.cos(angle), 0], [0, 0, 1]]
def mat_mul(m1, m2):
	res = [[0, 0, 0], [0, 0, 0], [0, 0, 0]]
	for r in range(0, 3):
		for s in range(0, 3):
			for i in range(0, 3):
				res[r][s] = res[r][s] + m1[r][i] * m2[i][s]
	return res

try:
	if (len(sys.argv)) == 1:						raise Exception('input', 'error')
	f = open(sys.argv[len(sys.argv) - 1], 'r')
	of = 0
	for i in range(1, len(sys.argv) - 1):
		if sys.argv[i].startswith("-x="):				tr[0] = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-y="):				tr[1] = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-z="):				tr[2] = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-xr="):			ro[0] = float(sys.argv[i][4:])
		elif sys.argv[i].startswith("-yr="):			ro[1] = float(sys.argv[i][4:])
		elif sys.argv[i].startswith("-zr="):			ro[2] = float(sys.argv[i][4:])
		else:											raise Exception('input', 'error')
	if of == 0:											of = open(sys.argv[len(sys.argv) - 1][:sys.argv[len(sys.argv) - 1].rfind(".")] + "_new.obj", 'w')
except:
	print """Usage: transformObj [options] obj_file
Options:
 -x=<x_translation>	Translate Obj-File by <x_translation> in along the X axis
 -y=<y_translation>	Translate Obj-File by <y_translation> in along the Y axis
 -z=<z_translation>	Translate Obj-File by <z_translation> in along the Z axis
 -xr=<x_rotation>	Rotate Obj-File by <x_rotation> around X axis (degrees)
 -yr=<y_rotation>	Rotate Obj-File by <y_rotation> around Y axis (degrees)
 -zr=<z_rotation>	Rotate Obj-File by <z_rotation> around Z axis (degrees)"""
	sys.exit()


mat = mat_mul(mat_mul(mat_rot_x(rad(ro[0])), mat_rot_y(rad(ro[1]))), mat_rot_z(rad(ro[2])))

lines = f.readlines()
for line in lines:
	i = line.strip().split(" ")[0]
	c = line[2:].strip().split(" ")
	if i == "v" or i == "vn":
		vec = [float(c[0]), float(c[1]), float(c[2])]
		if i == "v": vec = add(vec, tr)
		vec = transform(vec, mat)
		outlines.append(i + " " + str(vec[0]) + " " +  str(vec[1]) + " " +  str(vec[2]) + "\n")
	else:
		outlines.append(line)

for line in outlines:
	of.write(line)
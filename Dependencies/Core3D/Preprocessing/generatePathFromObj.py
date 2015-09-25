#!/usr/bin/env python
#
#  generatePathFromObj.py
#  Core3D
#
#  Created by Julian Mayer on 16.12.07.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


import sys, random
from struct import *
from vecmath import *

try:
	from numpy import *
	from scipy.interpolate import splprep, splev
except:
	print "Error: NumPy or SkiPy not found"
	sys.exit()

x = []
y = []
z = []
SCALE = 1.0
numberOfPoints = 3600
pointsPerUnit = 0
splineOrder = 2
smoothnessParameter = 3.0

try:
	if (len(sys.argv)) == 1:							raise Exception('input', 'error')
	f = open(sys.argv[len(sys.argv) - 1], 'r')
	of = 0
	for i in range(1, len(sys.argv) - 1):
		if sys.argv[i].startswith("-p="):				smoothnessParameter = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-i="):				splineOrder = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-s="):				SCALE = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-f="):				numberOfPoints = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-u="):				pointsPerUnit = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-o="):				of = open(sys.argv[i][3:], 'w')
		else:											raise Exception('input', 'error')
	if of == 0:											of = open(sys.argv[len(sys.argv) - 1][:sys.argv[len(sys.argv) - 1].rfind(".")] + ".path", 'w')
except:
	print """Usage: generateInterpolatedPathFromObj [options] obj_file
Options:
 -s=<scale>		Scale all coordinates by <scale>
 -p=<smoothness>	Use <smoothness> as smoothness parameter (Default: 3.0)
 -i=<spline_order>	Use interpolation of order <spline_order> (Default: 2)
 -f=<num_points>	Produce <num_points> points (Default: 3600)
 -u=<points_per_unit>	Produce <points_per_unit> points per unit of path length
 -o=<path_file>		Place the output path into <path_file>"""
	sys.exit()

lines = f.readlines()
for line in lines:
	c = line.split(" ")
	if c[0] == "v":
		x.append(SCALE * float(c[1]))
		y.append(SCALE * float(c[2]))
		z.append(SCALE * float(c[3]))

x.append(x[0])
y.append(y[0])
z.append(z[0])

if pointsPerUnit != 0:
	distance = 0
	for i in range(len(x)-1):
		prevvec = [x[i], y[i], z[i]]
		vec = [x[i+1], y[i+1], z[i+1]]
		distance += magnitude(substract(vec, prevvec))
	numberOfPoints = distance * pointsPerUnit

tckp,u = splprep([array(x), array(y), array(z)], s=smoothnessParameter, k=splineOrder, nest=-1) # find the knot points

xnew, ynew, znew = splev(linspace(0, 1, numberOfPoints), tckp) # evaluate spline, including interpolated points

out = ""
for i in range(len(xnew)):
	out += pack('fff', xnew[i], ynew[i], znew[i])
of.write(out)



nearest = []

for i in range(len(x)):
	nearest.append([1000, 0])
	for v in range(len(xnew)-1):
		vec1 = [x[i], y[i], z[i]]
		vec2 = [xnew[v], ynew[v], znew[v]]
		dist = magnitude(substract(vec1, vec2))
		if (dist < nearest[i][0]):
			nearest[i] = [dist, v]

random.seed()
for v in range(8):
	xr = []
	yr = []
	zr = []
	fewrandom = []
	manyrandom = []

	for i in range((len(nearest) / 10)):
		fewrandom.append(random.uniform(-8,8))
	fewrandom.append(fewrandom[0])
	fewrandom.append(fewrandom[0])

	for i in range(len(nearest)):
		manyrandom.append(fewrandom[i / 10] * ((10 - (i % 10)) / 10.0) + fewrandom[i / 10 + 1] * ((i % 10) / 10.0))

	for i in range(len(nearest)):
		vec = [0,0,0]
		if (i > 0):
			prevtocurr = subtract([xnew[nearest[i][1]], ynew[nearest[i][1]], znew[nearest[i][1]]], [xnew[nearest[i-1][1]], ynew[nearest[i-1][1]], znew[nearest[i-1][1]]])
			vec = add(vec, prevtocurr)
		if (i < len(nearest) - 1):
			currtonext = subtract([xnew[nearest[i+1][1]], ynew[nearest[i+1][1]], znew[nearest[i+1][1]]], [xnew[nearest[i][1]], ynew[nearest[i][1]], znew[nearest[i][1]]])
			vec = add(vec, currtonext)
		perpendicular = normalize([vec[2], 0, -vec[0]])
		perpendicular = multiply(perpendicular, manyrandom[i])
		xr.append(xnew[nearest[i][1]] + perpendicular[0])
		yr.append(ynew[nearest[i][1]])
		zr.append(znew[nearest[i][1]] + perpendicular[2])

	xr.append(xr[0])
	yr.append(yr[0])
	zr.append(zr[0])
	tckp,u = splprep([array(xr), array(yr), array(zr)], s=smoothnessParameter, k=splineOrder, nest=-1) # find the knot points
	xs, ys, zs = splev(linspace(0, 1, numberOfPoints), tckp) # evaluate spline, including interpolated points

	off = open(sys.argv[len(sys.argv) - 1][:sys.argv[len(sys.argv) - 1].rfind(".")] + ".path" + str(v), 'w')
	out = ""
	for i in range(len(xnew)):
		out += pack('fff', xs[i], ys[i], zs[i])
	off.write(out)
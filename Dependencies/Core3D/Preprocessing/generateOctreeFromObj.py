#!/usr/bin/env python
#
#  generateOctreeFromObj.py
#  Core3D
#
#  Created by Julian Mayer on 16.11.07.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


import sys, bz2
from struct import *
from vecmath import *

TEXTURING = 1
GENTEX = 0
MAX_FLOAT = 1e+308
MIN_FLOAT = -1e308
MAX_USHORT = 0xFFFF
MAX_FACES_PER_TREELEAF = 1000
MAX_RECURSION_DEPTH = 10
SCALE = 1.0
vertices = []
faces = []
normals = []
texcoords = []

def faceContent(f, i):
	if i == 0:
		if f.count("/") == 0: return f
		else: return f[:f.find("/")]
	elif i == 1:
		if f.count("/") == 0 or f.count("//") == 1: return 0
		else:
			if f.count("/") == 2:
				return f[f.find("/")+1:f.rfind("/")]
			else:
				return f[f.find("/")+1:]
	else:
		if f.count("/") != 2: return 0
		else: return f[f.rfind("/")+1:]

def calculateAABB(faces):
	mi = [MAX_FLOAT, MAX_FLOAT,MAX_FLOAT]
	ma =  [MIN_FLOAT, MIN_FLOAT, MIN_FLOAT]
	for face in faces:
		for i in range(3):
			for v in range(3):
				ma[i] = max(ma[i], vertices[face[v]][i])
				mi[i] = min(mi[i], vertices[face[v]][i])
	return mi,ma

def classifyVertex(vertex, splitpoint): #TODO: do splitting or other funny things
	if vertex[0] > splitpoint[0] and vertex[1] > splitpoint[1] and vertex[2] > splitpoint[2]:	return 0
	if vertex[0] <= splitpoint[0] and vertex[1] > splitpoint[1] and vertex[2] > splitpoint[2]:	return 1
	if vertex[0] > splitpoint[0] and vertex[1] > splitpoint[1] and vertex[2] <= splitpoint[2]:	return 2
	if vertex[0] > splitpoint[0] and vertex[1] <= splitpoint[1] and vertex[2] > splitpoint[2]:	return 3
	if vertex[0] <= splitpoint[0] and vertex[1] > splitpoint[1] and vertex[2] <= splitpoint[2]:	return 4
	if vertex[0] > splitpoint[0] and vertex[1] <= splitpoint[1] and vertex[2] <= splitpoint[2]:	return 5
	if vertex[0] <= splitpoint[0] and vertex[1] <= splitpoint[1] and vertex[2] > splitpoint[2]:	return 6
	if vertex[0] <= splitpoint[0] and vertex[1] <= splitpoint[1] and vertex[2] <= splitpoint[2]:return 7

def classifyFace(face, splitpoint):
	return max(classifyVertex(vertices[face[0]], splitpoint), classifyVertex(vertices[face[1]], splitpoint), classifyVertex(vertices[face[2]], splitpoint)) #TODO: random instead of max?

def buildOctree(faces, offset, level):
	mi,ma = calculateAABB(faces)
	ournum = buildOctree.counter
	buildOctree.counter += 1
	childoffset = offset

	if len(faces) > MAX_FACES_PER_TREELEAF and level < MAX_RECURSION_DEPTH:
		splitpoint = [mi[0] + (ma[0] - mi[0])/2, mi[1] + (ma[1] - mi[1])/2, mi[2] + (ma[2] - mi[2])/2]
		newfaces = [[],[],[],[],[],[],[],[]]
		newnodes = []
		childnums = []
		for face in faces:
			x = classifyFace(face, splitpoint)
			newfaces[x].append(face)
		for newface in newfaces:
			a,b = buildOctree(newface, childoffset, level+1)
			childoffset += len(newface)
			childnums.append(a)
			newnodes.extend(b)
		faces[:] = newfaces[0]+newfaces[1]+newfaces[2]+newfaces[3]+newfaces[4]+newfaces[5]+newfaces[6]+newfaces[7]
		newnodes.insert(0, [offset, len(faces), mi[0], mi[1], mi[2], ma[0] - mi[0], ma[1] - mi[1], ma[2] - mi[2], childnums[0], childnums[1], childnums[2], childnums[3], childnums[4], childnums[5], childnums[6], childnums[7]])
		return ournum, newnodes
	else:
		return ournum, [[offset, len(faces), mi[0], mi[1], mi[2], ma[0] - mi[0], ma[1] - mi[1], ma[2] - mi[2], MAX_USHORT, MAX_USHORT, MAX_USHORT, MAX_USHORT, MAX_USHORT, MAX_USHORT, MAX_USHORT, MAX_USHORT]]

try:
	if (len(sys.argv)) == 1:						raise Exception('input', 'error')
	f = open(sys.argv[len(sys.argv) - 1], 'r')
	of = 0
	for i in range(1, len(sys.argv) - 1):
		if sys.argv[i].startswith("-s="):				SCALE = float(sys.argv[i][3:])
		elif sys.argv[i].startswith("-t"):				TEXTURING = 0
		elif sys.argv[i].startswith("-g="):				GENTEX = int(sys.argv[i][3:]); TEXTURING = 0
		elif sys.argv[i].startswith("-m="):				MAX_FACES_PER_TREELEAF = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-r="):				MAX_RECURSION_DEPTH = int(sys.argv[i][3:])
		elif sys.argv[i].startswith("-o="):				of = open(sys.argv[i][3:], 'w')
		else:											raise Exception('input', 'error')
	if of == 0:											of = open(sys.argv[len(sys.argv) - 1][:sys.argv[len(sys.argv) - 1].rfind(".")] + ".octree.bz2", 'w')
except:
	print """Usage: generateOctreeFromObj [options] obj_file
Options:
 -t			Ignore texture coordinates, produce an untextured Octree
 -s=<scale>		Scale all coordinates by <scale>
 -m=<max_faces>		Limit faces per leafnode to <max_faces> (Default: 1000)
 -r=<max_recursion>	Limit tree depth to <max_recursion> (Default: 10)
 -o=<octree_file>	Place the output octree into <octree_file>
 -g=<0,1,2,3,4>		Texture coordinate generation:
 			0 = off, 1 = on, 2 = swap X, 3 = swap Y, 4 = swap XY"""
	sys.exit()

print "Info: Reading the OBJ-file"
lines = f.readlines()
for line in lines:
	i = line.strip().split(" ")[0]
	c = line[2:].strip().split(" ")
	if i == "v":
		vertices.append([float(c[0]) * SCALE, float(c[1]) * SCALE, float(c[2]) * SCALE])
	elif i == "vn":
		normals.append(normalize([float(c[0]), float(c[1]), float(c[2])]))
	elif i == "vt":
		texcoords.append([float(c[0]), float(c[1]), 0.0]) #TODO: if we discard W anyway we shouldnt store it
	elif i == "f":
		if (len(c) > 4):
			print "Error: faces with more than 4 edges not supported"
			sys.exit()
		elif (len(c) == 4): #triangulate
			faces.append([int(faceContent(c[0],0))-1, int(faceContent(c[1],0))-1, int(faceContent(c[2],0))-1, int(faceContent(c[0],2))-1, int(faceContent(c[1],2))-1, int(faceContent(c[2],2))-1, int(faceContent(c[0],1))-1, int(faceContent(c[1],1))-1, int(faceContent(c[2],1))-1])
			faces.append([int(faceContent(c[0],0))-1, int(faceContent(c[2],0))-1, int(faceContent(c[3],0))-1, int(faceContent(c[0],2))-1, int(faceContent(c[2],2))-1, int(faceContent(c[3],2))-1, int(faceContent(c[0],1))-1, int(faceContent(c[2],1))-1, int(faceContent(c[3],1))-1])
		else:
			faces.append([int(faceContent(c[0],0))-1, int(faceContent(c[1],0))-1, int(faceContent(c[2],0))-1, int(faceContent(c[0],2))-1, int(faceContent(c[1],2))-1, int(faceContent(c[2],2))-1, int(faceContent(c[0],1))-1, int(faceContent(c[1],1))-1, int(faceContent(c[2],1))-1])

print "Info: Building the Octree"
buildOctree.counter = 0
a,nodes = buildOctree(faces, 0, 0)

if len(nodes) > MAX_USHORT:
	print "Error: too many octree nodes generated, increase MAX_FACES_PER_TREELEAF"
	sys.exit()

print "Info: Unifying and Uniquing Vertices, Normals and Texcoords"
normalwarning = 0
newvertices = []
newvertices_dict = {} #it's perhaps not the most intuitive way to have the newvertices stored twice, but it prevents a quadratic runtime
for face in faces:
	for i in range(3):
		if face[i+3] == -1:
			normalwarning += 1
			normals.append(normalize(crossProduct(substract(vertices[face[0]],vertices[face[1]]), substract(vertices[face[2]],vertices[face[0]]))))
			face[i+3] = len(normals)-1
		if TEXTURING and face[i+6] == -1:
			print "Warning: some face without a texcoord detected, turning texturing off"
			TEXTURING = 0
	for i in range(3):
		if len(vertices[face[i]]) == 3:
			vertices[face[i]].extend(normals[face[i+3]])
			if TEXTURING:
				vertices[face[i]].extend(texcoords[face[i+6]])
		elif vertices[face[i]][3:6] != normals[face[i+3]] or (TEXTURING and vertices[face[i]][6:] != texcoords[face[i+6]]):	#if this vertex has a different normal/texcoord we have to duplicate it because opengl has only one index list
			sf = face[i]
			if TEXTURING:
				key = vertices[face[i]][0], vertices[face[i]][1], vertices[face[i]][2], normals[face[i+3]][0], normals[face[i+3]][1], normals[face[i+3]][2], texcoords[face[i+6]][0], texcoords[face[i+6]][1], texcoords[face[i+6]][2]
			else:
				key = vertices[face[i]][0], vertices[face[i]][1], vertices[face[i]][2], normals[face[i+3]][0], normals[face[i+3]][1], normals[face[i+3]][2]
			if newvertices_dict.has_key(key):
				face[i] = len(vertices)+newvertices_dict[key]
			if sf == face[i]:								#or create duplicate
				newvertices.append(list(key))
				newvertices_dict[key] = len(newvertices)-1
				face[i] = len(vertices)+len(newvertices)-1	#don't forget to update the index to the duplicated vertex+normal
vertices.extend(newvertices)
if normalwarning:
	print "Warning: some face without a normal detected, calculating it (x" + str(normalwarning) +")"

print "Info: Writing the resulting Octree-file"
dummywarning = 0
out = pack('III', 0xDEADBEEF if (TEXTURING or GENTEX) else 0x6D616C62, len(nodes), len(vertices))
for node in nodes:
	out += pack('IIffffffHHHHHHHH', node[0], node[1], node[2], node[3], node[4], node[5], node[6], node[7], node[8], node[9], node[10], node[11], node[12], node[13], node[14], node[15])
for vert in vertices:
	try:
		if TEXTURING:
			out += pack('ffffffff', vert[0], vert[1], vert[2], vert[3], vert[4], vert[5], vert[6], vert[7])
		elif GENTEX:
			xcoord = (vert[0] - nodes[0][2]) / nodes[0][5]
			ycoord = (vert[2] - nodes[0][4]) / nodes[0][7]
			out += pack('ffffffff', vert[0], vert[1], vert[2], vert[3], vert[4], vert[5], (1.0 - xcoord) if (GENTEX == 2 or GENTEX == 4) else xcoord, (1.0 - ycoord) if (GENTEX == 3 or GENTEX == 4) else ycoord)
		else:
			out += pack('ffffff', vert[0], vert[1], vert[2], vert[3], vert[4], vert[5]) #the vertex includes the normal now, if not the vertex is unreferenced and this throws
	except:
		dummywarning += 1
		if TEXTURING:
			out += pack('ffffffff', 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
		else:
			out += pack('ffffff', 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)

if (len(vertices) <= MAX_USHORT):	type = 'HHH'
else:								type = 'III'
for face in faces:
	out += pack(type, face[0], face[1], face[2])

of.write(bz2.compress(out))

if dummywarning:
	print "Warning: unreferenced vertex detected, writing dummy vertex (x" + str(dummywarning) +")"

print "\nSUCCESS:\n\nnodes:\t\t", len(nodes), "\nvertices:\t", len(vertices), "\t( duplicatesWithDifferentNormalsOrTexcoords:", len(newvertices), ")", "\nfaces:\t\t", len(faces), "\n"

#!/usr/bin/env python
#
#  plot_quality.py
#
#  Created by Julian Mayer on 03.02.07.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import sys
import commands
import time
#from pylab import *

colors = ['b-', 'g-', 'r-', 'c-', 'm-', 'y-', 'k-', 'b--', 'g--', 'r--', 'c--', 'm--', 'y--', 'k--']
testruns = 3
tests = [
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-base.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-delayedread.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-opencl.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-dxt.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-loop.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-64.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-1-128.app/Contents/MacOS/VTDemo-SDL",

#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-base.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-delayedread.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-opencl.app/Contents/MacOS/VTDemo-SDL",
		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-dxt.app/Contents/MacOS/VTDemo-SDL",
		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-loop.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-64.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-2-128.app/Contents/MacOS/VTDemo-SDL",


#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-base.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-dxt.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-loop.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-mipcalc.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-mipcalc-anis.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-mipcalc-anis-loop.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-trilinear.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-trilinear-loop.app/Contents/MacOS/VTDemo-SDL",
#		"/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-mipcalc-trilinear-anis.app/Contents/MacOS/VTDemo-SDL",
		]

args = []
t = []
offset = 0

#for test in tests:
#	for i in range(testruns):
#		output = commands.getoutput(test).splitlines()
#		args.append(arrayrange(0.0, float(len(output)), 1.0))
#		args.append([])
#		for fps_item in output:
#			if (fps_item.startswith("FPS")):
#				args[-1].append(int(fps_item.split("\t")[0].split(": ")[1]))
#		args.append(colors[offset])
#	offset+=1
#
#print args
#plot(*args)
#xlabel('frame')
#ylabel('fps')
#title('benchmarks')
#grid(True)
#show()


results = []
for test in tests:
	for i in range(testruns):
		bla = commands.getoutput("/usr/bin/purge")
		time.sleep(40)
		f = open('/Users/julian/Movies/Serien/flashforward.s01e13.hdtv.xvid-2hd.avi', 'r')
		lines = f.readlines()
		time.sleep(10)
		name = test.replace("/Users/julian/Documents/Development/_BUILD/Release/VTDemo-SDL-", "")
		name = name.replace(".app/Contents/MacOS/VTDemo-SDL", "")
		this = [name]


		print " NEWTEST ",
		print name
		print " "


		output = commands.getoutput(test).splitlines()

		for fps_item in output:
			print fps_item
	offset+=1



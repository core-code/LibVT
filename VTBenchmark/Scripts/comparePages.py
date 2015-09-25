#!/usr/bin/env python
#
#  comparePages.py
#
#  Created by Julian Mayer on 11.06.10.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import sys

def foo(ref):
	f_ = open('/Users/julian/Desktop/ref', 'r')
	lines_ = f_.readlines()
	fp = []
	frame = 0
	missing = 0
	for line in lines_:
		if line.find("NEWFRAME") > -1 and len(fp) > 0:
			for page in fp:
				if not(page[0] in ref[frame]):
					missing += int(page[1])
			print missing
			fp = []
			frame += 1
			missing = 0
		elif line.find("PAGE") > -1: fp.append(line.split()[1:])



f = open('/Users/julian/Desktop/res', 'r')
lines = f.readlines()

ref = []
framePages = []

for line in lines:
	if line.find("NEWTEST") > -1:
		testl = line
		if (len(ref) > 0):
			foo(ref)
		ref = []
		framePages = []
		print testl
	elif line.find("NEWFRAME") > -1 and len(framePages) > 0:
		ref.append(framePages)
		framePages = []

	elif line.find("PAGE") > -1:
		items = line.split()
		for item in items:
			if item.find("PAGE") == -1:
				framePages.append(item)

foo(ref)




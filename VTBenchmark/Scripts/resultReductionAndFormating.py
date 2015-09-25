#!/usr/bin/env python
#
#  resultReductionAndFormating.py
#
#  Created by Julian Mayer on 03.02.07.
#  Copyright (c) 2010 A. Julian Mayer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitationthe rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

results = []
f_ = open('/Users/julian/Desktop/results', 'r')
lines_ = f_.readlines()
bla = 0
num = 0
row = []
for line in lines_:
	if line.find("NEWTEST") > -1:
		#print bla
		#print num
			#print (bla / float(num))
		if (num > 0):
			row.append((bla / float(num)))
		if (len(row) > 0):
			results.append(row)
		num = 0
		bla = 0
		#print ""
		#print line,
		row = []
	else:
		try:
			bla += int(line)
		except:
			print "",
		num += 1
		if (num == 60):
			#print (bla / 60.0)
			row.append((bla / 60.0))
			bla = 0
			num = 0


results.append(row)

for z in range(max(map(lambda results: len(results), results))):
	for r in range(len(results)):
		if (len(results[r]) > z):
			print ("%.2f" % float(results[r][z])),
		else:
			print("N/A"),
		print("\t"),
		if (r % 5 == (4)):
			print("\t"),
	print("")

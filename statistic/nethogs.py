#! /usr/bin/python

#./client/5049/1000      2.42207 81.4693

import sys

argc = len(sys.argv)
if argc > 1:
	filepath = sys.argv[1]
else:
	filepath = raw_input("Input File Path: ")

print "reading ", filepath, "..."

f = open(filepath, "r")
rf = open(filepath+"res.txt", "wt")

count = 0
lineNo = 0

while 1:
	lines = f.readlines(10000)
	
	if not lines:
		break
	
	for line in lines:
		lineNo = lineNo + 1
		if 'client' in line:
			count=count+1
			i1 = line.find("./client/5049/1000      ") + len("./client/5049/1000      ")-4
			upTh = line[i1:]
			rf.write(upTh)
f.close()
rf.close()

print "counted ", count, " samples in ", lineNo, " rows."


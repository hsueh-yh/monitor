#! /usr/bin/python

#[FrameBuffer]	FILL /com/monitor/location1/stream0/video/73635/naluType/%01/4169022/73636(Dly:4ms Avg:5ms Invl:4ms) (Tot: 13 Rdy: 3)

import sys

argc = len(sys.argv)
if argc > 1:
	filepath = sys.argv[1]
else:
	filepath = raw_input("Input File Path: ")
f = open(filepath, "r")
rf = open("res.txt", "wt")

count = 0
lineNo = 0

while 1:
	lines = f.readlines(10000)
	
	if not lines:
		break
	
	for line in lines:
		lineNo = lineNo + 1
		if 'FILL' in line:
			count=count+1
			delayIdx = line.find('Dly:')
			timeIdx = line.find( 'ms', delayIdx )
			delay = line[delayIdx+len('Dly:') : timeIdx]
			rf.write(delay + '\t')
		
			delayIdx = line.find('Invl:', timeIdx)
			timeIdx = line.find( 'ms', delayIdx )
			delay = line[delayIdx+len('Invl:') : timeIdx]
			rf.write(delay + '\n')

f.close()
rf.close()

print "counted ", count, " samples in ", lineNo, " rows."


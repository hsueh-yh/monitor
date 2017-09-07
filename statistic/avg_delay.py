#! /usr/bin/python
import sys

f = open("delayAnalysis_30min_normal.txt", "r")

lines = f.readlines()

c = 0
cc = 0
delaySum = 0
interSum = 0
d = 0
i = 0
for line in lines:
	c=c+1
        s = line.split()
        if( len(s) > 0 ):
            delaySum += int(s[0])
            interSum += int(s[1])
            if( int(s[1]) > 10 ):
                d += int(s[0])
                i += int(s[1])
                cc = cc+1

f.close()

print "Total ", c, "line"
print "avg Dly: ", delaySum/c, "  ", d/cc
print "avg Invl: ", interSum/c, "  ", i/cc


#!/usr/bin/python

import random
import time
import datetime
import commands

clearcmd = "sudo wondershaper clear eth0"

startTime = datetime.datetime.now()

print "start: ", time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()) )

cmdout = commands.getoutput(clearcmd)
print cmdout


endTime = datetime.datetime.now()

while((endTime-startTime).seconds <= 30*60):
	bandwidth = random.randint(700, 1000)
	cmd = "sudo wondershaper eth0 %d %d" % (bandwidth, bandwidth)

	#set bandwidth threshold
	print '['+time.strftime('%H:%M:%S', time.localtime(time.time()) ) + ']>>>', cmd
	#print '>>>', cmd
	cmdout = commands.getoutput(cmd)
	if cmdout=='':
		print 'done'
	else:
		print cmdout

	#sleep
	sleeptime = random.randint(20,50)
	print 'sleep ' + str(sleeptime) + 's...\n'
	time.sleep(sleeptime)

	#clear bandwidth threshold
	print '['+time.strftime('%H:%M:%S', time.localtime(time.time()) ) + ']>>>', clearcmd
	cmdout = commands.getoutput(clearcmd)
	print cmdout

	sleeptime = random.randint(20,50)
	print 'sleep ' + str(sleeptime) + 's...\n'
	time.sleep(sleeptime)

	endTime = datetime.datetime.now()


print "end: ", time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()) )

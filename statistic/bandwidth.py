#!/usr/bin/python

import random
import time
import datetime
import commands

clearcmd = "sudo wondershaper clear enp0s25"

startTime = datetime.datetime.now()

print "start: ", time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()) )

endTime = datetime.datetime.now()

while((endTime-startTime).seconds <= 25*60):
    time.sleep(10)
    endTime = datetime.datetime.now()

stop_client = "sudo kill -9 14731"
print stop_client
cmdout = commands.getoutput(stop_client)
print cmdout

print "end: ", time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()) )

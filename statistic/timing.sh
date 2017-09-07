#!/bin/bash

startT=$(date +%s);
curT=$(date +%s);
diff=$((curT-startT));

while [ $diff -lt 1800 ];
do
    sleep 60;
    curT=`date +%s`;
    diff=$((curT-startT));
    echo "cur: " $curT;
done

PROCESS=`ps -A | grep -w 'client'|grep -v grep|cut -f 1 -d " "| awk '{print}'` # | xargs kill -9
for i in $PROCESS
do
  echo "Kill the client process [ $i ]"
  kill -9 $i
done

echo "ending"

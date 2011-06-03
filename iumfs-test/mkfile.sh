#!/bin/sh

cd /var/tmp/iumfsmnt
num=1

while :
do
	mkfile ${num}k ${num}k
	ls -l ${num}k
	num=`expr ${num} + 1`
done 

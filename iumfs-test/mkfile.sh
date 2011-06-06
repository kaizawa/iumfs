#!/bin/sh

num=1

cd /var/tmp/iumfsmnt
while :
do
	mkfile ${num}k ${num}k
	ls -l ${num}k
	num=`expr ${num} + 1`
done

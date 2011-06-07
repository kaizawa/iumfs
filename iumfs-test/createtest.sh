#!/bin/sh
# Test script.
# Previously paanic happened in iumf_write at kzero where handling pagefault.
# if file are created and deleted continualy.
# This issue had alread been resolved.

cd /var/tmp/iumfsmnt            
cnt=1
while [ $cnt -lt 50 ]; 
do
    num=1                           
    while [ $num -lt 30 ];          
    do                              
	#dd if=/dev/zero of=${num}k bs=1k count=${num}
    	mkfile ${num}k ${num}k   > /dev/null 2>&1
    	num=`expr ${num} + 1`   
    done                            
    rm /var/tmp/iumfsmnt/* 
    cnt=`expr ${cnt} + 1` 
done

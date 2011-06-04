#!/bin/sh
# Panic happen in iumf_write at kzero where handling pagefault.
# if file are created and deleted continualy.

cd /var/tmp/iumfsmnt            
while :
do
    num=1                           
    while [ $num -lt 30 ];          
    do                              
    	mkfile ${num}k ${num}k  
	#dd if=/dev/zero of=${num}k bs=1k count=${num}
    	ls -l ${num}k           
    	num=`expr ${num} + 1`   
    done                            
    rm /var/tmp/iumfsmnt/* 
done

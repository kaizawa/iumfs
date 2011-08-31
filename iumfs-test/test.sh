#!/bin/ksh
#
# Copyright (C) 2010 Kazuyoshi Aizawa. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

#
# Test script of iumfs filesystem
# After build iumfs, run this script.
# You will be prompted password for root user.
#
procs=5 # number of processes for stress test
stresstime=30 # number of seconds for stress test
daemonpid=""
maxumountretries=3 # number of try of umount
mnt="/var/tmp/iumfsmnt"
base="/var/tmp/iumfsbase"

init (){
	LOGDIR=$PWD/logs
        TESTLOGFILE=$LOGDIR/test-`date '+%Y%m%d-%H:%M:%S'`.log
        DAEMONLOGFILE=$LOGDIR/localfsd-`date '+%Y%m%d-%H:%M:%S'`.log
        # Create log directory 
	if [ ! -d "${LOGDIR}" ]; then
		mkdir ${LOGDIR}
	fi	

        touch $DAEMONLOGFILE
        touch $TESTLOGFILE
	# Just in case, umount ${mnt}
	do_umount

 	# kill iumfsd localfsd
	kill_daemon  >> $TESTLOGFILE 2>&1

        # Create mount point directory 
	if [ ! -d "${mnt}" ]; then
	    mkdir ${mnt}  >> $TESTLOGFILE 2>&1
	    if [ "$?" -ne 0 ]; then
		echo "cannot create ${mnt}"  >> $TESTLOGFILE 2>&1
		fini 1
	    fi
	fi
}

init_localfs(){
        echo "##"
        echo "## Preparing required directory for test."
        echo "##"
	# Just in case, remove existing test dir
	rm -rf ${base}   >> $TESTLOGFILE 2>&1
        # Create mount base directory 
	if [ ! -d "${base}" ]; then
	    mkdir ${base}  >> $TESTLOGFILE 2>&1
	    if [ "$?" -ne 0 ]; then
		echo "cannot create ${base}"  >> $TESTLOGFILE 2>&1
		fini 1
	    fi
	fi
        echo "Completed."
}

do_build(){
        echo "##"
        echo "## Start building binaries ."
        echo "##"
	#./configure --enable-debug  >> $TESTLOGFILE 2>&1
	./configure # >> $TESTLOGFILE 2>&1
	sudo make uninstall # >> $TESTLOGFILE 2>&1
	make clean  #>> $TESTLOGFILE 2>&1
	sudo umount ${mnt}  >> $TESTLOGFILE 2>&1
	return $?
}

do_mount () {
    sudo mount -F iumfs ${1}${base} ${mnt} >> $TESTLOGFILE 2>&1
    return $?
}

do_umount() {
	cnt=0;
	while [ $cnt -lt $maxumountretries ]
	do
		sudo umount ${mnt} >> $TESTLOGFILE 2>&1
		cnt=`expr $cnt + 1`
		mountexit=`mount |grep "${mnt} "`
		if [ -z "$mountexit" ]; then
			return 0
		fi
		sleep 1
	done
	echo "cannot umount ${mnt}"  | tee >> $TESTLOGFILE 2>&1
	return 1
}

start_localfsd() {
	./iumfs-test/localfsd -d 3 > $DAEMONLOGFILE 2>&1 &
	if [ "$?" -eq 0 ]; then
		daemonpid=$! 
		return 0		
	fi
	return 1
}

kill_daemon(){
	sudo pkill -KILL localfsd >> $TESTLOGFILE 2>&1
	daemonpid=""
	return 0
}

exec_mount_test () {

        for target in mount umount
        do
   	   cmd="do_${target}" 
	   $cmd  $1 >> $TESTLOGFILE 2>&1
	   if [ "$?" -eq "0" ]; then
		echo "${target} test: \tpass" 
	   else
		echo "${target} test: \tfail  See $TESTLOGFILE" 
		fini 1	
	   fi
        done    
}

exec_fstest() {
	target=$1

	./iumfs-test/fstest $target >> $TESTLOGFILE 2>&1
	if [ "$?" -eq "0" ]; then
		echo "${target} test: \tpass" 
	else
		echo "${target} test: \tfail  See $TESTLOGFILE" 
	fi
}

fini() {
        ## Kill unfinished processes
        for pid in $pids
        do
            kill $pid > /dev/null 2>&1
        done
	do_umount
	kill_daemon
        #rm -rf ${mnt} >> $TESTLOGFILE 2>&1
        echo "##"
        echo "## Finished."
        echo "##"
        echo "See log files for detail."
        echo "$TESTLOGFILE"
        echo "$DAEMONLOGFILE"
        exit 0
}

do_basic_test(){
    echo "##"
    echo "## Start filesystem operation test with $1 daemon."
    echo "##"
    exec_fstest "mkdir"
    exec_fstest "open"
    exec_fstest "write"
    exec_fstest "read"
    exec_fstest "getattr"
    exec_fstest "readdir"
    exec_fstest "remove"
    exec_fstest "rmdir"
    exec_fstest "mmap"
}

do_create_and_delete(){
     filename=$RANDOM
     start=`get_second`
     count=0
     cd ${mnt}
     while :
     do
         count=`expr $count + 1`
         current=`get_second`
         elapsed=`expr $current - $start`
         throughput=`echo "$count/$elapsed" | bc -l 2>/dev/null` 
         echo "$count $elapsed $throughput" > $LOGDIR/throughput.$filename
         echo $filename > $filename
	 if [ "$?" -ne 0 ]; then
	     echo "do_create_and_delete: cannot create $filenme." | tee >> $TESTLOGFILE 2>&1
             continue
	 fi
         /bin/ls -aF > /dev/null 2>&1
         rm $filename
	 if [ "$?" -ne 0 ]; then
	     echo "do_create_and_delete: cannot remove $filenme." | tee >> $TESTLOGFILE 2>&1
             continue
	 fi
     done
}

get_second (){
    date '+%H %M %S' | read hour minute second
    echo "$hour*60*60+$minute*60+$second" | bc
}

do_stress_test(){
    echo "##"
    echo "## Start stress test"
    echo "##"
    pids=""
    cnt=0

    trap fini 2 

    while [ $cnt -lt $procs ]
    do
        do_create_and_delete &
        pids="$pids $!"
        cnt=`expr $cnt + 1`
    done

    echo "$pids started"  >> $TESTLOGFILE 2>&1

    ## Sleep for complete
    echo "Sleep $stresstime sec..." >> $TESTLOGFILE 2>&1
    sleep $stresstime

    echo "stress test: \tpass" 
}

do_mkfile_test() {
    echo "##"
    echo "## Start mkfile test"
    echo "##"
    cnt=1
    while [ $cnt -lt 30 ]; 
    do
        num=1                           
        while [ $num -lt 10 ];          
        do                              
            #dd if=/dev/zero of=${num}k bs=1k count=${num}
    	    mkfile ${num}k ${mnt}/${num}k   > /dev/null 2>&1
    	    num=`expr ${num} + 1`   
        done                            
        rm ${mnt}/*k
        cnt=`expr ${cnt} + 1` 
    done
    echo "mkfile test: \tpass" 
}

main() { 
    cd ../

    init

    echo "##"
    echo "## Start mount test."
    echo "##"
    exec_mount_test "localfs://localhost"
    init_localfs
    do_mount "localfs://localhost"
    start_localfsd

    do_basic_test
    do_mkfile_test
    do_stress_test

    fini $1
}

main $1

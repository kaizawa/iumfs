#!/bin/sh -x
# Script which performs mount and start daemon

sudo umount /var/tmp/iumfsmnt > /dev/null 2>&1
pkill -f localfsd

sudo mount -F iumfs localfs://localhost/var/tmp/iumfsbase /var/tmp/iumfsmnt
/var/tmp/iumfs/iumfs-test/localfsd

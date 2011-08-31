#!/bin/sh
# script that performs umont and kill daemon

sudo umount /var/tmp/iumfsmnt
pkill -f localfsd

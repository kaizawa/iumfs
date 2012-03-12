/*
 * Copyright (C) 2010 Kazuyoshi Aizawa. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**************************************************************
 * readtest.c
 *
 * Test comand for IUMFS filesystem.
 * Specialized for read operation.
 *
 ***************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>
#include <sys/param.h>
#include <unistd.h>
#include <ctype.h>

#define BUF 8192
#define NUM_TARGET 4

#define TEXT "testtext"
#define TEXT_1 "wr1i1tes1st1"
#define TEXT_2 "wr2i2tes2st2"
#define TEXT_3 "wr3i3tes3st3"

#define DIR_PATH   "/var/tmp/iumfsmnt"

#define FILE_NAME  "testfile"
#define FILE_PATH  DIR_PATH "/" FILE_NAME

#define NEW_FILE_NAME_1  "testnewfile1"
#define NEW_FILE_PATH_1  DIR_PATH "/" NEW_FILE_NAME_1

#define NEW_FILE_NAME_2  "testnewfile2"
#define NEW_FILE_PATH_2  DIR_PATH "/" NEW_FILE_NAME_2

#define NEW_FILE_NAME_3  "testnewfile3"
#define NEW_FILE_PATH_3  DIR_PATH "/" NEW_FILE_NAME_3

#define NEW_FILE_NAME_4  "testnewfile4"
#define NEW_FILE_PATH_4  DIR_PATH "/" NEW_FILE_NAME_4


#define NEW_FILE_NAME_6  "testnewfile6"
#define NEW_FILE_PATH_6  DIR_PATH "/" NEW_FILE_NAME_6

#define NEW_FILE_NAME_7  "testnewfile7"
#define NEW_FILE_PATH_7  DIR_PATH "/" NEW_FILE_NAME_7

#define NEW_DIR_NAME_1  "testnewdir1"
#define NEW_DIR_PATH_1  DIR_PATH "/" NEW_DIR_NAME_1

#define NEW_DIR_NAME_2  "testnewdir2"
#define NEW_DIR_PATH_2  DIR_PATH "/" NEW_DIR_NAME_2

#define NEW_DIR_NAME_3  "testnewdir3"
#define NEW_DIR_PATH_3  NEW_DIR_PATH_1 "/" NEW_DIR_NAME_3

#define DUMMY_DIR_NAME_1  "testdummydir"
#define DUMMY_DIR_PATH_1  DIR_PATH "/" DUMMY_DIR_NAME_1

#define DUMMY_FILE_NAME_1  "testdummyfile"
#define DUMMY_FILE_PATH_1  DIR_PATH "/" DUMMY_FILE_NAME_1

#define NEW_FILE_NAME_5  "testnewfile5"
#define NEW_FILE_PATH_5  NEW_DIR_PATH_2 "/" NEW_FILE_NAME_5

int pagesize;

int
main(int argc, char *argv[])
{
    int fd = -1;
    int ret = 0;
    ssize_t rsize = 0;
    ssize_t wsize = 0;    
    size_t newsize;
    struct stat st[1];
    const char *path;
    int oflag;
    mode_t mode = 0;
    const char *data;
    offset_t off;
    size_t size;
    uchar_t *buf = NULL;
    int compare = 0;
    int i;

    if (argc != 7) {
        printf("Usage: %s path oflag mode data off size\n", argv[0]);
        printf("\n\
O_RDONLY   0\n\
O_WRONLY   1\n\
O_RDWR     2\n\
O_SEARCH   0x200000\n\
O_EXEC     0x400000\n\
O_CREAT    0x100   /* open with file create (uses third arg) */\n\
O_TRUNC    0x200   /* open with truncation */\n\
O_EXCL     0x400   /* exclusive open */\n\
O_NOCTTY   0x800   /* don't allocate controlling tty (POSIX) */\n\
O_XATTR    0x4000  /* extended attribute */\n\
O_NOFOLLOW 0x20000 /* don't follow symlinks */\n\
O_NOLINKS  0x40000 /* don't allow multiple hard links */\n");        
        ret = 1;
        goto out;
    }

    pagesize = getpagesize();

    path = argv[1];
    oflag = strtol(argv[2], (char **)NULL, 16);    
    mode = (mode_t)strtol(argv[3], (char **)NULL, 8);
    data = argv[4];
    off = atoi(argv[5]);
    size = atoi(argv[6]);

    /*
     * 引数 data に NULLを渡された場合はデータの比較の必要はない。
     */ 
    if(strlen(data) != 0){
        compare = 1;
    }    

    buf = malloc(size);
    if (buf == NULL) {
        printf("malloc(): %s\n", strerror(errno));
        return(errno);
    }    

    fd = open(path, oflag, mode);
    if (fd < 0) {
        printf("open(%s): %s\n", path, strerror(errno));

        ret = errno;
        goto out;
    }

    if ((rsize = pread(fd, buf, size, off)) < 0) {
        printf("pread(%s): %s\n", path, strerror(errno));
        return(errno);
    }
    printf("\"%s\"\n",buf);
    printf("read %zd bytes of data at offset %lld.\n", rsize, off);

    if ((wsize = pwrite(fd, data, size, off)) < 0) {
        printf("pwrite(%s): %s\n", path, strerror(errno));
        ret = errno;
        goto out;
    }    

    if ((rsize = pread(fd, buf, size, off)) < 0) {
        printf("pread(%s): %s\n", path, strerror(errno));
        return(errno);
    }
    printf("\"%s\"\n",buf);
    printf("read %zd bytes of data at offset %lld.\n", rsize, off);
    
    if (compare && (bcmp(buf, data, size) != 0)) {
        printf("bcmp failed\n");
        for (i = 0; i < size; i++) {
            if (isprint(buf[i]))
                printf("buf[%d]= 0x%0x \"%c\"\n", i, buf[i], buf[i]);
            else
                printf("buf[%d]= 0x%0x \n", i, buf[i]);                    
        }
        goto out;
    }    

    if ((fstat(fd, st)) < 0) {
        printf("fstat(%s): %s\n", path, strerror(errno));
        ret = errno;
        goto out;
    }

    newsize = st->st_size;
    printf("file size = %zd bytes\n", newsize);

out:
    if (buf != NULL)
        free(buf);        
    return (ret);
}

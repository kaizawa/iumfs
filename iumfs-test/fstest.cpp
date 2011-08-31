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
 * fstest.cpp
 *
 * Test comand for IUMFS filesystem.
 *
 ***************************************************************/

/*
 * iumfs ファイルシステムモジュールのテスト用のコマンド。
 * テスト用に決められたディレクトリ、ファイルを読み込んで
 * 期待通りの動作をするかどうかを確認する
 *
 * TODO: 4G を超えるサイズのファイルの読み書き
 * TODO: O_APPEND フラグつきでの open
 *
 */
#include <iostream>
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

using namespace std;

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

class Test
{
protected:
    int pagesize;        
    int ret;
    int fd;

public:
    Test() { pagesize = getpagesize(); ret = 0; fd = -1;}
    virtual int runTest() {return 0;};
    int doOpen(const char *, int, mode_t);
    int doCreat(const char *, mode_t);
    int doGetattr(const char *);    
    int doReaddir(const char *, const char *);    
    int doRead(const char *, const uchar_t *, offset_t , size_t , size_t *);                      
    int doWrite(const char *, const uchar_t *, offset_t, size_t );
    int doMkdir(const char *, mode_t);
    int doRmdir(const char *);
    int getFilesize(const char *);
    int doRemove(const char *);
    int doMmap(const char *, size_t);
};

int
Test::doOpen(const char *filepath, int oflag, mode_t mode)
{
    fd = open(filepath, oflag, mode);
    if (fd < 0) {
        cout << "\t\tdoOpen: open(" << filepath << "): "  << strerror(errno) << endl;
        return (errno);
    }
    close(fd);
    return 0;
}

int
Test::doCreat(const char *filepath, mode_t mode)
{
    fd = creat(filepath, mode);
    if (fd < 0) {
        cout << "\t\tdoCreat: creat(" << filepath << "): "  << strerror(errno) << endl;        
        return (errno);
    }
    close(fd);
    return 0;
}

int 
Test::doGetattr(const char *filepath)
{
    struct stat st[1];

    if ((stat(filepath, st)) < 0) {
        cout << "\t\tdoGetattr: stat(" << filepath << "): "  << strerror(errno) << endl;        
        return(errno);
    }

    if (S_ISDIR(st->st_mode) == 0) {
        cout << "\t\tdoGetattr: " << filepath << "is not recognized as a directory" << endl;
        return(errno);
    }
    return(0);
}

int
Test::doReaddir(const char *dirpath, const char *entryname)
{
    DIR *dirp = NULL;
    struct dirent *direntp = NULL;
    int entry_found = 0;

    if ((dirp = opendir(dirpath)) == NULL) {
        cout << "\t\tdoReaddir: opendir(" << dirpath << "): "  << strerror(errno) << endl;
        return(errno);
    }

    do {
        if ((direntp = readdir(dirp)) != NULL) {
            if (strcmp(direntp->d_name, entryname) == 0) {
                cout << "\t\tdoReaddir: found [\"" << entryname << "\"]" << endl;
                entry_found = 1;
                break;
            }
        }
    } while (direntp != NULL);

    if (!entry_found) {
        cout << "\t\tdoReaddir: can not find  [\"" << entryname << "%s\"]" << endl;
        return(errno);
    }
    return(0);
}

int
Test::doRead(const char *path, const uchar_t *data, offset_t off, size_t size, size_t *rsizep)
{
    size_t i;
    uchar_t *buf;
    struct stat st[1];
    int no_compare = 0;

    /*
     * 引数 data に NULLを渡された場合はデータの比較の必要はない。
     */ 
    if(data == NULL){
        no_compare = 1;
    }

    buf = (uchar_t *)malloc(size);
    if (buf == NULL) {
        cout << "\t\tdoRead: malloc(): " << strerror(errno) << endl;
        return(errno);
    }

    fd = open64(path, O_RDONLY);
    if (fd < 0) {
        cout << "\t\tdoRead: open(" << path << "): " << strerror(errno) << endl;
        return(errno);
    }

    if ((fstat(fd, st)) < 0) {
        cout << "\t\tdoRead: fstat(" << path << "): " << strerror(errno) << endl;
        return(errno);
    }

    if ((*rsizep = pread(fd, buf, size, off)) < 0) {
        cout << "\t\tread_tesst: pread(" << path << "): " << strerror(errno) << endl;
        return(errno);
    }

    cout << "\t\tdoRead: read " << *rsizep << " bytes from offset " << off << endl;

    if(no_compare){
        return(0);
    }

    if (*rsizep != size) {
        cout << "\t\tdoRead: read " << *rsizep << " bytes of data. (!= " << size << " bytes. Wrong size!!)" << endl;
        return(-1);
    }

    if (bcmp(buf, data, size) != 0) {
        cout << "\t\tdoRead: bcmp failed" << endl;
        for (i = 0; i < size; i++) {
            if (isprint(buf[i])){
                cout << "\t\tbuf[" << i << "]= 0x" << oct << buf[i] ;
                cout << " \"" <<  buf[i] << "\"" << endl;
            } else {
                cout << "\t\tbuf[" << i << "]= 0x" << oct << buf[i] << endl;
            }
        }
        free(buf);
        return(-1);
    }
    free(buf);    
    cout << "\t\tdoRead: success." << endl;
    return (0);
}


int
Test::doWrite(const char *path, const uchar_t *data, offset_t off, size_t size)
{
    int ret = 0;
    ssize_t wsize = 0;
    size_t newsize;
    struct stat st[1];

    //fd = open64(path, O_RDWR|O_CREAT|O_TRUNC);
    fd = open64(path, O_RDWR|O_CREAT, 0644);
    if (fd < 0) {
        cout << "\t\tdoWrite: open(" << path << "): " << strerror(errno) << endl;
        ret = errno;
        goto out;
    }

    if ((wsize = pwrite(fd, data, size, off)) < 0) {
        cout << "\t\tdoWrite: pwrite(" << path << "): " << strerror(errno) << endl;
        ret = errno;
        goto out;
    }

    if (wsize != (ssize_t) size) {
        cout << "\t\tdoWrite: wrote " << wsize << " bytes of data. (!= " << size << " bytes. Wrong size!!)" << endl;
        ret = errno;
        goto out;
    }
    cout << "\t\tdoWrite: wrote " << wsize << " bytes of data at offset " << off << endl;

    if ((fstat(fd, st)) < 0) {
        cout << "\t\tdoWrite: fstat(" << path << "): " << strerror(errno) << endl;
        ret = errno;
        goto out;
    }

    newsize = st->st_size;
    cout << "\t\tdoWrite: new file size = " << newsize << " bytes" << endl;

out:
    if (fd != -1) {
        close(fd);
    }
    return (ret);
}

int
Test::doMkdir(const char *dirpath, mode_t mode)
{
    if (mkdir(dirpath, mode) < 0) {
        cout << "\t\tdoMkdir: mkdir(" << dirpath << "): " << strerror(errno) << endl;
        return (errno);
    }
    return 0;
}

int
Test::doRmdir(const char *dirpath)
{
    if (rmdir(dirpath) < 0) {
        cout << "\t\tdoRmdir: rmdir(" << dirpath << "): " << strerror(errno) << endl;
        return (errno);
    }
    return 0;
}

int
Test::doRemove(const char *filepath)
{
    if(unlink(filepath) < 0){
        cout << "\t\tdoRemove: unlink(" << filepath << "): " << strerror(errno) << endl;
        return (errno);
    }
    return 0;
}

int
Test::getFilesize(const char *filepath)
{
    struct stat st[1];
    size_t size = 0;

    if ((stat(filepath, st)) < 0) {
        cout << "\t\tgetGilesize: stat(" << filepath << "): " << strerror(errno) << endl;
        return(-1);
    }
    size = st->st_size;
    
    cout << "\t\tgetFilesize: file size = " << size << endl;
    return(size);
}

int
Test::doMmap(const char *path, size_t size)
{
    size_t i;
    uchar_t *buf, *map;

    buf = (uchar_t *)malloc(size);
    if (buf == NULL) {
        cout << "\t\tdoMmap: malloc(): " << strerror(errno) << endl;
        return(errno);
    }

    // 10101010
    memset(buf, 0xaa, size);
 
    fd = open64(path, O_RDWR|O_CREAT, 0644);    
    if (fd < 0) {
        cout << "\t\tdoMmap: open(" << path << "): " << strerror(errno) << endl;
        free(buf);        
        return(errno);
    }

    if (write(fd, (void *)buf, size) < 0) {
        cout << "\t\tdoMmape: write(" << path << "): " << strerror(errno) << endl;
        free(buf);        
        return(errno);
    }
    
    map = (uchar_t *)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == (uchar_t *)-1) {
        cout << "\t\tdoMmap: mmap(" << path << "): " << strerror(errno) << endl;
        free(buf);        
        return(errno);
    }

    if (bcmp(buf, map, size) != 0) {
        cout << "\t\tdoMmap: bcmp failed" << endl;
        for (i = 0; i < size; i++) {
            if (isprint(buf[i])){
                cout << "\t\tbuf[" << i << "]= 0x" << oct << buf[i] ;
                cout << " \"" <<  buf[i] << "\"" << endl;
            } else {
                cout << "\t\tbuf[" << i << "]= 0x" << oct << buf[i] << endl;
            }
        }
        free(buf);
        return(-1);        
    }
    free(buf);    
    cout << "\t\tdoMmap: success." << endl;
    return (0);
}

class WriteTest : public Test
{
public:
    int runTest();
};

int
WriteTest::runTest()
{
    int ret = 0;
    uchar_t *dummydata = NULL;
    size_t datasize = 0;    
    size_t rsize = 0;
    offset_t offset = 0;
    size_t filesize = 0;

    cout << "write_test: Start" << endl;

    /*
     * 空の既存ファイルに小さな文字列を書き込むテスト
     */    
    cout << "\tbegin: Write small bytes to exising empty file(" << NEW_FILE_PATH_1 <<  ")" << endl;
    if ((ret = doWrite(NEW_FILE_PATH_1, (uchar_t *) TEXT_1,
            0, strlen(TEXT_1))) != 0) {
        goto out;
    }

    if ((ret = doRead(NEW_FILE_PATH_1, (uchar_t *) TEXT_1,
                       0, strlen(TEXT_1),&rsize)) != 0) {
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * 既存ファイルに追記するテスト
     */
    offset = getFilesize(NEW_FILE_PATH_1);    
    cout << "\tbegin: append small bytes to exising file(" << NEW_FILE_PATH_1 << ")" << endl;
    if ((ret = doWrite(NEW_FILE_PATH_1, (uchar_t *) TEXT_2,
                        offset, strlen(TEXT_2))) != 0) {
        goto out;
    }

    if ((ret = doRead(NEW_FILE_PATH_1, (uchar_t *) TEXT_2,
                       offset, strlen(TEXT_2),&rsize)) != 0) {
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * PAGE サイズを超える(5KB)書き込みテスト
     */    
    cout << "\tbegin: Write bytes greater than PAGESIZE(" << pagesize << ") to new file(" << NEW_FILE_PATH_3 << ")" << endl;
    datasize = 5 * 1024;
    dummydata = (uchar_t *)malloc(datasize);
    memset(dummydata, 0x77, datasize);
    if ((ret = doWrite(NEW_FILE_PATH_3, dummydata, 0, datasize)) != 0) {
        goto out;
    }

    if ((ret = doRead(NEW_FILE_PATH_3, dummydata, 0, datasize, &rsize)) != 0) {
        goto out;
    }
    free(dummydata);    
    cout << "\tend: Success" << endl;

    /*
     * MAXBSIZE サイズを超える(16KB)書き込みテスト
     */    
    cout << "\tbegin: Write bytes greater than MAXBSIZE(" << MAXBSIZE << ") to new file(" << NEW_FILE_PATH_4 << ")" << endl;
    datasize = MAXBSIZE * 2;
    dummydata = (uchar_t *)malloc(datasize);
    memset(dummydata, 0x77, datasize);
    if ((ret = doWrite(NEW_FILE_PATH_4, dummydata, 0, datasize)) != 0) {
        goto out;
    }

    if ((ret = doRead(NEW_FILE_PATH_4, dummydata, 0, datasize,&rsize)) != 0) {
        goto out;
    }
    free(dummydata);    
    cout << "\tend: Success" << endl;
    
    /*
     * PAGE サイズ以内で、ファイルサイズ以上の書き込みテスト
    */    
    cout << "\tbegin: Write bytes less than PAGESIZE(" << pagesize << ") to new file(" NEW_FILE_PATH_1 ")" << endl;
    if ((ret = doWrite(NEW_FILE_PATH_1, (uchar_t *) TEXT_1,
            0, strlen(TEXT_2))) != 0) {
        goto out;
    }
    if ((ret = doRead(NEW_FILE_PATH_1, (uchar_t *) TEXT_1,
                       0, strlen(TEXT_1),&rsize)) != 0) {
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * ファイルサイズを 9KB (>PAGESIZE * 2) 超えるオフセットに対する書き込み
     */
    cout << "\tbegin: Write bytes where offset is beyond PAGESIZE(" << pagesize << ") from files size to existing file(" <<  NEW_FILE_PATH_1 << endl;
    if((filesize = getFilesize(NEW_FILE_PATH_1)) < 0){
        ret = 1;
        goto out;
    }
    offset = filesize + (pagesize * 2) + 1024;;
    if ((ret = doWrite(NEW_FILE_PATH_1, (uchar_t *)TEXT_1,
                        offset, strlen(TEXT_1))) != 0) {
        goto out;
    }
    if ((ret = doRead(NEW_FILE_PATH_1, (uchar_t *)TEXT_1,
                       offset, strlen(TEXT_1),&rsize)) != 0) {
        goto out;

    }    
    cout << "\tend: Success" << endl;

  out:
    if(dummydata){
        free(dummydata);
    }
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "write_test: Finish" << endl;    
    return ret;
}

class RemoveTest: public Test 
{
public:
    int runTest();
};
    
int
RemoveTest::runTest()
{
    int ret = 0;

    cout << "remove_test: Start" << endl;    
    
    /*
     * Remove existing file 
     */
    cout << "\tbegin: Remove existing file(" <<  NEW_FILE_PATH_2 << ")" << endl;
    if((ret = doRemove(NEW_FILE_PATH_2)) != 0){
        goto out;        
    }
    cout << "\tend: Success" << endl;
    
    /*
     * Remove non-existing file 
     */
    cout << "\tbegin: Remove non-existing file(" << DUMMY_FILE_PATH_1 << ")" << endl;
    if((ret = doRemove(DUMMY_FILE_PATH_1)) == 0){
        cout << "\tNon-existing file was removed without error..." << endl;
        ret = 1;
        goto out;        
    }
    ret = 0;    
    cout << "\tend: Success" << endl;    

    /*
     * Remove directory
     */
    cout << "\tbegin: Remove directory (" << NEW_DIR_PATH_1 << ") using unlink" << endl;
    ret = doRemove(NEW_DIR_PATH_1);
    switch(ret){
        case 0:
            cout << "\tdirectory was removed!!" << endl;
            ret = 1;
            goto out;
        case EISDIR:
            break;
        default:
            cout << "\terror is not EISDIR, but \""  << strerror(ret) << "\"" << endl;
    }
    ret = 0;
    cout << "\tend: Success" << endl;                
    
out:
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "remove_test: Finish" << endl;
    return ret;
}

class MkdirTest : public Test
{
public:
    int runTest();
};

int
MkdirTest::runTest()
{
    int ret = 0;

    cout << "mkdir_test: Start" << endl;
    /*
     * Create new directory 
     */
    cout << "\tbegin: Create new directory("<< NEW_DIR_PATH_1 << ")" << endl;
    if((ret = doMkdir(NEW_DIR_PATH_1, 
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) != 0){        
        goto out;        
    }
    cout << "\tend: Success" << endl;

    /*
     * re-Create new directory
     */
    cout << "\tbegin: try to re-create existing directory(" <<  NEW_DIR_PATH_1 << ")" << endl;
    ret = doMkdir(NEW_DIR_PATH_1, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    switch(ret){
        case 0:
            cout << "\texisting directory can be re-created!!" << endl;
            ret = 1;
            goto out;
        case EEXIST:
            break;
        default:
            cout << "\terror is not EEXIT, but : " <<  strerror(ret) << endl;
    }
    ret = 0;
    cout << "\tend: Success" << endl;
    
    /*
     * Create another new directory 
     */
    cout << "\tbegin: Create new directory(" NEW_DIR_PATH_2 << ")" << endl;
    if((ret = doMkdir(NEW_DIR_PATH_2, 
                      S_IRUSR | S_IXUSR | S_IWUSR | S_IRGRP | S_IXUSR | S_IROTH |S_IXUSR )) != 0){        
        goto out;        
    }
    cout << "\tend: Success" << endl;    
    
out:
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "mkdir_test: Finish" << endl;
    return ret;
}

class RmdirTest : public Test
{
public:
    int runTest();
};

int
RmdirTest::runTest()
{
    int ret = 0;

    cout << "rmdir_test: Start" << endl;
    /*
     * Remove existing directory (=NEW_DIR_PATH_1)
     * which was created within mkdir_test
     */
    cout << "\tbegin: Remove existing directory(" <<  NEW_DIR_PATH_1 << endl;
    if((ret = doRmdir(NEW_DIR_PATH_1)) != 0){
        goto out;        
    }
    cout << "\tend: Success" << endl;

    /*
     * Remove non-existing directory
     */
    cout << "\tbegin: Remove non-existing directory(" << DUMMY_DIR_PATH_1 << ")" << endl;
    ret = doRmdir(DUMMY_DIR_PATH_1);
    switch(ret){
        case 0:
            cout << "\tnon existing directory was removed!" << endl;
            ret = 1;
            goto out;
        case ENOENT:
            break;
        default:
            cout << "\terror is not ENOENT, but " << strerror(ret) << ")" << endl;
    }                
    ret = 0;
    cout << "\tend: Success" << endl;
    
    /*
     * Remove non-empty directory
     */
    cout << "\tbegin: Remove non-empty directory(" << NEW_DIR_PATH_2 << ")" << endl;
    ret = doRmdir(NEW_DIR_PATH_2);
    switch(ret){
        case 0:
            cout << "\tnon-empty directory was removed!" << endl;
            ret = 1;
            goto out;
        case ENOTEMPTY:
            break;
        default:
            cout << "\terror is not ENOTEMPTY, but " <<  strerror(ret) << endl;
    }        
    ret = 0;
    cout << "\tend: Success" << endl;        
    
out:
    if (ret) {
        cout << "\tend: Failed" << endl;
    } 
    cout << "rmdir_test: Finish" << endl;
    return ret;
}

class OpenTest : public Test
{
public:
    int runTest();
};

int
OpenTest::runTest()
{
    int ret = 0;

    cout << "openTest: Start" << endl;    
    
    /*
     * Create new file
     */
    cout << "\tbegin: Create new file(" <<  NEW_FILE_PATH_1 << ")" << endl;
    if((ret = doCreat(NEW_FILE_PATH_1, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) != 0){            
        goto out;
    }
    cout << "\tend: Success" << endl;


    /*
     * Open Existing file
     */
    cout << "\tbegin: Open existing file(" << NEW_FILE_PATH_1 << ") with O_RDONLY flag" << endl;
    if((ret = doOpen(NEW_FILE_PATH_1, O_RDONLY, 0)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * Open new file with O_CREAT
     */
    cout << "\tbegin: Open new file(" << NEW_FILE_PATH_2 << ") with O_CREAT flag" << endl;
    if((ret = doOpen(NEW_FILE_PATH_2, O_WRONLY | O_CREAT | O_TRUNC| O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;    

    /*
     * Open Existing file with O_EXCL & O_CREATE flag
     */
    cout << "\tbegin: Open existing file(" << NEW_FILE_PATH_1 << ") with O_EXCL & O_CREAT flag" << endl;
    if((doOpen(NEW_FILE_PATH_1, O_WRONLY | O_CREAT | O_TRUNC| O_EXCL, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) != EEXIST){
        ret = 1;
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * Create new file on newly created dir
     */
cout << "\tbegin: Open new file(" << NEW_FILE_PATH_5 << ") on new directory" << endl;

    if((ret = doCreat(NEW_FILE_PATH_5, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) != 0){            
        goto out;
    }
    cout << "\tend: Success" << endl;    

out:
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "open_test: Finish" << endl;
    return ret;
}

class GetattrTest : public Test
{
public:
    int runTest();
};

int
GetattrTest::runTest() {
    int ret = 0;
    cout << "getattr_test: Start" << endl;

    cout << "\tbegin: Check entry type of " << NEW_DIR_PATH_1 << endl;
    if((ret = doGetattr(NEW_DIR_PATH_1)) != 0) {
        goto out;
    }
    cout << "\tend: Success" << endl;

out:
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "getattr_test: Finish" << endl;
    return ret;
}


class ReadTest : public Test
{
public:
    int runTest();
};
    
int
ReadTest::runTest()
{
    int ret = 0;
    offset_t off = 0;
    uchar_t *buf = NULL;
    size_t size = 0;
    size_t rsize = 0;
    
    cout << "read_test: Start" << endl;

    /*
     * ファイルサイズ以内のページファイルサイズ以下のサイズの読み込みテスト
     */
    cout << "\tbegin: Read bytes less than file size at offset 0(" << NEW_FILE_PATH_1 << ")" << endl;
    if((ret = doRead(NEW_FILE_PATH_1, (uchar_t *) TEXT_1, 0, strlen(TEXT_1),&rsize)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * ファイルサイズを超えるオフセットからの読み込みテスト
     * 期待される結果は､おそらく pread が 0 を返すこと。
     */
    off = 1024;
    size = 1024;
    cout << "\tbegin: Read bytes at offset(" << off << ") greater than file size(" << NEW_FILE_PATH_1 << ")" << endl;
    if((ret = doRead(NEW_FILE_PATH_1, buf, off, size, &rsize)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;    

  out :
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "read_test: end" << endl;
    return ret;
}

class ReaddirTest: public Test
{
public:
    int runTest();
};

int
ReaddirTest::runTest()
{
    int ret = 0;

    cout << "readdir_test: Start" << endl;

    cout << "\tbegin: look for \".\"" << endl;
    if((ret = doReaddir(DIR_PATH, ".")) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

    cout << "\tbegin: look for \"..\"" << endl;
    if((ret = doReaddir(DIR_PATH, "..")) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

    cout << "\tbegin: look for \"" << NEW_FILE_NAME_1 << "\"" << endl;
    if((ret = doReaddir(DIR_PATH, NEW_FILE_NAME_1)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

out:
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "getattr_test: Finish" << endl;
    return ret;
}

class MmapTest : public Test
{
public:
    int runTest();
};
    
int
MmapTest::runTest()
{
    int ret = 0;
    
    cout << "mmap_test: Start" << endl;

    /*
     * 1K のファイルを作ってマップ
     */
    cout << "\tbegin: Map 1K bytes " << endl;
    if((ret = doMmap(NEW_FILE_PATH_6, 1024)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;        

    /*
     * 4K のファイルを作ってマップ
     */
    cout << "\tbegin: Map 4K bytes " << endl;
    if((ret = doMmap(NEW_FILE_PATH_6, 1024 * 4)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * 8K のファイルを作ってマップ
     */
    cout << "\tbegin: Map 8K bytes " << endl;
    if((ret = doMmap(NEW_FILE_PATH_6, 1024 * 8)) != 0){
        goto out;
    }
    cout << "\tend: Success" << endl;

    /*
     * 1M のファイルを作ってマップ
     */
    cout << "\tbegin: Map 1M bytes " << endl;
    if((ret = doMmap(NEW_FILE_PATH_6, 1024 * 1024)) != 0){
        goto out;
    }

  out :
    if (ret) {
        cout << "\tend: Failed" << endl;
    }
    cout << "mmap_test: end" << endl;
    return ret;
}

void
print_usage(char *name)
{
    cout << "Usage: " << name << " [getattr|readdir|open|read]" << endl;
}


int
main(int argc, char *argv[])
{
    Test *test;
    int ret;

    if (argc != 2) {
        print_usage(argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "getattr") == 0) {
        test = new  GetattrTest();
    } else if (strcmp(argv[1], "readdir") == 0) {
        test = new ReaddirTest();
    } else if (strcmp(argv[1], "open") == 0) {
        test = new OpenTest();
    } else if (strcmp(argv[1], "read") == 0) {
        test = new ReadTest();
    } else if (strcmp(argv[1], "write") == 0) {
        test = new WriteTest();
    } else if (strcmp(argv[1], "remove") == 0) {
        test = new RemoveTest();
    }else if (strcmp(argv[1], "mkdir") == 0) {
        test = new MkdirTest();
    }else if (strcmp(argv[1], "rmdir") == 0) {
        test = new RmdirTest();
    }else if (strcmp(argv[1], "mmap") == 0) {
        test = new MmapTest();
    }else {        
        print_usage(argv[0]);
        exit(1);        
    }

    ret = test->runTest();
    exit(ret);
}

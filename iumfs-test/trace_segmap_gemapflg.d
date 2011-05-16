#pragma D option flowindent

fbt::segmap_getmapflt:entry
{
    @[probefunc] = count();
    printf("tid=%d", tid);    
    self->traceme = 1;
}

fbt::segmap_getmapflt:return
{
    printf("tid=%d", tid);    
    self->traceme = 0;
}

fbt::segmap_fault:entry
/self->traceme/
{
    @[probefunc] = count();
    printf("tid=%d", tid);    
}

fbt::segmap_fault:return
/self->traceme/
{
   printf("tid=%d", tid);  
}

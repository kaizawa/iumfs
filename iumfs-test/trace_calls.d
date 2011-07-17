#pragma D option flowindent
/*
 * Trace iumfs filesystem functions.
 * Usage: sudo dtrace -s trace_calls.d
 */
fbt:iumfs::entry
{
    @[probefunc] = count();
    printf("tid=%d", tid);    

}

fbt:iumfs::return
{
   printf("tid=%d", tid);  
}

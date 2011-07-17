#pragma D option flowindent
/*
 * Count iumfs filesystem functions.
 * Usage: sudo dtrace -s count_calls.d
 */ 
fbt:iumfs::
{
    @[probefunc] = count();
}

tick-5sec 
{
        printa(@[probevunc]);
}



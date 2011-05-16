#pragma D option flowindent

fbt::segmap_fault:
{
    @[probefunc] = count();
}
fbt::segmap_getmapflt:
{
    @[probefunc] = count();
}

tick-10sec 
{
        printa(@[probevunc]);
}



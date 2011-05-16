#pragma D option flowindent

fbt::segmap_getmapflt:entry
{
    self->traceme=1;
}

fbt::segmap_getmapflt:return
{
    self->traceme=0;
}

fbt::uiomove:entry
/self->traceme/
{
    @[probefunc] = count();
}

fbt::pagefault:entry
/self->traceme/
{
    @[probefunc] = count();
}

fbt::segmap_fault:entry
/self->traceme/
{
    @[probefunc] = count();
}

fbt::stand_bcopy:entry
/self->traceme/
{
    @[probefunc] = count();
}

tick-10sec 
{
        printa(@[probevunc]);
}



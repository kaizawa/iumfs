#pragma D option flowindent

fbt::iumfs_write:entry
{
    self->traceme=1;
}

fbt::iumfs_write:return
{
    self->traceme=0;
}

fbt::uiomove:entry
/self->traceme/
{
    @[probefunc] = count();
}

fbt::xcopyin_nta:entry /self->traceme/ { @[probefunc] = count(); }
fbt::as_fault:entry /self->traceme/ { @[probefunc] = count(); }
fbt::trap:entry /self->traceme/ { @[probefunc] = count(); }
fbt::ksyms_bcopy:entry /self->traceme/ { @[probefunc] = count(); }

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



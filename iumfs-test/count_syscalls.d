#pragma D option flowindent
/*
 * Count number of system calls of target process within 1 sec.
 * Usage: sudo dtrace -s count_syscalls.d  -p <TARGET_PID>
 */
syscall:::entry
/pid==$target/
{
	@[probefunc] = count()
}
tick-1sec {
	trunc(@,10);printa(@)
}

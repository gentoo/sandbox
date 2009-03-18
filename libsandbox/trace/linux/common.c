static long do_peekuser(long offset);

static int trace_errno(long err)
{
	return (err < 0 && err > -4096) ? err * -1 : 0;
}

static long trace_result(void *vregs, int *error)
{
	long sr = trace_raw_ret(vregs);
	*error = trace_errno(sr);
	return *error ? -1 : sr;
}

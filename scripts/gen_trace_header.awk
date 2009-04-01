BEGIN {
	COUNT = split(" " SYMBOLS_LIST, SYMBOLS);

	if (MODE == "gen") {
		for (x in SYMBOLS) {
			s = SYMBOLS[x]
			print "SB_" s " = SYS_" s
		}
		exit(0);
	}
}

function out(name, val)
{
	name = toupper(name)
	print "#define SB_SYS" syscall_prefix "_" name " " val;
	print "S(" name ")";
}

{
	# found:     SB_func = #
	# not found: SB_func = SYS_func
	if ($1 !~ /^SB_/)
		next;
	if ($3 ~ /^SYS_/)
		next;

	sub(/^SB_/, "", $1);

	for (i = 1; i <= COUNT; ++i)
		if (SYMBOLS[i] == $1) {
			SYMBOLS[i] = "";
			break;
		}

	out($1, $3);
}

END {
	if (MODE != "gen") {
		for (x in SYMBOLS) {
			s = SYMBOLS[x];
			if (s != "")
				out(s, "SB_NR_UNDEF");
		}
	}
}

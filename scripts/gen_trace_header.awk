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
	# found:     SB_func = <something>
	# not found: SB_func = SYS_func
	if ($1 !~ /^SB_/)
		next;
	if ($3 ~ /^SYS_/)
		next;

	sub(/^SB_/, "", $1);
	name = $1
	# accept everything after the "=" in case it's either
	# a straight number or an expression (a syscall base)
	sub(/^[^=]*= /, "");

	for (i = 1; i <= COUNT; ++i)
		if (SYMBOLS[i] == name) {
			SYMBOLS[i] = "";
			break;
		}

	out(name, $0);
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

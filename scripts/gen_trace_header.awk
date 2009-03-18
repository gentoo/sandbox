BEGIN {
	COUNT = split(" " SYMBOLS_LIST, SYMBOLS);
}

{
	if ($1 != "#define" || $2 !~ /^SYS_/)
		next;

	sub(/^SYS_/, "", $2);

	for (i = 1; i <= COUNT; ++i)
		if (SYMBOLS[i] == $2) {
			SYMBOLS[i] = "";
			break;
		}

	print "S(" $2 ")";
}

END {
	for (x in SYMBOLS) {
		s = SYMBOLS[x];
		if (s != "") {
			print "#define SYS_" s " SB_NR_UNDEF";
			print "S(" s ")";
		}
	}
}

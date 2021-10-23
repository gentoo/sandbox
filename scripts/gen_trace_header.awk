# Read the symbols list and create regexs to use for processing readelf output.
function read_symbols() {
	COUNT = 0;
	while ((getline line < SYMBOLS_FILE) > 0) {
		if (line ~ /^ *#/ || line ~ /^$/)
			continue;
		nfields = split(line, fields);
		symbol = fields[1];
		syscall = nfields > 1 ? fields[2] : symbol;

		c = ++COUNT
		SYMBOLS[c] = symbol;
		SYSCALLS[c] = syscall;
	}
}

BEGIN {
	read_symbols();

	if (MODE == "gen") {
		for (x in SYSCALLS) {
			print "SB_" SYMBOLS[x] " = SYS_" SYSCALLS[x];
		}
		exit(0);
	}
}

function out(name, syscall, val)
{
	uname = toupper(name)
	syscall_define = "SB_SYS" syscall_prefix "_" uname
	print "#define " syscall_define " " val;
	if (name == syscall)
		print "S(" uname ")";
	else
		print "{ " syscall_define ", SB_NR_" uname ", \"" uname "\" },";
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

	syscall = "<awk_script_error>";
	for (i = 1; i <= COUNT; ++i) {
		if (SYMBOLS[i] == name) {
			FOUND[i] = 1;
			syscall = SYSCALLS[i];
			break;
		}
	}

	out(name, syscall, $0);
}

END {
	if (MODE != "gen") {
		for (x in SYMBOLS) {
			if (!FOUND[x])
				out(SYMBOLS[x], SYSCALLS[x], "SB_NR_UNDEF");
		}
	}
}

# Read the symbols list and create regexs to use for processing readelf output.
BEGIN {
	COUNT = 0;

	sym_regex = "";
	while ((getline line < SYMBOLS_FILE) > 0) {
		if (line ~ /^ *#/ || line ~ /^$/)
			continue;
		split(line, fields);
		symbol = fields[1];

		SYMBOLS[++COUNT] = symbol;
		if (sym_regex)
			sym_regex = sym_regex "|";
		sym_regex = sym_regex symbol;
	}
	SYMBOL_REGEX = "^(" sym_regex ")(@|$)";
	WEAK_SYMBOL_REGEX = "^__(" sym_regx ")(@@|$)";
}

/^  OS\/ABI:/ {
	ABI = $NF
}

{
	# Unstripped libc's have '.symtab' section as well, and
	# we should stop processing when we hit that
	if ($0 ~ "^Symbol (.*)table '.symtab'")
		nextfile;

	# On x86, x86_64 and others, $8 is the symbol name, but on
	# alpha, its $10, so rather use $NF, as it should be the
	# last field
	if ($NF ~ SYMBOL_REGEX) {
		split($NF, symbol_array, /@|@@/);

		# Don't add local symbols of versioned libc's
		if (VERSIONED_LIBC && !symbol_array[2])
			next;

		# We have a versioned libc
		if (symbol_array[2] && !VERSIONED_LIBC)
			VERSIONED_LIBC = 1;

		ADD = 1;
		# Check that we do not add duplicates
		for (y in PROCESSED_SYMBOLS) {
			if (y == $NF) {
				ADD = 0;
				break;
			}
		}

		if (ADD) {
			SYMBOL_LIST[symbol_array[1]] = SYMBOL_LIST[symbol_array[1]] " " $NF;
			PROCESSED_SYMBOLS[$NF] = $NF;
		}
	}

	# No apparent need to handle weak __XXX symbols ... so disable
	# until we have documentation on why ...
	# If we do re-add this, need to update the `readelf` call in
	# libsandbox/ to include the -h flag again.
	next;

	if (($5 == "WEAK") && ($NF ~ WEAK_SYMBOL_REGEX)) {
		split($NF, symbol_array, /@@/);

		# Don't add local symbols of versioned libc's
		if (VERSIONED_LIBC && !symbol_array[2])
			next;

		# Blacklist __getcwd on FreeBSD
		# Unleashed - May 2006
		if ((symbol_array[1] == "__getcwd") && (ABI == "FreeBSD"))
			next;

		# We have a versioned libc
		if (symbol_array[2] && !VERSIONED_LIBC)
			VERSIONED_LIBC = 1;

		ADD = 1;
		# Check that we do not add duplicates
		for (y in PROCESSED_SYMBOLS) {
			if (y == $NF) {
				ADD = 0;
				break;
			}
		}

		if (ADD) {
			WEAK_SYMBOLS[SYMBOLS[x]] = WEAK_SYMBOLS[SYMBOLS[x]] " " $NF;
			PROCESSED_SYMBOLS[$NF] = $NF;
		}
	}
}

END {
	printf("#ifndef __symbols_h\n");
	printf("#define __symbols_h\n\n");

	SB_MAX_STRING_LEN = 0

	# We use the order in SYMBOLS, as some wrappers depends on others ...
	for (i = 1; i <= COUNT; ++i) {
		sym_index = SYMBOLS[i];
		full_count = split(SYMBOL_LIST[sym_index], sym_full_names);

		if (length(sym_index) > SB_MAX_STRING_LEN)
			SB_MAX_STRING_LEN = length(sym_index);

		if (full_count == 0)
			printf("#define SB_NR_%s (SB_NR_UNDEF - %i)\n", toupper(sym_index), i);

		for (x in sym_full_names) {
			split(sym_full_names[x], symbol_array, /@|@@/);

			# Defualt symbol have '@@' and not '@', so name it by
			# prepending '__' rather than the symbol version so
			# that we know what the name is in libsandbox.c ...
			# Also do this for non-versioned libc's ...
			if (sym_full_names[x] ~ /@@/ || !symbol_array[2]) {
				sym_is_default = 1;
				sym_real_name = sym_index "_DEFAULT";
			} else {
				sym_is_default = 0;
				sym_real_name = sym_full_names[x];
				gsub(/@|\./, "_", sym_real_name);
			}

			# Avoid libc's symbol rename via #define. musl defines aliases as:
			# #define mkstemp64 mkstemp
			# #define mkstemps64 mkstemps
			printf("#undef %s\n", sym_index);
			printf("#define symname_%s \"%s\"\n", sym_real_name, sym_index);

			# We handle non-versioned libc's by setting symver_*
			# to NULL ...
			if (!symbol_array[2])
				printf("#define symver_%s NULL\n", sym_real_name);
			else
				printf("#define symver_%s \"%s\"\n", sym_real_name,
				       symbol_array[2]);

			printf("#define STRING_NAME \"%s\"\n", sym_index);
			printf("#define EXTERN_NAME %s\n", sym_index);
			printf("#define WRAPPER_NAME %s\n", sym_real_name);
			printf("#define WRAPPER_TRUE_NAME true_%s\n", sym_real_name);
			printf("#define WRAPPER_SYMNAME symname_%s\n", sym_real_name);
			printf("#define WRAPPER_SYMVER symver_%s\n", sym_real_name);
			printf("#define SB_NR_%s %i\n", toupper(sym_index), i);
			printf("#define WRAPPER_NR SB_NR_%s\n", toupper(sym_index));
			printf("#include \"wrapper-funcs/%s.c\"\n", sym_index);
			pre_check = "wrapper-funcs/" sym_index "_pre_check.c"
			if (sym_is_default && system("test -e " srcdir "/" pre_check) == 0)
				printf("#include \"%s\"\n", pre_check);
			printf("#undef STRING_NAME\n");
			printf("#undef EXTERN_NAME\n");
			printf("#undef WRAPPER_NAME\n");
			printf("#undef WRAPPER_TRUE_NAME\n");
			printf("#undef WRAPPER_SYMNAME\n");
			printf("#undef WRAPPER_SYMVER\n");
			printf("#undef WRAPPER_NR\n");
			printf("#undef WRAPPER_ARGS\n");
			printf("#undef WRAPPER_ARGS_PROTO\n");
			printf("#undef WRAPPER_SAFE\n");
			printf("#undef WRAPPER_RET_TYPE\n");
			printf("#undef WRAPPER_RET_DEFAULT\n");
			printf("#undef WRAPPER_PRE_CHECKS\n");

			if (symbol_array[2]) {
				# Only add symbol versions for versioned libc's
				if (sym_full_names[x] ~ /@@/)
					printf("default_symbol_version(%s, %s, %s);\n",
					       sym_real_name, sym_index, symbol_array[2]);
				else
					printf("symbol_version(%s, %s, %s);\n",
					       sym_real_name, sym_index, symbol_array[2]);
			} else {
				# For non-versioned libc's we use strong aliases
				printf("strong_alias(%s, %s);\n", sym_real_name,
				       sym_index);
			}

			if (WEAK_SYMBOLS[sym_index]) {
				split(WEAK_SYMBOLS[sym_index], sym_weak_full);

				for (y in sym_weak_full) {
					split(sym_weak_full[y], sym_weak_array, /@@/);

					# Add weak symbols for libc's like glibc that
					# have them
					if (sym_weak_array[1] == "__" sym_index)
						    printf("weak_alias(%s, %s);\n",
							   sym_real_name,
							   sym_weak_array[1]);
				}
			}

			printf("\n");
		}
	}

	printf("#define SB_MAX_STRING_LEN %i\n\n", SB_MAX_STRING_LEN);

	printf("#endif /* __symbols_h */\n");
}

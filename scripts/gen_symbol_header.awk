BEGIN {
	COUNT = split(ENVIRON["SYMBOLS"], SYMBOLS);
}

{
	# Unstripped libc's have '.symtab' section as well, and
	# we should stop processing when we hit that
	if ($0 ~ "^Symbol (.*)table '.symtab'")
		nextfile;

	for (x in SYMBOLS) {
		sym_regex = "^" SYMBOLS[x] "(@|$)";
		if ($8 ~ sym_regex) {
			split($8, symbol_array, /@|@@/);

			# Don't add local symbols of versioned libc's
			if (VERSIONED_LIBC && !symbol_array[2])
				continue;

			# We have a versioned libc
			if (symbol_array[2] && !VERSIONED_LIBC)
			    	VERSIONED_LIBC = 1;

			ADD = 1;
			# Check that we do not add duplicates
			for (y in PROCESSED_SYMBOLS) {
				if (y == $8) {
					ADD = 0;
					break;
				}
			}
			
			if (ADD) {
				SYMBOL_LIST[symbol_array[1]] = SYMBOL_LIST[symbol_array[1]] " " $8;
				PROCESSED_SYMBOLS[$8] = $8;
			}
		}

		sym_regex = "^__" SYMBOLS[x] "(@@|$)";
		if (($5 == "WEAK") && ($8 ~ sym_regex)) {
			split($8, symbol_array, /@@/);
			
			# Don't add local symbols of versioned libc's
			if (VERSIONED_LIBC && !symbol_array[2])
				continue;

			# We have a versioned libc
			if (symbol_array[2] && !VERSIONED_LIBC)
			    	VERSIONED_LIBC = 1;

			ADD = 1;
			# Check that we do not add duplicates
			for (y in PROCESSED_SYMBOLS) {
				if (y == $8) {
					ADD = 0;
					break;
				}
			}
			
			if (ADD) {
				WEAK_SYMBOLS[SYMBOLS[x]] = WEAK_SYMBOLS[SYMBOLS[x]] " " $8;
				PROCESSED_SYMBOLS[$8] = $8;
			}
		}
	}
}

END {
	printf("#ifndef __symbols_h\n");
	printf("#define __symbols_h\n\n");

	# We use the order in SYMBOLS, as some wrappers depends on others ...
	for (i = 1; i <= COUNT; i++) {
		sym_index = SYMBOLS[i];
		split(SYMBOL_LIST[sym_index], sym_full_names);
		
		for (x in sym_full_names) {
			split(sym_full_names[x], symbol_array, /@|@@/);
	
			# Defualt symbol have '@@' and not '@', so name it by
			# prepending '__' rather than the symbol version so
			# that we know what the name is in libsandbox.c ...
			# Also do this for non-versioned libc's ...
			if (sym_full_names[x] ~ /@@/ || !symbol_array[2]) {
				sym_real_name = sym_index "_DEFAULT";
			} else {
				sym_real_name = sym_full_names[x];
				gsub(/@|\./, "_", sym_real_name);
			}
			
			printf("#define symname_%s \"%s\"\n", sym_real_name, sym_index);

			# We handle non-versioned libc's by setting symver_*
			# to NULL ...
			if (!symbol_array[2])
				printf("#define symver_%s NULL\n", sym_real_name);
			else
				printf("#define symver_%s \"%s\"\n", sym_real_name,
				       symbol_array[2]);
			
			printf("%s_decl(%s);\n", sym_index, sym_real_name);
		
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
							   "__" sym_index);
				}
			}
			
			printf("\n");
		}
	}

	printf("#endif /* __symbols_h */\n");
}

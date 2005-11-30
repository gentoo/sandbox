BEGIN {
	COUNT = split(ENVIRON["SYMBOLS"], SYMBOLS);
}

{
	for (x in SYMBOLS) {
		sym_regex = "^" SYMBOLS[x] "(@|$)"
		if ($8 ~ sym_regex) {
			split($8, symbol_array, /@|@@/);

			SYMBOL_LIST[symbol_array[1]] = SYMBOL_LIST[symbol_array[1]] " " $8;
		}
	}
}

END {
	printf("#ifndef __symbols_h\n");
	printf("#define __symbols_h\n\n");

	for (i = 1; i <= COUNT; i++) {
		sym_index = SYMBOLS[i];
		split(SYMBOL_LIST[sym_index], sym_full_names);
		
		for (x in sym_full_names) {
			split(sym_full_names[x], symbol_array, /@|@@/);
	
			# Defualt symbol have '@@' and not '@', so name it by
			# prepending '__' rather than the symbol version so
			# that we know what the name is in libsandbox.c ...
			if (sym_full_names[x] ~ /@@/) {
				sym_real_name = "__" sym_index;
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
		
			# Only add symbol versions for versioned libc's
			if (symbol_array[2]) {
				if (sym_full_names[x] ~ /@@/)
					printf("default_symbol_version(%s, %s, %s);\n",
					       sym_real_name, sym_index, symbol_array[2]);
				else
					printf("symbol_version(%s, %s, %s);\n",
					       sym_real_name, sym_index, symbol_array[2]);
			}
			
			printf("\n");
		}
	}

	printf("#endif /* __symbols_h */\n");
}

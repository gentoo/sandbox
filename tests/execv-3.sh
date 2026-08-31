#!/bin/sh
# a file that is not an ELF must not be walked as one

addwrite $PWD

# an ELF header shaped well enough to be walked, but without the magic:
# a program header claiming a PT_DYNAMIC nowhere near the mapping
dd if=/dev/zero of=notelf bs=1 count=128 2>/dev/null || exit 1
poke() { printf "$1" | dd of=notelf bs=1 seek=$2 conv=notrunc 2>/dev/null; }
poke '\002' 4                       # e_ident[EI_CLASS] = ELFCLASS64
poke '\100' 32                      # e_phoff = 64
poke '\070' 54                      # e_phentsize = 56
poke '\001' 56                      # e_phnum = 1
poke '\002' 64                      # phdr[0].p_type = PT_DYNAMIC
poke '\377\377\377\377\177' 74      # phdr[0].p_offset = 0x7fffffff0000

# not executable, so the exec fails once the wrapper has had its look
chmod a-x notelf
./notelf
test $? -eq 126

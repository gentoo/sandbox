#!/bin/sh
# http://bugs.gentoo.org/139591
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0
addwrite $PWD

cat << EOF > test1
#!/bin/sh
export PATH="\`pwd\`/:\${PATH}"
test2
EOF

cat << EOF > test2
#!/bin/sh
test3
EOF

cat << EOF > test3
#!/bin/sh
printf hi
EOF

chmod a+rx test[123]

./test1

exit 0

#!/bin/sh
# http://bugs.gentoo.org/139591
addwrite $PWD
td=".tmp.${0##*/}"
rm -rf $td
mkdir $td
pushd $td >/dev/null

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

popd >/dev/null
rm -rf $td
exit 0

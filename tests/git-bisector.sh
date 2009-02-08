#!/bin/sh

if [ $# -ne 3 ] ; then
	echo "Usage: $0 <test script> <bad> <good> [... more opts to git bisect ...]"
	exit 1
fi

script=$1
shift
if [ ! -x "$script" ] ; then
	echo "Script is not executable: $script"
	exit 1
fi

set -e
cd "${0%/*}/.."

cp "$script" git-run-sandbox.sh
rm -f config.cache
make="make -s -j"
cat << EOF > git-run.sh
#!/bin/sh
./autogen.sh
./configure -q -C $(sandbox -V | tail -n1)
${make} clean
${make}
./src/sandbox.sh ./git-run-sandbox.sh
EOF
chmod a+rx git-run.sh

git bisect start "$@"
exec git bisect run ./git-run.sh

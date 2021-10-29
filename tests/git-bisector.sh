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
# Newer versions of sandbox can run configure for us.
# Should drop old support around Jan 2023.
if sandbox --help | grep -q -e--run-configure ; then
	sandbox --run-configure -q -C
else
	./configure -q -C $(sandbox -V | tail -n1)
fi
${make} clean
${make}
opt=
# Older versions of sandbox implied -c all the time.
if ./src/sandbox.sh --help | grep -q -e--bash ; then
	opt="-c"
fi
./src/sandbox.sh ${opt} . ./data/sandbox.bashrc \; . ./git-run-sandbox.sh
EOF
chmod a+rx git-run.sh

git bisect start "$@"
exec git bisect run ./git-run.sh

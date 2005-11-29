#!/bin/tcsh
# Build utils into /cluster/bin/$MACHTYPE on dev or beta from branch or tip
# specify "tip" as command line parm 1 to build from tip sandbox 

cd $WEEKLYBLD

if ( "$MACHTYPE" == "i386" ) then
    if ( "$HOST" != "hgwdev" ) then
	echo "error: you must run this script on hgwdev!"
	exit 1
    endif
endif
if ( "$MACHTYPE" == "x86_64" ) then
    if ( "$HOST" != "hgwbeta" ) then
	echo "error: you must run this script on hgwbeta!"
	exit 1
    endif
endif

set branch=v${BRANCHNN}_branch

if ( "$1" == "tip" ) then
    set base=$WEEKLYBLD/hiding
    echo "updating tip sandbox"
    cd $base/kent
    cvs up -dP  >& /dev/null
    echo "done updating tip sandbox"
    cd $WEEKLYBLD
else
    set base=$BUILDDIR/$branch
endif

if ( -d ~/bin/${MACHTYPE}.orig ) then
 echo "restoring from last failed symlink."
 ./unsymtrick.csh
endif
if ( ! -d ~/bin/${MACHTYPE}.cluster ) then
 echo "something messed up in symlink"
 exit 1
endif

# Symlink Trick safe now
echo "Symlink Trick."
./symtrick.csh

if ( "$MACHTYPE" == "i386" ) then
    echo
    echo "Doing make clean on src."
    cd $base/kent/src
    make clean>& make.clean.log
endif

echo
echo "Building src utils."
cd $base/kent/src
make utils >& make.utils.log
make blatSuite >>& make.utils.log
sed -i -e "s/-DJK_WARN//" make.utils.log
sed -i -e "s/-Werror//" make.utils.log
#-- to check for errors: 
set res = `/bin/egrep -i "error|warn" make.utils.log`
set wc = `echo "$res" | wc -w` 
if ( "$wc" != "0" ) then
 echo "errs found:"
 echo "$res"
 $WEEKLYBLD/unsymtrick.csh
 exit 1
endif

# Undo Symlink trick
$WEEKLYBLD/unsymtrick.csh
echo "Restore: undoing Symlink Trick."

if ( "$MACHTYPE" == "i386" ) then
    echo
    echo "Doing make clean on src."
    cd $base/kent/src
    make clean>& make.clean.log
endif

echo
echo "Build of Utils on $HOST complete."
echo

exit 0


#!/bin/tcsh
cd $WEEKLYBLD

# ------------------------------------
# Note - this script assumes you have your ssh key in
# qateam@hgdownload:.ssh/authorized_keys. Without it,
#  this script can NOT be launched from beta
#  using something like ssh $BOX32 $WEEKLYBLD/buildCgi32.csh
#  because when scp needs the password typed in, apparently
#  the stdin is not available from the terminal.
# Instead, log directly into box32 and execute the script.
#  then when prompted for the password, put in the qateam pwd. 
# ------------------------------------

if (("$HOST" != "$BOX32") && ("$HOST" != "hgwbeta")) then
 echo "error: you must run this script on $BOX32 or on hgwbeta!"
 exit 1
endif

set ScriptStart=`date`

echo "Cleaning out $BUILDDIR/userApps"
rm -fr $BUILDDIR/userApps
mkdir $BUILDDIR/userApps

cd $BUILDDIR/userApps

echo "Checking out kent/src branch $BRANCHNN. [${0}: `date`]"

cvs -d hgwdev:$CVSROOT co -r "v"$BRANCHNN"_branch" kent/src >& /dev/null
set err = $status
if ( $err ) then
 echo "error running cvs co kent/src in $BUILDDIR/userApps : $err [${0}: `date`]" 
 exit 1
endif

set BINDIR=linux.$MACHTYPE
set DESTDIR=/usr/local/apache/htdocs/admin/exe/
if ("$HOST" == "$BOX32") then
  set DESTDIR=/tmp/
  rm -rf $DESTDIR/$BINDIR
  mkdir $DESTDIR/$BINDIR
endif

cd kent/src 
make BINDIR=$BINDIR DESTDIR=$DESTDIR userApps
./utils/userApps/mkREADME.sh $DESTDIR/$BINDIR FOOTER
cd ../..

# copy everything if 64 bit
if ("$HOST" == "hgwbeta") then
  scp -p ${DESTDIR}/${BINDIR}/* qateam@hgdownload:/mirrordata/apache/htdocs/admin/exe/$BINDIR/
endif

# copy liftOver if 32 bit
if ("$HOST" == "$BOX32") then
  scp -p ${DESTDIR}/${BINDIR}/liftOver qateam@hgdownload:/mirrordata/apache/htdocs/admin/exe/$BINDIR/
endif

echo "userApps $MACHTYPE built on $HOST and scp'd to hgdownload [${0}: START=${ScriptStart} END=`date`]"

exit 0


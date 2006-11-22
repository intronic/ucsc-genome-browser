#!/bin/tcsh

################################
#  
#  11-17-06
#  Robert Kuhn
#
#  checks an entire db between two nodes in realTime
#
################################

set db=""
set host1=""
set host2="-h hgwbeta"
set mach1="hgwbeta"
set mach2="hgw1"
set times=0

if ( $#argv < 1 || $#argv > 4 ) then
  echo
  echo "  checks an entire db between two nodes in realTime"
  echo "  ignores genbank tables"
  echo
  echo "    usage:  database [machine1 machine2] [times]"
  echo "              defaults to beta and hgw1"
  echo
  exit
else
  set db=$argv[1]
endif

if ( "$HOST" != "hgwdev" ) then
 echo "\n error: you must run this script on dev!\n"
 exit 1
endif


if ( $#argv == 2 ) then
  if ( $argv[2] == "times" ) then
    set times=1
  else
    echo
    echo " you must specify two machines if you specify one"
    echo
    echo ${0}:
    $0
    exit 1
  endif
endif

if ( $#argv == 3 || $#argv == 4 ) then
  set mach1=$argv[2]
  set mach2=$argv[3]
endif

if ( $#argv == 4 ) then
  if ( $argv[4] == "times" ) then
    set times=1
  else
    echo
    echo ' the fourth argument must be "times"\n'
    echo ${0}:
    $0
    exit 1
  endif
endif

if ( 1 == $times) then
  # get the table and update times, stripping out genbank
  ssh -x qateam@$mach1 mysql $db -A -N \
    -e '"'SHOW TABLE STATUS'"' | awk '{print $1, $13, $14}' > $mach1.status
  ssh -x qateam@$mach2 mysql $db -A -N \
    -e '"'SHOW TABLE STATUS'"' | awk '{print $1, $13, $14}' > $mach2.status
  cat /cluster/data/genbank/etc/genbank.tbls | sed -e 's/^^//; s/.$//' \
    > genbank.local
  cat $mach1.status | egrep -v -f genbank.local > $mach1.status2
  cat $mach2.status | egrep -v -f genbank.local > $mach2.status2
  commTrio.csh $mach1.status2 $mach2.status2 > trioFile
  cat trioFile

  # process the times of those tables that do not match
  set firstOnly=`cat trioFile | sed -n '1p' | awk '{print $1}'`
  set secondOnly=`cat trioFile | sed -n '2p' | awk '{print $1}'`
  rm -f outFile
  if ( 0 != $firstOnly || 0 !=  $secondOnly ) then
    cat $mach1.status2.Only | sed -e "s/^/$mach1\t/" >> outFile
    cat $mach2.status2.Only | sed -e "s/^/$mach2\t/" >> outFile
    sort -k2 outFile 
    echo
  endif
else
  ssh -x qateam@$mach1 mysql $db -A -N \
    -e '"'SHOW TABLES'"' > $mach1.tables
  ssh -x qateam@$mach2 mysql $db -A -N \
    -e '"'SHOW TABLES'"' > $mach2.tables
  echo
  commTrio.csh $mach1.tables $mach2.tables
endif

# allow files to pile up:
exit

rm -f $mach1.tables
rm -f $mach2.tables
rm -f $mach1*status*
rm -f $mach2*status*
rm -f "$mach1*Only"
rm -f "$mach2*Only"
rm -f trioFile
rm -f outFile
rm -f genbank.local

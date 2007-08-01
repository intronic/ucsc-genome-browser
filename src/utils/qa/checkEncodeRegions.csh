#!/bin/tcsh

################################
#
# 07-26-2007
# Ann Zweig
#
# This script is to be used on ENCODE tables.  It makes sure that
# the vales in the table fall within the ENCODE Regions.
#
################################

set num=""
set db=""
set tablelist=""
set count=""
set column=""
set theStart=""

# usage statement
if ( $#argv < 2 || $#argv > 3 ) then
  echo
  echo " Determines if all items in a table are within the ENCODE Regions."
  echo
  echo "  usage:  database tablelist (will accept single table)"
  echo "          [count] optional: list of count per chrom"
  echo
  exit
else
  set db=$argv[1]
  set tablelist=$argv[2]
endif

if ( $#argv == 3 ) then
  set count=$argv[3]
endif

# run this only on hgwdev
if ( "$HOST" != "hgwdev" ) then
  echo "\n ERROR: you must run this script on dev!\n"
  exit 1
endif

# make sure this is an assembly that has encode tables
if ( $db != hg16 && $db != hg17 && $db != hg18 ) then
  echo "\n ERROR: you must run this script on a db that has encode tables\n"
  exit 1
endif

# check if it is a file or a tablename
file $tablelist | egrep "ASCII text" > /dev/null
if (! $status) then
  set tables=`cat $tablelist`
 else
  set tables=$tablelist
endif

# loop through all of the tables found in the previous statement.
foreach tbl ($tables)  
  # make sure it's an encode table
  set match = `echo $tbl | egrep '^encode' | wc -l`
  if ( $match != 1 ) then
    echo "\n ERROR: $tbl is not an ENCODE table"
    exit 1
  endif

  # find out if the table has chromStart and/or txStart field 
  set column=`hgsql -Ne 'show columns from '$tbl' like "%Start"' $db`
  # if the table doesn't exist, it would be nice to print error and exit...

  set theStart=`echo $column | awk '{print $1}'`

  switch ($theStart)
   case 'chromStart':
     set num=`hgsql -Ne 'SELECT count(*) FROM '$tbl', encodeRegions \
      WHERE '$tbl'.chrom = encodeRegions.chrom \
      AND '$tbl'.chromStart <= encodeRegions.chromStart \
      AND '$tbl'.chromEnd >= encodeRegions.chromEnd' $db`
     breaksw
   case 'txStart':
     set num=`hgsql -Ne 'SELECT count(*) FROM '$tbl', encodeRegions \
      WHERE '$tbl'.chrom = encodeRegions.chrom \ 
      AND '$tbl'.txStart <= encodeRegions.chromStart \
      AND '$tbl'.txEnd >= encodeRegions.chromEnd' $db`
     breaksw
   default:
     echo "\n ERROR: the $tbl table has no chromStart or txStart columns.\n"
     exit 1
     breaksw
  endsw

  if ($num != 0) then
    echo "\n ERROR: The $tbl table has $num items OUTSIDE of ENCODE Regions\n"
  else
    echo "\n$tbl table: OK\n"
  endif

  if ( $count == 'count') then
    echo "\nNumber of items per chromosome for $tbl table:\n"
    countPerChrom.csh $db $tbl
  endif
 
  set num=0
  set column=""
  set theStart=""

end #foreach
exit

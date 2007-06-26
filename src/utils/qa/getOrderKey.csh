#!/bin/tcsh

################################
#  
#  06-26-07
#  Robert Kuhn
#
#  gets list of orderKeys from two machines between two limits
#
################################

set lowerLimit=0
set upperLimit=0

if ( $#argv != 2 ) then
  echo
  echo "  gets list of orderKeys from all machines between two limits."
  echo
  echo "    usage:  lowerLimit upperLimit "
  echo
  exit
else
  set lowerLimit=$argv[1]
  set upperLimit=$argv[2]
endif

if ( "$HOST" != "hgwdev" ) then
 echo "\n error: you must run this script on dev!\n"
 exit 1
endif

# get list of all dbs in range at all three levels
set dev=`hgsql -N -e "SELECT name FROM dbDb \
  WHERE orderKey >= $lowerLimit and orderKey <= $upperLimit \
  ORDER BY orderKey" hgcentraltest`

set beta=`hgsql -N -h hgwbeta -e "SELECT name FROM dbDb \
  WHERE orderKey >= $lowerLimit and orderKey <= $upperLimit \
  ORDER BY orderKey" hgcentralbeta`

set rr=`hgsql -N -h genome-centdb -e "SELECT name FROM dbDb \
  WHERE orderKey >= $lowerLimit and orderKey <= $upperLimit \
  ORDER BY orderKey" hgcentral`

set all=`echo $dev $beta $rr | sed -e "s/ /\n/g" | sort -u`

# get orderKey value at all three levels
rm -f orderKeyOutfile
foreach db ( $all )
  set dev=0
  set beta=0
  set rr=0
  set dev=`hgsql -N -e 'SELECT orderKey FROM dbDb \
    WHERE name = "'$db'"' hgcentraltest`
  set beta=`hgsql -N -h hgwbeta -e 'SELECT orderKey FROM dbDb \
    WHERE name = "'$db'"' hgcentralbeta`
  set rr=`hgsql -N -h genome-centdb -e 'SELECT orderKey FROM dbDb \
    WHERE name = "'$db'"' hgcentral`
  echo "$db $dev $beta $rr" | gawk '{ printf("%-8s  %3s  %3s  %3s \n", \
    $1, $2, $3, $4) }' >> orderKeyOutfile
end

echo
if ( -e orderKeyOutfile ) then
  sort -k2 -n orderKeyOutfile
else
  echo "none found"
endif
echo

rm -f orderKeyOutfile


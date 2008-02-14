#!/bin/tcsh

################################
#  
#  02-24-07
#  Robert Kuhn
#
#  runs sync on the database of the day.
#
################################



if ( $#argv != 1 ) then
  echo
  echo "  runs sync on the database of the day."
  echo "  replaces file in htdocs if run again same day."
  echo
  echo "    usage:  go"
  echo
  exit
endif

if ( "$HOST" != "hgwdev" ) then
 echo "\n error: you must run this script on dev!\n"
 exit 1
endif

set basePath='/usr/local/apache/htdocs/qa/test-results/sync'
set db=`databaseAday.csh today`
rm -f $basePath/$db
echo "\n$db\n" >> $basePath/$db
checkSync.csh $db times >> $basePath/$db
checkSync.csh $db hgw1 hgw8 times >> $basePath/$db
checkSync.csh $db hgw2 hgw3 times >> $basePath/$db
checkSync.csh $db hgw4 hgw5 times >> $basePath/$db
checkSync.csh $db hgw6 hgw7 times >> $basePath/$db
cat $basePath/$db
echo "http://hgwdev.cse.ucsc.edu/qa/test-results/sync/$db" | mail -s "sync for today" $USER@soe.ucsc.edu


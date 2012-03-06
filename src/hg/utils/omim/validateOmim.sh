#!/bin/sh -e

db=$1
tooMuch=0.2500   # how much change (either gain or loss) is too much

for i in `cat ../../omim.tables`
do 
    echo "select * from $i" |  hgsql $db | tail -n +2 | sort > $i.out
    f=$i"New"
    echo "select * from $f" |hgsql $db | tail -n +2 | sort > $f.out
    oldCount=`cat $i.out | wc -l`
    newCount=`cat $f.out | wc -l`
    common=`join -t '\001'  $i.out $f.out | wc -l`
    onlyOld=`join -t '\001' -v 1 $i.out $f.out | wc -l`
    onlyNew=`join -t '\001' -v 2 $i.out $f.out | wc -l`
    echo $i $newCount "-" $onlyNew "=" $common "=" $oldCount "-" $onlyOld  
    rm $i.out $f.out
done > newOmim.stats

cat newOmim.stats | awk -v db=$db -v tooMuch=$tooMuch ' 
{
    if (($4/$6 > tooMuch) || ($10/$6 > tooMuch))
	{
	print "valdate on " db "." $1 " failed:" $4,$6,$4/$6,$10,$6,$10/$6;
	exit 1
	}
}'

exit 0

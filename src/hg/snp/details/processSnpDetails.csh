#!/bin/csh

# check usage
if ($# != 3) then
    echo "Usage:   processSnpDetails.csh Database Organism dbSnpBuild"
    echo "Example: processSnpDetails.csh hg16     human    119"
    exit
endif

# parse input
set database  = $1
set organism  = $2
set snpBuild  = $3
set oo        = /cluster/data/$database
set fileBase  = dbSnpRS${database}Build$snpBuild
set DETAILDIR = /kent/src/hg/snp/details

# Run from directory $oo/bed/snp/build$snpBuild/details
mkdir -p $oo/bed/snp/build$snpBuild/details/Done
mkdir -p $oo/bed/snp/build$snpBuild/details/Observed
cd       $oo/bed/snp/build$snpBuild/details
# remove existing files that might get in the way.
rm -f snp* $organism* bed.tab seq_contig.md liftAll.lft Done/* Observed/* \
    |& grep -v "No match."

echo `date` Start command: processSnpDetails.csh $argv
echo `date` Files: \
    ftp://ftp.ncbi.nlm.nih.gov/snp/${organism}/XML/ds_chXX.xml.gz

if ("$organism" == "human") then
    set chroms = \
	"1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 X Y M"
    set table  = dbSnpRsHg
else if ("$organism" == "mouse") then
    set chroms = \
	"1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 X MT"
    set table = dbSnpRsMm
else if ("$organism" == "rat") then
    set chroms = \
	"1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 X Y M"
    set table = dbSnpRsRn
else
    echo "Organism ($organism) not supported yet."
endif

touch $fileBase.obs
foreach chrom ( $chroms )
    echo
    echo `date` Getting data for chr$chrom
    wget ftp://ftp.ncbi.nlm.nih.gov/snp/$organism/XML/ds_ch$chrom.xml.gz \
	|& grep ds_ch | grep -v "=" | grep -v XML
    
    echo `date` Unzipping XML file `pwd`/ds_ch$chrom.xml.gz
    gunzip ds_ch$chrom.xml.gz 

    echo `date` Getting details from ds_ch$chrom.xml
    getObsHet < ds_ch$chrom.xml > Observed/ch$chrom.obs

    echo `date` Zipping ds_ch$chrom.xml
    gzip ds_ch$chrom.xml
    mv   ds_ch*.xml.gz Done

    echo `date` Collecting details
    cat  Observed/ch$chrom.obs >> $fileBase.obs

    echo `date` Zipping ch$chrom.obs
    gzip Observed/ch$chrom.obs
end

echo
echo `date` Starting dbSnp
echo
$DETAILDIR/dbSnp $database $fileBase
echo
echo `date` Finished dbSnp
echo

# clean up
echo `date` Zipping files
gzip $fileBase.err $fileBase.obs
echo `date` Done.
echo
echo load data local infile "$fileBase.out" into table $table \
     | hgsql $database

# useful data checks
echo "=================================="
echo $database $organism $table
echo wc -l $table.bed
echo select count\(\*\) from $table | hgsql $database
echo
echo desc $table                    | hgsql $database
echo
echo show indexes from $table       | hgsql $database
echo
echo select \* from $table limit 5  | hgsql $database
echo

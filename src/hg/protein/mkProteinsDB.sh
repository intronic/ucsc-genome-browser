#!/bin/sh
#
#	mkProteinsDb.sh
#	- currently no arguments, but it should be modified to take
#	- a date stamp argument instead of creating one since you do
#	- want a consistent date stamp name between the proteins and
#	- the swissprot databases.
#
#	This script could be improved to do error checking for each step.
#
#	Thu Nov 20 11:31:51 PST 2003 - Created - Hiram
#
#	"$Id: mkProteinsDB.sh,v 1.7 2005/02/11 22:10:48 fanhsu Exp $"

TOP=/cluster/data/proteins
export TOP

if [ "$#" -ne 1 ]; then
    echo "usage: mkProteinsDB.sh <YYMMDD>"
    echo -e "\t<YYMMDD> - date stamp used to find spYYMMDD DB"
    exit 255
fi

SPDB_DATE=$1
export SPDB_DATE

type spToProteins > /dev/null 2> /dev/null

if [ "$?" -ne 0 ]; then
    echo "ERROR: can not find required program: spToProteins"
    echo -e "\tYou may need to build it in ~/kent/src/hg/protein/spToProteins"
    exit 255
fi

MACHINE=`uname -n`

if [ ${MACHINE} != "hgwdev" ]; then
    echo "ERROR: must run this script on hgwdev.  This is: ${MACHINE}"
    exit 255
fi

DATE=`date "+%y%m%d"`

PDB="proteins${SPDB_DATE}"
SPDB=sp"${SPDB_DATE}"

export DATE PDB SPDB

SP_TBLS=`hgsql -e "show tables;" "${SPDB}" | wc -l`

if [ "${SP_TBLS}" -lt 30 ]; then
    echo "ERROR: can not find database ${SPDB}"
    echo -e "\tor too few tables (< 30).  table count: ${SP_TBLS}"
    exit 255
fi

echo "Creating Db: ${PDB}"

if [ -d "$SPDB_DATE" ]; then
	echo "WARNING: ${PDB} already exists."
	echo -e "Do you want to remove it and recreate ? (ynq) \c"
	read YN
	if [ "${YN}" = "Y" -o "${YN}" = "y" ]; then
	echo "Recreating ${PDB}"
	rm -fr ./${SPDB_DATE}
	hgsql -e "drop database ${PDB}" proteins041115
	else
	echo "Will not recreate at this time."
	exit 255
	fi
fi

mkdir ${TOP}/${SPDB_DATE}
cd ${TOP}/${SPDB_DATE}
echo hgsql -e "create database ${PDB};" proteins041115
hgsql -e "create database ${PDB};" proteins041115
hgsqldump -d proteins041115 | ${TOP}/bin/rmSQLIndex.pl > proteins.sql
echo "hgsql ${PDB} < proteins.sql"
hgsql ${PDB} < proteins.sql
hgsql ${PDB} -e "drop index i2 on spXref2"
hgsql ${PDB} -e "drop index i3 on spXref2"
hgsql ${PDB} -e "drop index ii2 on spXref3"
hgsql ${PDB} -e "drop index ii3 on spXref3"

#	build HUGO database
mkdir /cluster/store5/proteins/hugo/${SPDB_DATE}
cd /cluster/store5/proteins/hugo/${SPDB_DATE}
wget --timestamping "http://www.gene.ucl.ac.uk/public-files/nomen/nomeids.txt"
sed -e "1d" nomeids.txt > hugo.tab
hgsql -e 'LOAD DATA local INFILE "hugo.tab" into table hugo;' ${PDB}

cd ${TOP}/${SPDB_DATE}
#	Create and load tables in proteins
echo spToProteins ${SPDB_DATE}
spToProteins ${SPDB_DATE}
cd ${TOP}/${SPDB_DATE}
hgsql -e 'LOAD DATA local INFILE "spXref2.tab" into table spXref2;' ${PDB}
hgsql -e 'LOAD DATA local INFILE "spXref3.tab" into table spXref3;' ${PDB}
hgsql -e 'LOAD DATA local INFILE "spOrganism.tab" into table spOrganism;' ${PDB}

#	Build spSecondaryID table
cd ${TOP}/${SPDB_DATE}
hgsql -e "select displayId.val, displayId.acc, otherAcc.val from displayId, \
        otherAcc where otherAcc.acc = displayId.acc;" ${SPDB} \
	| sed -e "1d" > spSecondaryID.tab
hgsql -e \
	'LOAD DATA local INFILE "spSecondaryID.tab" into table spSecondaryID;' \
	${PDB}

#	Build pfamXref and pfamDesc tables
mkdir /cluster/store5/proteins/pfam/${SPDB_DATE}
cd /cluster/store5/proteins/pfam/${SPDB_DATE}
wget --timestamping "ftp://ftp.sanger.ac.uk/pub/databases/Pfam/Pfam-A.full.gz"
#	100 Mb compressed, over 700 Mb uncompressed
rm -f Pfam-A.full
gunzip Pfam-A.full.gz
pfamXref ${PDB} Pfam-A.full pfamADesc.tab pfamAXref.tab
gzip Pfam-A.full &

hgsql -e 'LOAD DATA local INFILE "pfamADesc.tab" into table pfamDesc;' ${PDB}
hgsql -e 'LOAD DATA local INFILE "pfamAXref.tab" into table pfamXref;' ${PDB}

#	Build the pdbSP table
cd ${TOP}/${SPDB_DATE}
wget --timestamping "http://us.expasy.org/cgi-bin/lists?pdbtosp.txt" \
	-O pdbtosp.htm
pdbSP ${PDB}
hgsql -e 'LOAD DATA local INFILE "pdbSP.tab" into table pdbSP;' ${PDB}

#	Build the spDisease table
hgsql -e "select comment.acc, displayId.val, commentVal.val from \
	comment, commentVal, displayId where comment.commentType=19 \
	and commentVal.id=comment.commentVal and displayId.acc=comment.acc;" \
	${SPDB} | sed -e "1d" > spDisease.tab
hgsql -e 'LOAD DATA local INFILE "spDisease.tab" into table spDisease;' ${PDB}

# create swInterPro table
wget --timestamping "ftp://ftp.ebi.ac.uk/pub/databases/interpro/protein2interpro.dat.gz"
gzip -d protein2interpro.dat.gz
hgsql ${PDB} -e 'load data local infile "protein2interpro.dat" into table interProXref;'
#hgsql ${PDB} -e "drop table ${PDB}.swInterPro"
hgsql --skip-column-names ${PDB} -e 'select accession, interProId from interProXref;'|sort -u >swInterPro.tab
hgsql ${PDB} -e 'load data local infile "swInterPro.tab" into table swInterPro;'
 


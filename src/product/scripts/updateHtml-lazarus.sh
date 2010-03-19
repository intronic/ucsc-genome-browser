#!/bin/sh
#
#	$Id: updateHtml-lazarus.sh,v 1.1.2.1 2010/03/19 16:50:02 hiram Exp $
#
usage() {
    echo "usage: updateHtml.sh <browserEnvironment.txt>"
    echo "The browserEnvironment.txt file contains definitions of how"
    echo "  these scripts behave in your local environment."
    echo "  There should be an example template to start with in the"
    echo "  directory with these scripts."
    echo "This script will fetch the static HTML hierarchy from UCSC"
    echo "  into your specified DOCHOME from the browserEnvironment.txt file."
    echo "It specifically ignores the encode/ js/ and trash directories."
    exit 255
}

##########################################################################
# Minimal argument checking and use of the specified include file

if [ $# -ne 1 ]; then
    usage
fi

export includeFile=$1
if [ "X${includeFile}Y" = "XY" ]; then
    usage
fi

if [ -f "${includeFile}" ]; then
    . "${includeFile}"
else
    echo "ERROR: updateHtml.sh: can not find ${includeFile}"
    usage
fi

export DS=`date "+%Y-%m-%d"`
export FETCHLOG="${LOGDIR}/htdocs/update.${DS}"
mkdir -p "${LOGDIR}/htdocs"

${RSYNC} --stats --exclude="encode/" --exclude="js/" --exclude="trash" \
	${HGDOWNLOAD}/htdocs/ ${DOCHOME}/ > ${FETCHLOG} 2>&1
# remove garbage
rm -f ${DOCHOME}/html
rm -f ${DOCHOME}/ENCODE

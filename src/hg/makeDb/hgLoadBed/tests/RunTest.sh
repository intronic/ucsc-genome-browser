#!/bin/sh

TESTPROGRAM=hgLoadBed
tests=0
failures=0
verbose=""
NOLOAD="-noLoad"
export TESTPROGRAM verbose tests failures NOLOAD

usage() {
    echo "usage: `basename $0` [-verbose] [-load] [-home] [-help]"
    echo -e "\t-verbose - show all test command lines and results"
    echo -e "\t-load - actually load table 'bedLoadTest' into database 'test'"
    echo -e "\t-home - use PATH $HOME/bin/$MACHTYPE/$TESTPROGRAM"
    echo -e "\t-help - print this usage message"
    exit 255
}

oneTest() {
    tests=`expr $tests + 1`
    cmd=$1
    result=$2

    C=`echo $tests | awk '{printf "%4d", $1}'`
    eval $cmd > /dev/null 2> /dev/null
    T=`cat bed.tab | sum -r`

    if [ "${T}" != "$result" ]; then
	echo "${C} FAIL: $cmd"
	echo "expected: '$result' got: '$T'"
	failures=`expr $failures + 1`
    else
	if [ -n "${verbose}" ]; then
	    echo "${C}   OK: $cmd"
	fi
    fi
}

failedArgCheck=0
export failedArgCheck

while [ "$#" -gt 0 ]
do
    if [ "$1" = "-help" ]; then
	usage
    elif [ "$1" = "-verbose" ]; then
	verbose="verbose"
    elif [ "$1" = "-load" ]; then
	NOLOAD=""
    elif [ "$1" = "-home" ]; then
	TESTPROGRAM="$HOME/bin/$MACHTYPE/$TESTPROGRAM"
    else
	echo "unknown argument: $1"
	failedArgCheck=1
    fi
    shift
done

export TESTPROGRAM verbose tests failures NOLOAD

if [ "$failedArgCheck" -eq 1 ]; then
    exit 255
fi

type ${TESTPROGRAM} > /dev/null 2> /dev/null

if [ "$?" -ne 0 ]; then
    echo "ERROR: can not find test program binary:"
    echo -e "\t'${TESTPROGRAM}'"
    exit 255
fi

rm -f bed.tab

oneTest "zcat bed4.txt.gz | $TESTPROGRAM ${NOLOAD} test bedLoadTest stdin" "29784     1"
oneTest "zcat bed6.txt.gz | $TESTPROGRAM ${NOLOAD} test bedLoadTest stdin" "01708     1"
oneTest "zcat bed14.txt.gz | $TESTPROGRAM ${NOLOAD} test bedLoadTest stdin" "00496     2"
oneTest "zcat bed9.txt.gz | $TESTPROGRAM ${NOLOAD} test bedLoadTest stdin" "27920     2"

if [ -n "${verbose}" ]; then
    C=`echo $tests | awk '{printf "%4d", $1}'`
    echo "${C}   run, $failures failed";
fi

if [ -z "${verbose}" ]; then
    rm -f bed.tab
fi

exit 0

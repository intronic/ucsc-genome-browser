#!/usr/local/bin/perl -w
#
#	Generate a sine curve over the number of data points given,
#		values output in the range [0:127]
#	Can specify a frequency if desired, 1 is default
#

use strict;
use POSIX;	#	to define floor()

my $argc = @ARGV;	#	argument count

if ( $argc < 1 ) {
	print "usage: $0 <# of data points> [frequency [dataSpan]]\n";
	print "\t# of data points - generate this many numbers\n";
	print "\tfrequency - if given, this frequency over the range\n";
	print "\t\tdefault is 1\n";
	print "\tdataSpan - # of bases to each value, default 1\n";
	print "\t\tmust have frequency for this third arg to work\n";
	exit(255);
}

my $maxDataPointsAllowed = 16777216;	# limit output to 16M data values
my $count = shift;		#	number of points to generate
my $frequency = 1;
my $dataSpan = 1;
if( $argc > 1 ) {
	$frequency = shift;
}
if( $argc == 3 ) {
	$dataSpan = shift;
}

printf STDERR "Generating $count points, at frequency $frequency, span $dataSpan\n";
if( $count < 2 ) {
	print "ERROR: number of points must be > 1\n";
	exit(255);
}

if( $count > $maxDataPointsAllowed ) {
	print "ERROR: limit of number of points at this time < $maxDataPointsAllowed\n";
	exit(255);
}

my $PI = 3.14159265;
my $incr = 360 / $count;
my $Radians;
#       To be less than 128 after scaling, use 2.001
#       This produces a max of 127 at 1.0 = sin(x)
my $Scale = 128/2.001;
my $ByteValue;

my $pointsDone = 0;
my $i = 1;
for( my $j = 0; ($j < 360) && ($pointsDone < $count); $j += $incr ) {
	$Radians = $frequency * ((2.0 * $PI) * $j) / 360;
	$ByteValue = floor((1.0 + sin($Radians)) * $Scale);
	printf "%d\t%d\n", $i, $ByteValue;
	$i += $dataSpan;
	++$pointsDone;
}
#	round off errors in $incr sometimes misses a point or two
#	on the end.  Continue going to reach $count.
if( $pointsDone < $count ) {
for( my $j = 0; ($j < 360) && ($pointsDone < $count); $j += $incr ) {
	$Radians = $frequency * ((2.0 * $PI) * $j) / 360;
	$ByteValue = floor((1.0 + sin($Radians)) * $Scale);
	printf "%d\t%d\n", $i, $ByteValue;
	$i += $dataSpan;
	++$pointsDone;
}
}

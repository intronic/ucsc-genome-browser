#!/usr/bin/env perl
#
#	hexToHtml.pl [files with hex color definitions] > palette.html
#
#	Read in palette definitions, create an html page that displays
#		the colors with their r,g,b equivalents
#	    Output is to stdout, save in a file to use it. 
#
#	The input palette definition files have lines defined as such:
#996600	1
#	and yes, they do begin with the # character
#	That is the hex value 996600 for color with label 1
#
#	2005-02-02 - Created - Hiram
#
#	"$Id: hexToHtml.pl,v 1.1 2005/02/02 20:52:02 hiram Exp $"
#

use warnings;
use strict;

my $argc = @ARGV;

if( $argc < 1 ) {
    print "usage: $0 <files with color definitions> > palette.html\n";
    print "\tthe color definition files are two column files:\n";
    print "\t#rrggbb label\n";
    print "\twhere #rrggbb is a hex rgb designation, e.g. #a52a2a\n";
    print "\tand label is the label for that color\n";
    exit( 255 );
}

my @palettes;		#	an array of arrays
my @paletteNames;	#	derived from the file names
my $paletteCount = 0;	#	number of palette definitions read in

#	read each file, simply save each line in an array, then that
#	array save in the @palettes array.
while (my $fName = shift) {
    open (FH,"$fName") or die "Can not open color definition file $fName";
    my $paletteName = $fName;
    $paletteName =~ s/\..*//;	#	remove .txt extension
    $paletteNames[$paletteCount] = $paletteName;
    ++$paletteCount;
    my @palette;
    my $paletteKey = 0;
    while (my $line = <FH>) {
	chomp $line;
	$palette[$paletteKey] = $line;
	++$paletteKey;
    }
    close (FH);
    push (@palettes, \@palette);
}

print STDERR "read $paletteCount palette definitions\n";

print '<HTML><HEAD><TITLE> Color Palette Examples </TITLE></HEAD><BODY>', "\n";
print "<H3> Color Palette Examples </H3>\n";
print "<TABLE><TR>\n";

#	print a TABLE header line with each palette name
for (my $i=0; $i < $paletteCount; ++$i) {
    printf "    <TH> %s </TH>\n", $paletteNames[$i];
}
print "</TR><TR>\n";

#	Now, going through each palette, make its display table
for (my $i=0; $i < $paletteCount; ++$i) {
    print "    <TD><TABLE BORDER=1>\n";
    my $palette = $palettes[$i];
    my $paletteSize = scalar(@$palette);
    for (my $j=0; $j < $paletteSize; ++$j) {
	my $line = $palette->[$j];	#	recover the line read in above
	my ($rgb, $label) = split('\s+',$line);
	my $hexDigits = $rgb;
	$hexDigits =~ m/^\#(..)(..)(..)$/;
	my $red = hex($1);
	my $green = hex($2);
	my $blue = hex($3);
	print '    <TR><TD WIDTH=40 HEIGHT=40 BGCOLOR="';
	printf "%s", $rgb;
	print '">&nbsp;</TD><TD ALIGN=LEFT>&nbsp;', $label;
	print '&nbsp;</TD><TD ALIGN=RIGHT>&nbsp;';
	printf "%d,%d,%d", $red, $green, $blue;
	print '&nbsp;</TD></TR>', "\n";
    }
    print "    </TABLE></TD>\n";
}
print "</TR></TABLE>\n";
print "</BODY></HTML>\n";

#!/usr/bin/perl -W

# mkOrthologFrame - generate ENCODE region frame page including orthologs, from regions file
#       and description file
# positions file: <chrN:x-y>\t<region>
# description file: <region>|<description>
# header file: HTML header for file

#use strict;

# countBases <db> <chr> <start> <end>
#       counts non-N bases in a range
sub countBases {
    my ($db, $chr, $start, $end) = @_;
    my $nibDir;
    if (-d "/cluster/data/$db/mixedNib")     { $nibDir = "mixedNib"; }
    elsif (-d "/cluster/data/$db/softNib")   { $nibDir = "softNib";  }
    elsif (-d "/cluster/data/$db/nib")       { $nibDir = "nib";      }
    elsif (-d "/cluster/bluearc/$db/nibDir") { 
	my $faSize = `nibFrag /cluster/bluearc/$db/nibDir/$chr.nib $start $end + stdout | faSize stdin`;
	$faSize =~ /.* bases .* N's ([0-9]+) real/; # ' 
	my $bases = $1;
	return $bases; }
    else { die "can't find nib dir for $db\n"; }

    my $faSize = `nibFrag /cluster/data/$db/$nibDir/$chr.nib $start $end + stdout | faSize stdin`;
    $faSize =~ /.* bases .* N's ([0-9]+) real/; # ' 
    my $bases = $1;
    return $bases;
}
# MAIN

@ARGV == 8 or die "$#ARGV\tusage: mkOrthologFrame <description-file> <position-file> <header-file> <fromDb> <toDb> <consensus-bed-file> <liftOver-bed-file> <Mercator-bed-file>\n";
my ($descriptionFile, $positionFile, $headerFile, $fromDb, $toDb, $consensusFile, $liftOverFile, $MercatorFile) = @ARGV;
my $fromDbUpper = "\u$fromDb";
my $toDbUpper   = "\u$toDb";
my $baseTracks  = "&amp;net${fromDbUpper}=full&amp;refGene=pack&amp;encodeRegionsConsensus=pack";
   $baseTracks .= "&amp;encodeRegionsMercatorMerged=pack&amp;encodeRegionsLiftOver=pack&amp;encodeRegionsMercator=pack";
my $netHide     = "&amp;netCanFam1=hide&amp;netGalGal2=hide&amp;netMm5=hide&amp;netPanTro1=hide&amp;netRn3=hide";

open(POSITIONS, $positionFile)    or die "ERROR: can't open $positionFile\n";
open(DESCRS,    $descriptionFile) or die "ERROR: can't open $descriptionFile\n";
open(CONSENSUS, $consensusFile)   or die "ERROR: can't open $consensusFile\n";
open(LIFTOVER,  $liftOverFile)    or die "ERROR: can't open $liftOverFile\n";
open(MERCATOR,  $MercatorFile)    or die "ERROR: can't open $MercatorFile\n";

# print header
system("cat $headerFile");

# read in positions file

my %positions = ();
while (<POSITIONS>) {
    chomp;
    my ($position, $region) = split /\s/;
    $positions{$region} = $position;
}
close(POSITIONS);

# read in description file
my %descriptions = ();
while (<DESCRS>) {
    chomp;
    my ($region, $description) = split /\|/;
    $descriptions{$region} = $description;
}
close(DESCRS);

# read in consensus ortholog regions file
my %orthoRegionParts = ();
my %orthoRegionPartPosition = ();

while (<CONSENSUS>) {
    chomp;
    my ($chr, $start, $end, $regionPart)  = split /\s/;
    $orthoRegionPartPosition{$regionPart} = sprintf "%s:%d-%d", $chr, $start, $end;
    $regionPart =~ /(EN.\d\d\d)/;
    my $region = $1;
    if (defined $orthoRegionParts{$region}) { $orthoRegionParts{$region} = join (",", $orthoRegionParts{$region}, $regionPart); }
    else { $orthoRegionParts{$region} = $regionPart; }
}
close(CONSENSUS);

# read in liftOver regions file
my %LORegionParts = ();
my %LORegionPartPosition = ();
while (<LIFTOVER>) {
    chomp;
    my ($chr, $start, $end, $regionPart) = split /\s/;
    $LORegionPartPosition{$regionPart}   = sprintf "%s:%d-%d", $chr, $start, $end;
    $regionPart =~ /(EN.*)[_.][0-9]+$/;
    my $region = $1;
    if (defined $LORegionParts{$region}) {
        $LORegionParts{$region} = join (",", $LORegionParts{$region}, $regionPart);
    } else {
        $LORegionParts{$region} = $regionPart;
    }
}
close(LIFTOVER);

# read in Mercator regions file
my %MCRegionParts = ();
my %MCRegionPartPosition = ();
while (<MERCATOR>) {
    if (/MEN/) {next;} # ignore spurious Mercator ENCODE regions.
    chomp;
    my ($chr, $start, $end, $regionPart, @foo) = split /\s/;
    $MCRegionPartPosition{$regionPart} = sprintf "%s:%d-%d", $chr, $start, $end;
    $regionPart =~ /(EN.\d\d\d).\d+_of_\d+_[+-]$/;
    my $region = $1;
    if (defined $MCRegionParts{$region}) { $MCRegionParts{$region} = join (",", $MCRegionParts{$region}, $regionPart); }
    else { $MCRegionParts{$region} = $regionPart; }
}
close(MERCATOR);

# print table header
print "<table border cellpadding=\"2\">\n";
print "<TR><TH>Region<TH>Description<TH>Chr<TH>Size (Mb)<TH>Consensus <FONT COLOR=green>$toDb</FONT><TH>Size (Mb)";
print "<TH>Ratio<TH>LiftOver<TH>Size (Mb)<TH>Non-N Size<TH>Mercator<TH>Size (Mb)<TH>Non-N Size<TH>Diff (Kb)</TR>\n";

# print table entries
foreach $region (sort keys %descriptions) {
    $_ = $positions{$region};
    my ($chr, $start, $end) = /^chr(\S+):(\d+)-(\d+)$/;
    my $length = $end - $start + 1;
    my $regionLength = $length;
    $length /= (1000*1000);

    # human browser - region link, description, and size
    printf "<TR><TD><A HREF=\"/cgi-bin/hgTracks?db=%s&amp;position=%s&amp;encodeRegions=pack&amp;net%s=full\" ", $fromDb, $positions{$region}, $toDbUpper;
    printf "TARGET=browser>%s</A><TD>%s<TD ALIGN=\"right\">%s<TD ALIGN=\"center\">%.1f", $region, $descriptions{$region}, $chr, $length;

    # consensus regions in ortholog browser -  region links, size, and size ratio
    $length = 0;
    printf "<TD>";
    if ( defined $orthoRegionParts{$region} ) { 
	@parts = split (/,/, $orthoRegionParts{$region});  ### 9 errors
	if ($#parts == -1) {printf "-";}
	foreach $part (@parts) {
	    my ($chr, $range) = split (/:/, $orthoRegionPartPosition{$part});
	    my ($start, $end) = split (/-/, $range);
	    $length = $length + ($end - $start + 1);
	    printf("<A HREF=\"/cgi-bin/hgTracks?db=%s&amp;position=%s%s%s&amp;net%s=full\" TARGET=browser2>%s</A>\n", 
		   $toDb, $orthoRegionPartPosition{$part}, $baseTracks, $netHide, $fromDbUpper, $chr);
	}
    } else { printf "<font color=red>no predictions</font>"; } # "$toDb $region $orthoRegionParts{$region}\n"; }
    my $ratio = $length * 100 / $regionLength;
    $length /= (1000*1000); #MB
    printf "</TD><TD>%.1f</TD>\n", $length;
    printf "<TD ALIGN=right>%3.0f%%</TD>\n", $ratio;


    # liftOver regions in ortholog browser - region links, size
    printf "<TD>\n";
    if (defined $LORegionParts{$region})
    {
	@parts = split (/,/, $LORegionParts{$region});
	$length = $nonNlength = 0;
	foreach $part (@parts) {
	    my ($chr, $range) = split (/:/, $LORegionPartPosition{$part});
	    my ($start, $end) = split (/-/, $range);
	    $length = $length + ($end - $start + 1);
	    $nonNlength = $nonNlength + &countBases($toDb, $chr, $start, $end);
	    printf("<A HREF=\"/cgi-bin/hgTracks?db=%s&amp;position=%s%s%s&amp;net%s=full\" TARGET=browser2>%s</A>\n", 
		   $toDb, $LORegionPartPosition{$part}, $baseTracks, $netHide, $fromDbUpper, $chr);
	}
    } else {printf "-";}

    my $LOlength  = $length;     
    $length      /= (1000*1000); # MB
    $nonNlength  /= (1000*1000); # MB
    printf "</TD><TD>%.1f </TD><TD>%.1f</TD>\n", $length, $nonNlength;

    # Mercator regions in ortholog browser - region links, size, diff
    printf "<TD>";
    if (defined $MCRegionParts{$region})
    {
	@parts = split (/,/, $MCRegionParts{$region});
	$length = 0;
	foreach $part (@parts) {
	    my ($chr, $range) = split (/:/, $MCRegionPartPosition{$part});
	    my ($start, $end) = split (/-/, $range);
	    $length = $length + ($end - $start + 1);
	    $nonNlength = $nonNlength + &countBases($toDb, $chr, $start, $end);
	    printf("<A HREF=\"/cgi-bin/hgTracks?db=%s&amp;position=%s%s%s&amp;net%s=full\" TARGET=browser2>%s</A>\n", 
		   $toDb, $MCRegionPartPosition{$part}, $baseTracks, $netHide, $fromDbUpper, $chr);
	}
    } else {printf "-";}
    my $diff     = $length - $LOlength;
    $diff        = $diff/1000;
    $length     /= (1000*1000);
    $nonNlength /= (1000*1000);
    printf "</TD><TD>%.1f </TD><TD>%.1f</TD>\n", $length, $nonNlength;
    printf "<TD ALIGN=right>";
    if ($diff > 100 || $diff < -100) {
        printf("<FONT COLOR=red>");
    } elsif ($diff > 20 || $diff < -20) {
        printf("<FONT COLOR=orange>");
    }
    printf "%d</TD></TR>\n", $diff;
    printf("</FONT>");
}

# end table

print "</table>\n";



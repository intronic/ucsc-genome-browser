#!/usr/bin/env perl

# encodeValidate.pl - validate an ENCODE data submission generated by the
#                       automated submission pipeline
# Verifies that all files and metadata are present and of correct formats
# Creates a load file (load.ra) and track configuration (trackDb.ra) for the datasets
# Returns 0 if validation succeeds and sends email to wrangler for given lab.

# DO NOT EDIT the /cluster/bin/scripts copy of this file -- 
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeValidate/doEncodeValidate.pl,v 1.32 2008/07/22 22:34:25 larrym Exp $

use warnings;
use strict;

use lib "/cluster/bin/scripts";
use HgAutomate;
use HgDb;
use File::stat;
use Getopt::Long;
use English;
use Carp qw(cluck);
use Cwd;

use vars qw/
    $opt_configDir
    $opt_outDir
    $opt_verbose
    /;

# Global constants
our $fieldConfigFile = "fields.ra";
our $vocabConfigFile = "cv.ra";
our $labsConfigFile = "labs.ra";
our $loadFile = "load.ra";
our $trackFile = "trackDb.ra";
our $pifVersion = 0.2;

# Global variables
our $submitPath;        # full path of data submission directory
our $configPath;        # full path of configuration directory
our $outPath;           # full path of output directory
our %terms;             # controlled vocabulary

sub usage {
    print STDERR <<END;
usage: encodeValidate.pl submission-type project-submission-dir

submission-type is currently ignored.

options:
    -verbose num        Set verbose level to num (default 1).            -
    -configDir dir      Path of configuration directory, containing
                        metadata .ra files (default: submission-dir/../config)
    -outDir dir         Path of output directory, for validation files
                        (default: submission-dir/out)
END
exit 1;
}

sub readFile
{
# Return lines from given file, with EOL chomp'ed off.
# Handles either Unix or Mac EOL characters.
# Reads whole file into memory, so should NOT be used for huge files.
    my ($file) = @_;
    my $oldEOL = $/;
    open(FILE, $file) or die "ERROR: Can't open file \'$file\'\n";
    my @lines = <FILE>;
    if(@lines == 1 && $lines[0] =~ /\r/) {
        # rewind and re-read as a Mac file - obviously, this isn't the most efficient way to do this.
        seek(FILE, 0, 0);
        $/ = "\r";
        @lines = <FILE>;
    }
    for (@lines) {
        chomp;
    }
    close(FILE);
    $/ = $oldEOL;
    return \@lines;
}

sub splitKeyVal
{
# split a line into key/value, using the FIRST white-space in the line; we also trim key/value strings
    my ($str) = @_;
    my $key = undef;
    my $val = undef;
    if($str =~ /([^\s]+)\s+(.+)/) {
        $key = $1;
        $val = $2;
        $key =~ s/^\s+//;
        $key =~ s/\s+$//;
        $val =~ s/^\s+//;
        $val =~ s/\s+$//;
    }
    return ($key, $val);
}

############################################################################
# Validators -- extend when adding new metadata fields

# dispatch table
our %validators = (
    files => \&validateFileName,
    view => \&validateDatasetName,
    labVersion => \&validateLabVersion,
    cell => \&validateCellLine,
    gene => \&validateGeneType,
    antibody => \&validateAntibody,
    );

# standard validators (required or optional for all projects)
sub validateFileName {
    # Validate array of filenames, ordered by part
    # Check files exist and are of correct data format
    my ($files, $track, $pif) = @_;
    my @newFiles;
    for my $file (@{$files}) {
        my @list = glob $file;
        if(@list) {
            push(@newFiles, @list);
        } else {
            die "ERROR: File '$file' does not exist (possibly bad glob)\n";
        }
    }
    &HgAutomate::verbose(3, "     Track: $track    Files: " . join (' ', @newFiles) . "\n");
    for my $file (@newFiles) {
        -e $file || die "ERROR: File \'$file\' does not exist\n";
        -s $file || die "ERROR: File \'$file\' is empty\n";
        -r $file || die "ERROR: File \'$file\' is not readable \n";
        &checkDataFormat($pif->{TRACKS}{$track}{type}, $file);
    }
    $files = \@newFiles;
}

sub validatePart {
    my ($val) = @_;
    $val >= 0 && $val < 100 || die "ERROR: Part \'$val\' is invalid (must be 0-100)\n";
}

sub validateDatasetName {
    my ($val) = @_;
}

sub validateAssembly {
    my ($val) = @_;
    $val =~ /^hg1[78]$/ || die "ERROR: Assembly '$val' is invalid (must be 'hg17' or 'hg18')\n";
}

sub validateDataType {
    my ($val) = @_;
}

sub validateRawDataAcc {
# No validation
}

sub validateLabVersion {
# No validation
}

# project-specific validators
sub validateCellLine {
    my ($val) = @_;
    defined($terms{'Cell Line'}{$val}) || die "ERROR: Cell line \'$val\' is not known \n";
}

sub validateGeneType {
    my ($val) = @_;
    defined($terms{'Gene Type'}{$val}) || die "ERROR: Gene type \'$val\' is not known \n";
}

sub validateAntibody {
    my ($val) = @_;
    defined($terms{'Antibody'}{$val}) || die "ERROR: Antibody \'$val\' is not known \n";
}

############################################################################
# Format checkers - extend when adding new data format

# dispatch table
our %formatCheckers = (
    wig => \&validateWig,
    bed => \&validateBed,
    genePred => \&validateGene,
    mappedReads => \&validateMappedReads,
    );

sub validateWig {
    my ($file) = @_;
    my $outFile = "validateWig.out";
    my $filePath = "$submitPath/$file";
    my $err = system (
        "cd $outPath; head -10 $filePath | wigEncode stdin /dev/null /dev/null >$outFile 2>&1");
    if ($err) {
        print STDERR  "ERROR: File \'$file\' failed wiggle validation\n";
        open(ERR, "$outPath/$outFile") || die "ERROR: Can't open wiggle validation file \'$outPath/$outFile\': $!\n";
        my @err = <ERR>;
        die "@err\n";
    } else {
        &HgAutomate::verbose(2, "File \'$file\' passed wiggle validation\n");
    }
}

sub validateBed {
# Validate each line of a bed 5 or greater file.
    my ($file, $type) = @_;
    my $filePath = "$submitPath/$file";
    my $line = 0;
    open(FILE, $filePath) or die "Couldn't open file: $filePath; error: $!\n";
    while(<FILE>) {
        chomp;
        my @fields = split /\t/;
        $line++;
        next if(!@fields);
        my $prefix = "Failed bed validation, file '$file'; line $line:";
        if(/^(track|browser)/) {
            ;
        } elsif(@fields < 5) {
            die "$prefix not enough fields; " . scalar(@fields) . " present; at least 5 are required";
        } elsif ($fields[0] !~ /^chr(\d+|M|X|Y)$/) {
            die "$prefix field 1 value ($fields[0]) is invalid; not a valid chrom name";
        } elsif ($fields[1] !~ /^\d+$/) {
            die "$prefix field 2 value ($fields[1]) is invalid; value must be a positive number";
        } elsif ($fields[2] !~ /^\d+$/) {
            die "$prefix field 3 value ($fields[2]) is invalid; value must be a positive number";
        } elsif ($fields[2] < $fields[1]) {
            die "$prefix field 3 value ($fields[2]) is less than field 2 value ($fields[1])";
        } elsif ($fields[4] !~ /^\d+$/ && $fields[4] !~ /^\d+\.\d+$/) {
            die "$prefix field 5 value ($fields[4]) is invalid; value must be a positive number";
        } elsif ($fields[4] < 0 || $fields[4] > 1000) {
            die "$prefix field 5 value ($fields[4]) is invalid; score must be 0-1000";
        } else {
            ;
        }
    }
    close(FILE);
    HgAutomate::verbose(2, "File \'$file\' passed bed validation\n");
}

sub validateGene {
    my ($file, $type) = @_;
    my $outFile = "validateGene.out";
    my $filePath = "$submitPath/$file";
    my $err = system (
        "cd $outPath; egrep -v '^track|browser' $filePath | ldHgGene -out=genePred.tab -genePredExt hg18 testTable stdin >$outFile 2>&1");
    if ($err) {
        print STDERR  "ERROR: File \'$file\' failed GFF validation\n";
        open(ERR, "$outPath/$outFile") || die "ERROR: Can't open GFF validation file \'$outPath/$outFile\': $!\n";
        my @err = <ERR>;
        die "@err\n";
    } else {
        &HgAutomate::verbose(2, "File \'$file\' passed GFF validation\n");
    }
}

sub validateMappedReads {
    my ($file, $type) = @_;
    my $filePath = "$submitPath/$file";
    my $line = 0;
    open(FILE, $filePath) or die "Couldn't open file: $filePath; error: $!\n";
    while(<FILE>) {
        $line++;
        if(!(/chr(\d+|M|X|Y)\t\d+\t\d+\t[ATCG]+\t\d+\t[+-]\t.*/)) {
            die "Line number $line is invalid\nline: $_";
        }
    }
    close(FILE);
    HgAutomate::verbose(2, "File \'$file\' passed mappedReads validation\n");
}


############################################################################
# Misc subroutines

sub validateDdfField {
    # validate value for type of field
    my ($type, $val, $pif, $track) = @_;
    $type =~ s/ /_/g;
    &HgAutomate::verbose(4, "Validating $type: " . (defined($val) ? $val : "") . "\n");
    if($validators{$type}) {
        $validators{$type}->($val, $track, $pif);
    }
}

sub checkDataFormat {
    # validate file type
    my ($format, $file) = @_;
    &HgAutomate::verbose(3, "Checking data format for $file: $format\n");
    my $type = "";
    if ($format =~ m/(bed) (\d+)/) {
        $format = $1;
        $type = $2;
    }
    $formatCheckers{$format} || die "ERROR: Data format \'$format\' is unknown\n";
    $formatCheckers{$format}->($file, $type);
}

sub loadControlledVocab {
    %terms = ();
    my %termRa = &readRaFile("$configPath/$vocabConfigFile", "term");
    foreach my $term (keys %termRa) {
        my $type = $termRa{$term}->{'type'};
        $terms{$type}->{$term} = $termRa{$term};
    }
}

sub newestFile {
  # Get the most recently modified file from a list
    my @files = @_;
    my $newestTime = 0;
    my $newestFile = "";
    my $file = "";
    foreach $file (@files) {
        my $fileTime = (stat($file))->mtime;
        if ($fileTime > $newestTime) {
            $newestTime = $fileTime;
            $newestFile = $file;
        }
    }
    return $newestFile;
}

sub getPif
{
# Return PIF hash; hash keys are RA style plus an additional TRACKS key which is a nested hash for
# the track list at the end of the PIF file.
    my ($labs, $fields) = @_;

    # Read info from Project Information File.  Verify required fields
    # are present and that the project is marked active.
    my %pif = ();
    $pif{TRACKS} = {};
    my $pifFile = &newestFile(glob "*.PIF");
    &HgAutomate::verbose(2, "Using newest PIF file \'$pifFile\'\n");

    my $lines = readFile($pifFile);
    while (@{$lines}) {
        my $line = shift @{$lines};
        # strip leading and trailing spaces
        $line =~ s/^ +//;
        $line =~ s/ +$//;
        # ignore comments and blank lines
        next if $line =~ /^#/;
        next if $line =~ /^$/;

        my ($key, $val) = splitKeyVal($line);
        if(!defined($key)) {
            next;
        }
        if ($key ne "view") {
            &HgAutomate::verbose(3, "PIF field: $key = $val\n");
            $pif{$key} = $val;
        } else {
            my %track = ();
            my $track = $val;
            $pif{TRACKS}->{$track} = \%track;
            &HgAutomate::verbose(5, "  Found view: \'$track\'\n");
            while ($line = shift @{$lines}) {
                $line =~ s/^ +//;
                $line =~ s/ +$//;
                next if $line =~ /^#/;
                next if $line =~ /^$/;
                if ($line =~ /^track/) {
                    unshift @{$lines}, $line;
                    last;
                }
                my ($key, $val) = splitKeyVal($line);
                $track{$key} = $val;
                &HgAutomate::verbose(5, "    Property: $key = $val\n");
            }
        }
    }

    # Validate fields
    my @tmp = grep(!/^TRACKS$/, keys %pif);
    validateFieldList(\@tmp, $fields, 'pifHeader', "in PIF '$pifFile'");

    if($pif{pifVersion} ne $pifVersion) {
        die "ERROR: pifVersion '$pif{pifVersion}' does not match current version: $pifVersion\n";
    }
    if(!keys(%{$pif{TRACKS}})) {
        die "ERROR: no views defined for project \'$pif{project}\' in PIF '$pifFile'\n";
    }
    if(!defined($labs->{$pif{lab}})) {
        die "ERROR: invalid lab '$pif{lab}' for project \'$pif{project}\' in PIF '$pifFile'\n";
    }
    validateAssembly($pif{assembly});

    foreach my $track (keys %{$pif{TRACKS}}) {
        &HgAutomate::verbose(4, "  Track: $track\n");
        my %track = %{$pif{TRACKS}->{$track}};
        foreach my $key (keys %track) {
            &HgAutomate::verbose(4, "    Setting: $key   Value: $track{$key}\n");
        }
    }

    if (defined($pif{'variables'})) {
        my @variables = split (/\s*,\s*/, $pif{'variables'});
        my %variables;
        my $i = 0;
        foreach my $variable (@variables) {
            # replace underscore with space
            $variable =~ s/_/ /g;
            $variables[$i++] = $variable;
            $variables{$variable} = 1;
        }
        $pif{'variableHash'} = \%variables;
        $pif{'variableArray'} = \@variables;
    }
    return %pif;
}

sub readRaFile {
# Read records from a .ra file into a hash of hashes and return it.
# $type is the used as the primary key and as the key of the returned hash.
    my ($file, $type) = @_;
    open(RA, $file) || 
        die "ERROR: Can't open RA file \'$file\'\n";
    my @lines = <RA>;
    my %ra = ();
    my $raKey = undef;
    foreach my $line (@lines) {
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;
        if ($line =~ /^$/) {
            $raKey = undef;
            next;
        }
        next if $line =~ /^#/;
        chomp $line;
        if ($line =~ m/^$type\s+(.*)/) {
            $raKey = $1;
        } else {
            defined($raKey) || die "ERROR: Missing $type before $line\n";
            my ($key, $val) = split('\s+', $line, 2);
            $ra{$raKey}->{$key} = $val;
        }
    }
    close(RA);
    return %ra;
}

sub validateFieldList {
# validate the entries in a RA record or DDF header using labs.ra as our schema
    my ($fields, $schema, $file, $errStrSuffix) = @_;
    my %hash = map {$_ => 1} @{$fields};
    my @errors;

    # look for missing required fields
    for my $field (keys %{$schema}) {
        if($schema->{$field}{file} eq $file && $schema->{$field}{required} eq 'yes' && !defined($hash{$field})) {
            push(@errors, "field '$field' not defined");
        }
    }

    # now look for fields in list that aren't in schema
    for my $field (@{$fields}) {
        if(!defined($schema->{$field}{file}) || $schema->{$field}{file} ne $file) {
            push(@errors, "invalid field '$field'");
        }
    }
    if(@errors) {
        die "ERROR: " . join("; ", @errors) . " $errStrSuffix\n";
    }
}

sub ddfKey
{
# return key for given DDF line (e.g. "antibody=$antibody;cell=$cell" for ChIP-Seq data)
    my ($fields, $ddfHeader, $pif) = @_;
    if (defined($pif->{variables})) {
        return join(";", map("$_=" . $fields->[$ddfHeader->{$_}], sort @{$pif->{variableArray}}));
    } else {
        die "ERROR: no key defined for this PIF";
    }
}

############################################################################
# Main

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time());
my $i;
my @ddfHeader;	# list of field headers on the first line of DDF file
my %ddfHeader = ();
my @ddfLines = ();
my %ddfSets = ();	# info about DDF entries broken down by ddfKey
my $wd = cwd();

my $ok = GetOptions("configDir=s",
                    "outDir=s",
                    "verbose=i",
                    );
&usage() if (!$ok);
&usage() if (scalar(@ARGV) < 2);

# Get command-line args
my $submitType = $ARGV[0];	# currently not used
my $submitDir = $ARGV[1];

# Get general options
$opt_verbose = 1 if (!defined $opt_verbose);

# Determine submission, configuration, and output directory paths
&HgAutomate::verbose(2, "Validating submission in directory \'$submitDir\'\n");
if ($submitDir =~ /^\/.*/) {
    $submitPath = $submitDir;
} else {
    $submitPath = "$wd/$submitDir";
}
&HgAutomate::verbose(4, "Submission directory path: \'$submitPath\'\n");

if (defined $opt_configDir) {
    if ($opt_configDir =~ /^\//) {
        $configPath = $opt_configDir;
    } else {
        $configPath = "$wd/$opt_configDir";
    }
} else {
    $configPath = "$submitDir/../config"
}
&HgAutomate::verbose(4, "Config directory path: \'$configPath\'\n");

if (defined $opt_outDir) {
    if ($opt_outDir =~ /^\//) {
        $outPath = $opt_outDir;
    } else {
        $outPath = "$wd/$opt_outDir";
    }
} else {
    $outPath = "$submitDir/out"
}
&HgAutomate::verbose(4, "Output directory path: \'$outPath\'\n");

# Change dir to submission directory 
chdir $submitPath ||
    die ("SYS ERR; Can't change to submission directory \'$submitPath\': $OS_ERROR\n");
&HgAutomate::verbose(3, "Creating output in directory \'$outPath\'\n");
mkdir $outPath || 
    die ("SYS ERR: Can't create out directory \'$outPath\': $OS_ERROR\n");

my %labs;
if(-e "$configPath/$labsConfigFile") {
    # tolerate missing labs.ra in dev trees.
    %labs = &readRaFile("$configPath/$labsConfigFile", "lab");
}


# Gather fields defined for DDF file. File is in 
# ra format:  field <name>, required <true|false>
my %fields = readRaFile("$configPath/$fieldConfigFile", "field");

# Locate project information (PIF) file and verify that project is
#  ready for submission
my %pif = getPif(\%labs, \%fields);

my $db = HgDb->new(DB => $pif{assembly});

# Add required fields for this -- the variables in the PIF file
if (defined($pif{variables})) {
    for my $variable (keys %{$pif{variableHash}}) {
        $fields{$variable}->{required} = 'yes';
        $fields{$variable}->{file} = 'ddf';
    }
}

# Open dataset descriptor file (DDF)
my $ddfFile = &newestFile(glob "*.DDF");
&HgAutomate::verbose(2, "Using newest DDF file \'$ddfFile\'\n");
my $lines = readFile($ddfFile);

# Get header containing column names
while(@{$lines}) {
    my $line = shift(@{$lines});
    # remove leading and trailing spaces and newline
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    # ignore empty lines and comments
    next if $line =~ /^$/;
    next if $line =~ /^#/;
    if($line !~ /\t/) {
        die "ERROR: The DDF header has no tabs; the DDF is required to be tab delimited";
    }
    @ddfHeader = split(/\t/, $line);
    for ($i=0; $i < @ddfHeader; $i++) {
        $ddfHeader{$ddfHeader[$i]} = $i;
    }
    last;
}

validateFieldList(\@ddfHeader, \%fields, 'ddf', "in DDF '$ddfFile'");

# Process lines in DDF file. Create a hash with one entry per line;
# the entry is an array of field values.

while (@{$lines}) {
    my $line = shift(@{$lines});
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    next if $line =~ /^#/;
    next if $line =~ /^$/;

    if($line !~ /\t/) {
        die "ERROR: DDF entry has no tabs; the DDF is required to be tab delimited";
    }
    my @fields = split('\t', $line);
    my $fileField = $ddfHeader{files};
    my $files = $fields[$fileField];
    my $view = $fields[$ddfHeader{view}];
    if(!$pif{TRACKS}->{$view}) {
        die "Undefined view '$view' in DDF";
    }
    my @filenames = split(',', $files);
    $fields[$fileField] = \@filenames;
    push(@ddfLines, \@fields);

    $ddfSets{ddfKey(\@fields, \%ddfHeader, \%pif)}{VIEWS}{$view} = 1;
}

# die if there are missing required views
for my $key (keys %ddfSets) {
    for my $view (keys %{$pif{TRACKS}}) {
        if($pif{TRACKS}->{$view}{required} eq 'yes') {
            if(!defined($ddfSets{$key}{VIEWS}{$view})) {
                die "ERROR: view '$view' missing for DDF entry '$key'";
            }
        }
    }
}

# Validate files and metadata fields in all ddfLines using controlled
# vocabulary.  Create load.ra file for loader and trackDb.ra file for wrangler.

loadControlledVocab();
open(LOADER_RA, ">$outPath/$loadFile") || die "SYS ERROR: Can't write \'$outPath/$loadFile\' file; error: $!\n";
open(TRACK_RA, ">$outPath/$trackFile") || die "SYS ERROR: Can't write \'$outPath/$trackFile\' file; error: $!\n";
my $priority = 0;
foreach my $ddfLine (@ddfLines) {
    $priority++;
    my $view = $ddfLine->[$ddfHeader{view}];
    my $tableType = $pif{TRACKS}->{$view}{tableType};
    HgAutomate::verbose(2, "  View: $view\n");
    for ($i=0; $i < @ddfHeader; $i++) {
        validateDdfField($ddfHeader[$i], $ddfLine->[$i], \%pif, $view);
    }

    # Construct table name from track name and variables
    my $trackName = "wgEncode$pif{project}$view";
    my $tableName = $trackName;
    if(!defined($pif{TRACKS}->{$view}{shortLabelPrefix})) {
        $pif{TRACKS}->{$view}{shortLabelPrefix} = "";
    }
    my $shortLabel = defined($pif{TRACKS}->{$view}{shortLabelPrefix}) ? $pif{TRACKS}->{$view}{shortLabelPrefix} : "";
    my $longLabel = "ENCODE" . (defined($pif{TRACKS}->{$view}{longLabelPrefix}) ? " $pif{TRACKS}->{$view}{longLabelPrefix}" : "");
    my $subGroups = "view=$view";
    my $additional = "\n";
    if (defined($pif{variables})) {
        my @variables = @{$pif{variableArray}};
        my %hash = map { $_ => $ddfLine->[$ddfHeader{$_}] } @variables;
        for my $var (@variables) {
            $tableName = $tableName . $ddfLine->[$ddfHeader{$var}];
        }
        my $shortSuffix;
        my $longSuffix;
        if($hash{antibody} && $hash{cell}) {
            $shortSuffix = "$hash{antibody} $hash{cell}";
            $longSuffix = "$hash{antibody} in $hash{cell} cells";
        } elsif ($hash{"cell"}) {
            $shortSuffix = "$hash{cell}";
            $longSuffix = "in $hash{cell} cells";
        }
        if($shortSuffix) {
            $shortLabel = $shortLabel ? "$shortLabel ($shortSuffix)" : $shortSuffix;
        }
        if($longSuffix) {
            $longLabel .= " ($longSuffix)";
        }
        if($hash{antibody}) {
            $subGroups .= " factor=$hash{antibody}";
            $additional = "\tantibody\t$hash{antibody}\n" . $additional;
        }
        if($hash{cell}) {
            $subGroups .= " cellType=$hash{cell}";
            $additional = "\tcell\t$hash{cell}\n" . $additional;
        }
    }
    # mysql doesn't allow hyphens in table names and our naming convention doesn't allow underbars.
    $tableName =~ s/[_-]//g;

    # Is this really an error?
    my $sth = $db->execute("select count(*) from trackDb where tableName = ?", $tableName);
    my @row = $sth->fetchrow_array();
    if(@row && $row[0]) {
        die "view '$view' has already been loaded";
    }

    print LOADER_RA "tablename $tableName\n";
    print LOADER_RA "track $trackName\n";
    print LOADER_RA "type $pif{TRACKS}->{$view}{type}\n";
    print LOADER_RA "tableType $tableType\n" if defined($tableType);
    print LOADER_RA "assembly $pif{assembly}\n";
    print LOADER_RA "files @{$ddfLine->[$ddfHeader{files}]}\n";
    print LOADER_RA "\n";

    print TRACK_RA "\ttrack\t$tableName\n";
    print TRACK_RA "\tsubTrack\twgEncode$pif{project}\n";
    print TRACK_RA "\tshortLabel\t$shortLabel\n";
    print TRACK_RA "\tlongLabel\t$longLabel\n";
    print TRACK_RA "\tsubGroups\t$subGroups\n";
    print TRACK_RA "\ttype\t$pif{TRACKS}->{$view}{type}\n";
    print TRACK_RA sprintf("\tdateSubmitted\t%d-%02d-%d %d:%d:%d\n", 1900 + $year, $mon + 1, $mday, $hour, $min, $sec);
    print TRACK_RA "\tpriority\t$priority\n";
    # noInherit is necessary b/c composite track will often have a different dummy type setting.
    print TRACK_RA "\tnoInherit\ton\n";
    my %visibility = (Align => 'hide', Signal => 'full', Sites => 'dense');
    if($visibility{$view}) {
        print TRACK_RA "\tvisibility\t$visibility{$view}\n";
    }
    if($pif{TRACKS}->{$view}{type} eq 'wig') {
        print TRACK_RA <<END;
	yLineOnOff	On
	yLineMark	1.0
	maxHeightPixels	100:32:8
END
    } elsif($pif{TRACKS}->{$view}{type} eq 'bed 5 +') {
        print TRACK_RA "\tuseScore\t1\n";
    }
    print TRACK_RA $additional;
}
close(LOADER_RA);
close(TRACK_RA);

# Send "data is ready" email to email contact assigned to $pif{lab}

if($labs{$pif{lab}} && $labs{$pif{lab}}->{wranglerEmail}) {
    my $email = $labs{$pif{lab}}->{wranglerEmail};
    `echo "dir: $submitPath" | /bin/mail -s "ENCODE data from $pif{lab} lab is ready" $email`;
}

exit 0;

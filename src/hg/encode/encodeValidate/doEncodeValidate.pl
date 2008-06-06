#!/usr/bin/env perl

# encodeValidate.pl - validate an ENCODE data submission generated by the
#                       automated submission pipeline
# Verifies that all files and metadata are present and of correct formats
# Creates a load script (load.csh) and track configuration (trackDb.ra) 
#               for the datasets
# Returns 0 if validation succeeds.

# DO NOT EDIT the /cluster/bin/scripts copy of this file -- 
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeValidate/doEncodeValidate.pl,v 1.20 2008/06/06 07:47:09 larrym Exp $

use warnings;
use strict;
use lib "/cluster/bin/scripts";
use HgAutomate;
use File::stat;
use Getopt::Long;
use English;

use vars qw/
    $opt_configDir
    $opt_outDir
    $opt_verbose
    /;

sub usage {
    print STDERR "
usage: encodeValidate.pl submission-type project-submission-dir
options:
    -verbose num        Set verbose level to num (default 1).            -
    -configDir dir      Path of configuration directory, containing
                        metadata .ra files (default: submission-dir/../config)
    -outDir dir         Path of output directory, for validation files
                        (default: submission-dir/out)
";
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

# Global constants
our $fieldConfigFile = "fields.ra";
our $vocabConfigFile = "cv.ra";

our $loadFile = "load.ra";
our $trackFile = "trackDb.ra";

# Global variables
our $submitPath;        # full path of data submission directory
our $configPath;        # full path of configuration directory
our $outPath;           # full path of output directory
our $pifFile;           # project information filename (most recent found in 
                                # submission dir)
our %pif;               # project information
our %tracks;            # track information
our %terms;             # controlled vocabulary

############################################################################
# Validators -- extend when adding new metadata fields

# dispatch table
our %validators = (
    File_Name => \&validateFileName,
    Part => \&validatePart,
    Dataset_Name => \&validateDatasetName,
    Assembly_REF => \&validateAssemblyREF,
    Data_Type_REF => \&validateDataTypeREF,
    Raw_Data_Acc_REF => \&validateRawDataAccREF,
    Data_Version => \&validateDataVersion,
    Cell_Line_REF => \&validateCellLineREF,
    Gene_Type_REF => \&validateGeneTypeREF,
    Antibody_REF => \&validateAntibodyREF,
    );

# standard validators (required or optional for all projects)
sub validateFileName {
    # Validate array of filenames, ordered by part
    # Check files exist and are of correct data format
    my ($files, $track) = @_;
    my @files = @{$files};
    &HgAutomate::verbose(3, "     Track: $track    Files: " . join (' ', @files) . "\n");
    for (my $i=0; $i < @files; $i++) {
        my $file = $files[$i];
        my $part = $i + 1;
        defined($file) || die "ERROR: Dataset missing part \'$part\'\n";
        -e $file || die "ERROR: File \'$file\' does not exist\n";
        -s $file || die "ERROR: File \'$file\' is empty\n";
        -r $file || die "ERROR: File \'$file\' is not readable \n";
        &checkDataFormat($tracks{$track}->{'type'}, $file);
    }
}

sub validatePart {
    my ($val) = @_;
    $val >= 0 && $val < 100 || die "ERROR: Part \'$val\' is invalid (must be 0-100)\n";
}

sub validateDatasetName {
    my ($val) = @_;
}

sub validateAssemblyREF {
    my ($val) = @_;
    $val =~ /hg1[78]/ || die "ERROR: Assembly REF \'$val\' is invalid (must be 'hg17' or 'hg18\')\n";
}

sub validateDataTypeREF {
    my ($val) = @_;
}

sub validateRawDataAccREF {
# No validation
}

sub validateDataVersion {
# No validation
}

# project-specific validators
sub validateCellLineREF {
    my ($val) = @_;
    defined($terms{'Cell Line'}{$val}) || die "ERROR: Cell line \'$val\' is not known \n";
}

sub validateGeneTypeREF {
    my ($val) = @_;
    defined($terms{'Gene Type'}{$val}) || die "ERROR: Gene type \'$val\' is not known \n";
}

sub validateAntibodyREF {
    my ($val) = @_;
    defined($terms{'Antibody'}{$val}) || die "ERROR: Antibody \'$val\' is not known \n";
}

############################################################################
# Format checkers - extend when adding new data format

# dispatch table
our %formatCheckers = (
    wig => \&validateWig,
    bed => \&validateBed,
    genePred => \&validateGene
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
    my ($file, $type) = @_;
    my $outFile = "validateBed.out";
    my $filePath = "$submitPath/$file";
    my $err = system (
        "cd $outPath; head -10 $filePath | egrep -v '^track|browser' | hgLoadBed -noLoad hg18 testTable stdin >$outFile 2>&1");
    if ($err) {
        print STDERR  "ERROR: File \'$file\' failed bed validation\n";
        open(ERR, "$outPath/$outFile") || die "ERROR: Can't open bed validation file \'$outPath/$outFile\': $!\n";
        my @err = <ERR>;
        die "@err\n";
    } else {
        &HgAutomate::verbose(2, "File \'$file\' passed bed validation\n");
    }
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

############################################################################
# Misc subroutines

sub validateField {
    # validate value for type of field
    my ($type, $val, $arg) = @_;
    $type =~ s/ /_/g;
    &HgAutomate::verbose(4, "Validating $type: $val\n");
    $validators{$type}->($val, $arg);
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
    $formatCheckers{$format} || 
        die "ERROR: Data format \'$format\' in PIF file \'$pifFile\' is unknown\n";
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

sub getPif {
    # Read info from Project Information File.  Verify required fields
    # are present and that the project is marked active.
    my %pif = ();
    $pifFile = &newestFile(glob "*.PIF");
    &HgAutomate::verbose(2, "Using newest PIF file \'$pifFile\'\n");

    %tracks = ();  # this is a global
    my $track;

    my $lines = readFile($pifFile);
    while (@{$lines}) {
        my $line = shift @{$lines};
        # strip leading and trailing spaces
        $line =~ s/^ +//;
        $line =~ s/ +$//;
        # ignore comments and blank lines
        next if $line =~ /^#/;
        next if $line =~ /^$/;

        my ($key, $val) = split(/\t/, $line);

        if ($key ne "track") {
            &HgAutomate::verbose(3, "PIF field: $key = $val\n");
            $pif{$key} = $val;
        } else {
            my %track = ();
            $track = $val;
            $tracks{$track} = \%track;
            &HgAutomate::verbose(5, "  Found track: \'$track\'\n");
            while ($line = shift @{$lines}) {
                $line =~ s/^ +//;
                $line =~ s/ +$//;
                next if $line =~ /^#/;
                next if $line =~ /^$/;
                if ($line =~ /^track/) {
                    unshift @{$lines}, $line;
                    last;
                }
                my ($key, $val) = split(/\t/, $line);
                $track{$key} = $val;
                &HgAutomate::verbose(5, "    Property: $key = $val\n");
            }
        }
    }

    # Validate fields
    defined($pif{'project'}) || die "ERROR: Project not defined\n"; 
    $pif{'active'} =~ "yes" || 
        die "ERROR: Project \'$pif{'project'}\' not yet active\n";
    defined(%tracks) ||
        die "ERROR: Tracks not defined for project \'$pif{'project'}\'in $pifFile \n";

    foreach my $track (keys %tracks) {
        &HgAutomate::verbose(4, "  Track: $track\n");
        my %track = %{$tracks{$track}};
        foreach my $key (keys %track) {
            &HgAutomate::verbose(4, "    Setting: $key   Value: $track{$key}\n");
        }
    }

    if (defined($pif{'variables'})) {
        my @variables = split (' ', $pif{'variables'});
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

############################################################################
# Main

my $line;
my $i;
my @ddfHeader;
my %ddfHeader = ();
my %datasets = ();
my $wd = `pwd`; chomp $wd;

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

# Locate project information (PIF) file and verify that project is
#  ready for submission
%pif = &getPif();

# Gather fields defined for DDF file. File is in 
# ra format:  field <name>, required <true|false>
my %fields = &readRaFile("$configPath/$fieldConfigFile", "field");

# Add required fields for this -- the variables in the PIF file
if (defined($pif{'variables'})) {
    foreach my $variable (keys %{$pif{'variableHash'}}) {
        $fields{$variable}->{'required'} = 'yes';
    }
}

# Open dataset descriptor file (DDF)
my $ddfFile = &newestFile(glob "*.DDF");
&HgAutomate::verbose(2, "Using newest DDF file \'$ddfFile\'\n");
my $lines = readFile($ddfFile);

# Get header containing column names
print STDERR "lines len: " . scalar(@{$lines}) . "\n";
while(@{$lines}) {
    my $line = shift(@{$lines});
    print STDERR "line: $line\n";
    # remove leading and trailing spaces and newline
    $line =~ s/^ +//;
    $line =~ s/ +$//;
    # ignore empty lines and comments
    next if $line =~ /^$/;
    next if $line =~ /^#/;
    @ddfHeader = split(/\t/, $line);
    for ($i=0; $i < @ddfHeader; $i++) {
        $ddfHeader{$ddfHeader[$i]} = $i;
    }
    last;
}

# Validate DDF header -- assure field is recognized
foreach my $field (@ddfHeader) {
    defined($fields{$field}) || die "ERROR: Header \'$field\' is unknown\n"; 
    delete($fields{$field});
}

# Check that all required fields are present in DDF header -- any
# not yet deleted that are marked required but have not been found in header
foreach my $field (keys %fields) {
    $fields{$field}->{'required'} eq "yes" && 
        die "ERROR: DDF header is missing required field \'$field\'\n"; 
}

# Process lines in DDF file.  Create dataset hash with one entry per dataset.
# The entry contains an array of fields that are the same as the DDF fields,
# except when multiple files comprise one data set (multiple Parts).
# In this case, all files are included in the File Name field, 
my $dataset;
while (@{$lines}) {
    my $line = shift(@{$lines});
    $line =~ s/^ +//;
    $line =~ s/ +$//;
    next if $line =~ /^#/;
    next if $line =~ /^$/;
    my @fields = split('\t', $line);
    my $fileField = $ddfHeader{'File Name'};
    my $filename = $fields[$fileField];
    my $partField = $ddfHeader{'Part'};
    my $part = 1;
    if (defined($partField)) {
        validateField('Part', $fields[$partField]);
        $part =  $fields[$partField];
    }
    $dataset = $fields[$ddfHeader{'Dataset Name'}];
    my $offset = $part - 1;
    if (defined($datasets{$dataset})) {
        # add file to dataset, checking all fields with non-empty values 
        # are identical (except 'Part', which must differ)
        for ($i=0; $i < @fields; $i++) {
            next if ($i == $fileField || $i == $partField);
            $fields[$i] =~ $datasets{$dataset}->[$i] ||
                die "ERROR: Dataset \'$dataset\' has differing \'$ddfHeader[$i]\' values\n";
        }
        !defined($datasets{$dataset}->[$fileField]->[$offset]) ||
            die "ERROR: Dataset \'$dataset\' part \'$part\' has multiple files\n";
        $datasets{$dataset}->[$fileField]->[$offset] = $filename;
    } else {
        # add dataset
        my @filenames;
        $filenames[$offset] = $filename;
        $fields[$fileField] = \@filenames;
        $datasets{$dataset} = \@fields;
    }
}

# Validate files and metadata fields in all datasets using controlled
# vocabulary.  Create .ra file for loader .
&loadControlledVocab;
open(LOADER_RA, ">$outPath/$loadFile") ||
        die "SYS ERROR: Can't write \'$outPath/$loadFile\' file ($!)\n";
foreach $dataset (keys %datasets) {
    my $datasetRef = $datasets{$dataset};
    my $dataType = $datasetRef->[$ddfHeader{'Data Type REF'}];
    my $tableType = $tracks{$dataType}->{'tableType'};
    &HgAutomate::verbose(2, "  Dataset: $dataset\tTrack: $dataType\n");
    for ($i=0; $i < @ddfHeader; $i++) {
        &validateField($ddfHeader[$i], $datasetRef->[$i], $dataType);
    }

    # Construct table name from track name and variables
    my $trackName = "wgEncode" . $dataType;
    my $tableName = $trackName;
    if (defined($pif{'variables'})) {
        my @variables = @{$pif{'variableArray'}};
        for (my $i = 0; $i < @variables; $i++) {
            $tableName = $tableName . $datasetRef->[$ddfHeader{$variables[$i]}];
        }
    }
    print LOADER_RA "tablename $tableName\n";
    print LOADER_RA "track $trackName\n";
    print LOADER_RA "type $tracks{$dataType}->{'type'}\n";
    print LOADER_RA "tableType $tableType\n" if defined($tableType);
    print LOADER_RA "assembly $datasetRef->[$ddfHeader{'Assembly REF'}]\n";
    print LOADER_RA "files @{$datasetRef->[$ddfHeader{'File Name'}]}\n";
    print LOADER_RA "\n";
}
close(LOADER_RA);

exit 0;

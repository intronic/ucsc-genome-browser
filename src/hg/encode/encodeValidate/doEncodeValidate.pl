#!/usr/bin/env perl

# encodeValidate.pl - validate an ENCODE data submission generated by the
#                       automated submission pipeline
# Verifies that all files and metadata are present and of correct formats
# Creates a load script (load.csh) and track configuration (trackDb.ra) 
#               for the datasets
# Writes error or log information to STDOUT
# Returns 0 if validation succeeds.

# TODO:  Handle Mac and DOS EOLs

# DO NOT EDIT the /cluster/bin/scripts copy of this file -- 
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeValidate/doEncodeValidate.pl,v 1.11 2008/01/11 19:55:15 kate Exp $

use warnings;
use strict;
use lib "/cluster/bin/scripts";
use HgAutomate;
use File::stat;

sub usage {
    die "usage: encodeValidate.pl <submission type> <project submission dir>\n";
}

# Global constants
our $loaderPath = "/usr/local/apache/cgi-bin/loader";
our $encodeConfigDir = '../../config'; # change to top of scratch dir  before deployment
our $fieldConfigFile = $encodeConfigDir . "/fields.ra";
our $vocabConfigFile = $encodeConfigDir . "/cv.ra";
our $trackFile = 'trackDb.ra';
our $loadScript = 'load.sh';
our $loadRa = 'load.ra';
our $logDir = "log";

# Global variables
our $submitDir;
our $opt_verbose = 1;
our %terms;             # controlled vocabulary
our %pif;               # project information

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
    );

# standard validators (required or optional for all projects)
sub validateFileName {
    # Validate array of filenames, ordered by part
    # Check files exist and are of correct data format
    my ($files, $track) = @_;
    my @files = @{$files};
    for (my $i=0; $i < @files; $i++) {
        my $file = $files[$i];
        my $part = $i + 1;
        defined($file) || die "ERROR: Dataset missing part \'$part\'\n";
        -e $file || die "ERROR: File \'$file\' does not exist\n";
        -s $file || die "ERROR: File \'$file\' is empty\n";
        -r $file || die "ERROR: File \'$file\' is not readable \n";
        &checkDataFormat($pif{'trackHash'}->{$track}, $file);
    }
    print "    Files: ", join (' ', @files), "\n";
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
    if (!%terms) {
        &loadControlledVocab;
    }
    defined($terms{'Cell Line'}{$val}) || die "ERROR: Cell line \'$val\' is not known \n";
}

############################################################################
# Format checkers - extend when adding new data format

# dispatch table
our %formatCheckers = (
    wig => \&validateWig,
    bed => \&validateBed,
    );

sub validateWig {
    my ($file) = @_;
    my $logFile = $logDir . "/validateWig.out";
    my $err = system (
        "head -10 $file | $loaderPath/wigEncode stdin /dev/null /dev/null >$logFile 2>&1");
    if ($err) {
        print STDERR  "ERROR: File \'$file\' failed wiggle validation\n";
        open(ERR, $logFile) || die "ERROR: Can't open wiggle validation file \'$logFile\': $!\n";
        my @err = <ERR>;
        die "@err\n";
    } else {
        print "Passed\n";
    }
}

sub validateBed {
    my ($file, $type) = @_;
    my $logFile = $logDir . "/validateBed.out";
    my $err = system (
        "head -10 $file | egrep -v '^track|browser' | $loaderPath/hgLoadBed -noLoad hg18 testTable stdin >$logFile 2>&1");
    if ($err) {
        print STDERR  "ERROR: File \'$file\' failed bed validation\n";
        open(ERR, $logFile) || die "ERROR: Can't open bed validation file \'$logFile\': $!\n";
        my @err = <ERR>;
        die "@err\n";
    } else {
        print "Passed\n";
    }
}

############################################################################
# Misc subroutines

sub validateField {
    # validate value for type of field
    my ($type, $val, $arg) = @_;
    $type =~ s/ /_/g;
    &HgAutomate::verbose(2, "Validating $type : $val\n");
    $validators{$type}->($val, $arg);
}

sub checkDataFormat {
    # validate file type
    my ($format, $file) = @_;
    &HgAutomate::verbose(1, "Checking data format for $file : $format\n");
    my $type = "";
    if ($format =~ m/(bed)(\d+)/) {
        $format = $1;
        $type = $2;
    }
    $formatCheckers{$format} || 
        die "ERROR: Data format \'$format\' in PIF file is unknown\n";
    $formatCheckers{$format}->($file, $type);
}

sub loadControlledVocab {
    %terms = ();
    my %termRa = &readRaFile($vocabConfigFile, "term");
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
    my $pifFile = &newestFile(glob "*.PIF");
    &HgAutomate::verbose(1, "Using newest PIF file \'$pifFile\'\n");
    open(PIF, $pifFile) || die "ERROR: Can't open PIF file \'$pifFile\'\n";
    while (my $line = <PIF>) {
        # strip leading and trailing spaces
        $line =~ s/^ +//;
        $line =~ s/ +$//;
        # ignore empty lines and comments
        next if $line =~ /^$/;
        next if $line =~ /^#/;
        chomp $line;
        my ($key, $val) = split(/\t/, $line);
        $pif{$key} = $val;
        HgAutomate::verbose(1, "PIF field: $key = $val\n");
    }
    close(PIF);
    # Validate fields
    defined($pif{'project'}) || die "ERROR: Project not defined\n"; 
    defined($pif{'tracks'}) || 
        die "ERROR: Tracks not defined for project \'$pif{'project'}\'\n";
    $pif{'active'} =~ "yes" || 
        die "ERROR: Project \'$pif{'project'}\' not yet active\n";

    # Reformat fields in more convenient form
    my @tracks = split(' ', $pif{'tracks'});
    my %tracks = ();
    foreach my $track (@tracks) {
        my ($trackName, $trackType) = split(':', $track);
        $tracks{$trackName} = $trackType;
    }
    $pif{'trackHash'} = \%tracks;

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
    return %pif;
}

sub readRaFile {
    # Read records from a .ra file into a hash of hashes and return it.
    my ($file, $type) = @_;
    open(RA, $file) || die "ERROR: Can't open RA file \'$fieldConfigFile\'\n";
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

# Change dir to submission directory obtained from command-line
if (scalar(@ARGV) < 2) { usage(); }
my $submitType = $ARGV[0];	# currently not used
$submitDir = $ARGV[1];
&HgAutomate::verbose(1, "Validating submission in directory \'$submitDir\'\n");
chdir $submitDir ||
    die ("SYS ERROR; Can't change to submission directory \'$submitDir\': $!\n");
mkdir $logDir || 
    die ("SYS ERROR; Can't create log directory \'$logDir\': $!\n");

# Locate project information (PIF) file and verify that project is
#  ready for submission
%pif = &getPif();

# Gather fields defined for DDF file. File is in 
# ra format:  field <name>, required <true|false>
my %fields = &readRaFile($fieldConfigFile, "field");

# Add required fields for this -- the variables in the PIF file
foreach my $variable (keys %{$pif{'variableHash'}}) {
    $fields{$variable}->{'required'} = 'yes';
}

# Open dataset descriptor file (DDF)
my $ddfFile = &newestFile(glob "*.DDF");
&HgAutomate::verbose(1, "Using newest DDF file \'$ddfFile\'\n");
open(IN, $ddfFile) || die "ERROR: Can't open DDF file \'$ddfFile\'\n";

# Get header containing column names
while ($line = <IN>) {
    # remove leading and trailing spaces and newline
    $line =~ s/^ +//;
    $line =~ s/ +$//;
    # ignore empty lines and comments
    next if $line =~ /^$/;
    next if $line =~ /^#/;
    chomp $line;
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
while ($line = <IN>) {
    $line =~ s/^ +//;
    $line =~ s/ +$//;
    next if $line =~ /^#/;
    next if $line =~ /^$/;
    chomp $line;
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
close(IN);

# Validate files and metadata fields in all datasets.  Create .ra file
# for loader 
open(LOADER_RA, ">$loadRa") || 
        die "SYS ERROR: Can't write \'$loadRa\' file ($!)\n";

foreach $dataset (keys %datasets) {
    my $datasetRef = $datasets{$dataset};
    my $dataType = $datasetRef->[$ddfHeader{'Data Type REF'}];
    &HgAutomate::verbose(1, "Dataset: $dataset\tTrack: $dataType\n");
    for ($i=0; $i < @ddfHeader; $i++) {
        &validateField($ddfHeader[$i], $datasetRef->[$i], $dataType);
    }
    my $tableName = "wgEncode" . $dataType;
    my @variables = @{$pif{'variableArray'}};
    for (my $i = 0; $i < @variables; $i++) {
        $tableName = $tableName . $datasetRef->[$ddfHeader{$variables[$i]}];
    }
    print LOADER_RA "tablename $tableName\n";
    print LOADER_RA "type $pif{'trackHash'}->{$dataType}\n";
    print LOADER_RA "assembly $datasetRef->[$ddfHeader{'Assembly REF'}]\n";
    print LOADER_RA "files @{$datasetRef->[$ddfHeader{'File Name'}]}\n";
    print LOADER_RA "\n";
}
close(LOADER_RA);

exit 0;

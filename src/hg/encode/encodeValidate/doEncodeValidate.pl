#!/usr/bin/env perl

# encodeValidate.pl - validate an ENCODE data submission generated by the
#                       automated submission pipeline
#
# Verifies that all files and metadata are present and of correct formats
# Creates a load file (load.ra) and track configuration (trackDb.ra) for the datasets
#
# Returns 0 if validation succeeds
#
# Error reporting:
#
# We die immediately (with a human readable message) when internal errors are encountered (e.g. file I/O errors or misconfiguration).
#
# In order to facilitate debugging of often very large file uploads, we try to accumulate multiple user errors (e.g. DAF, DAS or 
# file syntax errors) before die'ing with a message with a list of errors.

# DO NOT EDIT the /cluster/bin/scripts copy of this file -- 
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeValidate/doEncodeValidate.pl,v 1.63 2008/09/16 20:08:57 larrym Exp $

use warnings;
use strict;

use File::stat;
use Getopt::Long;
use English;
use Carp qw(cluck);
use Cwd;

use lib "/cluster/bin/scripts";
use Encode;
use HgAutomate;
use HgDb;
use RAFile;
use SafePipe;

use vars qw/
    $opt_allowReloads
    $opt_configDir
    $opt_outDir
    $opt_validateDaf
    $opt_verbose
    /;

# Global variables
our $submitPath;        # full path of data submission directory
our $configPath;        # full path of configuration directory
our $outPath;           # full path of output directory
our %terms;             # controlled vocabulary

sub usage {
    print STDERR <<END;
usage: encodeValidate.pl submission-type project-submission-dir

submission-type is currently ignored.

Current dafVersion is: $Encode::dafVersion

options:
    -allowReloads       Allow reloads of existing tables
    -configDir dir      Path of configuration directory, containing
                        metadata .ra files (default: submission-dir/../config)
    -validateDaf	exit after validating DAF file (project-submission-dir is the DAF file name).
    -verbose num        Set verbose level to num (default 1).
    -outDir dir         Path of output directory, for validation files
                        (default: submission-dir/out)
END
exit 1;
}

sub pushError
{
    my ($errors, @new) = @_;
    if(@new) {
        push(@{$errors}, @new);
        HgAutomate::verbose(2, "pushing errors:\n\t" . join("\n\t", @new) . "\n");
    }
}

############################################################################
# Validators for DDF columns -- extend when adding new metadata fields
#
# validators should return list of errors encountered (empty list means no errors were found).
#
# validator callbacks are called thus:
#
# validator(value, track, daf);
#
# value is value in DDF column
# track is track/view value
# daf is daf hash

# dispatch table
our %validators = (
    files => \&validateFiles,
    view => \&validateDatasetName,
    labVersion => \&validateLabVersion,
    cell => \&validateCellLine,
    gene => \&validateGeneType,
    antibody => \&validateAntibody,
    rnaExtract => \&validateRnaExtract,
    localization => \&validateLocalization,
    replicate => \&validateReplicate,
    );

# standard validators (required or optional for all projects)

sub validateFiles {
    # Validate array of filenames, ordered by part
    # Check files exist and are of correct data format
    my ($files, $track, $daf) = @_;
    my @newFiles;
    my @errors;
    my $regex = "\`\|\\\|\|\"\|\'";
    for my $file (@{$files}) {
        my @list = glob $file;
        if(@list) {
            push(@newFiles, @list);
        } else {
            pushError(\@errors, "File '$file' does not exist (possibly bad glob?)");
        }
    }
    HgAutomate::verbose(3, "     Track: $track    Files: " . join (' ', @newFiles) . "\n");
    for my $file (@newFiles) {
        if($file =~ /($regex)/) {
            # Do not allows filenames with suspicious characters (b/c filename will be used in shell commands).
            pushError(\@errors, "File '$file' has invalid characters; files cannot contain following characters: \"'`|");
        } elsif(!-e $file) {
            pushError(\@errors, "File \'$file\' does not exist");
        } elsif(!(-s $file)) {
            pushError(\@errors, "File \'$file\' is empty");
        } elsif(!(-r $file)) {
            pushError(\@errors, "File \'$file\' is un-readable");
        } else {
            pushError(\@errors, checkDataFormat($daf->{TRACKS}{$track}{type}, $file));
        }
    }
    $files = \@newFiles;
    return @errors;
}

sub validateDatasetName {
    my ($val) = @_;
    return ();
}

sub validateDataType {
    my ($val) = @_;
    return ();
}

sub validateRawDataAcc {
# No validation
    return ();
}

sub validateLabVersion {
# No validation
    return ();
}

# project-specific validators

sub validateCellLine {
    my ($val) = @_;
    return defined($terms{'Cell Line'}{$val}) ? () : ("Cell line \'$val\' is not known");
}

sub validateRnaExtract {
    my ($val) = @_;
    return defined($terms{'rnaExtract'}{$val}) ? () : ("rnaExtract \'$val\' is not known");
}

sub validateLocalization {
    my ($val) = @_;
    return defined($terms{'localization'}{$val}) ? () : ("localization \'$val\' is not known");
}

sub validateGeneType {
    my ($val) = @_;
    return defined($terms{'Gene Type'}{$val}) ? () : ("Gene type \'$val\' is not known");
}

sub validateAntibody {
    my ($val) = @_;
    if(lc($val) eq 'input' || lc($val) eq 'control' || defined($terms{'Antibody'}{$val})) {
        return ();
    } else {
        return ("Antibody \'$val\' is not known");
    }
}

sub validateReplicate {
    return ();
}

############################################################################
# Format checkers - check file format for given types; extend when adding new 
# data formats
#
# Some of the checkers use regular expressions to validate syntax of the files.
# Others pass first 10 lines to utility loaders; the later has:
# advantages:
# 	checks semantics as well as syntax
# disadvantages;
# 	only checks the beginning of the file
# 	but some of the loaders tolerate (but give incorrect results) for invalid files

# dispatch table
our %formatCheckers = (
    wig => \&validateWig,
    bed => \&validateBed,
    bed5FloatScore => \&validateBed,
    genePred => \&validateGene,
    tagAlign => \&validateTagAlign,
    narrowPeak => \&validateNarrowPeak,
    fastq => \&validateFastQ,
    );

my $floatRegEx = "[+-]?(?:\\.\\d+|\\d+(?:\\.\\d+|))";

sub validateWig
{
    my ($path, $file, $type) = @_;
    my $filePath = "$path/$file";

    # XXXX why not do the whole thing, rather than just 1000 lines?
    my @cmds = ("head -1000 $filePath", "wigEncode stdin /dev/null /dev/null");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null");
    if(my $err = $safe->exec()) {
        my $err = $safe->stderr();
        chomp($err);
        return "File \'$file\' failed wiggle validation: " . $err;
    } else {
        HgAutomate::verbose(2, "File \'$file\' passed wiggle validation\n");
    }
    return ();
}

sub validateBed {
# Validate each line of a bed 5 or greater file.
    my ($path, $file, $type) = @_;
    my $filePath = "$path/$file";
    my $line = 0;
    open(FILE, $filePath) or die "Couldn't open file: $filePath; error: $!\n";
    while(<FILE>) {
        chomp;
        my @fields = split /\s+/;
        $line++;
        my $fieldCount = @fields;
        next if(!$fieldCount);
        my $prefix = "Failed bed validation, file '$file'; line $line:";
        if(/^(track|browser)/) {
            ;
        } elsif($fieldCount < 5) {
            die "$prefix not enough fields; " . scalar(@fields) . " present; at least 5 are required\n";
        } elsif ($fields[0] !~ /^chr(\d+|M|X|Y)$/) {
            # I have seen non-standard chrom names (e.g. "chr2_hap2" from Wold) and we want
            # to make sure we don't import those.
            die "$prefix field 1 value ($fields[0]) is invalid; not a valid chrom name\n";
        } elsif ($fields[1] !~ /^\d+$/) {
            die "$prefix field 2 value ($fields[1]) is invalid; value must be a positive number\n";
        } elsif ($fields[2] !~ /^\d+$/) {
            die "$prefix field 3 value ($fields[2]) is invalid; value must be a positive number\n";
        } elsif ($fields[2] < $fields[1]) {
            die "$prefix field 3 value ($fields[2]) is less than field 2 value ($fields[1])\n";
        } elsif ($fields[4] !~ /^\d+$/ && $fields[4] !~ /^\d+\.\d+$/) {
            die "$prefix field 5 value ($fields[4]) is invalid; value must be a positive number\n";
        } elsif ($fields[4] < 0 || $fields[4] > 1000) {
            die "$prefix field 5 value ($fields[4]) is invalid; score must be 0-1000\n";
        } elsif ($type eq 'bed5FloatScore' && $fieldCount < 6) {
            die "$prefix field 6 invalid; bed5FloatScore requires 6 fields";
        } elsif ($type eq 'bed5FloatScore' && $fields[5] !~ /^$floatRegEx^/) {
            die "$prefix field 6 value '$fields[5]' is invalid; must be a float\n";
        } else {
            ;
        }
    }
    close(FILE);
    HgAutomate::verbose(2, "File \'$file\' passed bed validation\n");
    return ();
}

sub validateGene {
    my ($path, $file, $type) = @_;
    my $outFile = "validateGene.out";
    my $filePath = "$path/$file";
    my $err = system (
        "cd $outPath; egrep -v '^track|browser' $filePath | ldHgGene -out=genePred.tab -genePredExt hg18 testTable stdin >$outFile 2>&1");
    if ($err) {
        print STDERR  "File \'$file\' failed GFF validation\n";
        open(ERR, "$outPath/$outFile") || die "ERROR: Can't open GFF validation file \'$outPath/$outFile\': $!\n";
        my @err = <ERR>;
        die "@err\n";
    } else {
        HgAutomate::verbose(2, "File \'$file\' passed GFF validation\n");
    }
    return ();
}

sub validateTagAlign
{
    my ($path, $file, $type) = @_;
    my $filePath = "$path/$file";
    my $line = 0;
    open(FILE, $filePath) or die "Couldn't open file '$filePath'; error: $!\n";
    while(<FILE>) {
        $line++;
        if(!(/^chr(\d+|M|X|Y)\s+\d+\s+\d+\s+[ATCG\.]+\s+\d+\s+[+-]$/)) {
            chomp;
            return "Invalid $type file; line $line in file '$file' is invalid:\nline: $_";
        }
    }
    close(FILE);
    HgAutomate::verbose(2, "File \'$file\' passed $type validation\n");
    return ();
}

sub validateNarrowPeak
{
    my ($path, $file, $type) = @_;
    my $filePath = "$path/$file";
    my $line = 0;
    open(FILE, $filePath) or die "Couldn't open file '$filePath'; error: $!\n";
    while(<FILE>) {
        $line++;
        if(!(/^chr(\d+|M|X|Y)\s+\d+\s+\d+\s+\S+\s+\d+\s+[+-\.]\s+$floatRegEx\s+$floatRegEx\s+\d+$/)) {
            chomp;
            return ("Invalid $type file; line $line in file '$file' is invalid:\nline: $_");
        }
    }
    close(FILE);
    HgAutomate::verbose(2, "File \'$file\' passed $type validation\n");
    return ();
}

sub validateFastQ
{
    # Syntax per http://maq.sourceforge.net/fastq.shtml
    my ($path, $file, $type) = @_;
    my $filePath = "$path/$file";
    my $line = 0;
    open(FILE, $filePath) or die "Couldn't open file '$filePath'; error: $!\n";
    my $state = 'firstLine';
    my $seqName;
    my $seqNameRegEx = "[A-Za-z0-9_.:-]+";
    my $seqRegEx = "[A-Za-z\n\.~]+";
    my $qualRegEx = "[!-~\n]+";
    my $states = {firstLine => {REGEX => "\@($seqNameRegEx)", NEXT => 'seqLine'},
                  seqLine => {REGEX => $seqRegEx, NEXT => 'plusLine'},
                  plusLine => {REGEX => "\\\+([A-Za-z0-9_.:-]*)", NEXT => 'qualLine'},
                  qualLine => {REGEX => $qualRegEx, NEXT => 'firstLine'}};
    while(<FILE>) {
        chomp;
        $line++;
        my $errorPrefix = "Invalid $type file; line $line in file '$file' is invalid";
        my $regex = $states->{$state}{REGEX};
        if(/^${regex}$/) {
	        $seqName = $1 if($state eq 'firstLine');
                if($state eq 'plusLine' && defined($1) && $1 && $1 ne $seqName) {
                    return("$errorPrefix: seqence name '$1' does not match previous seqence name '$seqName'\nline: $_");
                }
	        $state = $states->{$state}{NEXT};
        } else {
	         return("$errorPrefix (expecting $state):\nline: $_");
        }
    }
    close(FILE);
    HgAutomate::verbose(2, "File \'$file\' passed $type validation\n");
    return ();
}

############################################################################
# Misc subroutines

sub validateDdfField {
    # validate value for type of field
    my ($type, $val, $track, $daf) = @_;
    $type =~ s/ /_/g;
    HgAutomate::verbose(4, "Validating $type: " . (defined($val) ? $val : "") . "\n");
    if($validators{$type}) {
        return $validators{$type}->($val, $track, $daf);
    } else {
        die "Validator for type '$type' is missing";
    }
}

sub checkDataFormat {
    # validate file type
    my ($format, $file) = @_;
    HgAutomate::verbose(3, "Checking data format for $file: $format\n");
    my $type = $format;
    if ($format =~ m/(bed) (\d+)/) {
        $format = $1;
    }
    $formatCheckers{$format} || return "Data format \'$format\' is unknown\n";
    return $formatCheckers{$format}->($submitPath, $file, $type);
}

sub ddfKey
{
# return key for given DDF line (e.g. "antibody=$antibody;cell=$cell" for ChIP-Seq data)
    my ($fields, $ddfHeader, $daf) = @_;
    if (defined($daf->{variables})) {
        return join(";", map("$_=" . $fields->{$_}, sort @{$daf->{variableArray}}));
    } else {
        die "ERROR: no key defined for this DAF\n";
    }
}

############################################################################
# Main

my $now = time();
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($now);
my @ddfHeader;		# list of field names on the first line of DDF file
my %ddfHeader = ();	# convenience hash version of @ddfHeader (maps name to field index)
my @ddfLines = ();	# each line in DDF (except for fields header); value is a hash; e.g. {files => 'foo.bed', cell => 'HeLa-S3', ...}
my %ddfSets = ();	# info about DDF entries broken down by ddfKey
my $wd = cwd();

my $ok = GetOptions("allowReloads",
                    "configDir=s",
                    "outDir=s",
                    "validateDaf",
                    "verbose=i",
                    );
usage() if (!$ok);
usage() if (scalar(@ARGV) < 2);

# Get command-line args
my $submitType = $ARGV[0];	# currently not used
my $submitDir = $ARGV[1];

# Get general options
$opt_verbose = 1 if (!defined $opt_verbose);

# Determine submission, configuration, and output directory paths
HgAutomate::verbose(2, "Validating submission in directory \'$submitDir\'\n");
if ($submitDir =~ /^\/.*/) {
    $submitPath = $submitDir;
} else {
    $submitPath = "$wd/$submitDir";
}
HgAutomate::verbose(4, "Submission directory path: \'$submitPath\'\n");

if (defined $opt_configDir) {
    if ($opt_configDir =~ /^\//) {
        $configPath = $opt_configDir;
    } else {
        $configPath = "$wd/$opt_configDir";
    }
} else {
    $configPath = "$submitPath/../config"
}
if(!(-d $configPath)) {
    die "configPath '$configPath' is invalid; Can't find the config directory\n";
}
HgAutomate::verbose(4, "Config directory path: \'$configPath\'\n");

if (defined $opt_outDir) {
    if ($opt_outDir =~ /^\//) {
        $outPath = $opt_outDir;
    } else {
        $outPath = "$wd/$opt_outDir";
    }
} else {
    $outPath = "$submitPath/out"
}
HgAutomate::verbose(4, "Output directory path: '$outPath'; submitPath: '$submitPath'\n");

if(!$opt_validateDaf) {
    # Change dir to submission directory 
    if(!chdir($submitPath)) {
        die ("SYS ERR; Can't change to submission directory \'$submitPath\': $OS_ERROR\n");
    }
    HgAutomate::verbose(3, "Creating output in directory \'$outPath\'\n");
    if(!(-d $outPath)) {
        mkdir $outPath || die ("SYS ERR: Can't create out directory \'$outPath\': $OS_ERROR\n");
    }
}

# labs is now in fact the list of grants (labs are w/n grants, and are not currently validated).
my $grants = Encode::getGrants($configPath);
my $fields = Encode::getFields($configPath);

if($opt_validateDaf) {
    if(-f $submitDir) {
        Encode::parseDaf($submitDir, $grants, $fields);
    } else {
        Encode::getDaf($submitDir, $grants, $fields);
    }
    print STDERR "DAF is valid\n";
    exit(0);
}

my $daf = Encode::getDaf($submitDir, $grants, $fields);

my $db = HgDb->new(DB => $daf->{assembly});

# Add the variables in the DAF file to the required fields list
if (defined($daf->{variables})) {
    for my $variable (keys %{$daf->{variableHash}}) {
        $fields->{$variable}{required} = 1;
        $fields->{$variable}{file} = 'ddf';
    }
}

# make replicate column required when appropriate.
my $hasReplicates = 0;
my $maxOrder = 0;
for my $view (keys %{$daf->{TRACKS}}) {
    $hasReplicates += $daf->{TRACKS}{$view}{hasReplicates};
    if($daf->{TRACKS}{$view}{order} > $maxOrder) {
        $maxOrder = $daf->{TRACKS}{$view}{order}
    }
}

if($hasReplicates) {
    $fields->{replicate}{required} = 1;
}

# Open dataset descriptor file (DDF)
my @glob = glob "*.DDF";
push(@glob, glob "*.ddf");
my $ddfFile = Encode::newestFile(@glob);
HgAutomate::verbose(2, "Using newest DDF file \'$ddfFile\'\n");
my $lines = Encode::readFile($ddfFile);

my $ddfLineNumber = 0;
# Get header containing column names
while(@{$lines}) {
    my $line = shift(@{$lines});
    $ddfLineNumber++;
    # remove leading and trailing spaces and newline
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    # ignore empty lines and comments
    next if $line =~ /^$/;
    next if $line =~ /^#/;
    if($line !~ /\t/) {
        die "ERROR: The DDF header has no tabs; the DDF is required to be tab delimited\n";
    }
    @ddfHeader = split(/\t/, $line);
    for (my $i=0; $i < @ddfHeader; $i++) {
        $ddfHeader{$ddfHeader[$i]} = $i;
    }
    last;
}

my @errors = Encode::validateFieldList(\@ddfHeader, $fields, 'ddf');
if(@errors) {
    die "ERROR in DDF '$ddfFile':\n" . join("\n", @errors) . "\n";
}

%terms = Encode::getControlledVocab($configPath);

# Process lines in DDF file. Create a list with one entry per line;
# the entry is field/value hash (fields per @ddfHeader).

while (@{$lines}) {
    my $line = shift(@{$lines});
    $ddfLineNumber++;
    my $errorPrefix = "DDF lineNumber $ddfLineNumber:";
    HgAutomate::verbose(2, "Parsing ddf line $ddfLineNumber\n");

    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    next if $line =~ /^#/;
    next if $line =~ /^$/;

    if($line !~ /\t/) {
        pushError(\@errors, "$errorPrefix line has no tabs; the DDF is required to be tab delimited");
        next;
    }
    my $i = 0;
    my %line;
    for my $val (split('\t', $line)) {
        $line{$ddfHeader[$i]} = $val;
        $i++;
    }

    if(my @tmp = Encode::validateValueList(\%line, $fields, 'ddf')) {
        pushError(\@errors, $errorPrefix . "\n" . join("\n", @tmp));
        next;
    }

    my $view = $line{view};
    if($daf->{TRACKS}{$view}) {
        my $files = $line{files};
        if($fields->{replicate}{required}) {
            my $replicate = $line{replicate};
            if($daf->{TRACKS}{$view}{hasReplicates} && (!defined($replicate) || !length($replicate))) {
                pushError(\@errors, "$errorPrefix missing replicate number for view '$view'");
            }
        }
        my @filenames;
        for(split(',', $files)) {
            # Use glob explicitly so our error messages have the list of files actually used.
            if(my @glob = glob) {
                push(@filenames, @glob);
            } else {
                push(@filenames, $_);
            }
        }
        $line{files} = \@filenames;
        my @metadataErrors;
        for my $field (keys %line) {
            push(@metadataErrors, validateDdfField($field, $line{$field}, $view, $daf));
        }
        if(@metadataErrors) {
            pushError(\@errors, @metadataErrors);
        } else {
            # avoid spurious errors by not putting invalid lines into %ddfSets
            $ddfSets{ddfKey(\%line, \%ddfHeader, $daf)}{VIEWS}{$view} = \%line;
        }
        push(@ddfLines, \%line);
    } else {
        pushError(\@errors, "$errorPrefix undefined view '$view'");
    }
}

my $tmpCount = 1;

if(!@errors) {
    # Look for missing required views and create missing, optional views, but
    # but don't bother if we have already encountered errors.
    for my $key (keys %ddfSets) {
        for my $view (keys %{$daf->{TRACKS}}) {
            if($daf->{TRACKS}{$view}{required}) {
                if(!defined($ddfSets{$key}{VIEWS}{$view})) {
                    pushError(\@errors, "view '$view' missing for $key");
                }
            }
        }
        
        # create missing optional views (e.g. ChIP-Seq RawSignal)
        if(defined($ddfSets{$key}{VIEWS}{Alignments}) && !defined($ddfSets{$key}{VIEWS}{RawSignal})) {
            my $newView = 'RawSignal';
            my $alignmentLine = $ddfSets{$key}{VIEWS}{Alignments};
            my %line = %{$alignmentLine};
            $line{view} = $newView;
            $line{type} = 'wig';
            # hack for case where they have removed RawSignal view in the DAF
            if(!defined($daf->{TRACKS}{$newView}{order})) {
                $daf->{TRACKS}{$newView}{order} = ++$maxOrder;
            }
            $ddfSets{$key}{VIEWS}{$newView} = \%line;
            my $files = join(" ", @{$alignmentLine->{files}});
            my $tmpFile = $Encode::autoCreatedPrefix . "$tmpCount.bed";
            $tmpCount++;
            my @cmds = ("sort -k1,1 -k2,2n $files", "bedItemOverlapCount $daf->{assembly} stdin");
            my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => $tmpFile, DEBUG => $opt_verbose - 1);
            if(my $err = $safe->exec()) {
                print STDERR  "ERROR: failed creation of wiggle for $key" . $safe->stderr() . "\n";
                # don't show end-user pipe error(s)
                pushError(\@errors, "failed creation of wiggle for '$key'");
            }
            $line{files} = [$tmpFile];
            push(@ddfLines, \%line);
        }
    }
}

my $compositeTrack = Encode::compositeTrackName($daf);
if(!$db->quickQuery("select count(*) from trackDb where tableName = ?", $compositeTrack)) {
    pushError(\@errors, "Missing composite track '$compositeTrack'; please contact your data wrangler");
}

if(@errors) {
    my $prefix = @errors > 1 ? "Error(s)" : "Error";
    die "$prefix:\n\n" . join("\n\n", @errors) . "\n";    
}

# After this point, we don't use @errors and just die immediately.

# Validate files and metadata fields in all ddfLines using controlled
# vocabulary.  Create load.ra file for loader and trackDb.ra file for wrangler.

open(LOADER_RA, ">$outPath/$Encode::loadFile") || die "SYS ERROR: Can't write \'$outPath/$Encode::loadFile\' file; error: $!\n";
open(TRACK_RA, ">$outPath/$Encode::trackFile") || die "SYS ERROR: Can't write \'$outPath/$Encode::trackFile\' file; error: $!\n";
open(README, ">$outPath/README.txt") || die "SYS ERROR: Can't write '$outPath/READEME.txt' file; error: $!\n";

my @variables;
if (defined($daf->{variables})) {
    @variables = @{$daf->{variableArray}};
}

my $priority = $db->quickQuery("select max(priority) from trackDb where settings like '%subTrack $compositeTrack%'") || 0;
$ddfLineNumber = 1;
foreach my $ddfLine (@ddfLines) {
    $ddfLineNumber++;
    my $diePrefix = "ERROR on DDF lineNumber $ddfLineNumber:";
    my $view = $ddfLine->{view};
    my $type = $daf->{TRACKS}{$view}{type};

    HgAutomate::verbose(2, "  View: $view\n");
    my $replicate;
    if($hasReplicates && $daf->{TRACKS}{$view}{hasReplicates}) {
        $replicate = $ddfLine->{replicate};
        if(defined($replicate) && $replicate > 0) {
        } else {
            die "$diePrefix invalid or missing replicate value\n";
        }
    }
    # Construct table name from track name and variables
    my $tableName = "$compositeTrack$view";
    if(defined($replicate)) {
        $tableName .= "Rep$replicate";
    }
    if(!defined($daf->{TRACKS}{$view}{shortLabelPrefix})) {
        $daf->{TRACKS}{$view}{shortLabelPrefix} = "";
    }
    my $shortLabel = defined($daf->{TRACKS}{$view}{shortLabelPrefix}) ? $daf->{TRACKS}{$view}{shortLabelPrefix} : "";
    my $longLabel = "ENCODE" . (defined($daf->{TRACKS}{$view}{longLabelPrefix}) ? " $daf->{TRACKS}{$view}{longLabelPrefix}" : "");
    if(defined($replicate)) {
        $longLabel .= " Replicate $replicate";
    }
    my $subGroups = "view=$view";
    my $additional = "\n";
    my $pushQDescription = "";
    if (@variables) {
        my %hash = map { $_ => $ddfLine->{$_} } @variables;
        for my $var (@variables) {
            my $val = $hash{$var};
            if(lc($val) eq 'control' || lc($val) eq 'input') {
                $val = ucfirst(lc($val));
            }
            $tableName = $tableName . $val;
        }
        my $shortSuffix;
        my $longSuffix;
        if($hash{antibody} && $hash{cell}) {
            $pushQDescription = "$hash{antibody} in $hash{cell}";
            $shortSuffix = "$hash{antibody} $hash{cell}";
            $longSuffix = "$hash{antibody} in $hash{cell} cells";
        } elsif ($hash{"cell"}) {
            $pushQDescription = "$hash{cell}";
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
    
    # mysql doesn't allow hyphens in table names and our naming convention doesn't allow underbars; to be
    # safe, we strip non-alphanumerics.
    $tableName =~ s/[^A-Za-z0-9]//g;

    if(!$opt_allowReloads) {
        if($db->quickQuery("select count(*) from trackDb where tableName = ?", $tableName)) {
            die "view '$view' has already been loaded as track '$tableName'\nPlease contact your wrangler if you need to reload this data\n";
        }
    }

    # XXXX Move the decision about which views have tracks into the DAF?
    my $downloadOnly = $view eq 'RawData' || $view eq 'Alignments' ? 1 : 0;

    print LOADER_RA "tablename $tableName\n";
    print LOADER_RA "view $view\n";
    print LOADER_RA "type $type\n";
    print LOADER_RA "assembly $daf->{assembly}\n";
    print LOADER_RA "files @{$ddfLine->{files}}\n";
    print LOADER_RA "downloadOnly $downloadOnly\n";
    print LOADER_RA "pushQDescription $pushQDescription\n";
    print LOADER_RA "\n";

    if($downloadOnly) {
        print README "file: $tableName.$type\n";
        for my $var (@variables) {
            print README "$var: " . $ddfLine->{$var} . "\n";
        }
        if(defined($replicate)) {
            print README "replicate: $replicate\n";
        }
        
        my (undef, undef, undef, $rMDay, $rMon, $rYear) = Encode::restrictionDate($now);
        print README sprintf("data restricted until: %d-%02d-%d\n", 1900 + $rYear, $rMon + 1, $rMDay);
        print README "\n";
    } else {
        print TRACK_RA "\ttrack\t$tableName\n";
        print TRACK_RA "\tsubTrack\t$compositeTrack\n";
        print TRACK_RA "\tshortLabel\t$shortLabel\n";
        print TRACK_RA "\tlongLabel\t$longLabel\n";
        print TRACK_RA "\tsubGroups\t$subGroups\n";
        print TRACK_RA "\ttype\t$type\n";
        print TRACK_RA sprintf("\tdateSubmitted\t%d-%02d-%d %d:%d:%d\n", 1900 + $year, $mon + 1, $mday, $hour, $min, $sec);
        print TRACK_RA "\tpriority\t" . ($priority + $daf->{TRACKS}{$view}{order}) . "\n";
        # noInherit is necessary b/c composite track will often have a different dummy type setting.
        print TRACK_RA "\tnoInherit\ton\n";
        my %visibility = (Align => 'hide', RawWignal => 'hide', Signal => 'full', Sites => 'dense');
        if($visibility{$view}) {
            print TRACK_RA "\tvisibility\t$visibility{$view}\n";
        }
        if($type eq 'wig') {
            print TRACK_RA <<END;
	spanList	1
	windowingFunction	mean
	maxHeightPixels	100:16:16
END
	} elsif($type eq 'bed 5 +') {
		print TRACK_RA "\tuseScore\t1\n";
	}
        print TRACK_RA $additional;
    }
}
close(LOADER_RA);
close(TRACK_RA);
close(README);

exit 0;

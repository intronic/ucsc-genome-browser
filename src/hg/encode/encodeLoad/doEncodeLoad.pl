#!/usr/bin/env perl

# encodeLoad.pl - load ENCODE data submission generated by the
#                       automated submission pipeline
# Reads load.ra for information about what to do

# Writes error or log information to STDOUT
# Returns 0 if load succeeds and sends email to wrangler for lab specified in the PIF

# DO NOT EDIT the /cluster/bin/scripts copy of this file -- 
# edit the CVS'ed source at:
# ~/kent/src/hg/encode/encodeLoad/doEncodeLoad.pl

# Usage:
# 
# ./doEncodeLoad.pl unused /cluster/data/encode/pipeline/encpipeline_kate/83

use warnings;
use strict;

use Getopt::Long;
use Cwd;
use File::Temp;
use File::Basename;

use lib "/cluster/bin/scripts";
use Encode;
use RAFile;
use SafePipe;

use vars qw/$opt_configDir $opt_noEmail $opt_outDir $opt_verbose/;

my $loadRa = "out/$Encode::loadFile";
my $unloadRa = "out/$Encode::unloadFile";
my $submitDir = "";
my $submitFQP;
my $submitType = "";
my $tempDir = "/data/tmp";
my $encInstance = "";
my $sqlCreate = "/cluster/bin/sqlCreate";

# Add type names to this list for types that can be loaded via .sql files (e.g. bed5FloatScore.sql)
# You also have to make sure the .sql file is copied into the $sqlCreate directory.
my @extendedTypes = ("encodePeak", "tagAlign", "bed5FloatScore");

my $debug = 0;

sub usage
{
    die <<END
usage: doEncodeLoad.pl submission_type project_submission_dir

Assumes existence of a file called: project_submission_dir/$loadRa
END
}

sub dieFile
{
# read contents of $file and print it in a die message.
    my ($file) = @_;
    open(FILE, $file);
    die join("", <FILE>) . "\n";
}

sub loadGene
{
    my ($assembly, $tableName, $fileList, $pushQ) = @_;

    if(system("cat $fileList | egrep -v '^track|browser' | ldHgGene -genePredExt $assembly $tableName stdin > out/loadGene.out 2>&1")) {
        print STDERR "ERROR: File(s) '$fileList' failed gene load.\n";
        dieFile("out/loadGene.out");
    } else {
        print "$fileList loaded into $tableName\n";
        # debug restore: File.delete "genePred.tab";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadWig
{
    my ($assembly, $tableName, $fileList, $pushQ) = @_;

    my @cmds = ("cat $fileList", "wigEncode stdin stdout $tableName.wib", "hgLoadWiggle -pathPrefix=/gbdb/$assembly/wib -tmpDir=$tempDir $assembly $tableName stdin");
    my $stderrFile = "out/$tableName.err";
    unlink($stderrFile);
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", STDERR => $stderrFile, DEBUG => $debug);
    if(my $err = $safe->exec()) {
        print STDERR "ERROR: File(s) $fileList failed wiggle load:\n\n";
        dieFile($stderrFile);
    } elsif (system( "rm -f /gbdb/$assembly/wib/$tableName.wib") ||
             system( "ln -s $submitFQP/$tableName.wib /gbdb/$assembly/wib")) {
        die("ERROR: failed wiggle ln\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
    push(@{$pushQ->{FILES}}, "$submitFQP/$tableName.wib");
}

sub loadBed
{
    my ($assembly, $tableName, $fileList, $pushQ) = @_;
    #TEST by replacing "cat" with  "head -1000 -q"
    my @cmds = ("cat $fileList", "egrep -v '^track|browser'", "hgLoadBed $assembly $tableName stdin -tmpDir=out");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $debug);
    if(my $err = $safe->exec()) {
        die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadBedFromSchema
{
# Load bed using a .sql file
    my ($assembly, $tableName, $fileList, $sqlTable, $pushQ) = @_;

    if(!(-e "$sqlCreate/${sqlTable}.sql")) {
        die "SQL schema '$sqlCreate/${sqlTable}.sql' does not exist\n";
    }
    if(!(open(SQL, "$sqlCreate/${sqlTable}.sql"))) {
        die "Cannot open SQL schema '$sqlCreate/${sqlTable}.sql'; error: $!\n";
    }

    # create temporary copy of schema file with "CREATE TABLE $tableName"
    my $sql = join("", <SQL>);
    if(!($sql =~ s/CREATE TABLE $sqlTable/CREATE TABLE $tableName/g)) {
        die "sql names do not match for substitution: $sqlTable $tableName\n";
    }

    print STDERR "loadBedFromSchema: $sql\n" if($debug);

    my ($fh, $tempFile) = File::Temp::tempfile("sqlXXXX", UNLINK => 1);
    print STDERR "sql schema tempFile: $tempFile\n" if($debug);
    $fh->print($sql);

    #TEST by replacing "cat" with  "head -1000 -q"
    my @cmds = ("cat $fileList", "egrep -v '^track|browser'", "hgLoadBed $assembly $tableName stdin -tmpDir=out -sqlTable=$tempFile");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $debug);

    if(my $err = $safe->exec()) {
        die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    $fh->close();
    unlink($tempFile);
    close(SQL);
    push(@{$pushQ->{TABLES}}, $tableName);
}


############################################################################
# Main

my $wd = cwd();

GetOptions("configDir=s", "noEmail", "outDir=s", "verbose=i") || usage();
$opt_verbose = 1 if (!defined $opt_verbose);
$opt_noEmail = 0 if (!defined $opt_noEmail);

# Change dir to submission directory obtained from command-line

if(@ARGV != 2) {
    usage();
}

$submitType = $ARGV[0];	# currently not used
$submitDir = $ARGV[1];
if ($submitDir =~ /^\//) {
    $submitFQP = $submitDir;
} else {
    $submitFQP = "$wd/$submitDir";
}

my $configPath;
if (defined $opt_configDir) {
    if ($opt_configDir =~ /^\//) {
        $configPath = $opt_configDir;
    } else {
        $configPath = "$wd/$opt_configDir";
    }
} else {
    $configPath = "$submitDir/../config"
}

my %labs = Encode::getLabs($configPath);
my %fields = Encode::getFields($configPath);
my %pif = Encode::getPif($submitDir, \%labs, \%fields);

my $tableSuffix = "";
# yank out "beta" from encinstance_beta
if(dirname($submitDir) =~ /(_.*)/) {
    $tableSuffix = "$1_" . basename($submitDir);;
} else {
    $tableSuffix = "_" . basename($submitDir);;
}    

chdir($submitDir);

# clean out any stuff from previous load
# We assume unload program is in the same location as loader (fixes problem with misconfigured qateam environment).

my $programDir = dirname($0);
if(system("$programDir/doEncodeUnload.pl $submitType $submitDir")) {
    die "unload script failed\n";
}

if(!(-e $loadRa)) {
    die "ERROR: load.ra not found\n";
}

#TODO change to : FileUtils.cp $loadRa, $unloadRa
# XXXX shouldn't we do the cp AFTER we finish everything else successfully?
if(system("cp $loadRa $unloadRa")) {
    die "Cannot: cp $loadRa $unloadRa\n";
}

print STDERR "Loading project in directory $submitDir\n" if($debug);

# Load files listed in load.ra

my %ra = RAFile::readRaFile($loadRa, 'tablename');
my $pushQ = {};
$pushQ->{TABLES} = [];

print STDERR "$loadRa has: " . scalar(keys %ra) . " records\n" if($debug);

print STDERR "\n" if($debug);

for my $key (keys %ra) {
    my $h = $ra{$key};
    my $tablename = $h->{tablename} . $tableSuffix;

    my $str = "\nkeyword: $key\n";
    for my $field (qw(tablename type assembly files)) {
        if($h->{$field}) {
            $str .= "$field: " . $h->{$field} . "\n";
        }
    }
    $str .= "\n";
    HgAutomate::verbose(3, $str);

    # temporary work-around (XXXX, galt why is this "temporary?").
    my $assembly = $h->{assembly};
    my $type = $h->{type};
    my $files = $h->{files};
    my @files = split(/\s+/, $files);
    my %extendedTypes = map { $_ => 1 } @extendedTypes;
    if($type eq "genePred") {
        loadGene($assembly, $tablename, $files, $pushQ);
    } elsif ($type eq "wig") {
        loadWig($assembly, $tablename, $files, $pushQ);
    } elsif ($extendedTypes{$type}) {
        loadBedFromSchema($assembly, $tablename, $files, $type, $pushQ);
    } elsif ($type =~ /^bed (3|4|5|6)$/) {
        loadBed($assembly, $tablename, $files, $pushQ);
    } elsif ($type eq "fastq") {
        # XXXX anything else? Should we rename file(s) (or soft link) to get
        # consistent naming?
        push(@{$pushQ->{FILES}}, map("$submitDir/$_", @files));
    } else {
        die "ERROR: unknown type: $type in $Encode::loadFile\n";
    }
    print STDERR "\n" if($debug);
}

# Send "data is ready" email to email contact assigned to $pif{lab}

if($labs{$pif{lab}} && $labs{$pif{lab}}->{wranglerEmail} && !$opt_noEmail) {
    my $email = $labs{$pif{lab}}->{wranglerEmail};
    `echo "dir: $submitFQP" | /bin/mail -s "ENCODE data from $pif{lab} lab is ready" $email`;
}

open(PUSHQ, ">out/$Encode::pushQFile") || die "SYS ERROR: Can't write \'out/$Encode::pushQFile\' file; error: $!\n";
HgAutomate::verbose(2, "pushQ tables: " . join(",", @{$pushQ->{TABLES}}) . "\n");
HgAutomate::verbose(2, "pushQ files: " . join(",", @{$pushQ->{FILES}}) . "\n");
# figure out appropriate syntax for pushQ entries
print PUSHQ "tables: " . join(",", @{$pushQ->{TABLES}}) . "\n";
if(defined($pushQ->{FILES}) && @{$pushQ->{FILES}}) {
    print PUSHQ "files: " . join(",", @{$pushQ->{FILES}}) . "\n";
}
close(PUSHQ);

exit(0);

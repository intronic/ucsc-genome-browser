#!/usr/bin/env perl

# encodeLoad.pl - load ENCODE data submission generated by the
#                       automated submission pipeline
# Reads load.ra for information about what to do

# Writes error or log information to STDOUT
# Returns 0 if load succeeds and sends email to wrangler for lab specified in the DAF

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
use File::Basename;

use lib "/cluster/bin/scripts";
use Encode;
use RAFile;
use SafePipe;
use HgDb;

use vars qw/$opt_configDir $opt_noEmail $opt_outDir $opt_verbose/;

my $loadRa = "out/$Encode::loadFile";
my $unloadRa = "out/$Encode::unloadFile";
my $submitDir = "";
my $submitPath;			# full path of data submission directory
my $submitType = "";		# currently ignored
my $tempDir = "/data/tmp";
my $encInstance = "";

my $debug = 0;

sub usage
{
    die <<END
usage: doEncodeLoad.pl submission-type submission-dir

submission-type is currently ignored.

Requires file called: submission-dir/$loadRa

options:
    -verbose num        Set verbose level to num (default 1).
    -configDir dir      Path of configuration directory, containing
                        metadata .ra files (default: submission-dir/../config)
    -outDir dir         Path of output directory, for validation files
                        (default: submission-dir/out)
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
    } elsif (system("rm -f /gbdb/$assembly/wib/$tableName.wib") ||
             system("ln -s $submitPath/$tableName.wib /gbdb/$assembly/wib")) {
        die("ERROR: failed wiggle ln\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
    push(@{$pushQ->{FILES}}, "$submitPath/$tableName.wib");
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

    if(!(-e "$Encode::sqlCreate/${sqlTable}.sql")) {
        die "SQL schema '$Encode::sqlCreate/${sqlTable}.sql' does not exist\n";
    }

    my @cmds = ("cat $fileList", "egrep -v '^track|browser'", "hgLoadBed $assembly $tableName stdin -tmpDir=out -sqlTable=$Encode::sqlCreate/${sqlTable}.sql -renameSqlTable");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $debug);

    if(my $err = $safe->exec()) {
        die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}


############################################################################
# Main

my $wd = cwd();

GetOptions("configDir=s", "noEmail", "outDir=s", "verbose=i") || usage();
$opt_verbose = 1 if (!defined $opt_verbose);
$opt_noEmail = 0 if (!defined $opt_noEmail);
if($opt_outDir) {
    $loadRa = "$opt_outDir/$Encode::loadFile";
    $unloadRa = "$opt_outDir/$Encode::unloadFile";
}

# Change dir to submission directory obtained from command-line

if(@ARGV != 2) {
    usage();
}
$submitType = $ARGV[0];	# currently not used
$submitDir = $ARGV[1];
if ($submitDir =~ /^\//) {
    $submitPath = $submitDir;
} else {
    $submitPath = "$wd/$submitDir";
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

my $grants = Encode::getGrants($configPath);
my $fields = Encode::getFields($configPath);
my $daf = Encode::getDaf($submitDir, $grants, $fields);
my $db = HgDb->new(DB => $daf->{assembly});
my $email;
my %labels;

if($grants->{$daf->{grant}} && $grants->{$daf->{grant}}{wranglerEmail}) {
    $email = $grants->{$daf->{grant}}{wranglerEmail};
}

# Add a suffix for non-production loads (to avoid loading over existing tables).

my $tableSuffix = "";
if(dirname($submitDir) =~ /_(.*)/) {
    if($1 ne 'prod') {
        # yank out "beta" from encinstance_beta
        $tableSuffix = "_$1_" . basename($submitDir);;
    }
} else {
    $tableSuffix = "_" . basename($submitDir);;
}

chdir($submitDir);

# clean out any stuff from previous load
# We assume unload program is in the same location as loader (fixes problem with misconfigured qateam environment).

my $programDir = dirname($0);
my $unloader = "$programDir/doEncodeUnload.pl";
if(!(-e $unloader)) {
    # let's us use this in cvs tree
    $unloader = "$wd/doEncodeUnload.pl";
}
if(!(-e $unloader)) {
    die "Can't find unloader ($unloader)\n";
}
if(system("$unloader $submitType $submitDir")) {
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

my $compositeTrack = Encode::compositeTrackName($daf);
my $downloadDir = Encode::downloadDir($daf);
if(! -d $downloadDir) {
    die "download dir for 'compositeTrack' is not properly configured; please contact your wrangler at: $email\n";
}

for my $key (keys %ra) {
    my $h = $ra{$key};
    my $tablename = $h->{tablename} . $tableSuffix;
    $labels{$h->{pushQDescription}} = 1;

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
    my %extendedTypes = map { $_ => 1 } @Encode::extendedTypes;

    if ($h->{downloadOnly}) {
        # soft link file(s) into download dir
        my $target = "$downloadDir/$tablename.$type";
        unlink($target);
        if(@files == 1) {
            !system("/bin/ln -s $submitPath/$files[0] $target") || die "link failed: $?\n";
        } else {
            # have to make a concatenated copy of multiple files
            my $cmd = "cat " . join(" ", @files) . " > $tablename.$type";
            !system($cmd) || die "system '$cmd' failed: $?\n";
            !system("/bin/ln -s $submitPath/$tablename.$type $target") || die "link failed: $?\n";
        }
        push(@{$pushQ->{FILES}}, $target);
    } elsif($type eq "genePred") {
        loadGene($assembly, $tablename, $files, $pushQ);
    } elsif ($type eq "wig") {
        loadWig($assembly, $tablename, $files, $pushQ);
    } elsif ($extendedTypes{$type}) {
        loadBedFromSchema($assembly, $tablename, $files, $type, $pushQ);
    } elsif ($type =~ /^bed (3|4|5|6)$/) {
        loadBed($assembly, $tablename, $files, $pushQ);
    } else {
        die "ERROR: unknown type: $type in $Encode::loadFile\n";
    }
    print STDERR "\n" if($debug);
}

# Send "data is ready" email to email contact assigned to $daf{lab}

if($email) {
    if(!$opt_noEmail) {
        `echo "dir: $submitPath" | /bin/mail -s "ENCODE data from $daf->{grant}/$daf->{lab} lab is ready" $email`;
    }
} else {
    # XXXX Should this be fatal? Or s/d we send email to encode alias?
    # die "No wrangler is configured for '$daf->{grant}'\n";
}

my $wranglerName;
if($email && $email =~ /([^@]+)/) {
    $wranglerName = $1;
} else {
    $wranglerName = 'encode';
}

my $tables = join("\\n", @{$pushQ->{TABLES}});
my $files = "";
if(defined($pushQ->{FILES}) && @{$pushQ->{FILES}}) {
    $files = join("\\n", @{$pushQ->{FILES}});
}

my ($shortLabel, $longLabel);
my $sth = $db->execute("select shortLabel, longLabel from trackDb where tableName = ?", $compositeTrack);
my @row = $sth->fetchrow_array();
if(@row) {
    $shortLabel = $row[0];
    $longLabel = $row[1];
}

my $releaseLog = "$longLabel: " . join(", ", keys %labels);

HgAutomate::verbose(2, "pushQ tables: $tables\n");
HgAutomate::verbose(2, "pushQ files: $files\n");

open(PUSHQ, ">out/$Encode::pushQFile") || die "SYS ERROR: Can't write \'out/$Encode::pushQFile\' file; error: $!\n";
print PUSHQ <<_EOF_;
-- New entry in Main Push Queue, to alert QA to existence of new tables:
LOCK TABLES pushQ WRITE;
SELECT \@rank := max(rank)+1 FROM pushQ WHERE priority = 'A';
SELECT \@qid := right(concat("00000",convert(max(qid)+1,CHAR)),6) FROM pushQ;
INSERT INTO pushQ VALUES (\@qid,'','A',\@rank,now(),'Y','$shortLabel','$daf->{assembly}','$tables','','$files',0,'hgwdev','N','','N','N','','$wranglerName','','$daf->{grant}/$daf->{lab}','','','N',now(),'',0,'','','$releaseLog','');
UNLOCK TABLES;
_EOF_
close(PUSHQ);

exit(0);

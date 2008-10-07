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

use vars qw/$opt_configDir $opt_noEmail $opt_outDir $opt_verbose $opt_debug/;

my $loadRa = "out/$Encode::loadFile";
my $unloadRa = "out/$Encode::unloadFile";
my $trackDb = "out/trackDb.ra";
my $submitDir = "";
my $submitPath;			# full path of data submission directory
my $submitType = "";		# currently ignored
my $tempDir = "/data/tmp";
my $encInstance = "";

my $PROG = basename $0;

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

    HgAutomate::verbose(2, "loadGene ($assembly, $tableName, $fileList, $pushQ)\n");
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

    HgAutomate::verbose(2, "loadWig ($assembly, $tableName, $fileList, $pushQ)\n");
    my $catCmd = $opt_debug ? "head -1000 $fileList" : "cat $fileList"; # load 1000 records if $opt_debug
    my @cmds = ($catCmd, "wigEncode stdin stdout $tableName.wib", "hgLoadWiggle -pathPrefix=/gbdb/$assembly/wib -tmpDir=$tempDir $assembly $tableName stdin");
    HgAutomate::verbose(2, "loadWig cmds [".join(" ; ",@cmds)."]\n");
    my $stderrFile = "out/$tableName.err";
    unlink($stderrFile);
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", STDERR => $stderrFile, DEBUG => $opt_debug);
    if(my $err = $safe->exec()) {
        print STDERR "ERROR: File(s) $fileList failed wiggle load:\n\n";
        dieFile($stderrFile);
    } elsif (system("rm -f /gbdb/$assembly/wib/$tableName.wib") ||
             system("ln -s $submitPath/$tableName.wib /gbdb/$assembly/wib")) {
        die("ERROR: failed wiggle load ln\n");
    } else {
        print "$fileList loaded into $tableName\n";

        # retrieve lower and upper limits from $stderrFile and fix-up trackDb.ra file to add this to wig type line
        my $lines = Encode::readFile($stderrFile);
        for my $line (@{$lines}) {
            if($line =~ /Converted stdin, upper limit (.+), lower limit (.+)/) {
                my $max = $1;
                my $min = $2;
                my $placeHolder = Encode::wigMinMaxPlaceHolder($tableName);
                my $cmd = "/usr/local/bin/perl -i -ne 's/$placeHolder/$min $max/; print;' $trackDb";
                HgAutomate::verbose(2, "updating $trackDb to set min/max for $tableName to $min,$max\ncmd: $cmd\n");
                !system($cmd) || die "trackDb.ra update failed: $?\n";
            }
        }
    }
    push(@{$pushQ->{TABLES}}, $tableName);
    push(@{$pushQ->{FILES}}, "$submitPath/$tableName.wib");
}

sub loadBed
{
    my ($assembly, $tableName, $fileList, $pushQ) = @_;
    HgAutomate::verbose(2, "loadBed ($assembly, $tableName, $fileList, $pushQ)\n");
    my $catCmd = $opt_debug ? "head -1000 $fileList" : "cat $fileList"; # load 1000 records if $opt_debug
    my @cmds = ($catCmd, "egrep -v '^track|browser'", "hgLoadBed $assembly $tableName stdin -tmpDir=$tempDir");
    HgAutomate::verbose(2, "loadBed cmds [".join(" ; ",@cmds)."]\n");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_debug);
    if(my $err = $safe->exec()) {
        die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadBedGraph
{
    my ($assembly, $tableName, $fileList, $pushQ) = @_;
    HgAutomate::verbose(2, "loadBedGraph ($assembly, $tableName, $fileList, $pushQ)\n");
    my $catCmd = Encode::isZipped($fileList) ? "/bin/zcat $fileList" : "/bin/cat $fileList";
    my @cmds = ($catCmd, "egrep -v '^track|browser'", "hgLoadBed $assembly $tableName -bedGraph=4 stdin -tmpDir=$tempDir");
    HgAutomate::verbose(2, "loadBedGraph cmds [".join(" ; ",@cmds)."]\n");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_debug);
    if(my $err = $safe->exec()) {
        die("ERROR: File(s) '$fileList' failed bedGraph load:\n" . $safe->stderr() . "\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadBedFromSchema
{
# Load bed using a .sql file
    my ($assembly, $tableName, $fileList, $sqlTable, $pushQ) = @_;
    HgAutomate::verbose(2, "loadBedFromSchema ($assembly, $tableName, $fileList, $sqlTable, $pushQ)\n");

    if(!(-e "$Encode::sqlCreate/${sqlTable}.sql")) {
        die "SQL schema '$Encode::sqlCreate/${sqlTable}.sql' does not exist\n";
    }

    my $fillInArg = "";
    if($sqlTable =~ /peak/i) {
        # fill in zero score columns for narrowPeaks etc.
        $fillInArg = "-fillInScore=signalValue ";
    }

    my $catCmd = $opt_debug ? "head -1000 $fileList" : "cat $fileList"; # load 1000 records if $opt_debug
    my @cmds = ($catCmd, "egrep -v '^track|browser'", "hgLoadBed $assembly $tableName stdin -tmpDir=$tempDir -sqlTable=$Encode::sqlCreate/${sqlTable}.sql -renameSqlTable $fillInArg");
    HgAutomate::verbose(2, "loadBedFromSchema cmds [".join(" ; ",@cmds)."]\n");
    my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);

    if(my $err = $safe->exec()) {
        die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
    } else {
        print "$fileList loaded into $tableName\n";
    }
    # XXXX special case narrowPeak/broadPeak/gappedPeak to convert "." => "" in name column?
    push(@{$pushQ->{TABLES}}, $tableName);
}


############################################################################
# Main

my $wd = cwd();

GetOptions("configDir=s", "noEmail", "outDir=s", "verbose=i", "debug") || usage();
$opt_verbose = 1 if (!defined $opt_verbose);
$opt_noEmail = 0 if (!defined $opt_noEmail);
$opt_debug = 0 if (!defined $opt_debug);
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
if(system("$unloader $submitType $submitPath")) {
    die "unload script failed\n";
}

if(!(-e $loadRa)) {
    die "ERROR: load.ra not found ($PROG)\n";
}

#TODO change to : FileUtils.cp $loadRa, $unloadRa
# XXXX shouldn't we do the cp AFTER we finish everything else successfully?
if(system("cp $loadRa $unloadRa")) {
    die "Cannot: cp $loadRa $unloadRa\n";
}

HgAutomate::verbose(1, "Loading project in directory $submitDir\n");

# Load files listed in load.ra

my %ra = RAFile::readRaFile($loadRa, 'tablename');
my $pushQ = {};
$pushQ->{TABLES} = [];

HgAutomate::verbose(2, "loadRa ($loadRa) has: " . scalar(keys %ra) . " records\n");

my $compositeTrack = Encode::compositeTrackName($daf);
my $downloadDir = Encode::downloadDir($daf);
if(!(-d $downloadDir)) {
    mkdir $downloadDir || die ("Can't create download directory (error: '$!'); please contact your wrangler at: $email\n");
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
    HgAutomate::verbose(3, "key=[$key] tablename=[$tablename] str=[$str]\n");
    $str .= "\n";

    # temporary work-around (XXXX, galt why is this "temporary?").
    my $assembly = $h->{assembly};
    my $type = $h->{type};
    my $files = $h->{files};
    my $downloadOnly = (defined($h->{downloadOnly}) and $h->{downloadOnly});
    my @files = split(/\s+/, $files);
    my %extendedTypes = map { $_ => 1 } @Encode::extendedTypes;
    my $hgdownload = 0;

    HgAutomate::verbose(2, "TYPE=[$type] extendedTypes=[".(defined($extendedTypes{$type}) ? $extendedTypes{$type} : "")."] key=[$key] tablename=[$tablename] downloadOnly=[$downloadOnly]\n");
    if ($downloadOnly) {
        # XXXX convert solexa/illumina => sanger fastq when appropriate
        HgAutomate::verbose(3, "Download only; dont load [$key].\n");
        $hgdownload = 1;
    } elsif($type eq "genePred") {
        loadGene($assembly, $tablename, $files, $pushQ);
    } elsif ($type eq "wig") {
        # Copy signal data to hgdownload (unless we created it).
        if(@files == 1) {
            $hgdownload = $files[0] !~ /^$Encode::autoCreatedPrefix/;
        } else {
            $hgdownload = 1;
        }
        loadWig($assembly, $tablename, $files, $pushQ);
    } elsif ($extendedTypes{$type}) {
        loadBedFromSchema($assembly, $tablename, $files, $type, $pushQ);
    } elsif ($type =~ /^bed (3|4|5|6)$/) {
        loadBed($assembly, $tablename, $files, $pushQ);
    } elsif ($type =~ /^bedGraph (4)$/) {
        loadBedGraph($assembly, $tablename, $files, $pushQ);
    } else {
        die "ERROR: unknown type: $type in $Encode::loadFile ($PROG)\n";
    }
    HgAutomate::verbose(2, "Done loading. Now making links and copies. hgdownload=[$hgdownload]\n");
    if($hgdownload) {
        # soft link file(s) into download dir - gzip files as appropriate
        my $target = "$downloadDir/$tablename.$type.gz";
        unlink($target);
        HgAutomate::verbose(2, "unlink($target)\n");

        if(@files == 1) {
            my $srcFile = "$submitPath/$files[0]";
            HgAutomate::verbose(2, "One file: srcFile=[$srcFile]\n");
            if(Encode::isZipped($srcFile)) {
                HgAutomate::verbose(2, "soft-linking $srcFile => $target\n");
                !system("/bin/ln -s $srcFile $target") || die "link failed: $?\n";
            } else {
                HgAutomate::verbose(2, "copying/zipping $srcFile => $target\n");
                !system("/bin/gzip -c $srcFile > $target") || die "gzip: $?\n";
            }
        } else {
            # make a concatenated copy of multiple files
            my $unZippedTarget = "$downloadDir/$tablename.$type";
            unlink($unZippedTarget);
            HgAutomate::verbose(2, "Zero or multiple files: files=[@files] unlink($unZippedTarget)\n");
            for my $file (@files) {
                $file = "$submitPath/$file";
                my $cmd;
                if(Encode::isZipped($file)) {
                    $cmd = "/bin/zcat $file >> $unZippedTarget";
                } else {
                    $cmd = "/bin/cat $file >> $unZippedTarget";
                }
                HgAutomate::verbose(2, "copying $file to $target\n");
                !system($cmd) || die "system '$cmd' failed: $?\n";
            }
            !system("/bin/gzip $unZippedTarget") || die "gzip failed: $?\n";
        }
        push(@{$pushQ->{FILES}}, $target);
    }
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
push(@{$pushQ->{FILES}}, "/usr/local/apache/cgi-bin/encode/cv.ra");
my $files = join("\\n", @{$pushQ->{FILES}});
my $readmeFile = "$downloadDir/README.txt";
push(@{$pushQ->{FILES}}, "$downloadDir/README.txt");

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

if(-e $readmeFile) {
    open(README, ">>$readmeFile") || die "SYS ERROR: Can't write '$readmeFile' file; error: $!\n";
    print README "\n";
} else {
    open(README, ">$readmeFile") || die "SYS ERROR: Can't write '$readmeFile' file; error: $!\n";
    print README <<END;
This directory contains data generated by the $daf->{grant}/$daf->{lab} lab
as part of the Encode project.

Data is restricted from use in publication until the restriction date
noted for the given data file. See the ENCODE
data release policy at:

http://genome.ucsc.edu/encodeDCC/ENCODE_Consortia_Data_Release_Policy_2008-09-11.doc

Files included in this directory:

END
}

my $lines = Encode::readFile("out/README.txt");
print README join("\n", @{$lines});
print README "\n";
close(README);

exit(0);

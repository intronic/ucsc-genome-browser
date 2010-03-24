#!/usr/bin/env perl

# encodeLoad.pl - load ENCODE data submission generated by the automated
# submission pipeline Reads load.ra for information about what to do

# Writes error or log information to STDOUT Returns 0 if load succeeds and
# sends email to wrangler for lab specified in the DAF

# DO NOT EDIT the /cluster/bin/scripts copy of this file --
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeLoad/doEncodeLoad.pl,v 1.74 2010/03/24 18:08:00 krish Exp $

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

use vars qw/$opt_configDir $opt_noEmail $opt_outDir $opt_verbose $opt_debug $opt_skipLoad $opt_skipDownload/;

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
    -skipLoad           Skip table loading (useful if you just want to generate other
                        side-effects, like re-populating the download directory)
    -skipDownload       Skip setting up download files (useful if you want to skip
                        the data loading and table loading and just do the final
                        PushQ entries etc.
                        (implies -skipLoad so that loaded data is not unloaded)
END
}

sub dieFile
{
# read contents of $file and print it in a die message.
    my ($file) = @_;
    open(FILE, $file);
    die join("", <FILE>) . "\n";
}

sub pingMysql
{
    my $db = shift;
    HgAutomate::verbose(2, "pinging mysql (".scalar(localtime()).")\n");
    $db->execute("select 1");
}

sub makeCatCmd
# return the cat command, checking whether the files in the list are all zipped or not
{
    my $cmdName = shift;
    my $fileList = shift;
    my @files = split(/\s+/, $fileList);
    my $catCmd;
    if ($opt_debug) {
        $catCmd = "head -1000 $fileList"; # load 1000 records if $opt_debug
    } else {
        my $numZipped = 0;
        for my $f (@files) {
            ++$numZipped if $f =~ /.gz$/;
        }
        if ($numZipped > 0) { # check how many zipped files are in the list
            if (scalar(@files) != $numZipped) {
                die("ERROR: zipped and unzipped files in [$cmdName] file list\n");
            }
            else {
                $catCmd = "zcat $fileList";
            }
        } else {
            $catCmd = "cat $fileList"; # cat possibly a list of files
        }
    }
    return $catCmd;
}

sub loadGene
{
    my ($assembly, $tableName, $fileList, $pushQ, $ldHgGeneFlags) = @_;

    HgAutomate::verbose(2, "loadGene ($assembly, $tableName, $fileList, $pushQ, $ldHgGeneFlags)\n");
    if(!$opt_skipLoad) {
        my $stdoutFile = "out/loadGene.out";
        my $stderrFile = "out/loadGene.err";

        my @cmds = ("cat $fileList", "egrep -v '^track|browser'", "/cluster/bin/x86_64/ldHgGene $ldHgGeneFlags $assembly $tableName stdin ");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => $stdoutFile, STDERR => $stderrFile, DEBUG => $opt_debug);
        if(my $err = $safe->exec()) {
            print STDERR "ERROR: File(s) '$fileList' ($ldHgGeneFlags) failed gene load.\n";
            dieFile($stderrFile);
        } else {
            print "$fileList loaded into $tableName\n";
            # debug restore: File.delete "genePred.tab";
        }
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadWig
{
    my ($assembly, $tableName, $gbdbDir, $fileList, $pushQ) = @_;

    HgAutomate::verbose(2, "loadWig ($assembly, $tableName, $gbdbDir, $fileList, $pushQ)\n");
    if(!$opt_skipLoad) {
        my $catCmd = makeCatCmd("loadWig", $fileList);
        my @cmds = ($catCmd, "/cluster/bin/x86_64/wigEncode -noOverlapSpanData stdin stdout $tableName.wib", "/cluster/bin/x86_64/hgLoadWiggle -pathPrefix=/gbdb/$assembly/wib -tmpDir=$tempDir $assembly $tableName stdin");
        HgAutomate::verbose(2, "loadWig cmds [".join(" ; ",@cmds)."]\n");
        my $stderrFile = "out/$tableName.err";
        unlink($stderrFile);
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", STDERR => $stderrFile, DEBUG => $opt_debug);
        if(my $err = $safe->exec()) {
            print STDERR "ERROR: File(s) $fileList failed wiggle load:\n\n";
            dieFile($stderrFile);
        }

        if (system("rm -f /gbdb/$assembly/wib/$tableName.wib") ||
                 system("ln -s $gbdbDir/$tableName.wib /gbdb/$assembly/wib")) {
            die("ERROR: failed wiggle load ln\n");
        }

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
    if(!$opt_skipLoad) {
        my $catCmd = makeCatCmd("loadBed", $fileList);
        my @cmds = ($catCmd, "egrep -v '^track|browser'", "/cluster/bin/x86_64/hgLoadBed -noNameIx $assembly $tableName stdin -tmpDir=$tempDir");
        HgAutomate::verbose(2, "loadBed cmds [".join(" ; ",@cmds)."]\n");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_debug);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList loaded into $tableName\n";
        }
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadBedGraph
{
    my ($assembly, $tableName, $fileList, $pushQ) = @_;
    HgAutomate::verbose(2, "loadBedGraph ($assembly, $tableName, $fileList, $pushQ)\n");
    if(!$opt_skipLoad) {
        my $catCmd = makeCatCmd("loadBedGraph", $fileList);
        my @cmds = ($catCmd, "egrep -v '^track|browser'", "/cluster/bin/x86_64/hgLoadBed -noNameIx $assembly $tableName -bedGraph=4 stdin -tmpDir=$tempDir");
        HgAutomate::verbose(2, "loadBedGraph cmds [".join(" ; ",@cmds)."]\n");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_debug);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bedGraph load:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList loaded into $tableName\n";
        }
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

sub loadBedFromSchema
{
# Load bed using a .sql file
    my ($assembly, $tableName, $fileList, $sqlTable, $pushQ) = @_;
    HgAutomate::verbose(2, "loadBedFromSchema ($assembly, $tableName, $fileList, $sqlTable, $pushQ)\n");

    if(!$opt_skipLoad) {
        if(!(-e "$Encode::sqlCreate/${sqlTable}.sql")) {
            die "SQL schema '$Encode::sqlCreate/${sqlTable}.sql' does not exist\n";
        }

        my $fillInArg = "";
        if($sqlTable =~ /peak/i) {
            # fill in zero score columns for narrowPeaks etc.
            $fillInArg = "-fillInScore=signalValue ";
	    # change minScore for Stam Lab
	    $fillInArg .= "-minScore=500 " if $tableName =~ /^wgEncodeUwDnaseSeq/;
        }
        my $catCmd = makeCatCmd("loadBedFromSchema", $fileList);
        my @cmds = ($catCmd, "egrep -v '^track|browser'", "/cluster/bin/x86_64/hgLoadBed -noNameIx $assembly $tableName stdin -tmpDir=$tempDir -sqlTable=$Encode::sqlCreate/${sqlTable}.sql -renameSqlTable $fillInArg");
        HgAutomate::verbose(2, "loadBedFromSchema cmds [".join(" ; ",@cmds)."]\n");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);

        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList loaded into $tableName\n";
        }
    }
    # XXXX special case narrowPeak/broadPeak/gappedPeak to convert "." => "" in name column?
    push(@{$pushQ->{TABLES}}, $tableName);
}


sub loadBigBed
{
# Load bigBed using a .as file
    my ($assembly, $tableName, $gbdbDir, $fileList, $sqlTable, $pushQ, $configPath) = @_;
    HgAutomate::verbose(2, "loadBigBed ($assembly, $tableName, $gbdbDir, $fileList, $sqlTable, $pushQ)\n");

    if(!$opt_skipLoad) {
        if(!(-e "$Encode::sqlCreate/${sqlTable}.as")) {
            die "AutoSql schema '$Encode::sqlCreate/${sqlTable}.as' does not exist\n";
        }
        if ((() = split(" ", $fileList)) != 1) { 
	    die "BigBed must be loaded with a single file but a list of files was supplied ($fileList)\n";
	}
	# Create bigBed binary file
        my @cmds = ( "/cluster/bin/x86_64/bedToBigBed -as=$Encode::sqlCreate/${sqlTable}.as $fileList $configPath/${assembly}_chromInfo.txt ${gbdbDir}/${tableName}.bb");
        HgAutomate::verbose(2, "loadBigBed cmds [".join(" ; ",@cmds)."]\n");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bedToBigBed:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList created as bigBed ${gbdbDir}/${tableName}.bb\n";
        }
	# symlink bigBed binary file into gbdb bbi directory
        @cmds = ( "ln -sf ${gbdbDir}/${tableName}.bb /gbdb/${assembly}/bbi/");
        HgAutomate::verbose(2, "loadBigBed cmds [".join(" ; ",@cmds)."]\n");
        $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed symbolic link to gbdb bigBed directory:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList loaded into $tableName\n";
        }
	# create BigBed link table from trackDb to gbdb bigBed binary file
        @cmds = ( "/cluster/bin/x86_64/hgBbiDbLink $assembly $tableName /gbdb/${assembly}/bbi/${tableName}.bb");
        HgAutomate::verbose(2, "loadBigBed cmds [".join(" ; ",@cmds)."]\n");
        $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bed load:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList loaded into $tableName\n";
        }
    }
    push(@{$pushQ->{TABLES}}, $tableName);
}

############################################################################
# Main

my $wd = cwd();

GetOptions("configDir=s", "noEmail", "outDir=s", "verbose=i", "debug", "skipLoad", "skipDownload") || usage();
$opt_verbose = 1 if (!defined $opt_verbose);
$opt_noEmail = 0 if (!defined $opt_noEmail);
$opt_debug = 0 if (!defined $opt_debug);
$opt_skipLoad = 1 if $opt_skipDownload; # dont unload tables if not doing downloads
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
HgAutomate::verbose(1, "Using config path $configPath\n");

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
if ($submitDir eq ".") { # make sure command-line use specifies full path and directory
    die "ERROR: please specify full path to directory\n";
}
if(dirname($submitDir) =~ /_(.*)/) {
    if($1 ne 'prod') {
        # yank out "beta" from encinstance_beta
        $tableSuffix = "_$1_" . basename($submitDir);;
    }
} else {
    $tableSuffix = "_" . basename($submitDir);;
}

chdir($submitDir);
my $programDir = dirname($0);

if(!(-e $loadRa)) {
    die "ERROR: load.ra not found ($PROG)\n";
}

if(!$opt_skipLoad) {
    # clean out any stuff from previous load
    # We assume unload program is in the same location as loader (fixes problem with misconfigured qateam environment).

    my $unloader = "$programDir/doEncodeUnload.pl";
    if(!(-e $unloader)) {
        # let's us use this in cvs tree
        $unloader = "$wd/doEncodeUnload.pl";
    }
    if(!(-e $unloader)) {
        die "Can't find unloader ($unloader)\n";
    }
    my $unloader_cmd;
    if (defined $opt_configDir) {
        $unloader_cmd = "$unloader -configDir $opt_configDir $submitType $submitPath";
    } else {
        $unloader_cmd = "$unloader $submitType $submitPath";
    }
    if(system("$unloader_cmd")) {
        die "unload script failed\n";
    }

    #TODO change to : FileUtils.cp $loadRa, $unloadRa
    # XXXX shouldn't we do the cp AFTER we finish everything else successfully?
    if(system("cp $loadRa $unloadRa")) {
        die "Cannot: cp $loadRa $unloadRa\n";
    }
}

HgAutomate::verbose(1, "Loading project in directory $submitDir\n");

# Load files listed in load.ra

my %ra = RAFile::readRaFile($loadRa, 'tablename');

my $pushQ = {};
# $pushQ contains the data required to create the pushQ entry:
# $pushQ->{TABLES} is list of tables generated by loader
# $pushQ->{FILES} is list of files generated by loader
$pushQ->{TABLES} = [];

HgAutomate::verbose(2, "loadRa ($loadRa) has: " . scalar(keys %ra) . " records\n");

my $compositeTrack = Encode::compositeTrackName($daf);
my $downloadDir = Encode::downloadDir($daf);
my $gbdbDir = Encode::gbdbDir($daf);

# make the download dir if it doesn't exist
if(!$opt_skipDownload and !(-d $downloadDir)) {
    if(!(-d $downloadDir)) {
        mkdir $downloadDir || die ("Can't create download directory (error: '$!'); please contact your wrangler at: $email\n");
    }
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
    pingMysql($db);
    HgAutomate::verbose(3, "key=[$key] tablename=[$tablename] str=[$str]\n");
    $str .= "\n";

    # temporary work-around (XXXX, galt why is this "temporary?").
    my $assembly = $h->{assembly};
    my $type = $h->{type};
    my $sql = $h->{sql};
    my $files = $h->{files};
    my $downloadOnly = (defined($h->{downloadOnly}) and $h->{downloadOnly});  # Could also gzip and link files for displayed tracks!
    my @files = split(/\s+/, $files);
    my %extendedTypes = map { $_ => 1 } @Encode::extendedTypes;
    my %bigBedTypes = map { $_ => 1 } @Encode::bigBedTypes;
    my $hgdownload = 0;

    HgAutomate::verbose(2, "TYPE=[$type] extendedTypes=[".(defined($extendedTypes{$type}) ? $extendedTypes{$type} : "")."] key=[$key] tablename=[$tablename] downloadOnly=[$downloadOnly]\n");
    HgAutomate::verbose(2, "TYPE=[$type] bigBedTypes=[".(defined($bigBedTypes{$type}) ? $bigBedTypes{$type} : "")."] key=[$key] tablename=[$tablename] downloadOnly=[$downloadOnly]\n");
    if ($downloadOnly) {
        # XXXX convert solexa/illumina => sanger fastq when appropriate
        HgAutomate::verbose(3, "Download only; dont load [$key].\n");
        $hgdownload = 1;
    } elsif($type eq "gtf") {
        loadGene($assembly, $tablename, $files, $pushQ, "-gtf -genePredExt");
        $hgdownload = @files;
    } elsif($type eq "genePred" ) {
        loadGene($assembly, $tablename, $files, $pushQ, "-genePredExt");
    } elsif ($type eq "wig") {
        # Copy signal data to hgdownload (unless we created it).
        # NOTE: We are now making the RawSignals but they are put in the subdirectory "raw"
        #if(@files == 1) {
        #    $hgdownload = $files[0] !~ /^$Encode::autoCreatedPrefix/;
        #} else {
            $hgdownload = 1;
        #}
        loadWig($assembly, $tablename, $gbdbDir, $files, $pushQ);
    } elsif ($extendedTypes{$type}) {
        loadBedFromSchema($assembly, $tablename, $files, $type, $pushQ);
        $hgdownload = @files;
    } elsif ($type =~ /^bed/ and defined($sql)) {
        loadBedFromSchema($assembly, $tablename, $files, $sql, $pushQ);
        $hgdownload = @files;
    } elsif ($bigBedTypes{$type}) {
        loadBigBed($assembly, $tablename, $gbdbDir, $files, $type, $pushQ, $configPath);
        $hgdownload = @files;
    } elsif ($type =~ /^bed (3|4|5|6|8|9|12)$/) {
        loadBed($assembly, $tablename, $files, $pushQ);
        $hgdownload = @files;
    } elsif ($type =~ /^bedGraph (4)$/) {
        loadBedGraph($assembly, $tablename, $files, $pushQ);
        $hgdownload = @files;
    } else {
        die "ERROR: unknown type: $type in $Encode::loadFile ($PROG)\n";
    }
    HgAutomate::verbose(2, "Done loading. Now making links and copies. hgdownload=[$hgdownload]\n");
    if(!$opt_skipDownload and $hgdownload) {
        # soft link file(s) into download dir - gzip files as appropriate
        my $target = "$downloadDir/$tablename.$type.gz";
        # NOTE: We are now making the RawSignals but they are put in the subdirectory "raw"
        if(@files == 1 && $files[0] =~ /^$Encode::autoCreatedPrefix/) {
            $target = "$downloadDir/raw/$tablename.$type.gz";
	    if (! -d "$downloadDir/raw") {
		mkdir "$downloadDir/raw" or die "Could not create dir [$downloadDir/raw] error: [$!]\n";
		}
        }
        $target =~ s/ //g;  # removes space in ".bed 5.gz" for example
        unlink($target);
        HgAutomate::verbose(2, "unlink($target)\n");

        if(@files == 1) {
            my $srcFile = "$submitPath/$files[0]";
            HgAutomate::verbose(2, "One file: srcFile=[$srcFile]\n");
            if(Encode::isZipped($srcFile)) {
                HgAutomate::verbose(2, "soft-linking $srcFile => $target\n");
                !system("/bin/ln $srcFile $target") || die "link failed: $?\n";
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
        # XXXX add to FILES list and then copy files to unloadFiles.txt
    }
}

if(!$opt_skipDownload and !$opt_skipLoad) {
    # Send "data is ready" email to email contact assigned to $daf{lab}
    if($email) {
        if(!$opt_noEmail) {
            `echo "dir: $submitPath" | /bin/mail -s "ENCODE data from $daf->{grant}/$daf->{lab} lab is ready" $email`;
        }
    } else {
        # XXXX Should this be fatal? Or s/d we send email to encode alias?
        # die "No wrangler is configured for '$daf->{grant}'\n";
    }
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
my $preambleFile = "$downloadDir/preamble.html";
push(@{$pushQ->{FILES}}, "$downloadDir/index.html");
push(@{$pushQ->{FILES}}, "$downloadDir/preamble.html");

my ($shortLabel, $longLabel);
my $sth = $db->execute("select shortLabel, longLabel from trackDb where tableName = ?", $compositeTrack);
my @row = $sth->fetchrow_array();
if(@row) {
    $shortLabel = $row[0];
    $longLabel = $row[1];
}
$shortLabel = "ENCODE shortLabel" if (!defined($shortLabel));
$longLabel = "ENCODE longLabel Missing" if (!defined($longLabel));

my $releaseLog = "$longLabel: " . join(", ", keys %labels);
my $releaseLogUrl = "../../cgi-bin/hgTrackUi?db=$daf->{assembly}&g=$compositeTrack";

HgAutomate::verbose(2, "pushQ tables: $tables\n");
HgAutomate::verbose(2, "pushQ files: $files\n");

open(PUSHQ, ">out/$Encode::pushQFile") || die "SYS ERROR: Can't write \'out/$Encode::pushQFile\' file; error: $!\n";
print PUSHQ <<_EOF_;
-- New entry in Main Push Queue, to alert QA to existence of new tables:
LOCK TABLES pushQ WRITE;
SELECT \@rank := max(rank)+1 FROM pushQ WHERE priority = 'A';
SELECT \@qid := right(concat("00000",convert(max(qid)+1,CHAR)),6) FROM pushQ;
INSERT INTO pushQ VALUES (\@qid,'','B',\@rank,now(),'Y','ENCODE $shortLabel','$daf->{assembly}','$tables','','$files',0,'hgwdev','N','','N','N','','$wranglerName','','$daf->{grant}/$daf->{lab}','','','N',now(),'',0,'','','$releaseLog','','$releaseLogUrl');
UNLOCK TABLES;
_EOF_
close(PUSHQ);

if(! -e $preambleFile ) {
    open(PREAMBLE, ">$preambleFile") || die "SYS ERROR: Can't write '$preambleFile' file; error: $!\n";

    # NOTE: can remove the CHANGE-ME comment if we switch to SSI for the hostname
    print PREAMBLE <<END;
<p>This directory contains data generated by the $daf->{grant}/$daf->{lab} lab as part of the ENCODE project.
Further information is available in the
<!-- CHANGE-ME to genome.ucsc.edu at releaase time -->
<A TARGET=_BLANK HREF="http://genome-test.cse.ucsc.edu/cgi-bin/hgTrackUi?db=$daf->{assembly}&g=$compositeTrack">
<em>$daf->{lab}  $daf->{dataType}</em></A> track description. </p>

<p><B>Data is <A HREF="http://genome.ucsc.edu/ENCODE/terms.html">RESTRICTED FROM USE</a>
in publication  until the restriction date noted for the given data file.</B></p>
END
}

exit(0);

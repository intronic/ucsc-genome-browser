#!/usr/bin/env perl

# encodeLoad.pl - load ENCODE data submission generated by the automated
# submission pipeline Reads load.ra for information about what to do

# Writes error or log information to STDOUT Returns 0 if load succeeds and
# sends email to wrangler for lab specified in the DAF

# DO NOT EDIT the /cluster/bin/scripts copy of this file --
# edit the CVS'ed source at:
# $Header: /projects/compbiousr/cvsroot/kent/src/hg/encode/encodeLoad/doEncodeLoad.pl,v 1.76 2010/04/20 22:04:21 braney Exp $

# Usage:
#
# ./doEncodeLoad.pl unused /cluster/data/encode/pipeline/encpipeline_kate/83

use warnings;
use strict;

use Getopt::Long;
use Cwd;
use File::Basename;

use FindBin qw($Bin);
use lib "$Bin";
use Encode;
use HgAutomate;
use HgDb;
use RAFile;
use SafePipe;

use vars qw/$opt_configDir $opt_noEmail $opt_outDir $opt_verbose $opt_debug $opt_skipLoad $opt_skipDownload/;

my $loadRa = "out/$Encode::loadFile";
my $unloadRa = "out/$Encode::unloadFile";
my $trackDb = "out/trackDb.ra";
my $submitDir = "";
my $submitPath;			# full path of data submission directory
my $pipelineInstance = "";		# currently ignored
my $tempDir = "/data/tmp";
my $encInstance = "";

my $PROG = basename $0;

sub usage
{
    die <<END
usage: doEncodeLoad.pl pipeline-instance submission-dir

The pipeline instance variable is a switch that changes the behavior of doEncodeLoad.
The changes if the instance is:

standard
    allows use of hg19 and mm9 databases only

anything else
    allows use of the encodeTest database only

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
	my $dotCouldBeNull = "";
	if ($sqlTable =~ /bedRnaElements/) {
	    $dotCouldBeNull = "-dotIsNull=7";
	}
        my @cmds = ($catCmd, "egrep -v '^track|browser'", "/cluster/bin/x86_64/hgLoadBed $dotCouldBeNull -noNameIx $assembly $tableName stdin -tmpDir=$tempDir -sqlTable=$Encode::sqlCreate/${sqlTable}.sql -renameSqlTable $fillInArg");
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

sub loadBam
{
# Load bam
    my ($assembly, $tableName, $downloadDir,  $fileList, $sqlTable, $pushQ, $configPath) = @_;
    HgAutomate::verbose(2, "loadBam ($assembly, $tableName,  $fileList, $sqlTable, $pushQ)\n");

    if(!$opt_skipLoad) {
#        if(!(-e "$Encode::sqlCreate/${sqlTable}.as")) {
#            die "AutoSql schema '$Encode::sqlCreate/${sqlTable}.as' does not exist\n";
#        }
        if ((() = split(" ", $fileList)) != 1) {
	    die "Bam must be loaded with a single file but a list of files was supplied ($fileList)\n";
	}
        my $baiFile = ${fileList} . ".bai";
#        my @cmds = ( "mkdir -p ${gbdbDir}; ln  $fileList ${gbdbDir}/${tableName}.bam; ln  ${baiFile} ${gbdbDir}/${tableName}.bam.bai");
#        HgAutomate::verbose(2, "loadBam cmds [".join(" ; ",@cmds)."]\n");
#        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
#        if(my $err = $safe->exec()) {
#            die("ERROR: bad link of '$fileList' to ${gbdbDir}\n" . $safe->stderr() . "\n");
#        } else {
#            HgAutomate::verbose(2, "$fileList linked to ${gbdbDir}/${tableName}.bam\n");
#        }
	# symlink bam binary file into gbdb bbi directory
        my @cmds = ( "ln -sf ${downloadDir}/${tableName}.bam /gbdb/${assembly}/bbi/; ln -sf ${downloadDir}/${tableName}.bam.bai /gbdb/${assembly}/bbi/");
        HgAutomate::verbose(2, "loadBam cmds [".join(" ; ",@cmds)."]\n");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed symbolic link to /gbdb/${assembly}/bbi/\n" . $safe->stderr() . "\n");
        } else {
            HgAutomate::verbose(2, "${downloadDir}/${tableName}.bam linked to  /gbdb/${assembly}/bbi/\n");
        }
	# create Bam link table from trackDb to gbdb bam binary file
        @cmds = ( "/cluster/bin/x86_64/hgBbiDbLink $assembly $tableName /gbdb/${assembly}/bbi/${tableName}.bam");
        HgAutomate::verbose(2, "loadBam cmds [".join(" ; ",@cmds)."]\n");
        $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bam load:\n" . $safe->stderr() . "\n");
        } else {
            print "$fileList loaded into $tableName\n";
        }
    }
    push(@{$pushQ->{TABLES}}, $tableName);

}

sub loadBigWig
{
# Load bigWig
    my ($assembly, $tableName, $downloadDir, $fileList, $sqlTable, $pushQ, $configPath) = @_;
    HgAutomate::verbose(2, "loadBigWig ($assembly, $tableName, $downloadDir, $fileList, $sqlTable, $pushQ)\n");

    if(!$opt_skipLoad) {
#        if(!(-e "$Encode::sqlCreate/${sqlTable}.as")) {
#            die "AutoSql schema '$Encode::sqlCreate/${sqlTable}.as' does not exist\n";
#        }
        if ((() = split(" ", $fileList)) != 1) {
	    die "BigWig must be loaded with a single file but a list of files was supplied ($fileList)\n";
	}
	# link bigWig binary file to gbdbDir
#        my @cmds = ( "mkdir -p ${gbdbDir}; ln  $fileList ${gbdbDir}/${tableName}.bw");
#        HgAutomate::verbose(2, "loadBigWig cmds [".join(" ; ",@cmds)."]\n");
#        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
#        if(my $err = $safe->exec()) {
#            die("ERROR: bad link of '$fileList' to ${gbdbDir}\n" . $safe->stderr() . "\n");
#        } else {
#            HgAutomate::verbose(2, "$fileList linked to ${gbdbDir}/${tableName}.bw\n");
#        }
	# symlink bigWig binary file into gbdb bbi directory
        my @cmds = ( "ln -sf ${downloadDir}/${tableName}.bigWig /gbdb/${assembly}/bbi/");
        HgAutomate::verbose(2, "loadBigWig cmds [".join(" ; ",@cmds)."]\n");
        my $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed symbolic link to /gbdb/${assembly}/bbi/\n" . $safe->stderr() . "\n");
        } else {
            HgAutomate::verbose(2, "${downloadDir}/${tableName}.bigWig linked to  /gbdb/${assembly}/bbi/\n");
        }
	# create BigWig link table from trackDb to gbdb bigWig binary file
        @cmds = ( "/cluster/bin/x86_64/hgBbiDbLink $assembly $tableName /gbdb/${assembly}/bbi/${tableName}.bigWig");
        HgAutomate::verbose(2, "loadBigBed cmds [".join(" ; ",@cmds)."]\n");
        $safe = SafePipe->new(CMDS => \@cmds, STDOUT => "/dev/null", DEBUG => $opt_verbose > 2);
        if(my $err = $safe->exec()) {
            die("ERROR: File(s) '$fileList' failed bigWig load:\n" . $safe->stderr() . "\n");
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
$pipelineInstance = $ARGV[0];	# currently not used
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

my $fields = Encode::getFields($configPath);
my $daf = Encode::getDaf($submitDir, $fields, $pipelineInstance);
my $db = HgDb->new(DB => $daf->{assembly});
my %labels;

# Add a suffix for non-production loads (to avoid loading over existing tables).

my $loadRaTest = "$submitDir/$loadRa";

my $programDir = dirname($0);

if(!(-e $loadRaTest)) {
    die "ERROR: load.ra not found ($PROG)\n";
}

if(!$opt_skipLoad) {
    # clean out any stuff from previous load
    # We assume unload program is in the same location as loader (fixes problem with misconfigured qateam environment).
    my $localLoad = "$submitDir/$loadRa";
    my $localUnload = "$submitDir/$unloadRa";
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
        $unloader_cmd = "$unloader -configDir $opt_configDir $pipelineInstance $submitPath";
    } else {
        $unloader_cmd = "$unloader $pipelineInstance $submitPath";
    }
    if(system("$unloader_cmd")) {
        die "unload script failed\n";
    }

    #TODO change to : FileUtils.cp $loadRa, $unloadRa
    # XXXX shouldn't we do the cp AFTER we finish everything else successfully?
    if(system("cp $localLoad $localUnload")) {
        die "Cannot: cp $localLoad $localUnload\n";
    }
}

chdir($submitDir);
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

# make the download dir if it doesn't exist
if(!$opt_skipDownload and !(-d $downloadDir)) {
    if(!(-d $downloadDir)) {
        mkdir $downloadDir || die ("Can't create download directory (error: '$!'); please contact the ENCODE staff at encode-staff\@soe.ucsc.edu\n");
    }
}

for my $key (keys %ra) {
    my $h = $ra{$key};
    my $tablename = $h->{tablename};
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
    my %bigWigTypes = map { $_ => 1 } @Encode::bigWigTypes;
    my %bamTypes = map { $_ => 1 } @Encode::bamTypes;
    my $hgdownload = 0;

    HgAutomate::verbose(2, "Now making links and copies. hgdownload=[$hgdownload]\n");
    $hgdownload = @files;

    if(!$opt_skipDownload and $hgdownload) {
        my $targetFile = $downloadDir . "/" . $h->{targetFile};
        # NOTE: The validator is supposed to protect us from overwrites and allow them if -allowReloads.  Loader always loads

        # hard link file(s) into download dir - gzip files as appropriate

        # Removes any file in the way:
        unlink($targetFile);
        if ($type eq "bam") {
            my $baiFile = $targetFile . ".bai";
            unlink($targetFile);
        }
        HgAutomate::verbose(2, "unlink($targetFile)\n");

        if (@files == 1) {
            my $srcFile = "$submitPath/$files[0]";
            HgAutomate::verbose(2, "One file: srcFile=[$srcFile]\n");
            if ($type eq "bam") {
                HgAutomate::verbose(2, "hard-linking $srcFile => $targetFile\n");
                !system("/bin/ln $srcFile $targetFile") || die "link failed: $?\n";
                HgAutomate::verbose(2, "hard-linking $srcFile.bai => $targetFile.bai\n");
                !system("/bin/ln $srcFile.bai $targetFile.bai") || die "link failed: $?\n";
            } elsif(Encode::isZipped($srcFile) || Encode::isTarZipped($srcFile) || ($type eq "bigWig") || ($type eq "bigBed")) {
                HgAutomate::verbose(2, "hard-linking $srcFile => $targetFile\n");
                !system("/bin/ln $srcFile $targetFile") || die "link failed: $?\n";
            } else {
                HgAutomate::verbose(2, "copying/zipping $srcFile => $targetFile\n");
                !system("/usr/bin/pigz -c $srcFile > $targetFile") || die "gzip: $?\n";
            }
        } elsif (@files > 1) {
            if (($type eq "bam") || ($type eq "bigWig") || ($type eq "bigBed")) {
                die "Cannot concatenate '$type' files";
            }

            # Two possibilities: cat then gz or tgz
            if (($type eq "fastq") || ($type eq "doc")) {
                if ( ! Encode::isTarZipped($targetFile) ) {
                    die "Target $targetFile of multiple $type files must be tgz";
                }

                # Now lets tgz all the sources
                my $cmd = "cd $submitPath ; /bin/tar -c";

                for my $file (@files) {
                    $cmd .= " $file";
                }
                $cmd .= " | /usr/bin/pigz -c > $targetFile";  # pigz will parallelize and is faster!

                HgAutomate::verbose(2, "creating gzipped tar $targetFile of multiple files: [@files].\n");
                !system($cmd) || die "system '$cmd' failed: $?\n";

            } else {
                if ( ! Encode::isZipped($targetFile) ) {
                    die "Target $targetFile of multiple $type files must be gz";
                }

                # Create empty target then cat gz'd files together
                !system("cat /dev/null > $targetFile") || die "gzip failed: $?\n";
                HgAutomate::verbose(2, "Begin gzip and cat of multiple files: [@files] to $targetFile\n");
                for my $file (@files) {
                    my $cmd;
                    if(Encode::isZipped($file) || Encode::isTarZipped($file)) { # This doesn't seem right.  Should tar these!!!
                        $cmd = "/bin/cat $submitPath/$file >> $targetFile";
                    } else {
                        $cmd = "/usr/bin/pigz -c $submitPath/$file >> $targetFile";
                    }
                    HgAutomate::verbose(2, "appending gzip of $file to $targetFile\n");
                    !system($cmd) || die "system '$cmd' failed: $?\n";
                }
            }
        } else {
            die "No source files requested"; # assertable
        }
        push(@{$pushQ->{FILES}}, $targetFile);
        # XXXX add to FILES list and then copy files to unloadFiles.txt
    }

    HgAutomate::verbose(2, "TYPE=[$type] extendedTypes=[".(defined($extendedTypes{$type}) ? $extendedTypes{$type} : "")."] key=[$key] tablename=[$tablename] downloadOnly=[$downloadOnly]\n");
    HgAutomate::verbose(2, "TYPE=[$type] bigBedTypes=[".(defined($bigBedTypes{$type}) ? $bigBedTypes{$type} : "")."] key=[$key] tablename=[$tablename] downloadOnly=[$downloadOnly]\n");
    if ($downloadOnly) {
        # XXXX convert solexa/illumina => sanger fastq when appropriate
        HgAutomate::verbose(3, "Download only; dont load [$key].\n");
    } elsif($type eq "gtf") {
        loadGene($assembly, $tablename, $files, $pushQ, "-gtf -genePredExt");
    } elsif($type eq "genePred" ) {
        loadGene($assembly, $tablename, $files, $pushQ, "-genePredExt");
    } elsif ($extendedTypes{$type}) {
        loadBedFromSchema($assembly, $tablename, $files, $type, $pushQ);
    } elsif ($type =~ /^bed/ and defined($sql)) {
        loadBedFromSchema($assembly, $tablename, $files, $sql, $pushQ);
    } elsif ($bamTypes{$type}) {
        loadBam($assembly, $tablename, $downloadDir, $files, $type, $pushQ, $configPath);
    } elsif ($bigWigTypes{$type}) {
        loadBigWig($assembly, $tablename, $downloadDir, $files, $type, $pushQ, $configPath);
    } elsif ($type =~ /^bed (3|4|5|6|8|9|12)$/) {
        loadBed($assembly, $tablename, $files, $pushQ);
    } elsif ($type =~ /^bedGraph (4)$/) {
        loadBedGraph($assembly, $tablename, $files, $pushQ);
    } else {
        die "ERROR: unknown type: $type in $Encode::loadFile ($PROG)\n";
    }
}

#modification to put in README.txt if not already there in download directory.
my $readme = "$downloadDir/README.txt";
unless (-e $readme){
	my @template;
	open TEMPLATE, "$configPath/downloadsReadmeTemplate.txt";
	while (<TEMPLATE>){
	
		my $line = $_;
		chomp $line;
		#skip commented lines
		if ($line =~ m/^\s*#/){next}
		push @template, $line;
		
	}
	my $assm = $daf->{assembly};
	open README, ">$readme" or die "Can't open README file to write in directory $downloadDir\n";
	
	foreach my $line (@template){
		#interpolate in the name of the DB and composite name
		if ($line =~ m/\+\+/){
			$line =~ s/\+\+db\+\+/$assm/;
			$line =~ s/\+\+composite\+\+/$compositeTrack/;
			print README "$line\n";
		}
		else {
			print README "$line\n";
		}
	}
	close README;
}



my $wranglerName = 'encode';

my $tables = join("\\n", @{$pushQ->{TABLES}});
push(@{$pushQ->{FILES}}, "/usr/local/apache/cgi-bin/encode/cv.ra");
my $files = join("\\n", @{$pushQ->{FILES}});
#my $preambleFile = "$downloadDir/preamble.html";
push(@{$pushQ->{FILES}}, "$downloadDir/index.html");
#push(@{$pushQ->{FILES}}, "$downloadDir/preamble.html");

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

#if(! -e $preambleFile ) {
#    open(PREAMBLE, ">$preambleFile") || die "SYS ERROR: Can't write '$preambleFile' file; error: $!\n";
#
    # NOTE: can remove the CHANGE-ME comment if we switch to SSI for the hostname
#    print PREAMBLE <<END;
#<p>This directory contains data generated by the $daf->{grant}/$daf->{lab} lab as part of the ENCODE project.
#Further information is available in the
#<!-- CHANGE-ME to genome.ucsc.edu at releaase time -->
#<A TARGET=_BLANK HREF="http://<!--#echo var="BROWSER_HOST"-->/cgi-bin/hgTrackUi?db=$daf->{assembly}&g=$compositeTrack">
#<em>$daf->{lab}  $daf->{dataType}</em></A> track description. </p>
#
#<p><B>Data is <A HREF="http://genome.ucsc.edu/ENCODE/terms.html">RESTRICTED FROM USE</a>
#in publication  until the restriction date noted for the given data file.</B></p>
#<p>
#There are two files within this directory that contain information about the downloads:
#<LI><A HREF="files.txt">files.txt</A> which is a tab-separated file with the name and metadata for each download.</LI>
#<LI><A HREF="md5sum.txt">md5sum.txt</A> which is a list of the md5sum output for
# each download.</LI>
# <HR>
#END
#}

exit(0);

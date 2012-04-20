#!/usr/bin/env perl

# DO NOT EDIT the /cluster/bin/scripts copy of this file --
# edit ~/kent/src/hg/utils/automation/doCpgIslands.pl instead.

use Getopt::Long;
use warnings;
use strict;
use FindBin qw($Bin);
use lib "$Bin";
use HgAutomate;
use HgRemoteScript;
use HgStepManager;

# Option variable names, both common and peculiar to this script:
use vars @HgAutomate::commonOptionVars;
use vars @HgStepManager::optionVars;
use vars qw/
    $opt_buildDir
    $opt_maskedSeq
    /;

# Specify the steps supported with -continue / -stop:
my $stepper = new HgStepManager(
    [ { name => 'hardMask',   func => \&doHardMask },
      { name => 'cpg', func => \&doCpg },
      { name => 'makeBed', func => \&doMakeBed },
      { name => 'load', func => \&doLoadCpg },
      { name => 'cleanup', func => \&doCleanup },
    ]
				);

# Option defaults:
my $bigClusterHub = 'swarm';
my $smallClusterHub = 'encodek';
my $workhorse = 'hgwdev';
my $dbHost = 'hgwdev';
my $defaultWorkhorse = 'hgwdev';
my $maskedSeq = "$HgAutomate::clusterData/\$db/\$db.2bit";

my $base = $0;
$base =~ s/^(.*\/)?//;

sub usage {
  # Usage / help / self-documentation:
  my ($status, $detailed) = @_;
  # Basic help (for incorrect usage):
  print STDERR "
usage: $base db
options:
";
  print STDERR $stepper->getOptionHelp();
  print STDERR <<_EOF_
    -buildDir dir         Use dir instead of default
                          $HgAutomate::clusterData/\$db/$HgAutomate::trackBuild/cpgIslands
                          (necessary when continuing at a later date).
    -maskedSeq seq.2bit   Use seq.2bit as the masked input sequence instead
                          of default ($maskedSeq).
_EOF_
  ;
  print STDERR &HgAutomate::getCommonOptionHelp('dbHost' => $dbHost,
                                'bigClusterHub' => $bigClusterHub,
                                'smallClusterHub' => $smallClusterHub,
                                'workhorse' => $defaultWorkhorse);
  print STDERR "
Automates UCSC's CpG Island finder for genome database \$db.  Steps:
    hardMask:  Creates hard-masked fastas needed for the CpG Island program.
    cpg:       Run /scratch/data/cpgIslandExt/cpglh on the hard-masked fastas
    makeBed:   Transform output from cpglh into cpgIsland.bed
    load:      Load cpgIsland.bed into \$db.
    cleanup:   Removes hard-masked fastas and output from cpglh.
All operations are performed in the build directory which is
$HgAutomate::clusterData/\$db/$HgAutomate::trackBuild/cpgIslands unless -buildDir is given.
";
  # Detailed help (-help):
  print STDERR "
Assumptions:
1. $HgAutomate::clusterData/\$db/\$db.2bit contains RepeatMasked sequence for
   database/assembly \$db.
" if ($detailed);
  print "\n";
  exit $status;
}


# Globals:
# Command line args: db
my ($db);
# Other:
my ($buildDir, $secondsStart, $secondsEnd);

sub checkOptions {
  # Make sure command line options are valid/supported.
  my $ok = GetOptions(@HgStepManager::optionSpec,
		      'buildDir=s',
		      'maskedSeq=s',
		      @HgAutomate::commonOptionSpec,
		      );
  &usage(1) if (!$ok);
  &usage(0, 1) if ($opt_help);
  &HgAutomate::processCommonOptions();
  my $err = $stepper->processOptions();
  usage(1) if ($err);
  $workhorse = $opt_workhorse if ($opt_workhorse);
  $bigClusterHub = $opt_bigClusterHub if ($opt_bigClusterHub);
  $smallClusterHub = $opt_smallClusterHub if ($opt_smallClusterHub);
  $dbHost = $opt_dbHost if ($opt_dbHost);
}

#########################################################################
# * step: hard mask [bigClusterHub]
sub doHardMask {
  # Set up and perform the cluster run to run the hardMask sequence.
  my $paraHub = $bigClusterHub;
  my $runDir = "$buildDir/run.hardMask";
  my $outRoot = '../hardMaskedFa';

  # First, make sure we're starting clean.
  if (-e "$runDir/run.time") {
    die "doHardMask: looks like this was run successfully already " .
      "(run.time exists).  Either run with -continue cpg or some later " .
	"stage, or move aside/remove $runDir/ and run again.\n";
  } elsif ((-e "$runDir/gsub" || -e "$runDir/jobList") && ! $opt_debug) {
    die "doHardMask: looks like we are not starting with a clean " .
      "slate.\n\tPlease move aside or remove\n  $runDir/\n\tand run again.\n";
  }
  &HgAutomate::mustMkdir($runDir);
  my $templateCmd = ("./runOne.csh " . '$(root1) '
                . "{check out exists+ $outRoot/" . '$(lastDir1)/$(file1)}');
  &HgAutomate::makeGsub($runDir, $templateCmd);
 `touch "$runDir/para_hub_$paraHub"`;

  my $fh = &HgAutomate::mustOpen(">$runDir/runOne.csh");
  print $fh <<_EOF_
#!/bin/csh -ef
set chrom = \$1
set result = \$2
twoBitToFa $maskedSeq:\$chrom stdout \\
  | maskOutFa stdin hard \$result
_EOF_
  ;
  close($fh);

  my $whatItDoes = "Make hard-masked fastas for each chrom.";
  my $bossScript = new HgRemoteScript("$runDir/doHardMask.csh", $paraHub,
				      $runDir, $whatItDoes);

  $bossScript->add(<<_EOF_
mkdir -p $outRoot
chmod a+x runOne.csh
set perDirLimit = 4000
set ctgCount = `twoBitInfo $maskedSeq stdout | wc -l`
set subDirCount = `echo \$ctgCount | awk '{printf "%d", 1+\$1/4000}'`
@ dirCount = 0
set dirName = `echo \$dirCount | awk '{printf "%03d", \$1}'`
@ perDirCount = 0
mkdir $outRoot/\$dirName
/bin/rm -f chr.list
/bin/touch chr.list
foreach chrom ( `twoBitInfo $maskedSeq stdout | cut -f1` )
  if (\$perDirCount < \$perDirLimit) then
    @ perDirCount += 1
  else
    @ dirCount += 1
    set dirName = `echo \$dirCount | awk '{printf "%03d", \$1}'`
    set perDirCount = 1
    mkdir $outRoot/\$dirName
  endif
  echo \$dirName/\$chrom.fa >> chr.list
end
$HgAutomate::gensub2 chr.list single gsub jobList
$HgAutomate::paraRun
_EOF_
  );
  $bossScript->execute();
} # doHardMask

#########################################################################
# * step: cpg [workhorse] (will change to a cluster at some point)
sub doCpg {
  # Set up and perform the cluster run to run the CpG function on the
  #     hard masked sequence.
  my $paraHub = $bigClusterHub;
  my $runDir = $buildDir;
  # First, make sure we're starting clean.
  if (-e "$runDir/run.time") {
    die "doCpg: looks like this was run successfully already " .
      "(run.time exists).  Either run with -continue makeBed or some later " .
	"stage, or move aside/remove $runDir/ and run again.\n";
  } elsif ((-e "$runDir/gsub" || -e "$runDir/jobList") && ! $opt_debug) {
    die "doCpg: looks like we are not starting with a clean " .
      "slate.\n\tclean\n  $runDir/\n\tand run again.\n";
  }
  &HgAutomate::mustMkdir($runDir);

  my $templateCmd = ("./runCpg.csh " . '$(root1) $(lastDir1) '
                . '{check out exists results/$(lastDir1)/$(root1).cpg}');
  &HgAutomate::makeGsub($runDir, $templateCmd);
 `touch "$runDir/para_hub_$paraHub"`;

  my $fh = &HgAutomate::mustOpen(">$runDir/runCpg.csh");
  print $fh <<_EOF_
#!/bin/csh -ef
set chrom = \$1
set dir = \$2
set result = \$3
set resultDir = \$result:h
mkdir -p \$resultDir
set seqFile = hardMaskedFa/\$dir/\$chrom.fa
set C = `faCount \$seqFile | egrep -v "^#seq|^total" | awk '{print \$2 - \$7}'`
if ( \$C > 200 ) then
    /scratch/data/cpgIslandExt/cpglh \$seqFile > \$result
else
    touch \$result
endif
_EOF_
  ;
  close($fh);

  my $whatItDoes = "Run /scratch/data/cpgIslandExt/cpglh on masked sequence.";
  my $bossScript = new HgRemoteScript("$runDir/doCpg.csh", $paraHub,
				      $runDir, $whatItDoes);
  $bossScript->add(<<_EOF_
mkdir -p results
chmod a+x runCpg.csh
rm -f file.list
find ./hardMaskedFa -type f > file.list
$HgAutomate::gensub2 file.list single gsub jobList
$HgAutomate::paraRun
_EOF_
  );
  $bossScript->execute();
} # doCpg

#########################################################################
# * step: make bed [workhorse]
sub doMakeBed {
  my $runDir = $buildDir;
  &HgAutomate::mustMkdir($runDir);

  # First, make sure we're starting clean.
  if (-e "$runDir/cpgIsland.bed") {
    die "doMakeBed: looks like this was run successfully already " .
      "(cpgIsland.bed exists).  Either run with -continue load or cleanup " .
	"or move aside/remove $runDir/cpgIsland.bed and run again.\n";
  }

  my $whatItDoes = "Makes bed from cpglh output.";
  my $bossScript = new HgRemoteScript("$runDir/doMakeBed.csh", $workhorse,
				      $runDir, $whatItDoes);

  $bossScript->add(<<_EOF_
catDir -r results \\
     | awk \'\{\$2 = \$2 - 1; width = \$3 - \$2;  printf\(\"\%s\\t\%d\\t\%s\\t\%s \%s\\t\%s\\t\%s\\t\%0.0f\\t\%0.1f\\t\%s\\t\%s\\n\", \$1, \$2, \$3, \$5, \$6, width, \$6, width\*\$7\*0.01, 100.0\*2\*\$6\/width, \$7, \$9\);}\' \\
     | sort -k1,1 -k2,2n > cpgIsland.bed
_EOF_
  );
  $bossScript->execute();
} # doMakeBed

#########################################################################
# * step: load [dbHost]
sub doLoadCpg {
  my $runDir = $buildDir;
  &HgAutomate::mustMkdir($runDir);

  my $whatItDoes = "Loads cpgIsland.bed.";
  my $bossScript = new HgRemoteScript("$runDir/doLoadCpg.csh", $dbHost,
				      $runDir, $whatItDoes);

  $bossScript->add(<<_EOF_
set C=`cut -f1 cpgIsland.bed | sort -u | awk '{print length(\$0)}' | sort -rn | head -1`
sed -e "s/14/\${C}/" \$HOME/kent/src/hg/lib/cpgIslandExt.sql > cpgIslandExt.sql
hgLoadBed -sqlTable=cpgIslandExt.sql -tab $db cpgIslandExt cpgIsland.bed 
checkTableCoords -verboseBlocks -table=cpgIslandExt $db
_EOF_
  );
  $bossScript->execute();
} # doLoad

#########################################################################
# * step: cleanup [fileServer]
sub doCleanup {
  my $runDir = $buildDir;
  my $whatItDoes = "It cleans up or compresses intermediate files.";
  my $fileServer = &HgAutomate::chooseFileServer($runDir);
  my $bossScript = new HgRemoteScript("$runDir/doCleanup.csh", $fileServer,
				      $runDir, $whatItDoes);
  $bossScript->add(<<_EOF_
rm -rf hardMaskedFa/ results/ err/ run.hardMask/err/
rm -f batch.bak bed.tab cpgIslandExt.sql run.hardMask/batch.bak
gzip cpgIsland.bed
_EOF_
  );
  $bossScript->execute();
} # doCleanup


#########################################################################
# main

# Prevent "Suspended (tty input)" hanging:
&HgAutomate::closeStdin();

# Make sure we have valid options and exactly 1 argument:
&checkOptions();
&usage(1) if (scalar(@ARGV) != 1);
$secondsStart = `date "+%s"`;
chomp $secondsStart;
($db) = @ARGV;

# Force debug and verbose until this is looking pretty solid:
#$opt_debug = 1;
#$opt_verbose = 3 if ($opt_verbose < 3);

# Establish what directory we will work in.
$buildDir = $opt_buildDir ? $opt_buildDir :
  "$HgAutomate::clusterData/$db/$HgAutomate::trackBuild/cpgIslands";
$maskedSeq = $opt_maskedSeq ? $opt_maskedSeq :
  "$HgAutomate::clusterData/$db/$db.2bit";

# Do everything.
$stepper->execute();

# Tell the user anything they should know.
my $stopStep = $stepper->getStopStep();
my $upThrough = ($stopStep eq 'cleanup') ? "" :
  "  (through the '$stopStep' step)";

$secondsEnd = `date "+%s"`;
chomp $secondsEnd;
my $elapsedSeconds = $secondsEnd - $secondsStart;
my $elapsedMinutes = int($elapsedSeconds/60);
$elapsedSeconds -= $elapsedMinutes * 60;

HgAutomate::verbose(1,
	"\n *** All done !  Elapsed time: ${elapsedMinutes}m${elapsedSeconds}s\n");
&HgAutomate::verbose(1,
	"\n *** All done!$upThrough\n");
&HgAutomate::verbose(1,
	" *** Steps were performed in $buildDir\n");
&HgAutomate::verbose(1, "\n");


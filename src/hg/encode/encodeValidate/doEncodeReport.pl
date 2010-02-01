#!/usr/bin/env perl

# doEncodeReport.pl - munge trackDb and pipeline projects table to report
#                       submitted experiments.
# Reporting format:
#       Project, Lab, Data Type, Cell Type, Experiment Parameters, Freeze, Submission Date, Release Date, Status, Version, Submission Ids

# DO NOT EDIT the /cluster/bin/scripts copy of this file --
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeValidate/doEncodeReport.pl,v 1.13 2010/02/01 02:36:31 kate Exp $

# TODO: warn if variable not found in cv.ra

use warnings;
use strict;

use File::stat;
use File::Basename;
use Getopt::Long;
use English;
use Carp qw(cluck);
use Cwd;
use IO::File;
use File::Basename;

use lib "/cluster/bin/scripts";
#use lib "/cluster/home/kate/kent/src/hg/utils/automation";
use Encode;
use HgAutomate;
use HgDb;
use RAFile;
use SafePipe;

use vars qw/
    $opt_configDir
    $opt_verbose
    /;

# Global variables
our $pipeline = "encpipeline_prod";
our $pipelinePath = "/hive/groups/encode/dcc/pipeline/" . $pipeline;
our $configPath = $pipelinePath . "/config";
#our $configPath = "/cluster/home/kate/kent/src/hg/encode/encodeValidate" . "/config";
our $assembly = "hg18";

sub usage {
    print STDERR <<END;
usage: doEncodeReport.pl

options:
    -configDir=dir      Path of configuration directory, containing
                        metadata .ra files (default: submission-dir/../config)
    -verbose=num        Set verbose level to num (default 1).
END
exit 1;
}

############################################################################
# Main

my $wd = cwd();

my $ok = GetOptions("configDir=s",
                    "verbose=i",
                    );
usage() if (!$ok);
$opt_verbose = 1 if (!defined $opt_verbose);

# Determine submission, configuration, and output directory paths
HgAutomate::verbose(4, "Using submission directory tree at\'$pipelinePath\'\n");

if (defined $opt_configDir) {
    if ($opt_configDir =~ /^\//) {
        $configPath = $opt_configDir;
    } else {
        $configPath = "$wd/$opt_configDir";
    }
}

if(!(-d $configPath)) {
    die "configPath '$configPath' is invalid; Can't find the config directory\n";
}
HgAutomate::verbose(4, "Config directory path: \'$configPath\'\n");

my %terms = Encode::getControlledVocab($configPath);
my %tags = Encode::getControlledVocabTags($configPath);

# use pi.ra file to map pi/lab/institution/grant/project
my $labRef = Encode::getLabs($configPath);
my %labs = %{$labRef};

# parse all trackDb entries w/ metadata setting into hash of submissions
#   key is lab+dataType+cell+vars

my $dbh = HgDb->new(DB => $assembly);
my $sth = $dbh->execute(
        "select settings, tableName from trackDb_encodeReport where settings like \'\%metadata\%\'");
my @row;
my %experiments = ();
printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", 
        "Project", "Project-PI", "Lab", "Lab-PI", "Data_Type", "Cell_Type", 
        "Experiment_Parameters", "Experimental_Factors", 
        #"Version", 
        "Freeze", "Submit_Date", "Release_Date", 
        "Status", "Submission_IDs");
while (@row = $sth->fetchrow_array()) {
    my $settings = $row[0];
    my $tableName = $row[1];
    # strip down to just tag=value in the metadata line
    $settings =~ tr/\n/|/;
    $settings =~ s/^.*metadata //;  
    $settings =~ s/\|.*$//;
    print STDERR "     SETTINGS: $settings\n";
    my $ref = Encode::metadataLineToHash($settings);
    my %metadata =  %{$ref};
    my %experiment = ();
    if (!defined($metadata{"grant"})) {
        $metadata{"grant"} = "unknown";
    }
    $experiment{"project"} = $metadata{"grant"};
    # HACK:  to correct Wold lab (not grant)
    #$experiment{"project"} = "HudsonAlpha" if ($metadata{"grant"} eq "Wold");
    foreach my $lab (keys %labs) {
        die "no grant for $lab" unless defined($labs{$lab}->{"grant"});
        if ($labs{$lab}->{"grant"} eq $metadata{"grant"}) {
            $experiment{"project"} = $labs{$lab}->{"project"};
            $experiment{"projectPi"} = $labs{$lab}->{"grant"};
            $experiment{"labPi"} = $labs{$lab}->{"pi"};
            last;
        }
    }
    $experiment{"lab"} = $metadata{"lab"};
    # strip off PI name in parens
    $experiment{"lab"} =~ s/\(\w+\)//;

    # for now, force all Yale projects to be Yale lab (until metadata in projects table can match)
    #if ($metadata{"grant"} eq "Snyder") {
        #$experiment{"lab"} = "Yale";
    #}
    $experiment{"dataType"} = lc($metadata{"dataType"});

    $experiment{"cell"} = "none";
    if (defined($metadata{"cell"})) {
        $experiment{"cell"} = $metadata{"cell"};
    }
    print STDERR "    tableName  " . $tableName . "\n";

    # determine term type for experiment parameters 
    my %vars = ();
    my @termTypes = (keys %terms);
    foreach my $var (keys %metadata) {
        foreach my $termType (@termTypes) {
            next if ($termType eq "Cell Line");
            next if ($termType eq "dataType");
            if (defined($terms{$termType}{$metadata{$var}})) {
                $vars{$termType} = $metadata{$var};
            }
        }
    }
    # treat version like a variable
    if (defined($metadata{"submittedDataVersion"})) {
        my $version = $metadata{"submittedDataVersion"};
        # strip comment
        $version =~ s/^.*(V\d+).*/$1/;
        $vars{"version"} = $version;
    }
    # experimental params -- iterate through all tags, lookup in cv.ra, 
     $experiment{"vars"} = "none";
     $experiment{"varLabels"} = "none";
     $experiment{"factorLabels"} = "none";
    if (scalar(keys %vars) > 0) {
        # alphasort vars by term type, to assure consistency
        # TODO: define explicit ordering for term type in cv.ra
        $experiment{"vars"} = "";
        $experiment{"varLabels"} = "";
        $experiment{"factorLabels"} = "";
        foreach my $termType (sort keys %vars) {
            my $var = $vars{$termType};
            $experiment{"vars"} = $experiment{"vars"} .  $var . ";";
            my $varLabel = defined($terms{$termType}{$var}->{"label"}) ? 
                                $terms{$termType}{$var}->{"label"} : $var;
            $experiment{"varLabels"} = $experiment{"varLabels"} .  $varLabel . ";";
            $experiment{"factorLabels"} = $experiment{"factorLabels"} .  $termType . "=" . $varLabel . " ";
        }
        chop $experiment{"vars"};
        chop $experiment{"varLabels"};
        chop $experiment{"factorLabels"};
    }

    $experiment{"freeze"} = $metadata{"dataVersion"};
    $experiment{"freeze"} =~ s/^ENCODE (...).*20(\d\d).*$/$1-20$2/;

    $experiment{"submitDate"} = defined($metadata{"dateResubmitted"}) ?
                $metadata{"dateResubmitted"} : $metadata{"dateSubmitted"};

    $experiment{"releaseDate"} = "none";

    # note that this was found in trackDb (not just in pipeline, below)
    $experiment{"trackDb"} = 1;

    # get release date from 'lastUpdated' field of submission dir
    #$experiment{"status"} = "unknown";

    # a few tracks are missing the subId
    my $subId = (defined($metadata{"subId"})) ? $metadata{"subId"} : 0; 

    #die "undefined project" unless defined($experiment{"project"});
    #die "undefined lab" unless defined($experiment{"lab"});
    #die "undefined dataType" unless defined($experiment{"dataType"});
    #die "undefined cell" unless defined($experiment{"cell"});
    #die "undefined vars" unless defined($experiment{"vars"});
    #die "undefined freeze" unless defined($experiment{"freeze"});
    #die "undefined submitDate" unless defined($experiment{"submitDate"});
    #die "undefined releaseDate" unless defined($experiment{"releaseDate"});
    #die "undefined status" unless defined($experiment{"status"});

    # create key for experiment: lab+dataType+cell+vars
    my $expKey = $experiment{"lab"} . "+" . $experiment{"dataType"} . "+" . 
                                $experiment{"cell"} . "+" . 
                                $experiment{"vars"};

    print STDERR "KEY: " . $expKey . "\n";

    # save in a hash of experiments, 
    #   keyed by lab+dataType+cell+vars
    # (include version in vars)
    # subId -- lookup, add to list if exists
    if (defined($experiments{$expKey})) {
        my %ids = %{$experiments{$expKey}->{"ids"}};
        print STDERR "CHECKING: " . $expKey . " new ID: " . $subId . " IDs: " . join(",", keys %ids) . "\n";
        if (!defined($experiment{$expKey}->{"ids"}->{$subId})) {
            $experiments{$expKey}->{"ids"}->{$subId} = $subId;
            %ids = %{$experiments{$expKey}->{"ids"}};
            print STDERR "MERGED: " . $expKey . " IDs: " . join(",", keys %ids) . "\n";
        }
    } else {
        my %ids = ();
        $ids{$subId} = $subId;
        $experiment{"ids"} = \%ids;
        $experiments{$expKey} = \%experiment;
        print STDERR "ADDING: " . $expKey . " ID: " . $subId . "\n";
    }

}
$sth->finish;
$dbh->{DBH}->disconnect;

# Merge in submissions from pipeline projects table not found in trackDb
# (not yet configured, or else retired) that have valid metadata (validated, loaded)
# Also capture status of all submissions for later use

my %submissionStatus = ();
my %submissionUpdated = ();
$dbh = HgDb->new(DB => $pipeline);
$sth = $dbh->execute( 
    "select lab, data_type, metadata, created_at, updated_at, status, id, name from projects order by id");

while (@row = $sth->fetchrow_array()) {
    my $lab = $row[0];
    my $dataType = $row[1];
    my $created_at = $row[3];
    my $updated_at = $row[4];
    my $status = $row[5];
    my $id = $row[6];
    my $name = $row[7];
    $submissionStatus{$id} = $status;
    $submissionUpdated{$id} = $updated_at;
    $submissionUpdated{$id} =~ s/ \d\d:\d\d:\d\d//;
    # lookup in lab from hash to find project
    my $project = "unknown";
    if (defined($lab) && defined($labs{$lab})) {
        $project = $labs{$lab}->{"project"};
    } else {
        warn "lab undefined, subId=" . $id . " " . $status . " " . $name . "\n";
        next;
    }
    if ($project eq "unknown") {
        warn "     project unknown for lab " . $lab . " subId=" . $id . " " . $name . "\n";
    }
# note: lcase the datatype before generating key (inconsistent in DAFs)
    my $cell = "none";
    my $varString = "none";
    my $varLabelString = "none";
    my $factorLabelString = "none";

# get cell type and experimental vars
    if (defined($row[2])) {
        my @metadata = split("; ", $row[2]);
        #print STDERR "      metadata " . $row[2] . "\n";
        foreach my $expVars (@metadata) {
# one experiment
            #print STDERR "      expVars " . $expVars . "\n";
            my @vars = split(", ", $expVars);

            my $termType;
# extract cell type, order other vars by term type alpha order
# determine term type for experiment parameters 
            my %varHash = ();
            my @termTypes = (keys %terms);
            #print STDERR "      lab " . $lab . " subId=" . $id . " " . $name . "\n";
            foreach my $var (@vars) {
                #print STDERR $var . ",";
                foreach $termType (@termTypes) {
                    if (defined($terms{$termType}{$var})) {
                        if ($termType eq "Cell Line") {
                            $cell = $var;
                        } else {
                            $varHash{$termType} = $var;
                        }

                    }
                }
            print STDERR "\n";
            }
            $varString = "none";
            $varLabelString = "none";
            $factorLabelString = "none";
            # experimental params -- iterate through all tags, lookup in cv.ra, 
            if (scalar(keys %varHash) > 0) {
                $varString = "";
                $varLabelString = "";
                $factorLabelString = "";
                # alphasort vars by term type, to assure consistency
                # TODO: define a priority order in cv.ra
                foreach $termType (sort keys %varHash) {
                    my $var = $varHash{$termType};
                    $varString = $varString . $var . ";";
                    my $varLabel = defined($terms{$termType}{$var}->{"label"}) ? 
                                        $terms{$termType}{$var}->{"label"} : $var;
                    $varLabelString = $varLabelString . $varLabel . ";";
                    $factorLabelString = $factorLabelString . $termType . "=" . $varLabel . " ";
                }
                chop $varString;
                chop $varLabelString;
                chop $factorLabelString;
            }
            #print STDERR "    varString:" . $varString . "\n";
            my $expKey = $lab . "+" . lc($dataType) .  "+" .
                                $cell . "+" .  $varString;
            print STDERR "KEY+STATUS+ID: " . $expKey . "|" . $status . "|" . $id . "\n";
            if (defined($experiments{$expKey})) {
                my %experiment = %{$experiments{$expKey}};
                if (defined($experiment{"trackDb"})) {
                    if (!defined $experiment{"ids"}->{$id}) {
                        $experiment{"ids"}->{$id} = $id;
                    }
                    # this id is in trackDb
                    if (!defined($experiment{"statusId"}) || 
                            Encode::laterPipelineStatus($status, $experiment{"status"})) {
                        # need to set or change status
                        $experiment{"statusId"} = $id;
                        $experiment{"status"} = "unknown" if (!defined($experiment{"status"}));
                        if ($experiment{"status"} ne $status) {
                            warn("STATUS change for KEY: " . $expKey . 
                                    " OLD: " . $experiment{"status"} . 
                                            " from ID: " . $experiment{"statusId"} . 
                                    " NEW: " . $status . " from ID " . $id . "\n");
                        }
                        $experiment{"status"} = $status;
                        print STDERR "    Setting status: " . $expKey . 
                            " from ID: " . $id . " to " . $status . "\n";
                    }
                } else {
                    # not in trackDb but already added to hash, update tempStatus if id is bigger
                    if ($id gt $experiment{"tempStatusId"}) {
                        # need to change status
                        if ($experiment{"tempStatus"} ne $status) {
                            warn("Temp STATUS change for non-trackDb KEY: " . $expKey . 
                                    " OLD: " . $experiment{"tempStatus"} . 
                                            " from ID: " . $experiment{"tempStatusId"} . 
                                    " NEW: " . $status . " from ID " . $id . "\n");
                        }
                        $experiment{"tempStatus"} = $status;
                        $experiment{"tempStatusId"} = $id;
                        print STDERR "    Setting temp status of KEY: " . $expKey . 
                                " from ID: " . $id . " to " . $status . "\n";
                    }
                }
                # save updated experiment to hash
                $experiment{"releaseDate"} =  ($status eq "released") ? $updated_at : "none";
                $experiment{"releaseDate"} =~ s/ \d\d:\d\d:\d\d//;
                $experiments{$expKey} = \%experiment;
            } else {
                # not in trackDb or found yet in projects table -- add if status is good
                next if ($status ne 'loaded' && $status ne 'validated');
                my %experiment = ();
                $experiment{"project"} = $project;
                $experiment{"projectPi"} = $labs{$lab}->{"grant"};
                $experiment{"lab"} = $lab;
                $experiment{"labPi"} = $labs{$lab}->{"pi"};
                $experiment{"dataType"} = lc $dataType;
                $experiment{"cell"} = $cell;
                my %ids = ();
                $ids{$id} = $id;
                $experiment{"ids"} = \%ids;
                $experiment{"tempStatus"} = $status;
                $experiment{"tempStatusId"} = $id;
                $experiment{"vars"} = $varString;
                $experiment{"varLabels"} = $varLabelString;
                $experiment{"factorLabels"} = $factorLabelString;
                $experiment{"submitDate"} = $created_at;
                # trim off time
                $experiment{"submitDate"} =~ s/ \d\d:\d\d:\d\d//;
                $experiment{"releaseDate"} =  ($status eq "released") ? $updated_at : "none";
                $experiment{"releaseDate"} =~ s/ \d\d:\d\d:\d\d//;
                $experiment{"freeze"} = ($status eq "loaded") ? $Encode::dataVersion : "unknown";
                $experiment{"freeze"} =~ s/^ENCODE (...).*20(\d\d).*$/$1-$2/;
                $experiments{$expKey} = \%experiment;
                print STDERR "ADDING2: " . $expKey . " IDs: " . 
                        join(",", keys %ids) . " PROJ: " . $project . "\n";
            }
        }
    }
}
$sth->finish;
$dbh->{DBH}->disconnect;

# fill in statuses for those experiments missing them, and print out results
foreach my $key (keys %experiments) {
    my %experiment = %{$experiments{$key}};
    my %ids = %{$experiment{"ids"}};
    print STDERR "    IDs: " . join(",", keys %ids) . " KEY:  " . $key . "\n";
    die "undefined project" unless defined($experiment{"project"});
    die "undefined projectPi" unless defined($experiment{"projectPi"});
    die "undefined lab" unless defined($experiment{"lab"});
    die "undefined labPi" unless defined($experiment{"labPi"});
    die "undefined dataType" unless defined($experiment{"dataType"});
    die "undefined cell" unless defined($experiment{"cell"});
    die "undefined vars" unless defined($experiment{"vars"});
    die "undefined varLabels" unless defined($experiment{"varLabels"});
    die "undefined factorLabels" unless defined($experiment{"factorLabels"});
    die "undefined freeze" unless defined($experiment{"freeze"});
    die "undefined submitDate" unless defined($experiment{"submitDate"});
    die "undefined releaseDate" unless defined($experiment{"releaseDate"});
    foreach my $subId (keys %ids) {
        last if (defined $experiment{"status"});
        $experiment{"status"} = $submissionStatus{$subId};
        $experiment{"releaseDate"} = $submissionUpdated{$subId}
            if ($experiment{"status"} eq "released");
    }
    die "undefined status" unless defined($experiment{"status"});

    # Cosmetics so that auto-generated spreadsheet matches old manual version
    # map dataType tag to term from cv.ra
    my $dataType = $experiment{"dataType"};
    if (defined($tags{"dataType"}{$dataType})) {
        $experiment{"dataType"} = 
            (defined($tags{"dataType"}{$dataType}->{"label"})) ?
                $tags{"dataType"}{$dataType}->{"label"} :
                $tags{"dataType"}{$dataType}->{"term"};
    }
    # map lab to label in pi.ra
    my $lab = $experiment{"lab"};
    if (defined($labs{$lab}->{"label"})) {
        $experiment{"lab"} = $labs{$lab}->{"label"};
    }

    printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", 
                $experiment{"project"}, $experiment{"projectPi"}, 
                $experiment{"lab"}, $experiment{"labPi"}, 
                $experiment{"dataType"}, $experiment{"cell"}, 
                $experiment{"varLabels"}, $experiment{"factorLabels"}, 
                $experiment{"freeze"}, $experiment{"submitDate"}, 
                $experiment{"releaseDate"}, $experiment{"status"}, 
                join(",", keys %ids));
}

exit 0;

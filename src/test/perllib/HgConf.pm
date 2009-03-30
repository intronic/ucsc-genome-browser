#
# HgConf: interface to {.,}hg.conf files.
#
package HgConf;

use strict;
use vars qw($VERSION @ISA @EXPORT_OK);
use Exporter;
use Carp;

@ISA = qw(Exporter);
@EXPORT_OK = qw( new lookup );
$VERSION = '0.01';


#
# new: create an HgConf object.
# Mandatory argument: <none>
# Optional arguments: filename (if not given, we'll look in usual places)
#
sub new {
    my $class = shift;
    my $filename = shift;
    confess "Too many arguments" if (defined shift);
    if (! defined $filename) {
      $filename = $ENV{HGDB_CONF};
      if (! defined $filename) {
	$filename = $ENV{'HOME'} . "/.hg.conf";
	if (! -e $filename) {
	  $filename = "./hg.conf";
	}
      }
      if (! -e $filename) {
	die "HgConf::new: Error: can't find .hg.conf or hg.conf, and no filename given.\n";
      }
    }
    my $this = {};
    open(HGCONF, "<$filename")
      || die "Couldn't open $filename: $!\n";
    while (<HGCONF>) {
      next if (/^\s*#/ || /^\s*$/);
      if (/([\w.]+)\s*=\s*(\S+)/) {
	$this->{$1} = $2;
      }
    }
    close(HGCONF);
    bless $this, $class;
} # end new


#
# lookup: get the value of a variable.
# Mandatory argument: variable name, e.g. "central.db"
#
sub lookup {
    my $this = shift;
    my $var = shift;
    confess "Too many arguments" if (defined shift);
    confess "Too few arguments"  if (! defined $var);

    return($this->{$var});
}


# perl packages need to end by returning a positive value:
1;

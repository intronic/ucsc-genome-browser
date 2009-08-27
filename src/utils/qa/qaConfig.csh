# qaConfig.csh
#
# This file is meant to be sourced by all of the qa scripts that use
# tcsh.  There is a separate qaConfig.bash file for bash scripts.
# It is a place to set variables, and probably to do all kinds of other
# useful stuff.

# variable containing the host of the mysql server for hgwbeta
set sqlbeta = mysqlbeta
set sqlrr   = genome-centdb

# makeItemsItem.sql was originally generated by the autoSql program, which also 
# generated makeItemsItem.c and makeItemsItem.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#An item in a makeItems type track.
CREATE TABLE makeItemsItem (
    bin int unsigned not null,	# Bin for range index
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item - up to 16 chars
    strand char(1) not null,	# + or - for strand
    score int unsigned not null,	# 0-1000.  Higher numbers are darker.
    color varchar(255) not null,	# Comma separated list of RGB components.  IE 255,0,0 for red
    description longblob not null,	# Longer item description
              #Indices
    INDEX(chrom(16),bin)
);

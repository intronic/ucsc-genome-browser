# rmskOut.sql was originally generated by the autoSql program, which also 
# generated rmskOut.c and rmskOut.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#RepeatMasker .out record
CREATE TABLE rmskOut (
    swScore int unsigned not null,	# Smith Waterman alignment score
    milliDiv int unsigned not null,	# Base mismatches in parts per thousand
    milliDel int unsigned not null,	# Bases deleted in parts per thousand
    milliIns int unsigned not null,	# Bases inserted in parts per thousand
    genoName varchar(255) not null,	# Genomic sequence name
    genoStart int unsigned not null,	# Start in genomic sequence
    genoEnd int unsigned not null,	# End in genomic sequence
    genoLeft int not null,	# Size left in genomic sequence
    strand char(1) not null,	# Relative orientation + or -
    repName varchar(255) not null,	# Name of repeat
    repClass varchar(255) not null,	# Class of repeat
    repFamily varchar(255) not null,	# Family of repeat
    repStart int not null,	# Start in repeat sequence
    repEnd int unsigned not null,	# End in repeat sequence
    repLeft int not null,	# Size left in repeat sequence
    id char(1) not null,	# First digit of id field in RepeatMasker .out file.  Best ignored
              #Indices
    PRIMARY KEY(swScore)
);

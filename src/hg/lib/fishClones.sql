# fishClones.sql was originally generated by the autoSql program, which also 
# generated fishClones.c and fishClones.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Describes the positions of fishClones in the assembly
CREATE TABLE fishClones (
    chrom varchar(25) not null,	# Human chromosome number
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(25) not null,	# Name of clone
    score int unsigned not null,	# Always 1000
    placeCount int unsigned not null,	# Number of times FISH'd
    bandStarts longblob not null,	# Start FISH band
    bandEnds longblob not null,	# End FISH band
    labs longblob not null,	# Lab where clone FISH'd
    placeType varchar(255) not null,	# How clone was placed on the sequence assembly
    accCount int unsigned not null,	# Number of accessions associated with the clone
    accNames longblob not null,	# Accession associated with clone
    stsCount int unsigned not null,	# Number of STS markers associated with this clone
    stsNames longblob not null,	# Names of STS  markers
    beCount int unsigned not null,	# Number of BAC end sequences associated with this clone
    beNames longblob not null,	# Accessions of BAC ends
              #Indices
    INDEX(chrom,chromStart,name)
);

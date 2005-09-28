# mafFrames.sql was originally generated by the autoSql program, which also 
# generated mafFrames.c and mafFrames.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#codon frame assignment for MAF components
CREATE TABLE mafFrames (
    chrom varchar(255) not null,	# Chromosome
    chromStart int unsigned not null,	# Start range in chromosome
    chromEnd int unsigned not null,	# End range in chromosome
    src varchar(255) not null,	# Name of sequence source in MAF
    frame tinyint unsigned not null,	# frame (0,1,2) for first base(+) or last bast(-)
    strand char(1) not null,	# + or -
    name varchar(255) not null,	# Name of gene used to define frame
);

# bacEndPairs.sql was originally generated by the autoSql program, which also 
# generated bacEndPairs.c and bacEndPairs.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Positions of BAC end pairs for clones
CREATE TABLE bacEndPairs (
    bin smallint not null,	# Bin number for browser speedup
    chrom varchar(15) not null,	# Human chromosome
    chromStart int unsigned not null,	# Start position of clone in chromosome
    chromEnd int unsigned not null,	# End position of clone in chromosome
    name varchar(100) not null,	# Name of clone
    score int unsigned not null,	# Score = 1000/(# of times clone appears in assembly)
    strand char(2) not null,	# Value should be + or -
    pslTable varchar(50) not null,	# Table which contains corresponding PSL records for linked features
    lfCount int unsigned not null,	# Number of linked features in the series
    lfStarts longblob not null,	# Comma separated list of start positions of each linked feature in genomic
    lfSizes longblob not null,	# Comma separated list of sizes of each linked feature in genomic
    lfNames longblob not null,	# Comma separated list of names of linked features
              #Indices
    INDEX(chrom(8),bin),
    INDEX(chrom(8),chromStart),
    INDEX(chrom(8),chromEnd),
    INDEX(name(16))
);

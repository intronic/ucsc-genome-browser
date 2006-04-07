# bactigPos.sql was originally generated by the autoSql program, which also 
# generated bactigPos.c and bactigPos.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Bactig positions in chromosome coordinates (bed 4 +).
CREATE TABLE bactigPos (
    chrom varchar(255) not null,	# Chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Bactig
    startContig varchar(255) not null,	# First contig in this bactig
    endContig varchar(255) not null,	# Last contig in this bactig
              #Indices
    PRIMARY KEY(name(12)),
    INDEX(chrom,chromStart)
);

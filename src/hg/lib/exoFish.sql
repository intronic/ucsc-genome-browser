# exoFish.sql was originally generated by the autoSql program, which also 
# generated exoFish.c and exoFish.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#An evolutionarily conserved region (ecore) with Tetroadon
CREATE TABLE exoFish (
    chrom varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Just a dummy dot
    score int unsigned not null,	# Score from 0 to 1000
              #Indices
    PRIMARY KEY(chrom(12),chromStart),
    UNIQUE(chrom(12),chromEnd)
);

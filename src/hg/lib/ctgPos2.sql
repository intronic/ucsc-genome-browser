# ctgPos2.sql was originally generated by the autoSql program, which also 
# generated ctgPos2.c and ctgPos2.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Where a contig is inside of a chromosome including contig type information.
CREATE TABLE ctgPos2 (
    contig varchar(255) not null,	# Name of contig
    size int unsigned not null,	# Size of contig
    chrom varchar(255) not null,	# Chromosome name
    chromStart int unsigned not null,	# Start in chromosome
    chromEnd int unsigned not null,	# End in chromosome
    type char(1) not null,	# (W)GS contig, (F)inished, (P)redraft, (D)raft, (O)ther
              #Indices
    PRIMARY KEY(contig)
);

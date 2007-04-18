# jgiGene.sql was originally generated by the autoSql program, which also 
# generated jgiGene.c and jgiGene.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#JGI curated genes
CREATE TABLE jgiGene (
    bin int unsigned not null,
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# JGI locus tag
    score int unsigned not null,	# 1000
    strand char(1) not null,	# Value should be + or -
    jgiGeneId varchar(255) not null,	# JGI gene ID
    jgiSymbol varchar(255) not null,	# JGI gene symbol
    jgiGc float not null,	# GC content
    jgiDescription varchar(255) not null,	# ORF annotation
              #Indices
    INDEX(bin),
    INDEX(chrom),
    INDEX(chromStart),
    INDEX(chromEnd),
    PRIMARY KEY(name)
);

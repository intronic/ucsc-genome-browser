# bedLogR.sql was originally generated by the autoSql program, which also 
# generated bedLogR.c and bedLogR.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#BED9 +  floating point score
CREATE TABLE bedLogR (
    chrom varchar(255) not null,	# Chromosome (or contig, scaffold, etc.)
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    thickStart int unsigned not null,	# Start of where display should be thick (start codon)
    thickEnd int unsigned not null,	# End of where display should be thick (stop codon)
    reserved int unsigned not null,	# Used as itemRgb as of 2004-11-22
    logR float not null,	# logR value
              #Indices
    INDEX(bin),
    INDEX(chrom(16),chromStart),
    INDEX(name(16))
);

# cgh.sql was originally generated by the autoSql program, which also 
# generated cgh.c and cgh.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Comparative Genomic Hybridization data assembly position information
CREATE TABLE cgh (
    chrom varchar(255) not null,	# Chromosome name
    chromStart int unsigned not null,	# position in nucleotides where feature starts on chromosome
    chromEnd int unsigned not null,	# position in nucleotides where featrure stops on chromsome
    name varchar(255) not null,	# Name of the cell line (type 3 only)
    score float not null,	# hybridization score
    type int unsigned not null,	# 1 - overall average, 2 - tissue average, 3 - single tissue
    tissue varchar(255) not null,	# Type of tissue cell line derived from (type 2 and type 3)
    clone varchar(255) not null,	# Name of clone
    spot int unsigned not null,	# Spot number on array
              #Indices
    INDEX(chrom)
    INDEX(chromStart)
    INDEX(type)
    INDEX(tissue)
);

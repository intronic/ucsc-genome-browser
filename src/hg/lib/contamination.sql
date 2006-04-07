# contamination.sql was originally generated by the autoSql program, which also 
# generated contamination.c and contamination.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Contamination in assembly (bed 4 +)
CREATE TABLE contamination (
    chrom varchar(255) not null,	# chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name (type of contamination: E.coli, vector etc)
    acc varchar(255) not null,	# NCBI accession where contamination was found
    ctg varchar(255) not null,	# Alternate name (e.g. WGS contig name)
    len int not null,	# Length of contaminated portion of sequence
              #Indices
    INDEX(chrom(12),chromStart)
);

# cnpRedon.sql was originally generated by the autoSql program, which also 
# generated cnpRedon.c and cnpRedon.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#CNP data from Redon lab
CREATE TABLE cnpRedon (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chrom
    chromEnd int unsigned not null,	# End position in chrom
    name varchar(255) not null,	# Reference SNP identifier or Affy SNP name
    score float not null,	# Score
    strand char(1) not null,	# Strand
              #Indices
    PRIMARY KEY(chrom)
);

# snpTmp.sql was originally generated by the autoSql program, which also 
# generated snpTmp.c and snpTmp.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Polymorphism data subset used during processing
CREATE TABLE snpTmp (
    chrom varchar(255) not null,	# Chromosome
    chromStart int unsigned not null,	# Start position in chrom
    chromEnd int unsigned not null,	# End position in chrom
    name varchar(255) not null,	# Reference SNP identifier or Affy SNP name
    strand char(1) not null,	# Which DNA strand contains the observed alleles
    refNCBI longblob not null,	# Reference genomic from dbSNP
    locType enum ('unknown', 'range', 'exact', 'between',
                  'rangeInsertion', 'rangeSubstitution', 'rangeDeletion') 
		  DEFAULT 'unknown' NOT NULL, # locType
    func       set( 'unknown', 'locus', 'coding', 'coding-synon', 'coding-nonsynon', 
    		     'untranslated', 'intron', 'splice-site', 'cds-reference') 
		     DEFAULT 'unknown' NOT NULL,
    			# The functional category of the SNP
    contigName varchar(255) not null,   # Contig name
              #Indices
    PRIMARY KEY(name)
);

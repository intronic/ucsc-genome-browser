# snp125.sql was originally generated by the autoSql program, which also 
# generated snp125.c and snp125.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Polymorphism data from dbSnp database or genotyping arrays
CREATE TABLE snp125 (
    bin           smallint(5) unsigned not null,

    chrom      enum(
                   'chr1',  'chr1_random',
                   'chr2',  'chr2_random',
                   'chr3',  'chr3_random',
                   'chr4',  'chr4_random',
                   'chr5',  'chr5_random',
                   'chr6',  'chr6_random',
                   'chr7',  'chr7_random',
                   'chr8',  'chr8_random',
                   'chr9',  'chr9_random',
                   'chr10', 'chr10_random',
                   'chr11', 'chr11_random',
                   'chr12', 'chr12_random',
                   'chr13', 'chr13_random',
                   'chr14', 'chr14_random',
                   'chr15', 'chr15_random',
                   'chr16', 'chr16_random',
                   'chr17', 'chr17_random',
                   'chr18', 'chr18_random',
                   'chr19', 'chr19_random',
                   'chr20', 'chr20_random',
                   'chr21', 'chr21_random',
                   'chr22', 'chr22_random',
                   'chrX',  'chrY'
               ) ,

    chromStart    int unsigned not null,	# Start position in chrom
    chromEnd      int unsigned not null,	# End position in chrom
    name          varchar(255) not null,	# Reference SNP identifier or Affy SNP name
    score         int unsigned not null,	# Not used
    strand        char(1) not null,	# Which DNA strand contains the observed alleles
    refNCBI     blob not null,	# Reference genomic
    refUCSC     blob not null,	# Reference genomic
    observed      blob not null,	# The sequences of the observed alleles
    molType       enum( 'unknown', 'genomic', 'cDNA', 'mito', 'chloro') DEFAULT 'unknown' not null,
    					# Sample type from exemplar ss
    class         enum('unknown', 'single', 'in-del', 'het', 'microsatelite',
                  'named', 'no var', 'mixed', 'mnp', 'insertion', 'deletion')  DEFAULT 'unknown' NOT NULL,
    					# The class of variant

    valid      	set('unknown', 'by-cluster', 'by-frequency', 'by-submitter', 'by-2hit-2allele', 'by-hapmap') 
                                        DEFAULT 'unknown' NOT NULL,
    					# The validation status of the SNP

    avHet 	float not null,	# The average heterozygosity from all observations
    avHetSE 	float not null,	# The Standard Error for the average heterozygosity

    func       enum( 'unknown', 'locus-region', 'coding', 'coding-synon', 'coding-nonsynon', 
    			'mrna-utr', 'untranslated', 'intron', 'splice-site', 'cds-reference') 
			DEFAULT 'unknown' NOT NULL,

    				# The functional category of the SNP

   locType  enum( 'unknown', 'range', 'exact', 'between', 
   		  'rangeInsertion', 'rangeSubstitution', 'rangeDeletion') DEFAULT 'unknown' NOT NULL,

    source enum ('dbSNP125', 'Affy500k')	# Source of the data - dbSnp, Affymetrix, ...

);

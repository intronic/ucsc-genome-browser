# snp.sql was originally generated by the autoSql program, which also 
# generated snp.c and snp.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Single nucleotide polymorphisms
CREATE TABLE snpNih (
    bin        smallint unsigned not null, 
    chrom      varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd   int unsigned not null,	# End position in chromosome
    name       varchar(255) not null,	# Name of SNP
              #Indices
    INDEX(chrom(12),bin),
    INDEX(chrom(12),chromStart),
    INDEX(chrom(12),chromEnd),
    INDEX(name)
);

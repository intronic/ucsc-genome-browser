CREATE TABLE snpArrayIllumina300 (
    bin           smallint(5) unsigned not null,
    chrom         varchar(15) not null,			# Chromosome
    chromStart    int(10) unsigned not null,		# Start position in chrom
    chromEnd      int(10) unsigned not null,		# End position in chrom
    name          varchar(15) not null			# Reference SNP identifier 
);

# dbRIP.sql was originally generated by the autoSql program, which also 
# generated dbRIP.c and dbRIP.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Retrotransposon Insertion Polymorphisms in Humans
CREATE TABLE dbRIP (
    bin smallint not null,		# bin for browser speed up
    chrom varchar(255) not null,	# Chromosome (or contig, scaffold, etc.)
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Database ID
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    originalId varchar(255) not null,	# original ID
    forwardPrimer varchar(255) not null,	# forward Primer
    reversePrimer varchar(255) not null,	# reverse Primer
    polyClass varchar(255) not null,	# Repeat Class
    polyFamily varchar(255) not null,	# Repeat Family
    polySubfamily varchar(255) not null,	# Repeat Sub-family
    polySeq longblob not null,	# sequence
    polySource varchar(255) not null,	# source
    reference longblob not null,	# reference
    ascertainingMethod varchar(255) not null,	# Ascertaining Method
    remarks varchar(255) not null,	# remarks
    tm float not null,	# temperature
    filledSize int not null,	# filled size
    emptySize int not null,	# empty size
    disease varchar(255) not null,	# disease
    genoRegion varchar(255) not null,	# genome region
              #Indices
    INDEX(name(20)),
    INDEX(chrom(5),bin)
);

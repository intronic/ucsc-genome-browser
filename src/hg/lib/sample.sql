# sample.sql was originally generated by the autoSql program, which also 
# generated sample.c and sample.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Any track that has samples to display as y-values (first 6 fields are bed6)
CREATE TABLE sample (
    chrom varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    strand char(2) not null,	# # + or -
    sampleCount int unsigned not null,	# number of samples total
    samplePosition longblob not null,	# bases relative to chromStart (x-values)
    sampleHeight longblob not null,	# the height each pixel is drawn to [0,1000]
              #Indices
    PRIMARY KEY(chrom)
);

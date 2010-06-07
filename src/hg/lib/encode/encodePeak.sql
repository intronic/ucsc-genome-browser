# encodePeak.sql was originally generated by the autoSql program, which also 
# generated encodePeak.c and encodePeak.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Peaks format (BED 6+)
CREATE TABLE encodePeak (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# The name... probably just a period
    score int unsigned not null,	# Score 1-1000
    strand char(2) not null,	# +, -, or .
    signalValue float not null,	# Measurement of average enrichment for the region
    pValue float not null,	# Statistical significance of signal value (-log10)
    qValue float not null,	# Statistical significance normalized/corrected across datasets. Use -1 if no qValue is assigned.
    peak int not null,	# Point source called for this peak
    blockCount int unsigned not null,	# Number of blocks
    blockSizes longblob not null,	# Comma separated list of block sizes
    blockStarts longblob not null,	# Start positions relative to chromStart
              #Indices
    INDEX chromStart (chrom(8), chromStart)
);

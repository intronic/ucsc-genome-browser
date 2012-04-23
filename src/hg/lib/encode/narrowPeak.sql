# narrowPeak.sql was originally generated by the autoSql program, which also
# generated narrowPeak.c and narrowPeak.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly
# automatic way.

#peaks of signal enrichment based on pooled, normalized (interpreted) data. It is a BED6+4 format
CREATE TABLE narrowPeak (
    chrom varchar(255) not null,	# Name of the chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Optional. Name given to a region (preferably unique). Use . if no name is assigned.
    score int unsigned not null,	# Optional. Indicates how dark the peak will be displayed in the browser (1-1000). If '0', the DCC will assign this based on signal value. Ideally average signalValue per base spread between 100-1000.
    strand char(1) not null,	# Optional. +/- to denote strand or orientation (whenever applicable). Use '.' if no orientation is assigned.
    signalValue float not null,	# Measurement of average enrichment for the region
    pValue float not null default -1,	# Within dataset statistical significance of signal value (-log10)
    qValue float not null default -1,	# Cross-dataset statistical significance of signal value (-log10)
    peak int not null default -1,	# Point-source called for this peak. 0-based offset from chromStart (use -1 if no point-source called)
              #Indices
    INDEX chromStart (chrom(8), chromStart)
);

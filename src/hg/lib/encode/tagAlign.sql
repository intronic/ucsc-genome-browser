# tagAlign.sql was originally generated by the autoSql program, which also 
# generated tagAlign.c and tagAlign.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Tag Alignment format (BED 3+)
CREATE TABLE tagAlign (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    sequence varchar(255) not null,	# Sequence of this read
    score int unsigned not null,	# Indicates mismatches, quality, or other measurement (0-1000)
    strand char(1) not null,	        # Orientation of this read (+ or -)
              
#Indices
    INDEX chromStart (chrom(8), chromStart)
)
AVG_ROW_LENGTH = 100
MAX_ROWS = 10000000000
;

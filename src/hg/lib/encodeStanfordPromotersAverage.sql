# encodeStanfordPromotersAverage.sql was originally generated by the autoSql program, which also 
# generated encodeStanfordPromotersAverage.c and encodeStanfordPromotersAverage.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Stanford Promoter Activity in ENCODE Regions, average over all cell types (bed 9+)
CREATE TABLE encodeStanfordPromotersAverage (
    chrom varchar(255) not null,	# chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# accession of mRNA used to predict the promoter
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    thickStart int unsigned not null,	# Placeholder for BED9 format -- same as chromStart
    thickEnd int unsigned not null,	# Placeholder for BED9 format -- same as chromEnd
    reserved int unsigned not null,	# Used as itemRgb
    geneModel varchar(255) not null,	# Gene model ID. same ID may have multiple predicted promoters
    description varchar(255) not null,	# Gene description
    normLog2Ratio float not null,	# Normalized and log2 transformed Luciferase Renilla Ratio
              #Indices
    INDEX(chrom(12),chromStart),
    INDEX(name(12))
);

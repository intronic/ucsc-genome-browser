# recombRateMouse.sql was originally generated by the autoSql program, which also 
# generated recombRateMouse.c and recombRateMouse.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Describes the recombination rate in 5Mb intervals based on genetic maps
CREATE TABLE recombRateMouse (
    chrom varchar(255) not null,	# chromosome number
    chromStart int unsigned not null,	# Start position in genoSeq
    chromEnd int unsigned not null,	# End position in genoSeq
    name varchar(255) not null,	# Constant string recombRate
    wiAvg float not null,	# Calculated WI genetic map recombination rate
    mgdAvg float not null,	# Calculated MGD genetic map recombination rate
              #Indices
    INDEX(chrom(8),chromStart),
    INDEX(chrom(8),chromEnd)
);

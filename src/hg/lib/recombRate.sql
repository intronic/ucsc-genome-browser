# recombRate.sql was originally generated by the autoSql program, which also 
# generated recombRate.c and recombRate.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Describes the recombination rate in 1Mb intervals based on deCODE map
CREATE TABLE recombRate (
    chrom varchar(255) not null,	# Human chromosome number
    chromStart int unsigned not null,	# Start position in genoSeq
    chromEnd int unsigned not null,	# End position in genoSeq
    name varchar(255) not null,	# Constant string recombRate
    recombRate float not null,	# Calculated recombination rate for this interval
    femaleRate float not null,	# Calculated female recombination rate for this interval
    maleRate float not null,	# Calculated male recombination rate for this interval
              #Indices
    INDEX(chrom(8),chromStart),
    INDEX(chrom(8),chromEnd)
);

# simpleRepeat.sql was originally generated by the autoSql program, which also 
# generated simpleRepeat.c and simpleRepeat.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Describes the Simple Tandem Repeats
CREATE TABLE simpleRepeat (
    chrom varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Simple Repeats tag name
    period int unsigned not null,	# Length of repeat unit
    copyNum float not null,	# Mean number of copies of repeat
    consensusSize int unsigned not null,	# Length of consensus sequence
    perMatch int unsigned not null,	# Percentage Match
    perIndel int unsigned not null,	# Percentage Indel
    score int unsigned not null,	# Score between  and .  Best is .
    A int unsigned not null,	# Number of A's in repeat unit
    C int unsigned not null,	# Number of C's in repeat unit
    T int unsigned not null,	# Number of T's in repeat unit
    G int unsigned not null,	# Number of G's in repeat unit
    entropy float not null,	# Entropy
    sequence longblob not null,	# Sequence of repeat unit element
              #Indices
    INDEX(chrom(12),chromStart),
    INDEX(chrom(12),chromEnd)
);

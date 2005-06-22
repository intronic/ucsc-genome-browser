# bed5FloatScore.sql was originally generated by the autoSql program, which also 
# generated bed5FloatScore.c and bed5FloatScore.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#BED 5 (with 0-1000 score), but also with floating-point score.
CREATE TABLE bed5FloatScore (
    bin smallint not null,      # Index field
    chrom varchar(255) not null,	# Chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int not null,	# 0-1000 score for useScore shading
    floatScore float not null,	# Floating point score.
              #Indices
    INDEX(chrom(16),bin),
    INDEX(chrom(16),chromStart),
    INDEX(name(16))
);

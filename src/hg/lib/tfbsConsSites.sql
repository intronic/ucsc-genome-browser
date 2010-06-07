# tfbsConsSites.sql was originally generated by the autoSql program, which also 
# generated tfbsConsSites.c and tfbsConsSites.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#tfbsConsSites Data
CREATE TABLE tfbsConsSites (
    bin smallint not null,	# bin for browser speed up
    chrom varchar(255) not null,	# Human chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    zScore float not null,	# zScore
              #Indices
    INDEX(name(10)),
    INDEX(chrom(12),bin)
);

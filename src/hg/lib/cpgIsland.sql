# cpgIsland.sql was originally generated by the autoSql program, which also 
# generated cpgIsland.c and cpgIsland.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Describes the CpG Islands
CREATE TABLE cpgIsland (
    chrom varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# CpG Island
    length int unsigned not null,	# Island Length
    cpgNum int unsigned not null,	# Number of CpGs in island
    gcNum int unsigned not null,	# Number of C and G in island
    perCpg float not null,	# Percentage of island that is CpG
    perGc float not null,	# Percentage of island that is C or G
              #Indices
    INDEX(chrom(12),chromStart)
);

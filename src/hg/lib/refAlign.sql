# refAlign.sql was originally generated by the autoSql program, which also 
# generated refAlign.c and refAlign.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Contains a region of a reference alignment
CREATE TABLE refAlign (
    chrom varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    matches int unsigned not null,	# Number of bases that match
    misMatches int unsigned not null,	# Number of bases that don't match
    hNumInsert int unsigned not null,	# Number of inserts in human
    hBaseInsert int not null,	# Number of bases inserted in human
    aNumInsert int unsigned not null,	# Number of inserts in aligned seq
    aBaseInsert int not null,	# Number of bases inserted in query
    humanSeq longblob not null,	# Human sequence, contains - for aligned seq inserts
    alignSeq longblob not null,	# Aligned sequence, contains - for human seq inserts
    attribs varchar(255) not null,	# Comma seperated list of attribute names
              #Indices
    PRIMARY KEY(chrom)
);

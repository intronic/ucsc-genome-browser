# jaxQTL3.sql was originally generated by the autoSql program, which also 
# generated jaxQTL3.c and jaxQTL3.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Quantitative Trait Loci from Jackson Labs / Mouse Genome Informatics
CREATE TABLE jaxQTL3 (
    chrom varchar(255) not null,	# chromosome
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000 (bed6 compat.)
    strand char(1) not null,	# + or - (bed6 compat.)
    thickStart int unsigned not null,	# start of thick region
    thickEnd int unsigned not null,	# start of thick region
    marker varchar(255) not null,	# MIT SSLP Marker w/highest correlation
    mgiID varchar(255) not null,	# MGI ID
    description varchar(255) not null,	# MGI description
    cMscore float not null,	# cM position of marker associated with peak LOD score
    flank1 varchar(255) not null,	# flanking marker 1
    flank2 varchar(255) not null,	# flanking marker 2
              #Indices
    PRIMARY KEY(name),
    KEY(mgiID(16)),
    KEY(marker(16)),
    KEY(flank1(16)),
    KEY(flank2(16))
);

# clonePos.sql was originally generated by the autoSql program, which also 
# generated clonePos.c and clonePos.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#A clone's position and other info.
CREATE TABLE clonePos (
    name varchar(255) not null,	# Name of clone including version
    seqSize int unsigned not null,	# base count not including gaps
    phase tinyint unsigned not null,	# htg phase
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start in chromosome
    chromEnd int unsigned not null,	# End in chromosome
    stage char(1) not null,	# F/D/P for finished/draft/predraft
    faFile varchar(255) not null,	# File with sequence.
              #Indices
    PRIMARY KEY(name(12)),
    INDEX(chrom(12),chromStart)
);

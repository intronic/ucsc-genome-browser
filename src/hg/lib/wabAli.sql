# wabAli.sql was originally generated by the autoSql program, which also 
# generated wabAli.c and wabAli.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Information on a waba alignment
CREATE TABLE wabAli (
    query varchar(255) not null,	# Foreign sequence name.
    qStart int unsigned not null,	# Start in query (other species)
    qEnd int unsigned not null,	# End in query.
    strand char(1) not null,	# + or - relative orientation.
    chrom varchar(255) not null,	# Chromosome (current species).
    chromStart int unsigned not null,	# Start in chromosome.
    chromEnd int unsigned not null,	# End in chromosome.
    milliScore int unsigned not null,	# Base identity in parts per thousand.
    symCount int unsigned not null,	# Number of symbols in alignment.
    qSym longblob not null,	# Query bases plus '-'s.
    tSym longblob not null,	# Target bases plus '-'s.
    hSym longblob not null,	# Hidden Markov symbols.
              #Indices
    PRIMARY KEY(query)
);

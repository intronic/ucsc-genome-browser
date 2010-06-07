# pslWQueryID.sql was originally generated by the autoSql program, which also 
# generated pslWQueryID.c and pslWQueryID.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Summary info about a patSpace alignment with a query ID addition
CREATE TABLE pslWQueryID (
    matches int unsigned not null,	# Number of bases that match that aren't repeats
    misMatches int unsigned not null,	# Number of bases that don't match
    repMatches int unsigned not null,	# Number of bases that match but are part of repeats
    nCount int unsigned not null,	# Number of 'N' bases
    qNumInsert int unsigned not null,	# Number of inserts in query
    qBaseInsert int unsigned not null,	# Number of bases inserted in query
    tNumInsert int unsigned not null,	# Number of inserts in target
    tBaseInsert int unsigned not null,	# Number of bases inserted in target
    strand char(2) not null,	# + or - for query strand. For mouse second +/- for genomic strand
    qName varchar(255) not null,	# Query sequence name
    qSize int unsigned not null,	# Query sequence size
    qStart int unsigned not null,	# Alignment start position in query
    qEnd int unsigned not null,	# Alignment end position in query
    tName varchar(255) not null,	# Target sequence name
    tSize int unsigned not null,	# Target sequence size
    tStart int unsigned not null,	# Alignment start position in target
    tEnd int unsigned not null,	# Alignment end position in target
    blockCount int unsigned not null,	# Number of blocks in alignment
    blockSizes longblob not null,	# Size of each block
    qStarts longblob not null,	# Start of each block in query.
    tStarts longblob not null,	# Start of each block in target.
    queryID varchar(255) not null,	# query ID field
    index(tName(32),tStart,tEnd)
);

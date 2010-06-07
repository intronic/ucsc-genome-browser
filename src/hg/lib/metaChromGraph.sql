# metaChromGraph.sql was originally generated by the autoSql program, which also 
# generated metaChromGraph.c and metaChromGraph.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Information about each chromosome graph.
CREATE TABLE metaChromGraph (
    name varchar(255) not null,	# Corresponds to chrom graph table name
    minVal double not null,	# Minimum value observed
    maxVal double not null,	# Maximum value observed
    binaryFile varchar(255) not null,	# Location of binary data point file if any
              #Indices
    PRIMARY KEY(name(32))
);

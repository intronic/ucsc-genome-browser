# stsAlias.sql was originally generated by the autoSql program, which also 
# generated stsAlias.c and stsAlias.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#List of aliases for STS files
CREATE TABLE stsAlias (
    alias varchar(255) not null,	# This specific alias
    trueName varchar(255) not null,	# True in our stsMarker table
              #Indices
    INDEX(alias(12)),
    INDEX(trueName(12))
);

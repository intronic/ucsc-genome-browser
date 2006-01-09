# sangerLinks.sql was originally generated by the autoSql program, which also 
# generated sangerLinks.c and sangerLinks.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#relating sanger ORF name to Swiss Prot ID with description
CREATE TABLE sangerLinks (
    orfName varchar(255) not null,	# WormBase ORF Name
    protName varchar(255) not null,	# Swiss Prot ID
    description longblob not null,	# Freeform (except for no tabs) description
              #Indices
    PRIMARY KEY(orfName),
    INDEX(protName)
);

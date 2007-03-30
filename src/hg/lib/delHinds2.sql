# delHinds2.sql was originally generated by the autoSql program, which also 
# generated delHinds2.c and delHinds2.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Deletions from Hinds with frequency
CREATE TABLE delHinds2 (
    bin smallint unsigned not null,     # For browser speed-up
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chrom
    chromEnd int unsigned not null,	# End position in chrom
    name varchar(255) not null,		# Name
    frequency float not null		# Frequency
);

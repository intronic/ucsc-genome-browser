# variome.sql was originally generated by the autoSql program, which also 
# generated variome.c and variome.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#variome bed 6+ structure
CREATE TABLE variome (
    bin smallint unsigned not null,	# used for efficient position indexing
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    db varchar(36) not null,	# database for item
    owner varchar(255) not null,	# creator of item
    color varchar(255) not null,	# rgb color of item (currently unused)
    class varchar(255) not null,	# classification of item (browser group)
    creationDate varchar(255) not null,	# date item created
    lastModifiedDate varchar(255) not null,	# date item last updated
    descriptionKey varchar(255) not null,	# name of wiki description page
    id int unsigned not null auto_increment,	# auto-increment item ID
    geneSymbol varchar(255) not null,	# knownGene kgXref geneSymbol name
              #Indices
    PRIMARY KEY(id),
    INDEX chrom (db,bin,chrom),
    INDEX name (db,name),
    INDEX gene (geneSymbol)
);

# bgiGeneInfo.sql was originally generated by the autoSql program, which also 
# generated bgiGeneInfo.c and bgiGeneInfo.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Beijing Genomics Institute Genes track additional information
CREATE TABLE bgiGeneInfo (
    name varchar(255) not null,	# BGI annotation gene name
    source varchar(255) not null,	# Source of gene annotation
    go longblob not null,	# comma-sep list of GO numeric IDs
    ipr longblob not null,	# semicolon-sep list of IPR numeric IDs and comments
    snp longblob not null,	# comma-sep list of BGI SNP IDs
              #Indices
    PRIMARY KEY(name(32))
);

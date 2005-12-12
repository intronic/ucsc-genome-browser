# lsSnpStructure.sql was originally generated by the autoSql program, which also 
# generated lsSnpStructure.c and lsSnpStructure.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Structural Polymorphisms modeled in LS-SNP
CREATE TABLE lsSnpStructure (
    uniprotId varchar(255) not null,	# Uniprot ID
    rsId varchar(255) not null,	# dbSnp Reference Snp ID (rsId)
    INDEX rsId (rsId)
);

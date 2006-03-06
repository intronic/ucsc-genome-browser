# ensXRefZfish.sql was originally generated by the autoSql program, which also 
# generated ensXRefZfish.c and ensXRefZfish.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Link from an Ensembl Transcript ID to other database IDs and description.
CREATE TABLE ensXRefZfish (
    ensGeneId varchar(255) not null,	# Ensembl Transcript ID
    zfinId varchar(255) not null,	# ZFIN ID
    uniProtId varchar(255) not null,	# Unified UniProt protein accession
    geneId varchar(255) not null,	# NCBI Entrez Gene (formerly LocusLink) ID
    geneSymbol varchar(255) not null,	# NCBI Entrez Gene (formerly LocusLink) Symbol
    refSeq varchar(255) not null,	# RefSeq DNA Accession
    protAcc varchar(255) not null,	# RefSeq Protein Accession
    description varchar(255) not null,	# Description
              #Indices
    PRIMARY KEY(ensGeneId),
    KEY(zfinId), 
    KEY(uniProtId),
    KEY(geneId),
    KEY(geneSymbol),
    KEY(refSeq),
    KEY(protAcc)
);

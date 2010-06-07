# goa.sql was originally generated by the autoSql program, which also 
# generated goa.c and goa.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#GO Association. See http://www.geneontology.org/doc/GO.annotation.html#file
CREATE TABLE goa (
    db varchar(255) not null,	# Database - SPTR for SwissProt
    dbObjectId varchar(255) not null,	# Database accession - like 'Q13448'
    dbObjectSymbol varchar(255) not null,	# Name - like 'CIA1_HUMAN'
    notId varchar(255) not null,	# (Optional) If 'NOT'. Indicates object isn't goId
    goId varchar(255) not null,	# GO ID - like 'GO:0015888'
    dbReference varchar(255) not null,	# something like SGD:8789|PMID:2676709
    evidence varchar(255) not null,	# Evidence for association.  Somthing like 'IMP'
    withFrom varchar(255) not null,	# (Optional) Database support for evidence I think
    aspect varchar(255) not null,	#  P (process), F (function) or C (cellular component)
    dbObjectName varchar(255) not null,	# (Optional) multi-word name of gene or product
    synonym varchar(255) not null,	# (Optional) field for gene symbol, often like IPI00003084
    dbObjectType varchar(255) not null,	# Either gene, transcript, protein, or protein_structure
    taxon varchar(255) not null,	# Species (sometimes multiple) in form taxon:9606
    date varchar(255) not null,	# Date annotation made in YYYYMMDD format
    assignedBy varchar(255) not null,	# Database that made the annotation. Like 'SPTR'
              #Indices
    INDEX(dbObjectSymbol(8)),
    INDEX(goId(10))
);

# hgncXref.sql was originally generated by the autoSql program, which also 
# generated hgncXref.c and hgncXref.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#A cross-reference table between HUGO Gene Nomenclature Committee (HGNC) IDs and other database IDs.
CREATE TABLE hgncXref (
    symbol varchar(40),	# Approved Symbol
    refSeq varchar(40),	# RefSeq ID
    uniProt varchar(40),	# UniProt ID (mapped data)
    hgncId varchar(40),	# HGNC ID
    entrez varchar(40),	# ENTREZ ID
    description varchar(255),		# Approved Name
              #Indices
    KEY(symbol),
    KEY(refSeq),
    KEY(uniProt),
    KEY(hgncId),
    KEY(entrez)
);

# refLink.sql was originally generated by the autoSql program, which also 
# generated refLink.c and refLink.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Link together a refseq mRNA and other stuff
CREATE TABLE refLink (
    name varchar(255) not null,	# Name displayed in UI
    product varchar(255) not null,	# Name of protein product
    mrnaAcc varchar(255) not null,	# mRNA accession
    protAcc varchar(255) not null,	# protein accession
    geneId int unsigned not null,	# pointer to geneName table
    prodId int unsigned not null,	# pointer to prodName table
    locusLinkId int unsigned not null,	# Locus Link ID
    omimId int unsigned not null,	# OMIM ID
              #Indices
    PRIMARY KEY(mrnaAcc(12)),
    index(name(10)),
    index(protAcc(10)),
    index(locusLinkId),
    index(omimId),
    index(prodId),
    index(geneId)
);

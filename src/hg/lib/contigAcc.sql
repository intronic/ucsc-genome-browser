# contigAcc.sql was originally generated by the autoSql program, which also 
# generated contigAcc.c and contigAcc.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Map a contig to its accession.
CREATE TABLE contigAcc (
    contig varchar(255) not null,	# Contig name
    acc varchar(255) not null,	# Genbank accession
              #Indices
    PRIMARY KEY(contig),
    INDEX (name(16))
);

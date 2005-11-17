# kgXref.sql was originally generated by the autoSql program, which also 
# generated kgXref.c and kgXref.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Link together a Known Gene ID and a gene alias
CREATE TABLE refRa (
    acc  varchar(20) not null,	# RefSeq ID
    attr varchar(20),		# attribute type
    val  blob,		# attribute value
              #Indices
    KEY(acc)
);

# yaleGencodeAssoc.sql was originally generated by the autoSql program, which also 
# generated yaleGencodeAssoc.c and yaleGencodeAssoc.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#association of Yale pseudogenes with Gencode transcripts
CREATE TABLE yaleGencodeAssoc (
    transcript varchar(255) not null,	# Gencode transcript id
    yaleId varchar(255) not null,	# Yale id
    locus varchar(255) not null,	# Gencode locus id
              #Indices
    INDEX(transcript)
);

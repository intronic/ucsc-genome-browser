# affy10K.sql was originally generated by the autoSql program, which also 
# generated affy10K.c and affy10K.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Information from affy10K representing the Affymetrix 10K Mapping array
CREATE TABLE affy10K (
    chrom varchar(255) not null,	#  
    chromStart int unsigned not null,	#  
    chromEnd int unsigned not null,	#  
    affyId int unsigned not null,	# Affymetrix SNP id
              #Indices
    PRIMARY KEY(chrom)
);

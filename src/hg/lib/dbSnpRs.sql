# dbSnpRs.sql was originally generated by the autoSql program, which also 
# generated dbSnpRs.c and dbSnpRs.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Information from dbSNP at the reference SNP level
CREATE TABLE dbSnpRs (
    rsId       varchar(255) not null,	# dbSnp reference snp (rs) identifier
    avHet      float        not null,	# the average heterozygosity from all observations
    avHetSE    float        not null,	# the Standard Error for the average heterozygosity
    valid      varchar(255) not null,	# the validation status of the SNP
    allele1    varchar(255) not null,	# the sequence of the first allele
    allele2    varchar(255) not null,	# the sequence of the second allele
    assembly   varchar(255) not null,	# the sequence in the assembly
    alternate  varchar(255) not null,	# the sequence of the alternate allele
              #Indices
    PRIMARY KEY(rsId)
);

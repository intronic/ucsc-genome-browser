# tigrCmrGene.sql was originally generated by the autoSql program, which also 
# generated tigrCmrGene.c and tigrCmrGene.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#For TIGR CMR genes tracks
CREATE TABLE tigrCmrGene (
    tigrLocus varchar(255) not null,	# TIGR locus
    tigrCommon longblob not null,	# TIGR Common Name
    tigrGene varchar(255) not null,	# TIGR Gene Symbol
    tigrECN varchar(255) not null,	# TIGR Enzyme Commission Number
    primLocus varchar(255) not null,	# Primary Locus Name
    tigr5p int unsigned not null,	# TIGR 5' end
    tigr3p int unsigned not null,	# TIGR 3' end
    tigrLength int unsigned not null,	# TIGR sequence length
    tigrPepLength int unsigned not null,	# TIGR Protein length
    tigrMainRole longblob not null,	# TIGR Main Role
    tigrSubRole longblob not null,	# TIGR Sub Role
    swissProt varchar(255) not null,	# SwissProt TrEMBL Accession
    genbank varchar(255) not null,	# Genbank ID
    tigrMw float not null,	# Molecular Weight
    tigrPi float not null,	# Isoelectric point (I think)
    tigrGc float not null,	# GC content
              #Indices
    PRIMARY KEY(tigrLocus)
);

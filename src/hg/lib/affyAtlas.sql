# affyAtlas.sql was originally generated by the autoSql program, which also 
# generated affyAtlas.c and affyAtlas.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#analysis information from Affymetrix human atlas data
CREATE TABLE affyAtlas (
    annName varchar(255) not null,	# analysis name, array name?
    probeSet varchar(255) not null,	# probe set that signal corresponds to
    signal float not null,	# signal of probeset detected
    detection char(2) not null,	# not sure...
    pval float not null,	# p-value
    tissue varchar(255) not null,	# tissue sample comes from
              #Indices
    PRIMARY KEY(annName)
);

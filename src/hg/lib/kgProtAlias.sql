# kgProtAlias.sql was originally generated by the autoSql program, which also 
# generated kgProtAlias.c and kgProtAlias.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Link together a Known Gene ID and a protein alias
CREATE TABLE kgProtAlias (
    kgID varchar(255) not null,	# Known Gene ID
    displayID varchar(255) not null,	# protein display ID
    alias varchar(255) not null,	# a protein alias
              #Indices
    KEY(kgID),
    KEY(displayID),
    KEY(alias)
)TYPE=MyISAM;;

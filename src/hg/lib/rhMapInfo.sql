# rhMapInfo.sql was originally generated by the autoSql program, which also 
# generated rhMapInfo.c and rhMapInfo.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Radiation Hybrid map information
CREATE TABLE rhMapInfo (
    name varchar(255) not null,	# Name of Radiation Hybrid (RH) map marker
    zfinId varchar(255) not null,	# ZFIN ID for the marker
    linkageGp varchar(255) not null,	# Linkage group to which the marker was mapped
    position int unsigned not null,	# Position number in RH map for this linkage group
    distance int unsigned not null,	# Distance from the top of linkage group (cR)
    markerType varchar(255) not null,	# Type of marker
    source varchar(255) not null,	# Source of marker
    mapSite varchar(255) not null,	# Institution that mapped the marker
    leftPrimer varchar(255) not null,	# Forward primer sequence
    rightPrimer varchar(255) not null,	# Reverse primer sequence
              #Indices
    PRIMARY KEY(name),
    INDEX(zfinId)
);

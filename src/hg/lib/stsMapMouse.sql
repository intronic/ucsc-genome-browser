# stsMapMouse.sql was originally generated by the autoSql program, which also 
# generated stsMapMouse.c and stsMapMouse.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#STS marker and its position on mouse assembly
CREATE TABLE stsMapMouse (
    chrom varchar(255) not null,	# Chromosome or 'unknown'
    chromStart int not null,	# Start position in chrom - negative 1 if unpositioned
    chromEnd int unsigned not null,	# End position in chrom
    name varchar(255) not null,	# Name of STS marker
    score int unsigned not null,	# Score of a marker = 1000/(# of placements)
    identNo int unsigned not null,	# Marker Identification number of STS
    probeId int unsigned not null,	# Probe Identification number of STS
              #Indices
    INDEX(name),
    INDEX(chrom(8),chromStart),
    INDEX(chrom(8),chromEnd)
);

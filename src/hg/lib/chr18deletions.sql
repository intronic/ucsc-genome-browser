# chr18deletions.sql was originally generated by the autoSql program, which also 
# generated chr18deletions.c and chr18deletions.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Chromosome 18 deletion
CREATE TABLE chr18deletions (
    chrom varchar(255) not null,	# Chromosome or 'unknown'
    chromStart int not null,	# Start position in chrom - negative 1 if unpositioned
    chromEnd int unsigned not null,	# End position in chrom
    name varchar(255) not null,	# Name of STS marker
    score int unsigned not null,	# Score of a marker = 1000/(# of placements)
    strand char(1) not null,	# Strand = or -
    ssCount int unsigned not null,	# Number of small blocks
    smallStarts longblob not null,	# Small start positions
    seCount int unsigned not null,	# Number of small blocks
    smallEnds longblob not null,	# Small end positions
    lsCount int unsigned not null,	# Number of large blocks
    largeStarts longblob not null,	# Large start positions
    leCount int unsigned not null,	# Number of large blocks
    largeEnds longblob not null,	# Large end positions
              #Indices
    INDEX(name),
    INDEX(chrom(8),chromStart),
    INDEX(chrom(8),chromEnd)
);

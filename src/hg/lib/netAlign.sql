# netAlign.sql was originally generated by the autoSql program, which also 
# generated netAlign.c and netAlign.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#alignment portion of net file
CREATE TABLE netAlign (
    type varchar(255) not null,	# fill or gap
    tName varchar(255) not null,	# Human Chrom
    level int unsigned not null,	# level of alignment
    tStart int unsigned not null,	# Start on Human
    tEnd int unsigned not null,	# End on Human
    qName varchar(255) not null,	# Mouse Chromosome
    strand char(1) not null,	# + direction matches - opposite
    qStart int unsigned not null,	# Start on Mouse
    qEnd int unsigned not null,	# End on Mouse
    score int unsigned not null,	# score
    chainId int unsigned not null,	# chain Id
    qOver int unsigned not null,	# overlap with parent gap on query side(inverts)
    qFar int unsigned not null,	# local or tandem
    qDup int unsigned not null,	# two or more copies in query region
    tN int unsigned not null,	# unsequenced bases on target
    qN int unsigned not null,	# unsequenced bases on query
    tR int unsigned not null,	# repeatMasker bases on target
    qR int unsigned not null,	# repeatMasker bases on query
    tNewR int unsigned not null,	# lineage specific repeats on target
    qNewR int unsigned not null,	# lineage specific repeats on query
    tOldR int unsigned not null,	# bases of ancient repeats on target
    qOldR int unsigned not null,	# bases of ancient repeats on query
    tTrf int unsigned not null,	# bases of Tandam repeats on target
    qTrf int unsigned not null,	# bases of Tandam repeats on query
              #Indices
    PRIMARY KEY(type)
);

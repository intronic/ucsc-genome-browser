# netAlign.sql was originally generated by the autoSql program, which also 
# generated netAlign.c and netAlign.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Database representation of a net of alignments.
CREATE TABLE netAlign (
    level int unsigned not null,	# Level of alignment
    tName varchar(255) not null,	# Target chromosome
    tStart int unsigned not null,	# Start on target
    tEnd int unsigned not null,	# End on target
    strand char(1) not null,	# Orientation of query. + or -
    qName varchar(255) not null,	# Query chromosome
    qStart int unsigned not null,	# Start on query
    qEnd int unsigned not null,	# End on query
    chainId int unsigned not null,	# Associated chain Id with alignment details
    ali int unsigned not null,	# Bases in gap-free alignments
    score double not null,	# Score - a number proportional to 100x matching bases
    qOver int not null,	# Overlap with parent gap on query side. -1 for undefined
    qFar int not null,	# Distance from parent gap on query side. -1 for undefined
    qDup int not null,	# Bases with two or more copies in query. -1 for undefined
    type varchar(255) not null,	# Syntenic type: gap/top/syn/nonsyn/inv
    tN int not null,	# Unsequenced bases on target. -1 for undefined
    qN int not null,	# Unsequenced bases on query. -1 for undefined
    tR int not null,	# RepeatMasker bases on target. -1 for undefined
    qR int not null,	# RepeatMasker bases on query. -1 for undefined
    tNewR int not null,	# Lineage specific repeats on target. -1 for undefined
    qNewR int not null,	# Lineage specific repeats on query. -1 for undefined
    tOldR int not null,	# Bases of ancient repeats on target. -1 for undefined
    qOldR int not null,	# Bases of ancient repeats on query. -1 for undefined
    tTrf int not null,	# Bases of Tandam repeats on target. -1 for undefined
    qTrf int not null,	# Bases of Tandam repeats on query. -1 for undefined
              #Indices
    PRIMARY KEY(level)
);

# pseudoGeneLink.sql was originally generated by the autoSql program, which also 
# generated pseudoGeneLink.c and pseudoGeneLink.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#links a gene/pseudogene prediction to an ortholog or paralog.
CREATE TABLE pseudoGeneLink (
    bin smallint not null,              # bin for speedup
    chrom varchar(255) not null,	# Chromosome name for pseudogene
    chromStart int unsigned not null,	# pseudogene alignment start position
    chromEnd int unsigned not null,	# pseudogene alignment end position
    name varchar(255) not null,	# Name of pseudogene
    score int unsigned not null,	# score of pseudogene with gene
    strand varchar(255) not null,	# strand of pseudoegene
    assembly varchar(255) not null,	# assembly for gene
    geneTable varchar(255) not null,	# mysql table of gene
    gene varchar(255) not null,	# Name of gene
    gChrom varchar(255) not null,	# Chromosome name
    gStart int unsigned not null,	# gene alignment start position
    gEnd int unsigned not null,	# gene alignment end position
    gStrand varchar(255) not null,	# strand of gene
    exonCount int unsigned not null,	# # of exons in gene 
    geneOverlap int unsigned not null,	# bases overlapping
    polyA int unsigned not null,	# length of polyA
    polyAstart int not null,	# start of polyA, relative to end of pseudogene
    exonCover int unsigned not null,	# number of exons in Gene covered
    intronCount int unsigned not null,	# number of introns in pseudogene
    bestAliCount int unsigned not null,	# number of good mrnas aligning
    matches int unsigned not null,	# matches + repMatches
    qSize int unsigned not null,	# aligning bases in pseudogene
    qEnd int unsigned not null,	# end of cdna alignment
    tReps int unsigned not null,	# repeats in gene
    qReps int unsigned not null,	# repeats in pseudogene
    overlapDiag int unsigned not null,	# bases on the diagonal to mouse
    coverage int unsigned not null,	# bases on the diagonal to mouse
    label int unsigned not null,	# 1=pseudogene,-1 not pseudogene
    milliBad int unsigned not null,	# milliBad score, pseudogene aligned to genome
    chainId int unsigned not null,	# chain id of gene/pseudogene alignment
    axtScore int not null,	# blastz score, gene mrna aligned to pseudogene
    refSeq varchar(255) not null,	# Name of closest regSeq to gene
    rStart int not null,	# refSeq alignment start position
    rEnd int not null,	# refSeq alignment end position
    mgc varchar(255) not null,	# Name of closest mgc to gene
    mStart int not null,	# mgc alignment start position
    mEnd int not null,	# mgc alignment end position
    kgName varchar(255) not null,	# Name of closest knownGene to gene
    kStart int not null,	# kg alignment start position
    kEnd int not null,	# kg alignment end position
    kgId int not null,	# kg id
              #Indices
    PRIMARY KEY(chrom(8),bin, name(10), chromStart),
    INDEX (name(8))
);

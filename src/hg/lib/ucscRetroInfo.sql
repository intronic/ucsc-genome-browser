# ucscRetroInfo.sql was originally generated by the autoSql program, which also 
# generated ucscRetroInfo.c and ucscRetroInfo.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Retrogenes based on cDNA alignments that are expressed or processed pseudogenes.
CREATE TABLE ucscRetroInfo (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold col1
    chromStart int unsigned not null,	# pseudogene alignment start position col2
    chromEnd int unsigned not null,	# pseudogene alignment end position col3
    name varchar(255) not null,	# Name of pseudogene col4
    score int unsigned not null,	# score of pseudogene with gene col5
    strand char(2) not null,	# + or -
    thickStart int unsigned not null,	# Start of where display should be thick (start codon)
    thickEnd int unsigned not null,	# End of where display should be thick (stop codon)
    reserved int unsigned not null,	# Always zero for now
    blockCount int not null,	# Number of blocks
    blockSizes longblob not null,	# Comma separated list of block sizes
    chromStarts longblob not null,	# Start positions relative to chromStart
    retroExonCount int not null,	# number of exons in retroGene col13
    axtScore int not null,	# blastz score, parent mrna aligned to pseudogene col14
    type varchar(255) not null,	# type of evidence col15
    gChrom varchar(255) not null,	# Chromosome name col16
    gStart int not null,	# gene alignment start position col17
    gEnd int not null,	# gene alignment end position col18
    gStrand char(2) not null,	# strand of gene col19
    parentSpliceCount int unsigned not null,	# # of splice sites in parent gene col20
    geneOverlap int unsigned not null,	# bases overlapping col21
    polyA int unsigned not null,	# count of As in polyA col22
    polyAstart int not null,	# start of polyA, relative to end of pseudogene col23
    exonCover int not null,	# number of exons in Gene covered col24
    intronCount int unsigned not null,	# number of introns in pseudogene col25
    bestAliCount int unsigned not null,	# number of good mrnas aligning col26
    matches int unsigned not null,	# matches + repMatches col27
    qSize int unsigned not null,	# aligning bases in pseudogene col28
    qEnd int unsigned not null,	# end of cdna alignment col29
    tReps int unsigned not null,	# repeats in gene col30
    coverage int unsigned not null,	# % of bases that align to gene col31
    label int not null,	# 1=pseudogene,-1 not pseudogene -2 expressed retroGene col32
    milliBad int unsigned not null,	# milliBad score, pseudogene aligned to genome col33
    oldScore int unsigned not null,	# another heuristic col34
    oldIntronCount int not null,	# old simple intron count col35
    processedIntrons int not null,	# count of introns removed via retrotransposition col36
    conservedSpliceSites int not null,	# conserved splice site count col37
    maxOverlap int not null,	# largest overlap with another mrna col38
    refSeq varchar(255) not null,	# Name of closest regSeq to gene col39
    rStart int not null,	# refSeq alignment start position col40
    rEnd int not null,	# refSeq alignment end position col41
    mgc varchar(255) not null,	# Name of closest mgc to gene col42
    mStart int not null,	# mgc alignment start position col43
    mEnd int not null,	# mgc alignment end position col44
    kgName varchar(255) not null,	# Name of closest knownGene to gene col45
    kStart int not null,	# kg alignment start position col46
    kEnd int not null,	# kg alignment end position col47
    overName varchar(255) not null,	# name of overlapping mrna col48
    overStart int not null,	# overlapping mrna start position col49
    overExonCover int not null,	# count of overlapping mrna exons col50
    overStrand char(2) not null,	# strand of overlapping mrna col51
    posConf float not null,	# pvalue for positive col52
    polyAlen int unsigned not null,	# length of polyA col53
              #Indices
    PRIMARY KEY(name),
    index(kgName(10)),
    index(refSeq(10))
);

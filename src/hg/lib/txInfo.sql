# txInfo.sql was originally generated by the autoSql program, which also 
# generated txInfo.c and txInfo.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Various bits of information about a transcript from the txGraph/txCds system (aka KG3)
CREATE TABLE txInfo (
    name varchar(255) not null,	# Name of transcript
    category varchar(255) not null,	# coding/nearCoding/noncoding for now
    sourceAcc varchar(255) not null,	# Accession of genbank transcript patterned on (may be refSeq)
    isRefSeq tinyint unsigned not null,	# Is a refSeq
    sourceSize int not null,	# Number of bases in source, excluding poly-A tail.
    aliCoverage double not null,	# Fraction of bases in source aligning.
    aliIdRatio double not null,	# matching/total bases in alignment
    genoMapCount int not null,	# Number of times source aligns in genome.
    exonCount int not null,	# Number of exons (excludes gaps from frame shift/stops)
    orfSize int not null,	# Size of ORF
    cdsScore double not null,	# Score of best CDS according to txCdsPredict
    startComplete tinyint unsigned not null,	# Starts with ATG
    endComplete tinyint unsigned not null,	# Ends with stop codon
    nonsenseMediatedDecay tinyint unsigned not null,	# If true, is a nonsense mediated decay candidate.
    retainedIntron tinyint unsigned not null,	# True if has a retained intron compared to overlapping transcripts
    bleedIntoIntron int not null,	# If nonzero number of bases start or end of tx bleeds into intron
    strangeSplice int not null,	# Count of splice sites not gt/ag, gc/ag, or at/ac
    atacIntrons int not null,	# Count of number of at/ac introns
    cdsSingleInIntron tinyint unsigned not null,	# True if CDS is single exon and in intron of other transcript.
    cdsSingleInUtr3 tinyint unsigned not null,	# True if CDS is single exon and in 3' UTR of other transcript.
    selenocysteine tinyint unsigned not null,	# If true TGA codes for selenocysteine
    genomicFrameShift tinyint unsigned not null,	# True if genomic version has frame shift we cut out
    genomicStop tinyint unsigned not null,	# True if genomic version has stop codon we cut out
              #Indices
    INDEX(name(20))
);

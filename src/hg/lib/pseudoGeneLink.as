table pseudoGeneLink
"links a gene/pseudogene prediction to an ortholog or paralog."
    (
    string chrom;	"Chromosome name for pseudogene"
    uint chromStart;	"pseudogene alignment start position"
    uint chromEnd;      "pseudogene alignment end position"
    string name;        "Name of pseudogene"
    uint score;         "score of pseudogene with gene"
    char[2] strand;     "+ or -"
    uint thickStart;    "Start of where display should be thick (start codon)"
    uint thickEnd;      "End of where display should be thick (stop codon)"
    uint reserved;      "Always zero for now"
    int blockCount;     "Number of blocks"
    int[blockCount] blockSizes; "Comma separated list of block sizes"
    int[blockCount] chromStarts; "Start positions relative to chromStart"
    float trfRatio;	"ratio of tandem repeats"
    string type;	"type of evidence"
    int axtScore;       "blastz score, gene mrna aligned to pseudogene"
    string gChrom;	"Chromosome name"
    int gStart;	        "gene alignment start position"
    int gEnd;           "gene alignment end position"
    char[2] gStrand;    "strand of gene"
    uint exonCount;     "# of exons in gene "
    uint geneOverlap;   "bases overlapping"
    uint polyA;         "count of As in polyA"
    int polyAstart;     "start of polyA, relative to end of pseudogene"
    uint exonCover;     "number of exons in Gene covered"
    uint intronCount;   "number of introns in pseudogene"
    uint bestAliCount;  "number of good mrnas aligning"
    uint matches;       "matches + repMatches"
    uint qSize;         "aligning bases in pseudogene"
    uint qEnd;          "end of cdna alignment"
    uint tReps;         "repeats in gene"
    uint qReps;         "repeats in pseudogene"
    uint overlapDiag;   "bases on the diagonal to mouse"
    uint coverage;      "bases on the diagonal to mouse"
    int label;          "1=pseudogene,-1 not pseudogene"
    uint milliBad;      "milliBad score, pseudogene aligned to genome"
    uint oldScore;      "another heuristic"
    int oldIntronCount; "old simple intron count"
    int conservedIntrons; "conserved intron count"
    string intronScores; "Intron sizes in gene/pseudogene"
    int maxOverlap ;    "largest overlap with another mrna"
    string refSeq;	"Name of closest regSeq to gene"
    int rStart;	        "refSeq alignment start position"
    int rEnd;           "refSeq alignment end position"
    string mgc;	        "Name of closest mgc to gene"
    int mStart;	        "mgc alignment start position"
    int mEnd;           "mgc alignment end position"
    string kgName;	"Name of closest knownGene to gene"
    int kStart;	        "kg alignment start position"
    int kEnd;           "kg alignment end position"
    string overName;	"name of overlapping mrna"
    int overStart;	"overlapping mrna start position"
    int overEnd;       "overlapping mrna end position"
    char[2] overStrand;  "strand of overlapping mrna"
    int adaBoost;       "adaBoost label"
    float posConf;      "pvalue for positive"
    uint polyAlen;      "length of polyA"
    )

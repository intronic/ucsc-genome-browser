table pseudoGeneLink
"links a gene/pseudogene prediction to an ortholog or paralog."
    (
    short bin;          "bin for browser speedup"
    string chrom;	"Chromosome name for pseudogene"
    uint chromStart;	"pseudogene alignment start position"
    uint chromEnd;      "pseudogene alignment end position"
    string name;        "Name of pseudogene"
    uint score;         "score of pseudogene with gene"
    string strand;      "strand of pseudoegene"
    string assembly;	"assembly for gene"
    string geneTable;	"mysql table of gene"
    string gene;	"Name of gene"
    string gChrom;	"Chromosome name"
    uint gStart;	"gene alignment start position"
    uint gEnd;          "gene alignment end position"
    string gStrand;     "strand of gene"
    uint exonCount;     "# of exons in gene "
    uint geneOverlap;   "bases overlapping"
    uint polyA;         "length of polyA"
    uint polyAstart;    "start f polyA"
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
    uint label;         "1=pseudogene,-1 not pseudogene"
    uint milliBad;      "milliBad score, pseudogene aligned to genome"
    uint chainId;       "chain id of gene/pseudogene alignment"
    string refSeq;	"Name of closest regSeq to gene"
    uint rStart;	"refSeq alignment start position"
    uint rEnd;          "refSeq alignment end position"
    string mgc;	        "Name of closest mgc to gene"
    uint mStart;	"mgc alignment start position"
    uint mEnd;          "mgc alignment end position"
    string kgName;	"Name of closest knownGene to gene"
    uint kStart;	"kg alignment start position"
    uint kEnd;          "kg alignment end position"
    uint kgId;          "kg id"
    )

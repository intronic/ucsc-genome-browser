table pseudoGeneLink
"links a gene/pseudogene prediction to an ortholog or paralog."
    (
    short bin;          "bin for browser speedup"
    string chrom;	"Chromosome name for pseudogene"
    uint chromStart;	"pseudogene alignment start position"
    uint chromEnd;      "pseudogene alignment end position"
    string name;        "Name of pseudogene"
    uint score;        "score of pseudogene with gene"
    string strand;     "strand of pseudoegene"
    uint thickStart;   "Start of where display should be thick (start codon)"
    uint thickEnd;     "End of where display should be thick (stop codon)"
    uint reserved;     "Always zero for now"
    int blockCount;    "Number of blocks"
    int[blockCount] blockSizes; "Comma separated list of block sizes"
    int[blockCount] chromStarts; "Start positions relative to chromStart"
    string assembly;	"assembly for gene"
    string geneTable;	"mysql table of gene"
    string gene;	"Name of gene"
    string gChrom;	"Chromosome name"
    uint gStart;	"gene alignment start position"
    uint gEnd;         "gene alignment end position"
    uint score2;          "intron score of pseudogene with gap"
    uint score3;          "intron score of pseudogene"
    uint chainId;          "chain id of gene/pseudogene alignment"
    string gStrand;         "strand of gene"
    uint polyA;         "length of polyA"
    uint polyAstart;    "start f polyA"
    uint exonCover;     "number of exons in Gene covered"
    uint intronCount;   "number of introns in pseudogene"
    uint bestAliCount;  "number of good mrnas aligning"
    uint matches;       "matches + repMatches"
    uint qSize;         "aligning bases in pseudogene"
    uint tReps;         "repeats in gene"
    uint qReps;         "repeats in pseudogene"
    uint overlapDiag;   "bases on the diagonal to mouse"
    )

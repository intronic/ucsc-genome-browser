table pseudoGeneLink
"links a gene/pseudogene prediction to an ortholog or paralog."
    (
    string name;	"Name of pseudogene"
    uint score;          "score of pseudogene with gene"
    string assembly;	"assembly for gene"
    string geneTable;	"mysql table of gene"
    string gene;	"Name of gene"
    string chrom;	"Chromosome name"
    uint gStart;	"gene alignment start position"
    uint gEnd;         "gene alignment end position"
    uint score2;          "intron score of pseudogene with gap"
    uint score3;          "intron score of pseudogene"
    uint chainId;          "chain id of gene/pseudogene alignment"
    string strand;         "strand of gene"
    uint polyA;         "length of polyA"
    uint polyAstart;    "start f polyA"
    string pchrom;	"Chromosome name for pseudogene"
    uint pStart;	"pseudogene alignment start position"
    uint pEnd;         "pseudogene alignment end position"
    string pStrand;         "strand of pseudoegene"
    )

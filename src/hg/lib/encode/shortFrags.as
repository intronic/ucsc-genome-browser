table cshlShortTransfrag
"CSHL transfrags format (BED 6+)"
(
    string chrom;         "Reference sequence chromosome or scaffold"
    uint   chromStart;    "Start position in chromosome"
    uint   chromEnd;      "End position in chromosome"
    string name;          "Name of item"
    uint   score;         "Score from 0-1000"
    char[1] strand;       "+ or -"
    uint   length;        "contig's length (number of covered bases = end - start)"
    uint   numUnique;     "number of unique sequences in this contig"
    uint   numReads;      "total reads count in this contig"
    uint   minSeqCount;   "minimum sequence-count value"
    uint   maxSeqCount;   "maximum sequence-count value"
    float  aveSeqCount;   "average seqeunce-count value"
    uint   firstSeqCount; "first-quartile sequence-count value"
    uint   medSeqCount;   "median sequence-count value"
    uint   thirdSeqCount; "third-quartile sequence-count value"
    uint   minReadCount;  "minimum reads-count value"
    uint   maxReadCount;  "maximum reads-count value"
    float  aveReadCount;  "average seqeunce-count value"
    uint   firstReadCount;"first-quartile reads-count value'
    uint   medReadCount;  "median reads-count value"
    uint   thirdReadCount;"third-quartile reads-count value"
    uint   numRegions;    "number of regions in this contig (each region has different value for sequence-count and reads-count)"
    uint   regStart; "starting coordinates of significant regions in this contig (see example below)"
    uint   regLength;"length (in bases) of each significant regions sequence-count for each significant region"
    uint   regCount; "reads-count for each significant region"
    uint   sumCount;      "Integrated reads-count sum (inner-product of columns 22 and 24)"
)

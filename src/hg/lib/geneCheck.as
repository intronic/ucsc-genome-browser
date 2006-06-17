table geneCheck
"results from gene-check program"
    (
    string acc;                 "gene id or accession"
    string chr;                 "chromosome"
    uint chrStart;              "chromosome start"
    uint chrEnd;                "chromosome end"
    char[1] strand;             "strand"
    char[4] stat;               "status of overall check: ok or err"
    char[12] frame;             "frame status: ok, bad, noCDS, mismatch, discontig"
    char[2] start;              "is there a start codon: ok or no"
    char[2] stop;               "is there a stop codon: ok or no"
    uint orfStop;               "number of in-frame stop codons"
    uint cdsGap;                "number of gaps in CDS"
    uint cdsMult3Gap;           "number of CDS gaps that are a multiple of 3"
    uint utrGap;                "number of gaps in UTR"
    uint cdsSplice;             "number of CDS introns with unknown splice sites"
    uint utrSplice;             "number of CDS introns with unknown splice sites"
    uint numExons;              "number of exons"
    uint numCds;                "number of CDS exons"
    uint numUtr5;               "number of 5'UTR exons"
    uint numUtr3;               "number of 3'UTR exons"
    uint numCdsIntrons;         "number of CDS introns"
    uint numUtrIntrons;         "number of UTR introns"
    char[4] nmd;                "Is this an NMD candidate: ok or nmd"
    string causes;              "comma separated string with list of causes (for easy matching)"
    )

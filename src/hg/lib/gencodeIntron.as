table gencodeIntron
"Gencode intron status"
    (
    string chrom;      "Chromosome or FPC contig"
    uint   chromStart; "Start position in chromosome"
    uint   chromEnd;   "End position in chromosome"
    string name;       "Intron_id from GTF"
    string status;     "enum('not_tested', 'RT_positive','RT_negative','RT_wrong_junction', 'RT_submitted')"
    char[1] strand;    "+ or -"
    string transcript; "Transcript_id from GTF"
    string geneId;     "Gene_id from GTF"
    )

table snp126
"Polymorphism data from dbSnp database or genotyping arrays"
    (
    string   chrom;      "Reference sequence chromosome or scaffold"
    uint     chromStart; "Start position in chrom"
    uint     chromEnd;   "End position in chrom"
    string   name;       "Reference SNP identifier or Affy SNP name"
    uint     score;      "Not used"
    string   strand;     "Which DNA strand contains the observed alleles"
    lstring  refNCBI;  	"Reference genomic from dbSNP"
    lstring  refUCSC;   "Reference genomic from nib lookup"
    string   observed;   "The sequences of the observed alleles from rs-fasta files"
    string   molType;    "Sample type from exemplar ss"
    string   class;      "The class of variant (simple, insertion, deletion, range, etc.)"
    string   valid;      "The validation status of the SNP"
    float    avHet;      "The average heterozygosity from all observations"
    float    avHetSE;    "The Standard Error for the average heterozygosity"
    string   func;       "The functional category of the SNP (coding-synon, coding-nonsynon, intron, etc.)"
    string   locType;    "How the variant affects the reference sequence"
    uint     weight;     "The quality of the alignment"
    )

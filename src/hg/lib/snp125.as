table snp125
"Polymorphism data from dbSnp database or genotyping arrays"
    (
    string  chrom;      "Chromosome"
    uint    chromStart; "Start position in chrom"
    uint    chromEnd;   "End position in chrom"
    string  name;       "Reference SNP identifier or Affy SNP name"
    uint    score;      "Not used"
    char[1] strand;     "Which DNA strand contains the observed alleles"
    lstring  refNCBI;  	"Reference genomic from dbSNP"
    lstring  refUCSC;   "Reference genomic from nib lookup"
    lstring  observed;   "The sequences of the observed alleles from rs-fasta files"
    string  molType;    "Sample type from exemplar ss"
    string  class;      "The class of variant (simple, insertion, deletion, range, etc.)"
    string  valid;      "The validation status of the SNP"
    float   avHet;      "The average heterozygosity from all observations"
    float   avHetSE;    "The Standard Error for the average heterozygosity"
    string  func;       "The functional category of the SNP (coding-synon, coding-nonsynon, intron, etc.)"
    string  source;     "Source of the data - dbSnp, Affymetrix, ..."
    )

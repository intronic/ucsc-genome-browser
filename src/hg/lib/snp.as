table snp
"Polymorphism data from dbSnp XML files or genotyping arrays"
    (
    string  chrom;      "Chromosome"
    uint    chromStart; "Start position in chrom"
    uint    chromEnd;   "End position in chrom"
    string  name;       "Reference SNP identifier or Affy SNP name"
    uint    score;      "Not used"
    char[1] strand;     "Which DNA strand contains the observed alleles"
    string  observed;   "The sequences of the observed alleles"
    string  molType;    "Sample type from exemplar ss"
    string  class;      "The class of variant"
    string  valid;      "The validation status of the SNP"
    float   avHet;      "The average heterozygosity from all observations"
    float   avHetSE;    "The Standard Error for the average heterozygosity"
    string  func;       "The functional category of the SNP"
    string  locType;    "How the variant affects the reference sequence"
    string  source;     "Source of the data - dbSnp, Affymetrix, ..."
    )

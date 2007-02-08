table hapmapAlleles
"HapMap allele counts"
    (
    string  chrom;      "Chromosome"
    uint    chromStart; "Start position in chrom (0 based)"
    uint    chromEnd;   "End position in chrom (1 based)"
    string  name;       "Reference SNP identifier from dbSnp"
    uint    score;      "Not used"
    char[1] strand;     "Which genomic strand contains the observed alleles"

    string  observed;   "Observed string from genotype file"

    char[1] allele1;    "This allele has been observed"
    uint    allele1Count;       "Count of individuals who are homozygous for allele1"

    char[1] allele2;    "This allele may not have been observed"
    uint    allele2Count;       "Count of individuals who are homozygous for allele2"

    uint    heteroCount;   "Count of individuals who are heterozygous"
    )


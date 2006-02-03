table cnpIafrate
"CNP data from Iafrate lab"
    (
    string  chrom;      "Reference sequence chromosome or scaffold"
    uint    chromStart; "Start position in chrom"
    uint    chromEnd;   "End position in chrom"
    string  name;       "Reference SNP identifier or Affy SNP name"
    string  variationType;   "{Gain},{Loss},{Gain and Loss}"
    float    score;      "Score"
    )

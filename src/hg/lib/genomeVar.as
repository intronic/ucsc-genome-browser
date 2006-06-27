table genomeVar
"track for mutation data"
    (
    string  mutId;          "unique ID for this mutation"
    string  name;           "Official nomenclature description of mutation"
    string  srcId;          "source ID for this mutation"
    string  baseChangeType; "enum('insertion', 'deletion', 'substitution','duplication','complex','unknown')"
    string  location;       "enum('intron', 'exon', '5'' UTR', '3'' UTR', 'not within known transcription unit')"
    ubyte   coordinateAccuracy; "0=estimated, 1=definite, others?"
    )

table genomeVarPos
"location of mutation"
    (
    ushort  bin;            "A field to speed indexing"
    string  chrom;          "Chromosome"
    uint    chromStart;     "Start position in chrom"
    uint    chromEnd;       "End position in chrom"
    string  name;           "ID for this mutation"
    )

table genomeVarSrc
"sources for mutation track"
    (
    string srcId;	    "key into genomeVar table"
    string src;		    "name of genome wide source or LSDB"
    string lsdb; 	    "for LSDB name of actual source DB"
    )

table genomeVarAttr
"attributes asssociated with the mutation"
    (
    string mutId;	    "mutation ID"
    string attrKey;         "attribute type"
    string attrVal;         "value for this attribute"
    )

table genomeVarLink
"links both urls and local table lookups"
    (
    string mutId;           "id for attribute link"
    string attrKey;         "attribute type"
    string raKey;           "key into .ra file on how to do link"
    string acc;             "accession or id used by link"
    string displayVal;      "value to display if different from acc"
    )

table wiggle
"Wiggle track values to display as y-values (first 6 fields are bed6)"
(
string chrom;         "Human chromosome or FPC contig"
uint chromStart;      "Start position in chromosome"
uint chromEnd;        "End position in chromosome"
string name;          "Name of item"
uint score;           "range [0:127] == maximum in this block"
char[1] strand;       "+ or - (may not be needed for wiggle)"
uint Min;             "range [0:126] == minimum in this block"
uint Span;            "each value spans this many bases"
uint Count;           "number of values in this block"
uint Offset;          "offset in File to fetch data"
string File;          "path name to data file, one byte per value"
)

table wiggle
"Wiggle track values to display as y-values (first 6 fields are bed6)"
(
string chrom;         "Human chromosome or FPC contig"
uint chromStart;      "Start position in chromosome"
uint chromEnd;        "End position in chromosome"
string name;          "Name of item"
uint Span;            "each value spans this many bases"
uint Count;           "number of values in this block"
uint Offset;          "offset in File to fetch data"
string File;          "path name to data file, one byte per value"
double lowerLimit;    "lowest data value in this block"
double dataRange;     "lowerLimit + dataRange = upperLimit"
uint validCount;      "number of valid data values in this block"
double sumData;       "sum of the data points, for average and stddev calc"
double sumSquares;    "sum of data points squared, for stddev calc"
)

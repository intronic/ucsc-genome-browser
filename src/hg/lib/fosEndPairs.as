table fosEndPairs
"Positions of end pairs for fosmids"
   (
   short  bin;        "Bin number for browser speedup"
   string chrom;      "Human chromosome"
   uint   chromStart; "Start position of fosmid in chromosome"
   uint   chromEnd;   "End position of fosmid in chromosome"
   string name;       "Name of fosmid"

   uint   score;      "Score = 1000/(# of times fosmid appears in assembly)"
   char[1] strand;    "Value should be + or -"
   string pslTable;   "Table which contains corresponding PSL records for linked features"
   uint   lfCount;    "Number of linked features in the series"
   uint[lfCount]   lfStarts;  "Comma separated list of start positions of each linked feature in genomic"
   uint[lfCount]   lfSizes;   "Comma separated list of sizes of each linked feature in genomic"
   string[lfCount] lfNames;   "Comma separated list of names of linked features"
   )

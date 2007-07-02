table rnaHybridization
"perfect hybridizations on RNA level"
  (
  string chrom;      "Reference sequence chromosome or scaffold"
  uint chromStart;   "Start position in chromosome for pattern region"
  uint chromEnd;     "End position in chromosome for pattern region"
  string name;       "match name"
  uint dummy;        "dummy field to align with bed 6 structure" 
  char[1] strand;    "strand for pattern region" 	
  string chromTarget; "Reference sequence chromosome for target region"
  uint chromStartTarget; "Start position in chromosome for target region"	
  uint chromEndTarget;   "End position in chromosome for target region"
  char[1] strandTarget;  "strand for target region" 			
  string patternSeq; "Sequence of pattern region 5'-3'"
  string targetSeq;  "Sequence of target region 3'-5'"	
  float gcContent;   "GC content ranging from 0 to 1"	
  )


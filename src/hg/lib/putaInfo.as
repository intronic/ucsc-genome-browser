table putaInfo
"store the general info for a putative real or pseudo gene"
    (
     string chrom; "stamp's chrom"
     uint chromStart; "stamp's chrom start"
     uint chromEnd; "stamp's chromEnd"
     string name; "stamp's name"
     uint score;  "aligned score"
     string strand; "stamp's strand"
     string oChrom;    "stamper chrom"
     uint oChromStart; "stamper chrom start"
     uint oChromEnd; "stamper chrom end"
     uint blockCount; "total stamper blocks"
     uint stop; "1 .. with stop; 0 .. without stop"
     uint[2] tExons; "stamper (cds) exons"
     uint[4] qExons;     "stamp (mapped) (cds) exons"
     uint[4] qBases;    "stamp (aligend) (cds) bases"
     uint[2] repeats;  "(cds) has repeat inserted"
     uint[blockCount] stops; "stops for each exon"
     double[2] id; "(cds) aligned Identity"	
   )

table openChromCombinedPeaks
"Loacationss of regions of open chromatin as determined by DNaseI HS and FAIRE experiments. It is a BED8+12 format"
(
    string chrom;        "Name of the chromosome"
    uint   chromStart;   "Start position in chromosome"
    uint   chromEnd;     "End position in chromosome"
    string name;	 "Optional. Name given to a region (preferably unique). Use . if no name is assigned."
    uint   score;        "Optional. Indicates how dark the peak will be displayed in the browser (1-1000). If '0', the DCC will assign this based on signal value. Ideally average signalValue per base spread between 100-1000."
    char[2]   strand;       "Optional. +/- to denote strand or orientation (whenever applicable). Use '.' if no orientation is assigned."
    uint thickStart;   "Start of where display should be thick (start codon)"
    uint thickEnd;     "End of where display should be thick (stop codon)"
    uint color;        "RGB color value for peak"
	float pValue;      "Fisher's Combined P-Value (-log 10)"
    float dnaseSignal; "DNaseI HS Signal"
    float dnasePvalue; "DNaseI HS P-Value (-log 10)"
    float faireSignal; "FAIRE Signal"
    float fairePvalue; "FAIRE P-Value (-log 10)"
    float polIISignal; "Pol-II Signal"
    float polIIPvalue; "Pol-II P-Value (-log 10)"
    float ctcfSignal;  "CTCF Signal"
    float ctcfPvalue;  "CTCF P-Value (-log 10)"
    float cmycSignal;  "c-Myc Signal"
    float cmycPvalue;  "c-Myc P-Value (-log 10)"
    int   ocCode;      "Open Chromatin (OC) Code"
)

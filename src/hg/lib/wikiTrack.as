table wikiTrack
"wikiTrack bed 6+ structure"
    (
    ushort bin;        "used for efficient position indexing"
    string chrom;      "Reference sequence chromosome or scaffold"
    uint   chromStart; "Start position in chromosome"
    uint   chromEnd;   "End position in chromosome"
    string name;       "Name of item"
    uint   score;      "Score from 0-1000"
    char[1] strand;    "+ or -"
    string db;         "database for item"
    string owner;      "creator of item"
    string color;        "rgb color of item (currently unused)"
    string class;        "classification of item (browser group)"
    string creationDate;      "date item created"
    string lastModifiedDate;  "date item last updated"
    string descriptionKey;    "name of wiki description page"
    uint id;	       "auto-increment item ID"
    )

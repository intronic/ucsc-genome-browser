table gbCdnaInfo
"Links together various info associated with a GenBank mRNA or EST"
    (
    uint id;	"Unique numerical id for cDNA"
    string acc; "Genbank/EMBL accession (without .version)"
    ushort version; "Version number in Genbank"
    string moddate; "Date last modified, in SQL DATE format/ascii YYYY-MM-DD"
    string type; "Either EST or mRNA.  In SQL an ENUM"
    string direction; "Enum 5, 3 for read direction 5'/3', or 0 unknown."
    uint source;      "Link to id in source table saying where cDNA came from"
    uint organism;    "Link to id in organism table."
    uint library;     "Link to id in library table."
    uint mrnaClone;   "Link to id in mrnaClone table."
    uint sex;         "Link to id in sex table"
    uint tissue;      "Link to id in tissue table"
    uint development; "Link to id in development table"
    uint cell;        "Link to id in cell table"
    uint cds;         "Link to id in cds table"
    uint keyword;     "Link to id in keyword table"
    uint description; "Link to id in description table"
    uint geneName;    "Link to id in geneName table"
    uint productName; "Link to id in productName table"
    uint author;      "Link to id in author table"
    uint gi;          "Genbank identifier"
    )

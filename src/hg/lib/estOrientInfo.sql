# estOrientInfo.sql was originally generated by the autoSql program, which also 
# generated estOrientInfo.c and estOrientInfo.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Extra information on ESTs - calculated by polyInfo program
CREATE TABLE estOrientInfo (
    bin smallint unsigned not null,   # Bin for fast index
    chrom varchar(255) not null,	# Human chromosome or FPC contig
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Accession of EST
    intronOrientation smallint not null,	# Orientation of introns with respect to EST
    sizePolyA smallint not null,	# Number of trailing A's
    revSizePolyA smallint not null,	# Number of trailing A's on reverse strand
    signalPos smallint not null,	# Position of start of polyA signal relative to end of EST or 0 if no signal
    revSignalPos smallint not null,	# PolyA signal position on reverse strand if any
              #Indices
    INDEX(chrom(8),bin),
    INDEX(chrom(8),chromStart),
    INDEX(chrom(8),chromEnd)
);

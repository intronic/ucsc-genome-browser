# affyTransLifted.sql was originally generated by the autoSql program, which also 
# generated affyTransLifted.c and affyTransLifted.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Data file format for Affymetrix transcriptome data from Simon Cawley which has been normalized and lifted to a new assembly of the human genome.
CREATE TABLE affyTransLifted (
    chrom varchar(255) not null,	# Chromosome in hs.NCBIfreeze.chrom fomat originally
    chromPos int unsigned not null,	# Coordinate in chromosome (location of central base of the 25-mer)
    xCoord int not null,	# x-coordinate (column) of PM feature on chip
    yCoord int not null,	# y-coordinate (row) of PM feature on chip
    rawPM int not null,	# raw value of PM
    rawMM int not null,	# raw value of MM
    normPM float not null,	# normalized value of PM
    normMM float not null,	# normalized value of MM
              #Indices
    PRIMARY KEY(chrom)
);

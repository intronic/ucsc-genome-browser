# vax004Cons.sql was originally generated by the autoSql program, which also 
# generated vax004Cons.c and vax004Cons.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#VAX004 HIV-1 DNA Sequence Conservation
CREATE TABLE vax004Cons (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    span int unsigned not null,	# each value spans this many bases
    count int unsigned not null,	# number of values in this block
    offset int unsigned not null,	# offset in File to fetch data
    file varchar(255) not null,	# path name to data file, one byte per value
    lowerLimit double not null,	# lowest data value in this block
    dataRange double not null,	# lowerLimit + dataRange = upperLimit
    validCount int unsigned not null,	# number of valid data values in this block
    sumData double not null,	# sum of the data points, for average and stddev calc
    sumSquares double not null,	# sum of data points squared, for stddev calc
              #Indices
    PRIMARY KEY(chrom)
);

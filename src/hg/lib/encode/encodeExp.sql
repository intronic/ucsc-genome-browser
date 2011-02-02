# encodeExp.sql was originally generated by the autoSql program, which also 
# generated encodeExp.c and encodeExp.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#ENCODE experiments
CREATE TABLE encodeExp (
    ix int not null AUTO_INCREMENT,	# auto-increment ID
    organism varchar(255) not null,	# human | mouse
    lab varchar(255) not null,	# lab name from ENCODE cv.ra
    dataType varchar(255) not null,	# dataType from ENCODE cv.ra
    cellType varchar(255) not null,	# cellType from ENCODE cv.ra
    vars varchar(255) not null,	# RA of experiment-defining variables, defined per dataType
              #Indices
    PRIMARY KEY(ix)
);

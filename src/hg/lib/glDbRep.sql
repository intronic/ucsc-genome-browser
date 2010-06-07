# glDbRep.sql was originally generated by the autoSql program, which also 
# generated glDbRep.c and glDbRep.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Fragment positions in golden path
CREATE TABLE gl (
    frag varchar(255) not null,	# Fragment name
    start int unsigned not null,	# Start position in golden path
    end int unsigned not null,	# End position in golden path
    strand char(1) not null,	# + or - for strand
              #Indices
    PRIMARY KEY(frag(20))
);

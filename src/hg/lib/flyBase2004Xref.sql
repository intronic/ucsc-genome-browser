# flyBase2004Xref.sql was originally generated by the autoSql program, which also 
# generated flyBase2004Xref.c and flyBase2004Xref.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#FlyBase cross references circa late 2004 (dm2/4.0; dp3/1.0 uses a subset)
CREATE TABLE flyBase2004Xref (
    name varchar(255) not null,	# FlyBase annotation gene ID
    symbol varchar(255) not null,	# Symbolic gene name
    synonyms longblob not null,	# Comma-separated list of synonyms
    fbtr varchar(255) not null,	# FlyBase FBtr acc
    fbgn varchar(255) not null,	# FlyBase FBgn acc
    fbpp varchar(255) not null,	# FlyBase FBpp acc
    fban varchar(255) not null,	# FlyBase FBan acc
    type varchar(255) not null,	# Annotation type (for noncoding only)
              #Indices
    PRIMARY KEY(name(10)),
    INDEX(symbol(16)),
    INDEX(fbtr(11)),
    INDEX(fbgn(11)),
    INDEX(fbpp(11)),
    INDEX(fban(11)),
    INDEX(type(5))
);

# trackVersion.sql was originally generated by the autoSql program, which also 
# generated trackVersion.c and trackVersion.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

# This sql was hacked to insert the datetime object, autoSql could not do that

#version information for database tables to monitor data loading history
CREATE TABLE trackVersion (
    ix int not null,	# auto-increment ID
    db varchar(255) not null,	# UCSC database name
    name varchar(255) not null,	# table name in database
    who varchar(255) not null,	# Unix userID that performed this update
    version varchar(255) not null,	# version string, whatever is meaningful for data source
    updateTime datetime not null,	#  "YYYY-MM-DD HH:MM:SS most-recent-update time"
    comment varchar(255) not null,	# other comments about version
    source varchar(255) not null,	# perhaps a URL for the data source
    dateReference varchar(255) not null,	# Ensembl date string for archive reference
              #Indices
    PRIMARY KEY(ix),
    INDEX dbTable (db(32),name)
);

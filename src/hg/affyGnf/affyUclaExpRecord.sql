# expRecord.sql was originally generated by the autoSql program, which also 
# generated expRecord.c and expRecord.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#minimal descriptive data for an experiment in the browser
CREATE TABLE affyUclaExps (
    id int unsigned not null,	# internal id of experiment
    name varchar(255) not null,	# name of experiment
    description longblob not null,	# description of experiment
    url longblob not null,	# url relevant to experiment
    ref longblob not null,	# reference for experiment
    credit longblob not null,	# who to credit with experiment
    numExtras int unsigned not null,	# number of extra things
    extras longblob not null,	# extra things of interest, i.e. classifications
              #Indices
    PRIMARY KEY(id)
);

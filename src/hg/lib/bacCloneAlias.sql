# bacCloneAlias.sql was originally generated by the autoSql program, which also 
# generated bacCloneAlias.c and bacCloneAlias.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#BAC clones aliases and associated STS aliases and external BAC clone names
CREATE TABLE bacCloneAlias (
    alias varchar(255) not null,	# BAC clone alias (internal name) and STS aliases
    name varchar(255) not null,	# External name for BAC clone
              #Indices
    INDEX(alias),
    INDEX "name"
);

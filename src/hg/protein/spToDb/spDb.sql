# spDb.sql was originally generated by the autoSql program, which also 
# generated spDb.c and spDb.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Relate ID and primary accession. A good table to use just get handle on all records.
CREATE TABLE displayId (
    acc char(8) not null,	# Primary accession
    val char(12) not null,	# SwissProt display ID
              #Indices
    PRIMARY KEY(acc),
    UNIQUE(val)
);

#Relate ID and other accessions
CREATE TABLE otherAcc (
    acc char(8) not null,	# Primary accession
    val char(8) not null,	# Secondary accession
              #Indices
    INDEX(val),
    INDEX(acc)
);

#A part of a cell that has it's own genome
CREATE TABLE organelle (
    id int not null,	# Organelle ID - we create this
    val varchar(255) not null,	# Text description
              #Indices
    PRIMARY KEY(id)
);

#Small stuff with at most one copy associated with each SwissProt record
CREATE TABLE info (
    acc char(8) not null,	# Primary accession
    isCurated tinyint not null,	# True if curated (SwissProt rather than trEMBL)
    aaSize int not null,	# Size in amino acids
    molWeight int not null,	# Molecular weight
    createDate date not null,	# Creation date
    seqDate date not null,	# Sequence last update date
    annDate date not null,	# Annotation last update date
    organelle int not null,	# Pointer into organelle table
              #Indices
    PRIMARY KEY(acc)
);

#Description lines
CREATE TABLE description (
    acc char(8) not null,	# Primary accession
    val longtext not null,	# SwissProt DE lines
              #Indices
    PRIMARY KEY(acc)
);

#Gene including and/or logic if multiple
CREATE TABLE geneLogic (
    acc char(8) not null,	# Primary accession
    val longtext not null,	# Gene(s) and logic to relate them.
              #Indices
    PRIMARY KEY(acc)
);

#Gene/accession relationship. Both sides can be multiply valued.
CREATE TABLE gene (
    acc char(8) not null,	# Primary accession
    val varchar(255) not null,	# Single gene name
              #Indices
    INDEX(acc),
    INDEX(val(8))
);

#An NCBI taxon
CREATE TABLE taxon (
    id int not null,	# Taxon NCBI ID
    binomial varchar(255) not null,	# Binomial format name
    toGenus longtext not null,	# Taxonomy - superkingdom to genus
              #Indices
    INDEX(id),	# NCBI may have updated 1/2 way through SwissProt it seems.
    INDEX(binomial(12))
);

#Common name for a taxon
CREATE TABLE commonName (
    taxon int not null,	# Taxon table ID
    val varchar(255) not null,	# Common name
              #Indices
    INDEX(taxon),
    INDEX(val(12))
);

#accession/taxon relationship
CREATE TABLE accToTaxon (
    acc char(8) not null,	# Primary accession
    taxon int not null,	# ID in taxon table
              #Indices
    INDEX(acc),
    INDEX(taxon)
);

#A keyword
CREATE TABLE keyword (
    id int not null,	# Keyword ID - we create this
    val varchar(255) not null,	# Keyword itself
              #Indices
    PRIMARY KEY(id),
    INDEX(val(8))
);

#Relate keywords and accessions
CREATE TABLE accToKeyword (
    acc char(8) not null,	# Primary accession
    keyword int not null,	# ID in keyword table
              #Indices
    INDEX(acc),
    INDEX(keyword)
);

#A type of comment
CREATE TABLE commentType (
    id int not null,	# Comment type ID, we create this
    val varchar(255) not null,	# Name of comment type
              #Indices
    PRIMARY KEY(id),
    INDEX(val(8))
);

#Text of a comment
CREATE TABLE commentVal (
    id int not null,	# Comment value ID - we create this
    val longtext not null,	# Amino acids
              #Indices
    PRIMARY KEY(id)
);

#A structured comment
CREATE TABLE comment (
    acc char(8) not null,	# Primary accession
    commentType int not null,	# ID in commentType table
    commentVal int not null,	# ID in commentVal table
              #Indices
    INDEX(acc)
);

#Amino acid sequence
CREATE TABLE protein (
    acc char(8) not null,	# Primary accession
    val longblob not null,	# Amino acids
              #Indices
    PRIMARY KEY(acc)
);

#Name of another database
CREATE TABLE extDb (
    id int not null,	# Database id - we make this up
    val varchar(255) not null,	# Name of database
              #Indices
    PRIMARY KEY(id),
    INDEX(val(8))
);

#A reference to another database
CREATE TABLE extDbRef (
    acc char(8) not null,	# Primary SwissProt accession
    extDb int not null,	# ID in extDb table
    extAcc1 varchar(255) not null,	# External accession
    extAcc2 varchar(255) not null,	# External accession
    extAcc3 varchar(255) not null,	# External accession
              #Indices
    INDEX(acc),
    INDEX(extAcc1(10))
);

#A class of feature
CREATE TABLE featureClass (
    id int not null,	# Database id - we make this up
    val varchar(255) not null,	# Name of class
              #Indices
    PRIMARY KEY(id),
    INDEX(val(8))
);

#A type of feature
CREATE TABLE featureType (
    id int not null,	# Database id - we make this up
    val longtext not null,	# Name of type
              #Indices
    PRIMARY KEY(id),
    INDEX(val(8))
);

#A description of part of a protein
CREATE TABLE feature (
    acc char(8) not null,	# Primary accession
    start int not null,	# Start coordinate (zero based)
    end int not null,	# End coordinate (non-inclusive)
    featureClass int not null,	# ID of featureClass
    featureType int not null,	# ID of featureType
    softEndBits char not null,  # 1 for start <, 2 for start ?, 4 for end >, 8 for end ?
              #Indices
    INDEX(acc)
);

#A single author
CREATE TABLE author (
    id int not null,	# ID of this author
    val varchar(255) not null,	# Name of author
              #Indices
    PRIMARY KEY(id),
    INDEX(val(12))
);

#An article (or book or patent) in literature.
CREATE TABLE reference (
    id int not null,	# ID of this reference
    title longtext not null,	# Title
    cite longtext not null,	# Enough info to find journal/patent/etc.
    pubMed varchar(10) not null,	# Pubmed cross-reference
    medline varchar(12) not null,	# Medline cross-reference
              #Indices
    PRIMARY KEY(id),
    INDEX(pubMed(10))
);

#This associates references and authors
CREATE TABLE referenceAuthors (
    reference int not null,	# ID in reference table
    author int not null,	# ID in author table
              #Indices
    INDEX(reference),
    INDEX(author)
);

#SwissProt RP (Reference Position) line.  Often includes reason for citing.
CREATE TABLE citationRp (
    id int not null,	# ID of this citationRp
    val longtext not null,	# Reason for citing/position in sequence of cite.
              #Indices
    PRIMARY KEY(id)
);

#A SwissProt citation of a reference
CREATE TABLE citation (
    id int not null,	# ID of this citation
    acc char(8) not null,	# Primary accession
    reference int not null,	# ID in reference table
    rp int not null,	# ID in rp table
              #Indices
    PRIMARY KEY(id),
    INDEX(acc),
    INDEX(reference)
);

#Types found in a swissProt reference RC (reference comment) line
CREATE TABLE rcType (
    id int not null,	# ID of this one
    val varchar(255) not null,	# name of this
              #Indices
    PRIMARY KEY(id)
);

#Values found in a swissProt reference RC (reference comment) line
CREATE TABLE rcVal (
    id int not null,	# ID of this
    val longtext not null,	# associated text
              #Indices
    PRIMARY KEY(id)
);

#Reference comments associated with citation
CREATE TABLE citationRc (
    citation int not null,	# ID in citation table
    rcType int not null,	# ID in rcType table
    rcVal int not null,	# ID in rcVal table
              #Indices
    INDEX(citation)
);

# bioImage.sql was originally generated by the autoSql program, which also 
# generated bioImage.c and bioImage.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Location of image, typically a file directory
CREATE TABLE location (
    id int auto_increment not null,	# ID of location
    name longblob not null,	# Directory path usually
              #Indices
    PRIMARY KEY(id)
);

#Brain, eye, kidney, etc.  Use 'whole' for whole body
CREATE TABLE bodyPart (
    id int auto_increment not null,	# ID of body part
    name varchar(255) not null,	# Name of body part
              #Indices
    PRIMARY KEY(id),
    UNIQUE(name(32))
);

#Horizontal, coronal, whole mount, etc.
CREATE TABLE sliceType (
    id int auto_increment not null,	# ID of section
    name varchar(255) not null,	# Name of horizontal/whole mount, etc
              #Indices
    PRIMARY KEY(id),
    UNIQUE(name(32))
);

#Fixation and other treatment conditions
CREATE TABLE treatment (
    id int auto_increment not null,	# ID of treatment
    conditions varchar(255) not null,	# Text string describing conditions
              #Indices
    PRIMARY KEY(id)
);

#Type of image - mRNA in situ, fluorescent antibody, etc.
CREATE TABLE imageType (
    id int auto_increment not null,	# ID of image type
    name varchar(255) not null,	# Name of image type
              #Indices
    PRIMARY KEY(id),
    UNIQUE(name(32))
);

# Info on contributor
CREATE TABLE contributer (
    id int auto_increment not null,   # ID of contributer
    name varchar(255) not null,	# Name in format like Kend W.J."
         #Indices
    PRIMARY KEY(id),
    INDEX(name(32))
);

#Info on a batch of images submitted at once
CREATE TABLE submissionSet (
    id int auto_increment not null,	# ID of submission set
    publication longblob not null,	# Name of publication
    pubUrl longblob not null,	# Publication URL
    setUrl longblob not null,	# URL for whole set
    itemUrl longblob not null,	# URL for item.  Put %s where image.submitId should go
              #Indices
    PRIMARY KEY(id)
);

# Association between contributers and submissionSets"
CREATE TABLE submissionContributer (
    submissionSet int not null,  # ID in submissionSet table
    contributer int not null,    # ID in contributer table
          #Indices
    INDEX(submissionSet),
    INDEX(contributer)
);

#Info on a bunch of sections through same sample
CREATE TABLE sectionSet (
    id int auto_increment not null,	# Section id
    count int not null,	# Count of sections
              #Indices
    PRIMARY KEY(id)
);

#A single biological image
CREATE TABLE image (
    id int auto_increment not null,	# ID of image
    fileName varchar(255) not null,	# Image file name not including directory.
    fullLocation int not null,	# Location of full image
    thumbLocation int not null,	# Location of thumbnail image
    submissionSet int not null,	# Submission set this is part of
    sectionSet int not null,	# Set of sections this is part of or 0 if none
    sectionIx int not null,	# Position (0 based) within set of sections
    submitId varchar(255) not null,	# ID within submission set
    gene varchar(255) not null,	# Gene symbol (HUGO if available)
    locusLink varchar(255) not null,	# Locus link ID or blank if none
    refSeq varchar(255) not null,	# RefSeq ID or blank if none
    genbank varchar(255) not null,	# Genbank accession or blank if none
    defaultImage tinyint not null,	# Set to 1 if this is default for gene
    taxon int not null,	# NCBI taxon ID of organism
    isEmbryo tinyint not null,	# TRUE if embryonic.  Age will be relative to conception
    age float not null,	# Age in days since birth or conception depending on isEmbryo
    bodyPart int not null,	# Part of body image comes from
    sliceType int not null,	# How section is sliced
    imageType int not null,	# Type of image - in situ, etc.
    treatment int not null,	# How section is treated
              #Indices
    PRIMARY KEY(id),
    INDEX(submissionSet),
    INDEX(sectionSet),
    INDEX(gene(32)),
    INDEX(locusLink(8)),
    INDEX(genbank(12)),
    INDEX(taxon),
    INDEX(age),
    INDEX(imageType)
);

# visiGene.sql was originally generated by the autoSql program, which also 
# generated visiGene.c and visiGene.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Location of image, typically a file directory
CREATE TABLE fileLocation (
    id int auto_increment not null,	# ID of location
    name longblob not null,	# Directory path usually
              #Indices
    PRIMARY KEY(id)
);

#Name of strain (eg C57BL for a mouse)
CREATE TABLE strain (
    id int auto_increment not null,	# ID of strain
    taxon int not null,	# NCBI taxon of organism
    name varchar(255) not null,	# Name of strain
              #Indices
    PRIMARY KEY(id),
    INDEX(taxon),
    INDEX(name(8))
);

#Brain, eye, kidney, etc.  Use 'whole' for whole body
CREATE TABLE bodyPart (
    id int auto_increment not null,	# ID of body part
    name varchar(255) not null,	# Name of body part
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8))
);

#Sex of a specimen
CREATE TABLE sex (
    id int auto_increment not null,	# Sex ID
    name varchar(255) not null,	# Name of sex - male, female, hermaphrodite, mixed
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8))
);


#Horizontal, coronal, whole mount, etc.
CREATE TABLE sliceType (
    id int auto_increment not null,	# ID of section
    name varchar(255) not null,	# Name of horizontal/whole mount, etc
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8))
);

#Fixation conditions - 3% formaldehyde or the like
CREATE TABLE fixation (
    id int auto_increment not null,	# ID of fixation
    description varchar(255) not null,	# Text string describing fixation
              #Indices
    PRIMARY KEY(id)
);

#Embedding media for slices - paraffin, etc.
CREATE TABLE embedding (
    id int auto_increment not null,	# ID of embedding
    description varchar(255) not null,	# Text string describing embedding
              #Indices
    PRIMARY KEY(id)
);

#Permeablization conditions
CREATE TABLE permeablization (
    id int auto_increment not null,	# ID of treatment
    description varchar(255) not null,	# Text string describing conditions
              #Indices
    PRIMARY KEY(id)
);

#Info on contributor
CREATE TABLE contributor (
    id int auto_increment not null,	# ID of contributor
    name varchar(255) not null,	# Name in format like Kent W.J.
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8))
);

#Information on a journal
CREATE TABLE journal (
    id int auto_increment not null,	# ID of journal
    name varchar(255) not null,	# Name of journal
    url varchar(255) not null,	# Journal's main url
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8))
);

#Info on a batch of images submitted at once
CREATE TABLE submissionSet (
    id int auto_increment not null,	# ID of submission set
    name varchar(255) not null,  # Name of submission set
    contributors longblob not null,	# Comma separated list of contributors in format Kent W.J., Wu F.Y.
    publication longblob not null,	# Name of publication
    pubUrl longblob not null,	# Publication URL
    journal int not null,	# Journal for publication
    setUrl longblob not null,	# URL for whole set
    itemUrl longblob not null,	# URL for item.  Put %s where image.submitId should go
              #Indices
    PRIMARY KEY(id),
    UNIQUE(name(32))
);

#Association between contributors and submissionSets
CREATE TABLE submissionContributor (
    submissionSet int not null,	# ID in submissionSet table
    contributor int not null,	# ID in contributor table
              #Indices
    INDEX(submissionSet),
    INDEX(contributor)
);

#Info on a bunch of sections through same sample
CREATE TABLE sectionSet (
    id int auto_increment not null,	# Section id
              #Indices
    PRIMARY KEY(id)
);

#Information on an antibody
CREATE TABLE antibody (
    id int auto_increment not null,	# Antibody ID
    name varchar(255) not null,	# Name of antibody
    description longblob not null,	# Description of antibody
    taxon int not null,	# Taxon of animal antibody is from
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8)),
    INDEX(taxon)
);

#Info on a gene
CREATE TABLE gene (
    id int auto_increment not null,	# ID of gene
    name varchar(255) not null,	# Gene symbol (HUGO if available)
    locusLink varchar(255) not null,	# NCBI locus link ID or blank if none
    refSeq varchar(255) not null,	# RefSeq ID or blank if none
    genbank varchar(255) not null,	# Genbank/EMBL accession or blank if none
    uniProt varchar(255) not null,	# SwissProt/Uniprot accession or blank if none
    taxon int not null,	# NCBI taxon ID of organism
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8)),
    INDEX(locusLink(8)),
    INDEX(refSeq(10)),
    INDEX(genbank(10)),
    INDEX(uniProt(10)),
    INDEX(taxon)
);

#A synonym for a gene
CREATE TABLE geneSynonym (
    gene int not null,	# ID in gene table
    name varchar(255) not null,	# Synonymous name for gene
              #Indices
    INDEX(gene),
    INDEX(name(8))
);

#Name of a gene allele
CREATE TABLE allele (
    id int auto_increment not null,	# ID of allele
    gene int not null,	# ID of gene
    name varchar(255) not null,	# Allele name
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8))
);

#How different from wild type.  Associated with genotypeAllele table
CREATE TABLE genotype (
    id int auto_increment not null,	# Genotype id
    taxon int not null,	# Taxon of organism
    strain int not null,	# Strain of organism
    alleles longblob not null,	# Comma separated list of gene:allele in alphabetical order
              #Indices
    PRIMARY KEY(id),
    INDEX(taxon),
    INDEX(strain),
    INDEX(alleles(16))
);

#Association between genotype and alleles
CREATE TABLE genotypeAllele (
    genotype int not null,	# Associated genotype
    allele int not null,	# Associated allele
              #Indices
    INDEX(genotype),
    INDEX(allele)
);

#A biological specimen - something mounted, possibly sliced up
CREATE TABLE specimen (
    id int auto_increment not null,	# Specimen ID
    name varchar(255) not null,	# Name of specimen, frequently blank
    taxon int not null,		# NCBI Taxon of specimen
    genotype int not null,	# Genotype of specimen
    bodyPart int not null,	# Body part of specimen
    age float not null,	# Age in days since birth or conception depending on isEmbryo
    minAge float not null,	# Minimum age.
    maxAge float not null,	# Maximum age.  May differ from minAge if uncertain of age
    notes longblob not null,	# Any notes on specimen
              #Indices
    PRIMARY KEY(id),
    INDEX(name(8)),
    INDEX(taxon),
    INDEX(genotype),
    INDEX(bodyPart),
    INDEX(age)

);

#How a specimen is prepared
CREATE TABLE preparation (
    id int auto_increment not null,	# Preparation ID
    fixation int not null,	# How fixed
    embedding int not null,	# How embedded
    permeablization int not null,	# How permeablized
    sliceType int not null,	# How it was sliced
    notes longblob not null,	# Any other notes on preparation
              #Indices
    PRIMARY KEY(id)
);

#Type of probe - RNA, antibody, etc.
CREATE TABLE probeType (
    id int auto_increment not null,	# ID of probe type
    name varchar(255) not null,	# Name of probe type
              #Indices
    PRIMARY KEY(id)
);

#Info on a probe
CREATE TABLE probe (
    id int auto_increment not null,	# ID of probe
    gene int not null,	# Associated gene if any
    antibody int not null,	# Associated antibody if any
    probeType int not null,	# Type of probe - antibody, RNA, etc..
    fPrimer varchar(255) not null,	# Forward PCR primer if any
    rPrimer varchar(255) not null,	# Reverse PCR primer if any
    seq longblob not null,	# Associated sequence if any
              #Indices
    PRIMARY KEY(id),
    INDEX(gene),
    INDEX(antibody)
);

#Color - what color probe is in
CREATE TABLE probeColor (
    id int auto_increment not null,	# Id of color
    name varchar(255) not null,	# Color name
              #Indices
    PRIMARY KEY(id)
);

#A biological image file
CREATE TABLE imageFile (
    id int auto_increment not null,	# ID of imageFile
    fileName varchar(255) not null,	# Image file name not including directory
    priority float not null,	# Lower priorities are displayed first
    fullLocation int not null,	# Location of full-size image
    screenLocation int not null,	# Location of screen-sized image
    thumbLocation int not null,	# Location of thumbnail-sized image
    submissionSet int not null,	# Submission set this is part of
    submitId varchar(255) not null,	# ID within submission set
              #Indices
    PRIMARY KEY(id),
    INDEX(submissionSet)
);

#An image.  There may be multiple images within an imageFile
CREATE TABLE image (
    id int auto_increment not null,	# ID of image
    imageFile int not null,	# ID of image file
    imagePos int not null,	# Position in image file, starting with 0
    sectionSet int not null,	# Set of sections this is part of or 0 if none
    sectionIx int not null,	# Position (0 based) within set of sections
    specimen int not null,	# Pointer to info on specimen
    preparation int not null,	# Pointer to info on how specimen prepared
              #Indices
    PRIMARY KEY(id),
    INDEX(specimen)
);

#Associate probe and image
CREATE TABLE imageProbe (
    image int not null,	# ID of image
    probe int not null,	# ID of probe
    probeColor int not null,	# ID of probeColor
              #Indices
    INDEX(image),
    INDEX(probe)
);

#Annotated expression level if any
CREATE TABLE expressionLevel (
    imageProbe int not null,	# Image and probe
    bodyPart int not null,	# Location of expression
    level float not null,	# Expression level (0.0 to 1.0)
              #Indices
    INDEX(imageProbe),
    INDEX(bodyPart)
);

#Information of ages critical points in life cycle
CREATE TABLE lifeTime (
    taxon int not null,	# NCBI taxon
    birth float not null,	# Typical number of days from conception to birth/hatching
    adult float not null,	# Typical number of days from conception to adulthood
    death float not null,	# Typical number of days from conception to death
              #Indices
    PRIMARY KEY(taxon)
);

#List of schemes for developmental stages
CREATE TABLE lifeStageScheme (
    id int auto_increment not null,	# ID of scheme
    taxon int not null,	# NCBI taxon
    name varchar(255) not null,	# Theiler, or whatever
              #Indices
    PRIMARY KEY(id)
);

#List of life stages according to a particular scheme
CREATE TABLE lifeStage (
    lifeStageScheme int not null,	# Which staging scheme this is
    name varchar(255) not null,	# Name of this stage
    age float not null,	# Start age of this stage measured in days since conception
              #Indices
    INDEX(lifeStageScheme),
    INDEX(name(8)),
    INDEX(age)
);

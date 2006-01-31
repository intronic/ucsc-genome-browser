/* vgLoadJax - Load visiGene database from jackson database. More 
 * specifically create a directory full of .ra and .tab files from
 * jackson database that can be loaded into visiGene with visiGeneLoad. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "options.h"
#include "portable.h"
#include "obscure.h"
#include "jksql.h"
#include "spDb.h"
#include "jpegSize.h"

int testMax=0;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "vgLoadJax - Load visiGene database from jackson database. More\n"
  "specifically create a directory full of .ra and .tab files from\n"
  "jackson database that can be loaded into visiGene with visiGeneLoad.\n"
  "usage:\n"
  "   vgLoadJax /gbdb/visiGene jaxDb outputDir\n"
  "Load everything in jackson database tagged after date to\n"
  "visiGene database.  Most commonly run as\n"
  "   vgLoadJax /gbdb/visiGene jackson visiGene\n"
  "where /gbdb/visiGene is a symlink to the pictures home dir\n"
  " used to determine file exists and\n"
  " size (width and height in pixels)\n"
  "options:\n"
  "   -testMax=N - for testing purposes only output first N directories\n"
  );
}

static struct optionSpec options[] = {
   {"testMax", OPTION_INT},
   {NULL, 0},
};

struct sqlResult *sqlGetResultVerbose(struct sqlConnection *conn, char *s)
/* Get result, printing out query if verbosity is high enough. */
{
verbose(2, "%s\n", s);
return sqlGetResult(conn, s);
}

char *sqlQuickStringVerbose(struct sqlConnection *conn, char *s)
/* Get result, printing out query if verbosity is high enough. */
{
verbose(2, "%s\n", s);
return sqlQuickString(conn, s);
}

struct slName *jaxSpecList(struct sqlConnection *conn)
/* Get list of specimen id's. */
{
return sqlQuickList(conn, "select _Specimen_key from GXD_Specimen");
}

void dumpRow(char **row, int size)
/* Dump out row if verbosity >= 3. */
{
int i;
for (i=0; i<size; ++i)
    verbose(3, "%s\t", row[i]);
verbose(3, "\n");
}

char *colorFromLabel(char *label, char *gene)
/* Return color from labeling method.   This could be 
 * something in either the GXD_Label or the GXD_VisualizationMethod
 * tables in the jackson database. */
{
if (label == NULL)
    return "";
else if (sameString(label, "Not Applicable"))
    return "";
else if (sameString(label, "Not Specified"))
    return "";
else if (sameString(label, "Alexa Fluor"))	/* Which Alexa fluor? */
    return "";
else if (sameString(label, "Alkaline phosphatase"))
    return "purple";
else if (sameString(label, "Autoradiography"))
    return "";
else if (sameString(label, "Beta-galactosidase"))
    return "blue";
else if (sameString(label, "Biotin"))
    return "";
else if (sameString(label, "Colloidal gold"))
    return "";
else if (sameString(label, "Cy2"))
    return "green";
else if (sameString(label, "Cy3"))
    return "green";
else if (sameString(label, "Cy5"))
    return "red";
else if (sameString(label, "Digoxigenin"))
    return "digoxigenin";
else if (sameString(label, "Ethidium bromide"))
    return "orange";
else if (sameString(label, "Fluorescein"))
    return "green";
else if (sameString(label, "Horseradish peroxidase"))
    return "purple";
else if (sameString(label, "I125"))
    return "";
else if (sameString(label, "Oregon Green 488"))
    return "green";
else if (sameString(label, "other - see notes"))
    return "";
else if (sameString(label, "P32"))
    return "";
else if (sameString(label, "Phosphorimaging"))
    return "";
else if (sameString(label, "P33"))
    return "";
else if (sameString(label, "Rhodamine"))
    return "red";
else if (sameString(label, "S35"))
    return "";
else if (sameString(label, "SYBR green"))
    return "green";
else if (sameString(label, "Texas Red"))
    return "red";
else 
    {
    warn("Don't know color of %s in %s", label, gene);
    return "";
    }
}


boolean isUnknown(char *text)
/* Return TRUE if it looks like info really isn't in database. */
{
return (sameWord("Not Applicable", text)
   || sameWord("Not Specified", text)
   || sameWord("Other - see notes", text));
}

char *blankOutUnknown(char *text)
/* Return empty string in place of uninformative text. */
{
if (isUnknown(text))
   text = "";
return text;
}

void undupeCopyFile(char *source, char *dest)
/* Copy file, removing duplicate lines in process.  */
{
struct lineFile *lf = lineFileOpen(source, TRUE);
int size;
char *line;
char *lastLine = strdup("");
FILE *f = mustOpen(dest, "w");

while (lineFileNext(lf, &line, &size))
    {
    if (lastLine == NULL || differentString(line, lastLine))
        {
	fprintf(f, "%s\n", line);
	freeMem(lastLine);
	lastLine = cloneString(line);
	}
    }
freeMem(lastLine);
lineFileClose(&lf);
carefulClose(&f);
}

void genotypeAndStrainFromKey(char *genotypeKey, struct sqlConnection *conn,
	char **retGenotype, char **retStrain)
/* Return dynamically allocated string describing genotype */
{
int key = atoi(genotypeKey);
char *genotype = NULL, *strain = NULL;

if (key > 0)
    {
    struct dyString *query = dyStringNew(0);
    struct dyString *geno = dyStringNew(256);
    struct sqlResult *sr;
    char **row;

    /* Figure out genotype.  Create string that looks something like:
     *     adh:cheap date,antp:+,  
     * That is a comma separated list gene:allele. */
    dyStringPrintf(query, 
    	"select MRK_Marker.symbol,ALL_Allele.symbol "
	"from GXD_AlleleGenotype,MRK_Marker,ALL_Allele "
	"where GXD_AlleleGenotype._Genotype_key = %s "
	"and GXD_AlleleGenotype._Marker_key = MRK_Marker._Marker_key "
	"and GXD_AlleleGenotype._Allele_key = ALL_Allele._Allele_key "
	, genotypeKey);
    sr = sqlGetResultVerbose(conn, query->string);
    while ((row = sqlNextRow(sr)) != NULL)
	dyStringPrintf(geno, "%s:%s,", row[0], row[1]);
    sqlFreeResult(&sr);
    genotype = dyStringCannibalize(&geno);

    /* Figure out strain */
    dyStringClear(query);
    dyStringPrintf(query,
        "select PRB_Strain.strain from GXD_Genotype,PRB_Strain "
	"where GXD_Genotype._Genotype_key = %s "
	"and GXD_Genotype._Strain_key = PRB_Strain._Strain_key"
	, genotypeKey);
    strain = sqlQuickStringVerbose(conn, query->string);
    if (isUnknown(strain))
        freez(&strain);

    dyStringFree(&query);
    }
if (genotype == NULL)
    genotype = cloneString("");
if (strain == NULL)
    strain = cloneString("");
*retGenotype = genotype;
*retStrain = strain;
}

void printExpression(FILE *f, char *assayKey, struct sqlConnection *conn)
/* Print associated expression info on assay as indented lines. */
{
struct dyString *query = dyStringNew(0);
struct sqlResult *sr;
char **row;

dyStringPrintf(query, 
	"select GXD_StructureName.structure,GXD_Expression.expressed "
	"from GXD_Expression,GXD_Structure,GXD_StructureName "
	"where GXD_Expression._Assay_key = %s "
	"and GXD_Expression._Structure_key = GXD_Structure._Structure_key "
	"and GXD_Structure._StructureName_key = GXD_StructureName._StructureName_key"
	, assayKey);
sr = sqlGetResultVerbose(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *bodyPart = skipLeadingSpaces(row[0]);
    if (bodyPart[0] != 0)
	{
	/* Strip trailing # if any from body part. */
	int lastChar = strlen(bodyPart)-1;
	if (bodyPart[lastChar] == '#')
	    bodyPart[lastChar] = 0;
	fprintf(f, "\texpression\t%s\t%s\n", bodyPart, row[1]);
	}
    }
sqlFreeResult(&sr);
}

char *fixCopyright(char *s)
/* Fix copyright notice which sometimes is truncated in
 * Jackson labs database. */
{
if (endsWith(s, " and is"))
    {
    char *tail = NULL;
    if (stringIn(" Development ", s))
        {
	char *tail = " displayed with the permission of The Company of Biologists Limited who owns the Copyright.";
	int newSize = strlen(tail) + strlen(s) + 1;
	char *fixed = needMem(newSize);
	strcpy(fixed, s);
	strcat(fixed, tail);
	uglyf("Patched %s to %s\n", s, fixed);
	freez(&s);
	s = fixed;
	}
    }
return s;
}

void submitRefToFiles(struct sqlConnection *conn, struct sqlConnection *conn2, char *ref, char *fileRoot,
	char *inJax)
/* Create a .ra and a .tab file for given reference. */
{
/* Initially the tab file will have some duplicate lines, so
 * write to temp file, and then filter. */
char raName[PATH_LEN], tabName[PATH_LEN], tmpName[PATH_LEN], capName[PATH_LEN];
FILE *ra = NULL, *tab = NULL, *cap = NULL;
struct dyString *query = dyStringNew(0);
struct sqlResult *sr, *sr2;
char **row;
char *pubMed;
struct slName *list, *el;
boolean gotAny = FALSE;
struct hash *uniqImageHash = newHash(0);
struct hash *captionHash = newHash(0);
int imageWidth = 0, imageHeight = 0;
char path[PATH_LEN];
struct dyString *caption = dyStringNew(0);
struct dyString *copyright = dyStringNew(0);
boolean lookedForCopyright = FALSE;
	
safef(raName, sizeof(raName), "%s.ra", fileRoot);
safef(tabName, sizeof(tabName), "%s.tab", fileRoot);
safef(capName, sizeof(capName), "%s.txt", fileRoot);
safef(tmpName, sizeof(tmpName), "%s.tmp", fileRoot);
tab = mustOpen(tmpName, "w");
cap = mustOpen(capName, "w");


dyStringAppend(query, "select authors,journal,title,year from BIB_Refs where ");
dyStringPrintf(query, "_Refs_key = %s", ref);
sr = sqlGetResultVerbose(conn, query->string);
row = sqlNextRow(sr);
if (row == NULL)
    errAbort("Can't find _Refs_key %s in BIB_Refs", ref);


/* Make ra file with stuff common to whole submission set. */
ra = mustOpen(raName, "w");
fprintf(ra, "submissionSource MGI\n");
fprintf(ra, "acknowledgement Thanks to the Gene Expression Database group at "
            "Mouse Genome Informatics (MGI) for collecting, annotating and sharing "
	    "this image. The MGI images were last updated in VisiGene on June 17, 2005. "
	    "Additional and more up to date annotations and images may be available "
	    "directly at <A HREF=\"http://www.informatics.jax.org\">MGI.</A>\n");
fprintf(ra, "submitSet jax%s\n", ref);
fprintf(ra, "taxon 10090\n");	/* Mus musculus taxon */
fprintf(ra, "fullDir ../visiGene/full/inSitu/Mouse/jax\n");
fprintf(ra, "thumbDir ../visiGene/200/inSitu/Mouse/jax\n");
fprintf(ra, "setUrl http://www.informatics.jax.org/\n");
fprintf(ra, "itemUrl http://www.informatics.jax.org/searches/image.cgi?%%s\n");
fprintf(ra, "journal %s\n", row[1]);
fprintf(ra, "publication %s\n", row[2]);
fprintf(ra, "year %s\n", row[3]);

/* The contributor (author) list is in format Kent WJ; Haussler DH; format in
 * Jackson.  We convert it to Kent W.J.,Haussler D.H., format for visiGene. */
fprintf(ra, "contributor ");
list = charSepToSlNames(row[0], ';');
for (el = list; el != NULL; el = el->next)
    {
    char *lastName = skipLeadingSpaces(el->name);
    char *initials = strrchr(lastName, ' ');
    if (initials == NULL)
	initials = "";
    else
	*initials++ = 0;
    fprintf(ra, "%s", lastName);
    if (initials[0] != 0)
	{
	char c;
	fprintf(ra, " ");
	while ((c = *initials++) != 0)
	    fprintf(ra, "%c.", c);
	}
    fprintf(ra, ",");
    }
fprintf(ra, "\n");
slNameFreeList(&list);
sqlFreeResult(&sr);

/* Add in link to PubMed record on publication. */
dyStringClear(query);
dyStringPrintf(query,
   "select ACC_Accession.accID from ACC_Accession,ACC_LogicalDB "
   "where ACC_Accession._Object_key = %s "
   "and ACC_Accession._LogicalDB_key = ACC_LogicalDB._LogicalDB_key "
   "and ACC_LogicalDB.name = 'PubMed'", ref);
pubMed = sqlQuickStringVerbose(conn, query->string);
if (pubMed != NULL)
    fprintf(ra, "pubUrl http://www.ncbi.nlm.nih.gov/entrez/query.fcgi?cmd=Retrieve&db=pubmed&dopt=Abstract&list_uids=%s\n", pubMed);
freez(&pubMed);

dyStringClear(query);
dyStringAppend(query, 
	"select distinct MRK_Marker.symbol as gene,"
               "GXD_Specimen.sex as sex,"
	       "GXD_Specimen.age as age,"
	       "GXD_Specimen.ageMin as ageMin,"
	       "GXD_Specimen.ageMax as ageMax,"
	       "IMG_ImagePane.paneLabel as paneLabel,"
	       "ACC_Accession.numericPart as fileKey,"
	       "IMG_Image._Image_key as imageKey,"
	       "GXD_Assay._ProbePrep_key as probePrepKey,"
	       "GXD_Assay._AntibodyPrep_key as antibodyPrepKey,"
	       "GXD_Assay._ReporterGene_key as reporterGeneKey,"
	       "GXD_FixationMethod.fixation as fixation,"
	       "GXD_EmbeddingMethod.embeddingMethod as embedding,"
	       "GXD_Assay._Assay_key as assayKey,"
	       "GXD_Specimen.hybridization as sliceType,"
	       "GXD_Specimen._Genotype_key as genotypeKey\n"
	"from MRK_Marker,"
	     "GXD_Assay,"
	     "GXD_Specimen,"
	     "GXD_InSituResult,"
	     "GXD_InSituResultImage,"
	     "GXD_FixationMethod,"
	     "GXD_EmbeddingMethod,"
	     "IMG_ImagePane,"
	     "IMG_Image,"
	     "ACC_Accession\n"
	"where MRK_Marker._Marker_key = GXD_Assay._Marker_key "
	  "and GXD_Assay._Assay_key = GXD_Specimen._Assay_key "
	  "and GXD_Specimen._Specimen_key = GXD_InSituResult._Specimen_key "
	  "and GXD_InSituResult._Result_key = GXD_InSituResultImage._Result_key "
	  "and GXD_InSituResultImage._ImagePane_key = IMG_ImagePane._ImagePane_key "
	  "and GXD_FixationMethod._Fixation_key = GXD_Specimen._Fixation_key "
	  "and GXD_EmbeddingMethod._Embedding_key = GXD_Specimen._Embedding_key "
	  "and IMG_ImagePane._Image_key = IMG_Image._Image_key "
	  "and IMG_Image._Image_key = ACC_Accession._Object_key "
	  "and ACC_Accession.prefixPart = 'PIX:' "
	  "and GXD_Assay._ImagePane_key = 0 "
	);
dyStringPrintf(query, "and GXD_Assay._Refs_key = %s", ref);
sr = sqlGetResultVerbose(conn, query->string);


fprintf(tab, "#");
fprintf(tab, "gene\t");
fprintf(tab, "probeColor\t");
fprintf(tab, "sex\t");
fprintf(tab, "age\t");
fprintf(tab, "ageMin\t");
fprintf(tab, "ageMax\t");
fprintf(tab, "paneLabel\t");
fprintf(tab, "fileName\t");
fprintf(tab, "submitId\t");
fprintf(tab, "fPrimer\t");
fprintf(tab, "rPrimer\t");
fprintf(tab, "abName\t");
fprintf(tab, "abTaxon\t");
fprintf(tab, "fixation\t");
fprintf(tab, "embedding\t");
fprintf(tab, "bodyPart\t");
fprintf(tab, "sliceType\t");
fprintf(tab, "genotype\t");
fprintf(tab, "strain\t");
fprintf(tab, "priority\t");
fprintf(tab, "captionId\t");
fprintf(tab, "imageWidth\t");
fprintf(tab, "imageHeight\n");
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *gene = row[0];
    char *sex = row[1];
    char *age = row[2];
    char *ageMin = row[3];
    char *ageMax = row[4];
    char *paneLabel = row[5];
    char *fileKey = row[6];
    char *imageKey = row[7];
    char *probePrepKey = row[8];
    char *antibodyPrepKey = row[9];
    char *reporterGeneKey = row[10];
    char *fixation = row[11];
    char *embedding = row[12];
    char *assayKey = row[13];
    char *sliceType = row[14];
    char *genotypeKey = row[15];
    double calcAge = -1;
    char *probeColor = "";
    char *bodyPart = "";
    char *abName = NULL;
    char *rPrimer = NULL, *fPrimer = NULL;
    char *genotype = NULL;
    char *strain = NULL;
    char *priority = NULL;
    char abTaxon[32];
    char *captionId = "";

    verbose(3, "   "); dumpRow(row, 16);

    if (age == NULL)
        continue;

    if (!lookedForCopyright)
	{
	lookedForCopyright = TRUE;

	dyStringClear(query);
	dyStringPrintf(query, 
	     "select note from MGI_NoteChunk,MGI_Note,MGI_NoteType,ACC_MGIType "
	     "where MGI_Note._Object_key = %s "
	     "and ACC_MGIType.name = 'Image' "
	     "and ACC_MGIType._MGIType_key = MGI_Note._MGIType_key "
	     "and MGI_NoteType.noteType='Copyright' "
	     "and MGI_Note._NoteType_key = MGI_NoteType._NoteType_key "
	     "and MGI_Note._Note_key = MGI_NoteChunk._Note_key "
	     "order by sequenceNum"
	     , imageKey);
	sr2 = sqlGetResultVerbose(conn2, query->string);
	while ((row = sqlNextRow(sr2)) != NULL)
	   dyStringAppend(copyright, row[0]);
	sqlFreeResult(&sr2);

	if (copyright->stringSize != 0)
	    {
	    char *fixed = fixCopyright(copyright->string);
	    fprintf(ra, "copyright %s\n", fixed);
	    }
	}

    /* Massage sex */
        {
	if (sameString(sex, "Male"))
	    sex = "male";
	else if (sameString(sex, "Female"))
	    sex = "female";
	else
	    sex = "";
	}

    /* Massage age */
	{
	char *embryoPat = "embryonic day ";
	char *newbornPat = "postnatal newborn";
	char *dayPat = "postnatal day ";
	char *weekPat = "postnatal week ";
	char *adultPat = "postnatal adult";
	double calcMinAge = atof(ageMin);
	double calcMaxAge = atof(ageMax);
	double mouseBirthAge = 21.0;
	double mouseAdultAge = 63.0;	/* Relative to conception, not birth */

	if (age[0] == 0)
	    {
	    warn("age null, ageMin %s, ageMax %s\n", ageMin, ageMax);
	    calcAge = (calcMinAge + calcMaxAge) * 0.5;
	    }
	else if (startsWith(embryoPat, age))
	    calcAge = atof(age+strlen(embryoPat));
	else if (sameString(newbornPat, age))
	    calcAge = mouseBirthAge;
	else if (startsWith(dayPat, age))
	    calcAge = atof(age+strlen(dayPat)) + mouseBirthAge;
        else if (startsWith(weekPat, age))
	    calcAge = 7.0 * atof(age+strlen(weekPat)) + mouseBirthAge;
	else if (sameString(adultPat, age) && calcMaxAge - calcMinAge > 1000 
		&& calcMinAge < 365)
	    calcAge = 365;	/* Most adult mice are relatively young */
	else
	    {
	    warn("Calculating age from %s\n", age);
	    calcAge = (calcMinAge + calcMaxAge) * 0.5;
	    }
	if (calcAge < calcMinAge)
	    calcAge = calcMinAge;
	if (calcAge > calcMaxAge)
	    calcAge = calcMaxAge;
	}
    
    /* Massage probeColor */
        {
	if (!sameString(reporterGeneKey, "0"))
	    {
	    /* Fixme: make sure that reporterGene's end up in probeType table. */
	    char *name = NULL;
	    dyStringClear(query);
	    dyStringPrintf(query, 
	    	"select term from VOC_Term where _Term_key = %s", 
	    	reporterGeneKey);
	    name = sqlQuickStringVerbose(conn2, query->string);
	    if (name == NULL)
	        warn("Can't find _ReporterGene_key %s in VOC_Term", 
			reporterGeneKey);
	    else if (sameString(name, "GFP"))
	        probeColor = "green";
	    else if (sameString(name, "lacZ"))
	        probeColor = "blue";
	    else 
	        warn("Don't know color of reporter gene %s", name);
	    freez(&name);
	    }
	if (!sameString(probePrepKey, "0"))
	    {
	    char *name = NULL;
	    dyStringClear(query);
	    dyStringPrintf(query, 
	      "select GXD_VisualizationMethod.visualization "
	      "from GXD_VisualizationMethod,GXD_ProbePrep "
	      "where GXD_ProbePrep._ProbePrep_key = %s "
	      "and GXD_ProbePrep._Visualization_key = GXD_VisualizationMethod._Visualization_key"
	      , probePrepKey);
	    name = sqlQuickStringVerbose(conn2, query->string);
	    if (name == NULL)
	        warn("Can't find visualization from _ProbePrep_key %s", probePrepKey);
	    probeColor = colorFromLabel(name, gene);
	    freez(&name);
	    if (probeColor[0] == 0)
	        {
		dyStringClear(query);
		dyStringPrintf(query, 
			"select GXD_Label.label from GXD_Label,GXD_ProbePrep "
		        "where GXD_ProbePrep._ProbePrep_key = %s " 
			"and GXD_ProbePrep._Label_key = GXD_Label._Label_key"
		        , probePrepKey);
		name = sqlQuickStringVerbose(conn2, query->string);
		if (name == NULL)
		    warn("Can't find label from _ProbePrep_key %s", 
		    	probePrepKey);
		probeColor = colorFromLabel(name, gene);
		}
	    freez(&name);
	    }
	if (!sameString(antibodyPrepKey, "0") && probeColor[0] == 0 )
	    {
	    char *name = NULL;
	    dyStringClear(query);
	    dyStringPrintf(query, 
		  "select GXD_Label.label from GXD_Label,GXD_AntibodyPrep "
		  "where GXD_AntibodyPrep._AntibodyPrep_key = %s "
		  "and GXD_AntibodyPrep._Label_key = GXD_Label._Label_key"
		  , antibodyPrepKey);
	    name = sqlQuickStringVerbose(conn2, query->string);
	    if (name == NULL)
		warn("Can't find label from _AntibodyPrep_key %s", antibodyPrepKey);
	    probeColor = colorFromLabel(name, gene);
	    freez(&name);
	    }
	}

    /* Get abName, abTaxon */
    abTaxon[0] = 0;
    if (!sameString(antibodyPrepKey, "0"))
        {
	struct sqlResult *sr = NULL;
	int orgKey = 0;
	char **row;
	dyStringClear(query);
	dyStringPrintf(query, 
		"select antibodyName,_Organism_key "
		"from GXD_AntibodyPrep,GXD_Antibody "
		"where GXD_AntibodyPrep._AntibodyPrep_key = %s "
		"and GXD_AntibodyPrep._Antibody_key = GXD_Antibody._Antibody_key"
		, antibodyPrepKey);
	sr = sqlGetResultVerbose(conn2, query->string);
	row = sqlNextRow(sr);
	if (row != NULL)
	    {
	    abName = cloneString(row[0]);
	    orgKey = atoi(row[1]);
	    }
	sqlFreeResult(&sr);

	if (orgKey > 0)
	    {
	    struct sqlConnection *sp = sqlConnect("uniProt");
	    char *latinName = NULL, *commonName = NULL;
	    int spTaxon = 0;
	    dyStringClear(query);
	    dyStringPrintf(query, "select latinName from MGI_Organism "
	                          "where _Organism_key = %d", orgKey);
	    latinName = sqlQuickStringVerbose(conn2, query->string);
	    if (latinName != NULL && !sameString(latinName, "Not Specified"))
		{
		char *e = strchr(latinName, '/');
		if (e != NULL) 
		   *e = 0;	/* Chop off / and after. */
		spTaxon = spBinomialToTaxon(sp, latinName);
		}
	    else
	        {
		dyStringClear(query);
		dyStringPrintf(query, "select commonName from MGI_Organism "
	                          "where _Organism_key = %d", orgKey);
		commonName = sqlQuickStringVerbose(conn2, query->string);
		if (commonName != NULL && !sameString(commonName, "Not Specified"))
		    {
		    spTaxon = spCommonToTaxon(sp, commonName);
		    }
		}
	    if (spTaxon != 0)
	        safef(abTaxon, sizeof(abTaxon), "%d", spTaxon);
	    freez(&latinName);
	    freez(&commonName);
	    sqlDisconnect(&sp);
	    }
	}
    if (abName == NULL)
        abName = cloneString("");

    /* Get rPrimer, lPrimer */
    /* Note that this code seems to be correct, but the
     * Jackson database actually stores the primers very
     * erratically.  In all the cases I can find for in situs
     * the primers are actually stored in free text in the PRB_Notes
     * tabel. At this point I'm going to move on rather than figure out
     * how to dig it out of there. */
    if (!sameString(probePrepKey, "0"))
        {
	struct sqlResult *sr = NULL;
	char **row;
	dyStringClear(query);
	dyStringPrintf(query,
	    "select primer1sequence,primer2sequence "
	    "from PRB_Probe,GXD_ProbePrep "
	    "where PRB_Probe._Probe_key = GXD_ProbePrep._Probe_key "
	    "and GXD_ProbePrep._ProbePrep_key = %s"
	    , probePrepKey);
	sr = sqlGetResultVerbose(conn2, query->string);
	row = sqlNextRow(sr);
	if (row != NULL)
	    {
	    fPrimer = cloneString(row[0]);
	    rPrimer = cloneString(row[1]);
	    }
	sqlFreeResult(&sr);
	}
    if (fPrimer == NULL)
        fPrimer = cloneString("");
    if (rPrimer == NULL)
        rPrimer = cloneString("");

    fixation = blankOutUnknown(fixation);
    embedding = blankOutUnknown(embedding);

    /* Massage body part and slice type.  We only handle whole mounts. */
    if (sameString(sliceType, "whole mount"))
	{
	bodyPart = "whole";
	priority = "100";
	}
    else
	{
        sliceType = "";
	priority = "1000";
	}

    genotypeAndStrainFromKey(genotypeKey, conn2, &genotype, &strain);

    stripChar(paneLabel, '"');	/* Get rid of a difficult quote to process. */
    
    /* Fetch image dimensions from file. */
    imageWidth=0;
    imageHeight=0;
    safef(path, sizeof(path), "%s/%s.jpg", inJax, fileKey);
    if (fileExists(path))
    	jpegSize(path,&imageWidth,&imageHeight);  /* will errAbort if no valid .jpeg exists */
    else
	warn("Picture Missing! %s ",path);
    
    /* Deal caption if any.  Most of the work only happens the
     * first time see the image. */
    if (!hashLookup(uniqImageHash, imageKey))
        {
	struct sqlResult *sr = NULL;
	char **row;
	hashAdd(uniqImageHash, imageKey, NULL);
	dyStringClear(caption);
	dyStringClear(query);
	dyStringPrintf(query, 
	     "select note from MGI_NoteChunk,MGI_Note,MGI_NoteType,ACC_MGIType "
	     "where MGI_Note._Object_key = %s "
	     "and ACC_MGIType.name = 'Image' "
	     "and ACC_MGIType._MGIType_key = MGI_Note._MGIType_key "
	     "and MGI_NoteType.noteType='Caption' "
	     "and MGI_Note._NoteType_key = MGI_NoteType._NoteType_key "
	     "and MGI_Note._Note_key = MGI_NoteChunk._Note_key "
	     "order by sequenceNum"
	     , imageKey);
	sr2 = sqlGetResultVerbose(conn2, query->string);
	while ((row = sqlNextRow(sr2)) != NULL)
	   dyStringAppend(caption, row[0]);
	sqlFreeResult(&sr2);

	if (caption->stringSize > 0)
	    {
	    subChar(caption->string, '\t', ' ');
	    subChar(caption->string, '\n', ' ');
	    fprintf(cap, "%s\t%s\n", imageKey, caption->string);
	    hashAdd(captionHash, imageKey, imageKey);
	    }
	}
    if (hashLookup(captionHash, imageKey))
        captionId = imageKey;
    else
        captionId = "";

    fprintf(tab, "%s\t", gene);
    fprintf(tab, "%s\t", probeColor);
    fprintf(tab, "%s\t", sex);
    fprintf(tab, "%3.2f\t", calcAge);
    fprintf(tab, "%s\t", ageMin);
    fprintf(tab, "%s\t", ageMax);
    fprintf(tab, "%s\t", paneLabel);
    fprintf(tab, "%s.jpg\t", fileKey);
    fprintf(tab, "%s\t", imageKey);
    fprintf(tab, "%s\t", fPrimer);
    fprintf(tab, "%s\t", rPrimer);
    fprintf(tab, "%s\t", abName);
    fprintf(tab, "%s\t", abTaxon);
    fprintf(tab, "%s\t", fixation);
    fprintf(tab, "%s\t", embedding);
    fprintf(tab, "%s\t", bodyPart);
    fprintf(tab, "%s\t", sliceType);
    fprintf(tab, "%s\t", genotype);
    fprintf(tab, "%s\t", strain);
    fprintf(tab, "%s\t", priority);
    fprintf(tab, "%s\t", captionId);
    fprintf(tab, "%d\t", imageWidth);
    fprintf(tab, "%d\n", imageHeight);

    printExpression(tab, assayKey, conn2);
    gotAny = TRUE;
    freez(&genotype);
    freez(&abName);
    freez(&rPrimer);
    freez(&fPrimer);
    }
sqlFreeResult(&sr);

carefulClose(&ra);
carefulClose(&tab);
carefulClose(&cap);
if (gotAny)
    undupeCopyFile(tmpName, tabName);
else
    {
    remove(raName);
    remove(capName);
    }
remove(tmpName);
dyStringFree(&copyright);
dyStringFree(&caption);
dyStringFree(&query);
hashFree(&uniqImageHash);
hashFree(&captionHash);
}

void submitToDir(struct sqlConnection *conn, struct sqlConnection *conn2, char *outDir,
	char *inJax)
/* Create directory full of visiGeneLoad .ra/.tab files from
 * jackson database connection.  Creates a pair of files for
 * each submission set.   Returns outDir. */
{
struct dyString *query = dyStringNew(0);
struct slName *ref, *refList = sqlQuickList(conn, "select distinct(_Refs_key) from GXD_Assay");
int refCount = 0;

makeDir(outDir);

for (ref = refList; ref != NULL; ref = ref->next)
    {
    char path[PATH_LEN];
    char *pub;
    boolean skip;

    /* Check that it isn't on our skip list - one that we
     * have already in the database from a higher resolution
     * source. */
    dyStringClear(query);
    dyStringPrintf(query, "select title from BIB_Refs where _Refs_key = %s", 
    	ref->name);
    pub = sqlQuickString(conn, query->string);
    skip = sameString(pub, 
	"Mouse Brain Organization Revealed Through Direct Genome-Scale TF Expression Analysis.");

     if (!skip)
	{
	safef(path, sizeof(path), "%s/%s", outDir, ref->name);
	submitRefToFiles(conn, conn2, ref->name, path, inJax);
	refCount += 1;
	if (testMax != 0 && refCount >= testMax)
	     errAbort("Reached testMax %d output dirs [%s]\n", testMax, path);
	}
    freeMem(pub);
    }

slNameFreeList(&refList);
dyStringFree(&query);
}

void vgLoadJax(char *visiGeneDir, char *jaxDb, char *outDir)
/* vgLoadJax - Load visiGene database from jackson database. */
{
struct sqlConnection *conn = sqlConnect(jaxDb);
struct sqlConnection *conn2 = sqlConnect(jaxDb);

char inFull[PATH_LEN];
char *jaxPath = "inSitu/Mouse/jax";
char inJax[PATH_LEN];
safef(inFull, sizeof(inFull), "%s/full", visiGeneDir);
safef(inJax, sizeof(inJax), "%s/%s", inFull, jaxPath);
submitToDir(conn, conn2, outDir, inJax);
sqlDisconnect(&conn2);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
testMax = optionInt("testMax", testMax);
if (argc != 4)
    usage();
vgLoadJax(argv[1], argv[2], argv[3]);
return 0;
}

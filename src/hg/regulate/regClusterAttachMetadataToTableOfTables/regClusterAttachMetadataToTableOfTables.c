/* regClusterAttachMetadataToTableOfTables - Try and find metadata (cell line, antibody, etc) for tables - using metaDb first, and if no metaDb object then trying to parse it out of file name. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"

static char const rcsid[] = "$Id: newProg.c,v 1.30 2010/03/24 21:18:33 hiram Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "regClusterAttachMetadataToTableOfTables - Try and find metadata (cell line, antibody, etc)\n"
  "for tables - using metaDb first, and if no metaDb object then trying to parse it out of\n"
  "file name\n"
  "usage:\n"
  "   regClusterAttachMetadataToTableOfTables database partial.table output.table\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

boolean getMetaFromMetaDb(struct sqlConnection *conn,
	char *obj, char **retCell, char **retAntibody, char **retTreatment)
/* Look in metadata for object and return cell, antibody and treatment from it. */
{
char query[256];
safef(query, sizeof(query), "select val from metaDb where obj='%s' and var='cell'", obj);
*retCell = sqlQuickString(conn, query);
safef(query, sizeof(query), "select val from metaDb where obj='%s' and var='antibody'", obj);
*retAntibody = sqlQuickString(conn, query);
safef(query, sizeof(query), "select val from metaDb where obj='%s' and var='treatment'", obj);
*retTreatment = sqlQuickString(conn, query);
return *retCell != NULL || *retAntibody != NULL || *retTreatment != NULL;
}

char *firstUpper(char *s)
/* Return pointer to first upper case letter in string */
{
for (;;)
    {
    char c = *s;
    if (c == 0)
        return NULL;
    else if (isupper(c))
        return s;
    ++s;
    }
}

void getMetaFromObjName(char *obj, char **retCell, char **retAntibody, char **retTreatment)
/* Look in metadata for object and return cell, antibody and treatment from it. */
{
char *cell = NULL, *antibody = NULL, *treatment = NULL;

/* Skip past first bits. */
char *pattern = "wgEncodeHaibTfbs";
int patLen = strlen(pattern);
if (!startsWith(pattern, obj))
   errAbort("getMetaFromObjName can't handle %s since it doesn't begin with %s", obj, pattern);

/* Rely on CamelCasing to pick out cell/antibody/treatment. */
char *cellStart = obj+patLen;
char *abStart = firstUpper(cellStart+1);
assert(abStart != NULL);
char *treatmentStart = firstUpper(abStart+1);
assert(treatmentStart != NULL);

/* Look up cell */
char *cellId = cloneStringZ(cellStart, abStart - cellStart);
if (sameString(cellId, "Gm12878"))
    cell = "GM12878";
else if (sameString(cellId, "Gm12891"))
    cell = "GM12891";
else if (sameString(cellId, "Gm12892"))
    cell = "GM12892";
else if (sameString(cellId, "H1hesc"))
    cell = "H1-hESC";
else if (sameString(cellId, "Hepg2"))
    cell = "HepG2";
else if (sameString(cellId, "K562"))
    cell = "K562";
else
    errAbort("Unrecognized cellId %s in %s", cellId, obj);

/* Look up antibody */
char *abId = cloneStringZ(abStart, treatmentStart - abStart);
if (sameString(abId, "Bcl3"))
    antibody = "BCL3";
else if (sameString(abId, "Ebf"))
    antibody = "EBF";
else if (sameString(abId, "Egr1"))
    antibody = "Egr-1";
else if (sameString(abId, "Irf4"))
    antibody = "IRF4_(M-17)";
else if (sameString(abId, "Oct2"))
    antibody = "Oct-2";	 // This is just a guess, Antibody not even on wiki yet
else if (sameString(abId, "Pou2f2"))
    antibody = "POU2F2"; // Again a guess. Ironically Oct-2 and POU2F2 are same gene
else if (sameString(abId, "Sin3ak20"))
    antibody = "Sin3Ak-20";
else if (sameString(abId, "Taf1"))
    antibody = "TAF1";
else if (sameString(abId, "Yy1"))
    antibody = "YY1_(C-20)";
else if (sameString(abId, "Pol24h8"))
    antibody = "Pol2-4H8";
else if (sameString(abId, "Pol2"))
    antibody = "Pol2";
else if (sameString(abId, "Nrsf"))
    antibody = "NRSF";
else if (sameString(abId, "P300"))
    antibody = "p300";
else if (sameString(abId, "Atf3"))
    antibody = "ATF3";
else if (sameString(abId, "Gabp"))
    antibody = "GABP";
else if (sameString(abId, "Max"))
    antibody = "Max";
else
    errAbort("Unrecognized abId %s in %s", abId, obj);

/* Looks like all of the HAIB missing metadata have no treatment. */
treatment = "None";

/* Set return variables, clean up, and go home. */
freez(&cellId);
freez(&abId);
*retCell = cell;
*retAntibody = antibody;
*retTreatment = treatment;
}

void regClusterAttachMetadataToTableOfTables(char *database, char *partialTable, char *outputTable)
/* regClusterAttachMetadataToTableOfTables - Try and find metadata (cell line, antibody, etc) for 
 * tables - using metaDb first, and if no metaDb object then trying to parse it out of file name. */
{
struct sqlConnection *conn = sqlConnect(database);
struct lineFile *lf = lineFileOpen(partialTable, TRUE);
FILE *f = mustOpen(outputTable, "w");
char *row[7];

while (lineFileRow(lf, row))
    {
    char *obj = row[6];
    char *cell = NULL, *antibody=NULL, *treatment=NULL;
    if (!getMetaFromMetaDb(conn, obj, &cell, &antibody, &treatment))
        getMetaFromObjName(obj, &cell, &antibody, &treatment);
    int i;
    for (i=0; i<7; ++i)
    	fprintf(f, "%s\t", row[i]);
    fprintf(f, "%s\t", naForNull(cell));
    fprintf(f, "%s\t", naForNull(antibody));
    if (treatment == NULL)
        treatment = "None";
    fprintf(f, "%s\n", treatment);
    }

carefulClose(&f);
lineFileClose(&lf);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
regClusterAttachMetadataToTableOfTables(argv[1], argv[2], argv[3]);
return 0;
}

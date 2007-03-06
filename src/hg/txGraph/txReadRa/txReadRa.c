/* txReadRa - Read ra files from genbank and parse out relevant info into some tab-separated files.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "portable.h"
#include "ra.h"

static char const rcsid[] = "$Id: txReadRa.c,v 1.1 2007/03/06 06:25:55 kent Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "txReadRa - Read ra files from genbank and parse out relevant info into some tab-separated files.\n"
  "usage:\n"
  "   txReadRa genbank.ra refseq.ra outDir\n"
  "Output files are in outDir, including\n"
  "   cds.tab - cds entries for both genbank and refSeq\n"
  "   mrnaSize.tab - size entries for both genbank and refSeq\n"
  "   refSeqStatus.tab - Reviewed/Validated/Preliminary/etc. status for refSeq\n"
  "   refPepStatus.tab - Similar info for refSeq proteins\n"
  "   refToPep.tab - Maps refSeq mRNA to proteins\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

FILE *openToWrite(char *dir, char *file)
/* Return dir/file open for writing. */
{
char path[PATH_LEN];
safef(path, sizeof(path), "%s/%s", dir, file);
return mustOpen(path, "w");
}

char *requiredField(struct hash *ra, struct lineFile *lf, char *field)
/* Get field from ra, or die with approximate line number. */
{
char *val = hashFindVal(ra, field);
if (val == NULL)
    errAbort("Missing required %s field in record ending line %d of %s",
    	field, lf->lineIx, lf->fileName);
return val;
}

void txReadRa(char *mrnaRa, char *refSeqRa, char *outDir)
/* txReadRa - Read ra files from genbank and parse out relevant info into some 
 * tab-separated files. */
{
struct lineFile *mrna = lineFileOpen(mrnaRa, TRUE);
struct lineFile *refSeq = lineFileOpen(refSeqRa, TRUE);
makeDir(outDir);
FILE *fCds = openToWrite(outDir, "cds.tab");
FILE *fStatus = openToWrite(outDir, "refSeqStatus.tab");
FILE *fSize = openToWrite(outDir, "mrnaSize.tab");
FILE *fRefToPep = openToWrite(outDir, "refToPep.tab");
FILE *fPepStatus = openToWrite(outDir, "refPepStatus.tab");

struct hash *ra;
while ((ra = raNextRecord(refSeq)) != NULL)
    {
    char *acc = requiredField(ra, refSeq, "acc");
    char *rss = requiredField(ra, refSeq, "rss");
    char *siz = requiredField(ra, refSeq, "siz");
    char *prt = hashFindVal(ra, "prt");
    char *cds = hashFindVal(ra, "cds");

    /* Translate rss into status. */
    char *status = NULL;
    if (sameString(rss, "rev"))
        status = "Reviewed";
    else if (sameString(rss, "pro"))
        status = "Provisional";
    else if (sameString(rss, "pre"))
        status = "Predicted";
    else if (sameString(rss, "val"))
        status = "Validated";
    else if (sameString(rss, "inf"))
        status = "Inferred";
    else
        errAbort("Unrecognized rss field %s after line %d of %s", rss, 
		refSeq->lineIx, refSeq->fileName);

     fprintf(fStatus, "%s\t%s\n", acc, status);
     if (prt != NULL)
	 {
         fprintf(fPepStatus, "%s\t%s\n", prt, status);
	 fprintf(fRefToPep, "%s\t%s\n", acc, prt);
	 }
     fprintf(fSize, "%s\t%s\n", acc, siz);
     if (cds != NULL)
         fprintf(fCds, "%s\t%s\n", acc, cds);
    hashFree(&ra);
    }

while ((ra = raNextRecord(mrna)) != NULL)
    {
    char *acc = requiredField(ra, mrna, "acc");
    char *siz = requiredField(ra, mrna, "siz");
    char *cds = hashFindVal(ra, "cds");
    fprintf(fSize, "%s\t%s\n", acc, siz);
    if (cds != NULL)
    	fprintf(fCds, "%s\t%s\n", acc, cds);
    hashFree(&ra);
    }

carefulClose(&fCds);
carefulClose(&fStatus);
carefulClose(&fSize);
carefulClose(&fRefToPep);
carefulClose(&fPepStatus);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
txReadRa(argv[1], argv[2], argv[3]);
return 0;
}

/* bam -- interface to binary alignment format files using Heng Li's samtools lib. */

#ifdef USE_BAM

#include "common.h"
#include "hdb.h"
#include "bamFile.h"

static char const rcsid[] = "$Id: bamFile.c,v 1.1 2009/07/27 21:52:08 angie Exp $";

static char *bbiNameFromTable(struct sqlConnection *conn, char *table)
/* Return file name from little table. */
/* This should be libified somewhere sensible -- copied from hgTracks/bedTrack.c. */
{
char query[256];
safef(query, sizeof(query), "select fileName from %s", table);
char *fileName = sqlQuickString(conn, query);
if (fileName == NULL)
    errAbort("Missing fileName in %s table", table);
return fileName;
}

void bamFetch(char *db, char *table, char *position, bam_fetch_f callbackFunc, void *callbackData)
/* Open the .bam file given in db.table, fetch items in the seq:start-end position range,
 * and call callbackFunc on each bam item retrieved from the file plus callbackData. 
 * Note: if sequences in .bam file don't begin with "chr" but cart position does, pass in 
 * cart position + strlen("chr") to match the .bam file sequence names. */
{
// TODO: if bamFile is URL, convert URL to path a la UDC, but under udcFuse mountpoint.
// new hg.conf setting for udcFuse mountpoint/support.
struct sqlConnection *conn = hAllocConn(db);
char *bamFileName = bbiNameFromTable(conn, table);
hFreeConn(&conn);

samfile_t *fh = samopen(bamFileName, "rb", NULL);
if (fh == NULL)
    errAbort("samopen(%s, \"rb\") returned NULL", bamFileName);

int chromId, start, end;
int ret = bam_parse_region(fh->header, position, &chromId, &start, &end);
if (ret != 0)
    errAbort("bam_parse_region(%s) failed (%d)", position, ret);
//?? Could this happen if there is no data on some _random?  can avoid with tdb chromosomes...

bam_index_t *idx = bam_index_load(bamFileName);
ret = bam_fetch(fh->x.bam, idx, chromId, start, end, callbackData, callbackFunc);
if (ret != 0)
    errAbort("bam_fetch(%s, %s (chromId=%d) failed (%d)", bamFileName, position, chromId, ret);
samclose(fh);
}

#endif//def USE_BAM

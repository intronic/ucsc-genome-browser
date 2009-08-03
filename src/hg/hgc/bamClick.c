/* bamClick - handler for alignments in BAM format (produced by MAQ,
 * BWA and some other short-read alignment tools). */
#ifdef USE_BAM

#include "common.h"
#include "hash.h"
#include "hdb.h"
#include "bamFile.h"
#include "hgc.h"

static char const rcsid[] = "$Id: bamClick.c,v 1.2 2009/08/03 22:00:24 angie Exp $";

#include "bamFile.h"

struct bamTrackData
    {
    int itemStart;
    char *itemName;
    struct hash *pairHash;
    };

static void singleBamDetails(const bam1_t *bam)
/* Print out the properties of this alignment. */
{
const bam1_core_t *core = &bam->core;
char *itemName = bam1_qname(bam);
int length = bamGetTargetLength(bam);
printPosOnChrom(seqName, core->pos, core->pos+length, (core->flag & BAM_FREVERSE) ? "-" : "+",
		FALSE, itemName);
printf("<B>CIGAR string: </B><tt>%s</tt><BR>\n", bamGetCigar(bam));
char *qSeq = bamGetQuerySequence(bam);
printf("<B>Read Sequence: </B><tt>%s</tt><BR>\n", qSeq);
//TODO: show flags, display alignment properly, maybe display quality scores
}

static int oneBam(const bam1_t *bam, void *data)
/* This is called on each record retrieved from a .bam file. */
{
const bam1_core_t *core = &bam->core;
if (core->flag & BAM_FUNMAP)
    return 0;
struct bamTrackData *btd = (struct bamTrackData *)data;
if (sameString(bam1_qname(bam), btd->itemName))
    {
    if (btd->pairHash == NULL || (core->flag & BAM_FPAIRED) == 0)
	{
	if (core->pos == btd->itemStart)
	    singleBamDetails(bam);
	}
    else
	{
// TODO: paired: if this is first of pair, stash; if second, print details
uglyf("<B>Note: </B>just showing half of a pair.<BR>\n");
singleBamDetails(bam); // just for now.
	}
    }
return 0;
}

void doBamDetails(struct trackDb *tdb, char *item)
/* Show details of an alignment from a BAM file. */
{
int start = cartInt(cart, "o");
// make data structure and callback for pairing if necessary and printing out info
// when we find our alignment.  If it has a stub, better search for the stub...
//show position, sequence, quality, flags

// TODO: libify
char varName[1024];
safef(varName, sizeof(varName), "%s_pairEndsByName", tdb->tableName);
boolean isPaired = cartUsualBoolean(cart, varName,
				    (trackDbSetting(tdb, "pairEndsByName") != NULL));

// TODO: libify
char *seqNameForBam = seqName;
char *stripPrefix = trackDbSetting(tdb, "stripPrefix");
if (stripPrefix && startsWith(stripPrefix, seqName))
    seqNameForBam = seqName + strlen(stripPrefix);
char posForBam[512];
safef(posForBam, sizeof(posForBam), "%s:%d-%d", seqNameForBam, winStart, winEnd);

struct hash *pairHash = isPaired ? hashNew(0) : NULL;
struct bamTrackData btd = {start, item, pairHash};
bamFetch(database, tdb->tableName, posForBam, oneBam, &btd);
if (isPaired)
    {
    struct hashEl *hel;
    struct hashCookie cookie = hashFirst(pairHash);
    while ((hel = hashNext(&cookie)) != NULL)
	{
	// uh-oh -- we need to search for the mate.
	}
    }
}

#endif//def USE_BAM

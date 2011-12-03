/* liftOverMerge - merge multiple regions in BED5's generated by liftOver -multiple */

#include "common.h"
#include "bed.h"
#include "linefile.h"
#include "options.h"


struct bedList 
    {
    struct bedList *next;
    char *name;
    struct bed *bed;
    };

struct optionSpec optionSpecs[] =
/* command line option specification */
    {
        { "mergeGap", OPTION_INT },
        { NULL, 0 }
    };

int mergeGap = 0;

void usage()
/* Explain usage and exit. */
{
errAbort(
    "liftOverMerge - Merge multiple regions in BED 5 files\n"
    "                   generated by liftOver -multiple\n"
    "usage:\n"
    "   liftOverMerge oldFile newFile\n"
    "options:\n"
    "   -mergeGap=N    Max size of gap to merge regions (default 0)\n"
    );
}

void liftOverMerge(char *oldFile, char *newFile)
/* liftOverMerge - Merge regions in BED5  generated by liftOver -multiple */
{
struct bed *bedList = NULL, *bed = NULL, *otherBed = NULL, *nextBed = NULL;
struct bedList *bedListHeaders = NULL, *bedListHeader = NULL;
FILE *f = mustOpen(newFile, "w");

bedList = bedLoadNAll(oldFile, 5);

/* break down bed list into a list of lists, one per "region", where region
 * is the name field in the bed */
for (bed = bedList; bed != NULL; bed = nextBed)
    {
    verbose(3, "%s:%d-%d %s %d\n", bed->chrom, bed->chromStart, bed->chromEnd,
                                        bed->name, bed->score);
    if (bedListHeader == NULL || 
            differentString(bed->name, bedListHeader->name))
        {
        verbose(2, "region %s\n", bed->name);
        AllocVar(bedListHeader);
        bedListHeader->name = cloneString(bed->name);
        slAddHead(&bedListHeaders, bedListHeader);
        }
    nextBed = bed->next;
    slAddHead(&bedListHeader->bed, bed);
    }
slReverse(&bedListHeaders);

for (bedListHeader = bedListHeaders; bedListHeader != NULL; 
        bedListHeader = bedListHeader->next)
    {
    int ix = 1;
    verbose(3, "region %s\n", bedListHeader->name);
    slReverse(&bedListHeader->bed);

    /* traverse list of bed lists, merging overlapping entries 
     * for each region */
    for (bed = bedListHeader->bed; bed != NULL; bed = bed->next)
        {
        for (otherBed = bed->next; otherBed != NULL; otherBed = nextBed)
            {
            nextBed = otherBed->next;
            if (sameString(bed->chrom, otherBed->chrom) && 
                (max(bed->chromStart, otherBed->chromStart) <= 
                 min(bed->chromEnd, otherBed->chromEnd) + mergeGap))
                {
                /* these regions overlap (or are within the merge gap),
                 * so create one that is a merge, and drop the other */
                verbose(2,"merging %s:%d-%d, %s:%d-%d (overlap=%d)",
                    otherBed->chrom, otherBed->chromStart, otherBed->chromEnd,
                    bed->chrom, bed->chromStart, bed->chromEnd,
                    min(bed->chromEnd, otherBed->chromEnd) -
                        max(bed->chromStart, otherBed->chromStart)); 
                bed->chromStart = min(otherBed->chromStart, bed->chromStart);
                bed->chromEnd = max(otherBed->chromEnd, bed->chromEnd);
                verbose(2," to %s:%d-%d\n",
                        bed->chrom, bed->chromStart, bed->chromEnd);
                slRemoveEl(&bedListHeader->bed, otherBed);
                }
            }
        }
    for (otherBed = bedListHeader->bed; otherBed != NULL; 
            otherBed = otherBed->next)
        {
        otherBed->score = ix++;
        bedOutputN(otherBed, 5, f, '\t', '\n');
        }
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc != 3)
    usage();
mergeGap = optionInt("mergeGap", mergeGap);
liftOverMerge(argv[1], argv[2]);
return 0;
}



/* lift genome annotations between assemblies using chain files */

#ifndef LIFTOVER_H
#define LIFTOVER_H

#define LIFTOVER_MINMATCH        0.95
#define LIFTOVER_MINBLOCKS       1.00

struct liftOverChain *liftOverChainList();
/* Get list of all liftOver chains in the central database */

char *liftOverChainFile(char *fromDb, char *toDb);
/* Get filename of liftOver chain */

int liftOverBed(char *fileName, struct hash *chainHash, 
                        double minMatch,  double minBlocks, 
                        int minSizeT, int minSizeQ,
                        bool fudgeThick, FILE *f, FILE *unmapped, 
                        bool multiple, int *errCt);
/* Open up file, decide what type of bed it is, and lift it. 
 * Return the number of records successfully converted */

int liftOverPositions(char *fileName, struct hash *chainHash, 
                        double minMatch,  double minBlocks, bool fudgeThick,
                                FILE *f, FILE *unmapped, int *errCt);
/* Create bed file from positions (chrom:start-end) and lift.
 * Return the number of records successfully converted */

char *liftOverRemapRange(struct hash *chainHash, double minRatio,
			char *chrom, int s, int e, char strand, double minMatch,
			 char **retChrom, int *retStart, int *retEnd, char *retStrand);
/* Remap a range through chain hash.  If all is well return NULL
 * and results in retChrom, retStart, retEnd.  Otherwise
 * return a string describing the problem. */

void liftOverGff(char *fileName, struct hash *chainHash, 
                    double minMatch, double minBlocks, 
                    FILE *mapped, FILE *unmapped);
/* Lift over GFF file */

void liftOverPsl(char *fileName, struct hash *chainHash, 
                            double minMatch, double minBlocks, bool fudgeThick,
                                FILE *f, FILE *unmapped);
/* Open up PSL file, and lift it. */

void liftOverGenePred(char *fileName, struct hash *chainHash, 
                        double minMatch, double minBlocks, bool fudgeThick,
                        FILE *mapped, FILE *unmapped);
/* Lift over file in genePred format. */

void liftOverSample(char *fileName, struct hash *chainHash, 
                        double minMatch, double minBlocks, bool fudgeThick,
                        FILE *mapped, FILE *unmapped);
/* Open up sample file,  and lift it */

void readLiftOverMap(char *fileName, struct hash *chainHash);
/* Read map file into hashes. */

char *liftOverErrHelp();
/* Help message explaining liftOver failures */


#endif



/*
  File: pslPairs.c
  Author: Terry Furey
  Date: 10/1/2003
  Description: Create end sequence pairs from psl files
*/

#include "common.h"
#include "linefile.h"
#include "memalloc.h"
#include "hash.h"
#include "jksql.h"
#include "hdb.h"
#include "fuzzyFind.h"
#include "portable.h"
#include "psl.h"
#include "options.h"
#include "binRange.h"

/* command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"verbose", OPTION_BOOLEAN},
    {"slop", OPTION_BOOLEAN},
    {"short", OPTION_BOOLEAN},
    {"long", OPTION_BOOLEAN},
    {"mismatch", OPTION_BOOLEAN},
    {"orphan", OPTION_BOOLEAN},
    {"noRandom", OPTION_BOOLEAN},
    {"noBin", OPTION_BOOLEAN},
    {"min", OPTION_INT},
    {"max", OPTION_INT},
    {"slopval", OPTION_INT},
    {"nearTop", OPTION_FLOAT},
    {"minId", OPTION_FLOAT},
    {"minOrphanId", OPTION_FLOAT},
    {"tInsert", OPTION_INT},
    {"hardMax", OPTION_INT},
    {NULL, 0}
};

boolean VERBOSE = FALSE;
boolean SLOP = FALSE;
boolean SHORT = FALSE;
boolean LONG = FALSE;
boolean MISMATCH = FALSE;
boolean ORPHAN = FALSE;
boolean NORANDOM = FALSE;
boolean NOBIN = FALSE;
int MIN;
int MAX;
int SLOPVAL;
float NEARTOP;
int TINSERT;
int HARDMAX;
float MIN_ID;
float MIN_ORPHAN_ID;

FILE *of, *orf, *sf, *mf, *esf, *elf;

struct cloneName
{
  char name[32];
};

struct pslAli
{
  struct pslAli *next;
  int score;
  float id;
  float cov;
  struct psl *psl;
};

struct pslPair
{
  struct pslPair *next;
  struct pslAli *f;
  struct pslAli *r;
  int score;
  int distance;
  boolean orien;
  boolean random;
};

struct clone
{
  struct clone *next;
  char name[32];
  struct pslAli *end1;
  struct pslAli *end2;
  struct pslPair *pairs;
  struct pslPair *pairsRandom;
  struct pslPair *pairsSlop;
  struct pslPair *pairsExtremeS;
  struct pslPair *pairsExtremeL;
  struct pslPair *pairsMM;
  boolean orphan;
} *cloneList = NULL;

struct hash *clones = NULL;
struct hash *leftNames = NULL;
struct hash *rightNames = NULL;

void pslAliFree(struct pslAli **pa)
{
  struct pslAli *el;

  if ((el = *pa) == NULL) return;
  pslFree(&(el->psl));
  freez(pa);
}

void pslAliListFree(struct pslAli **pList)
{
  struct pslAli *i, *next;

  for (i = *pList; i != NULL; i = next)
    {
      next = i->next;
      pslAliFree(&i);
    }
  *pList = NULL;
}

void pslPairFree(struct pslPair **pp)
{
  freez(pp);
}

void pslPairListFree(struct pslPair **pList)
{
  struct pslPair *i, *next;

  for (i = *pList; i != NULL; i = next)
    {
      next = i->next;
      pslPairFree(&i);
    }
  *pList = NULL;
}

void cloneFree(struct clone **c)
{
  struct clone *el;

  if ((el = *c) == NULL) return;
  pslAliListFree(&(el->end1));
  pslAliListFree(&(el->end2));
  pslPairListFree(&(el->pairs));
  freez(c);
}

struct pslAli *createPslAli(struct psl *psl)
/* Create a pslAli element from a psl record */
{
  struct pslAli *pa;
  
  AllocVar(pa);
  pa->next = NULL;
  pa->psl = psl;
  pa->score = pslScore(psl);
  pa->id = (float)(psl->match + psl->repMatch + psl->nCount)/(psl->match + psl->misMatch + psl->repMatch + psl->nCount);
  pa->cov = (float)(psl->match + psl->misMatch + psl->repMatch + psl->nCount)/psl->qSize;
  return(pa);
}

struct clone *createClone(char *name, char *eName1, char *eName2)
/* Create a clone element from the names */
{
  struct clone *clone;

  AllocVar(clone);
  clone->next = NULL;
  sprintf(clone->name,"%s",name);
  clone->end1 = NULL;
  clone->end2 = NULL;
  clone->pairs = NULL;
  clone->orphan = FALSE;

  return(clone);
}
struct pslPair *createPslPair(struct pslAli *e1, struct pslAli *e2)
{
  struct pslPair *pp;

  if (strcmp(e1->psl->tName, e2->psl->tName)) return NULL;
  if ((e1->id < MIN_ID) && (e2->id < MIN_ID)) return NULL;
  AllocVar(pp);
  pp->next = NULL;
  if ((e1->psl->tStart) < (e2->psl->tStart))
    {
      pp->f = e1;
      pp->r = e2;
    }
  else
    {
       pp->f = e2;
      pp->r = e1;
    }
  pp->distance = pp->r->psl->tEnd - pp->f->psl->tStart;
  if ((pp->f->psl->strand[0] == '+') && (pp->r->psl->strand[0] == '-'))
    pp->orien = TRUE;
  else 
    pp->orien = FALSE;
  if (strlen(e1->psl->tName) > 6)
    pp->random = TRUE;
  else 
    pp->random = FALSE;
  pp->score = e1->score + e2->score;

  return(pp);
}

void readPairFile(struct lineFile *prf)
/* Read in pairs and initialize clone list */
{
  int lineSize, i;
  char *line;
  char *words[4];
  char *names[16];
  int wordCount, nameCount;
  struct clone *clone;
  struct cloneName *cloneName;
  
  while (lineFileNext(prf, &line, &lineSize))
    {
      wordCount = chopTabs(line,words);
      if (wordCount != 3)
	errAbort("Bad line %d of %s\n", prf->lineIx, prf->fileName);
      if (!hashLookup(clones, words[2])) 
	{
	  clone = createClone(words[2],NULL,NULL);
	  hashAdd(clones, words[2], clone);
	  slAddHead(&cloneList,clone);
	}
      char *test;
      AllocVar(cloneName);
      sprintf(cloneName->name, "%s", words[2]);
      nameCount = chopCommas(words[0],names);
      for (i = 0; i < nameCount; i++) 
	hashAdd(leftNames, names[i], cloneName);
      nameCount = chopCommas(words[1],names);
      for (i = 0; i < nameCount; i++) 
	hashAdd(rightNames, names[i], cloneName);
    }     
}

void readPslFile(struct lineFile *pf)
/* Process all records in a psl file of mRNA alignments */
{
 int lineSize;
 char *line;
 char *words[32];
 int  wordCount;
 struct psl *psl;
 struct clone *clone;
 struct pslAli *pa = NULL;
 struct cloneName *cloneName;
 
 while (lineFileNext(pf, &line, &lineSize))
   {
     wordCount = chopTabs(line, words);
     if (wordCount != 21)
       errAbort("Bad line %d of %s\n", pf->lineIx, pf->fileName);
     psl = pslLoad(words);
     if (hashLookup(leftNames, psl->qName))
       cloneName = hashMustFindVal(leftNames, psl->qName);
     else if (hashLookup(rightNames, psl->qName))
       cloneName = hashMustFindVal(rightNames, psl->qName);
     else
       continue;
     clone = hashMustFindVal(clones, cloneName->name);
     if ((psl->tBaseInsert < TINSERT) && ((!NORANDOM) || (strlen(psl->tName) < 7))) 
       {
	 pa = createPslAli(psl);
	 if (hashLookup(leftNames, psl->qName))
	   slAddHead(&(clone->end1), pa);
	 else
	   slAddHead(&(clone->end2), pa);
       }
   }
}

void pslPairs()
{
  struct clone *clone;
  struct pslAli *e1, *e2;
  struct pslPair *pp;

  for (clone = cloneList; clone != NULL; clone = clone->next)
    {
      for (e1 = clone->end1; e1 != NULL; e1 = e1->next)
	for (e2 = clone->end2; e2 != NULL; e2 = e2->next)
	  {
	    if ((pp = createPslPair(e1, e2)) != NULL)
	      {		
		if ((pp->distance >= MIN) && (pp->distance <= MAX))
		  {
		    if ((!pp->orien) && (!pp->random) && (MISMATCH)) 
		      slAddHead(&(clone->pairsMM),pp);
		    else if ((pp->orien) && (pp->random) && (!NORANDOM))
		      slAddHead(&(clone->pairsRandom),pp);
		    else if ((pp->orien) && (!pp->random))
		      slAddHead(&(clone->pairs),pp);
		    else 
		      pslPairFree(&pp);
		  }
		else if (!pp->random && pp->orien)
		  {
		    if ((pp->distance >= (MIN-SLOPVAL)) && (pp->distance <= (MAX+SLOPVAL)) && (SLOP))
		      slAddHead(&(clone->pairsSlop),pp);
		    else if ((pp->distance < MIN) && (SHORT))
		      slAddHead(&(clone->pairsExtremeS),pp);
		    else if ((pp->distance > MAX) && (pp->distance <= HARDMAX) && (LONG))
		      slAddHead(&(clone->pairsExtremeL),pp);
		    else 
		      pslPairFree(&pp);
		  }
		else 
		  pslPairFree(&pp);
	      }
	  }
      if (!(clone->pairs) && !(clone->pairsRandom) && !(clone->pairsSlop) 
	  && !(clone->pairsExtremeS) && !(clone->pairsExtremeL) && !(clone->pairsMM))
	{
	  clone->orphan = TRUE;
	}
    }
}

void printBed(FILE *out, struct pslPair *ppList, struct clone *clone, char *pslTable)
{
  struct pslPair *pp, *ppPrev;
  int count = 0, best = 0;

  for (pp = ppList; pp != NULL; pp=pp->next)
    if (pp->score > best)
      best = pp->score;
  while (((ppList->score)/best) < NEARTOP)
    {
      struct pslPair *temp = ppList;
      ppList = ppList->next;
      pslPairFree(&temp);
    }
  ppPrev = ppList;
  for (pp = ppList; pp != NULL; pp=pp->next)
    {
      if (((pp->score)/best) >= NEARTOP)
	count++;
      else
	{
	  ppPrev->next = pp->next;
	  pslPairFree(&pp);
	  pp = ppPrev;
	}
      ppPrev = pp;
    }
  for (pp = ppList; pp != NULL; pp=pp->next)
    {
      int bin = binFromRange(pp->f->psl->tStart,pp->r->psl->tEnd);
      int score = 1000;
      char *strand;
      int d1, d2;

      if (count != 1)
	score = 1500/count;
      if (hashLookup(leftNames, pp->f->psl->qName))
	strand = "+";
      else 
	strand = "-";
      d1 = pp->f->psl->tEnd - pp->f->psl->tStart;
      d2 = pp->r->psl->tEnd - pp->r->psl->tStart;
      
      if (!NOBIN) 
	fprintf(out, "%d\t",bin);
      fprintf(out, "%s\t%d\t%d\t%s\t%d\t%s\t%s\t2\t%d,%d\t%d,%d\t%s,%s\n",
	      pp->f->psl->tName,pp->f->psl->tStart,pp->r->psl->tEnd,clone->name,
	      score, strand, pslTable, pp->f->psl->tStart, pp->r->psl->tStart, d1, d2, 
	      pp->f->psl->qName, pp->r->psl->qName);
    }
}

void printOrphan(FILE *out, struct pslAli *paList, struct clone *clone, char *pslTable)
{
  struct pslAli *pa;
  int best = 0, count = 0;

  for (pa = paList; pa != NULL; pa=pa->next)
    if (pa->score > best)
      best = pa->score;
  for (pa = paList; pa != NULL; pa=pa->next)
    if ((((pa->score)/best) > NEARTOP) && (pa->id >= MIN_ORPHAN_ID))
      count++;
  for (pa = paList; pa != NULL; pa=pa->next)
    if ((((pa->score)/best) > NEARTOP) && (pa->id >= MIN_ORPHAN_ID))
      {
      int bin = binFromRange(pa->psl->tStart,pa->psl->tEnd);
      int score = 1000;
      char *strand;
      int d1, genStart = 0, genEnd = 0;

      if (count != 1)
	score = 1500/count;
      if (((hashLookup(leftNames,pa->psl->qName)) && (pa->psl->strand[0] == '+')) ||
	  ((hashLookup(rightNames,pa->psl->qName)) && (pa->psl->strand[0] == '-')))
	{
	strand = "+";
	genStart = pa->psl->tStart;
	genEnd = pa->psl->tEnd+(MIN/2);
	}
      else 
	{
	strand = "-";
	genStart = pa->psl->tStart-(MIN/2);
	if (genStart < 0)
	  genStart = 0;
	genEnd = pa->psl->tEnd;
	}
      d1 = pa->psl->tEnd - pa->psl->tStart;

      if (!NOBIN) 
	fprintf(out, "%d\t",bin);
      fprintf(out, "%s\t%d\t%d\t%s\t%d\t%s\t%s\t1\t%d\t%d\t%s\n",
	      pa->psl->tName,genStart,genEnd,clone->name,
	      score, strand, pslTable, pa->psl->tStart, d1, pa->psl->qName);        
      }
}

void printOut(char *pslTable)
{
  struct clone *clone = NULL;

  for (clone = cloneList; clone != NULL; clone = clone->next)
    {
      if (clone->pairs)
	printBed(of, clone->pairs, clone, pslTable);
      else if (clone->pairsRandom)
	printBed(of, clone->pairsRandom, clone, pslTable);
      else if ((clone->pairsSlop) && (SLOP))
	printBed(sf, clone->pairsSlop, clone, pslTable);
      else if ((clone->pairsMM) && (MISMATCH))
	printBed(mf, clone->pairsMM, clone, pslTable);
      else if ((clone->pairsExtremeS) && (SHORT))
	printBed(esf, clone->pairsExtremeS, clone, pslTable);
      else if ((clone->pairsExtremeL) && (LONG))
	printBed(elf, clone->pairsExtremeL, clone, pslTable);
      else if ((clone->orphan) && (ORPHAN))
	{
	printOrphan(orf, clone->end1, clone, pslTable);
	printOrphan(orf, clone->end2, clone, pslTable);
	}
    }
}

int main(int argc, char *argv[])
{
  struct lineFile *pf, *prf;
  char filename[64], *db, *pslTable;

  optionInit(&argc, argv, optionSpecs);
  if (argc < 4)
    {
      fprintf(stderr, "USAGE: pslPairs <psl file> <pair file> <psl table name> <out file prefix>\n  Options:\n\t-max=N\t\tmaximum length of clone sequence (default=50000)\n\t-min=N\t\tminimum length of clone sequence (default=30000)\n\t-slopval=N\tdeviation from max/min clone lengths allowed for slop report (default=5000)\n\t-nearTop=N\tmaximium deviation from best match allowed (default=0.001)\n\t-minId=N\tminimum pct ID of at least one end (default=0.96)\n\t-minOrphanId=N\tminimum pct ID for orphan alignment (default=0.96)\n\t-tInsert=N\tmaximum insert bases allowed in sequence alignment (default=500)\n\t-hardMax=N\tabsolute maximum clone length for long report (default=75000)\n\t-verbose\tdisplay all informational messages\n\t-noBin\t\tdo not include bin column in output file\n\t-noRandom\tdo not include placements on random portions\n\t-slop\t\tcreate file of pairs that fall within slop length\n\t-short\t\tcreate file of pairs shorter than min size\n\t-long\t\tcreate file of pairs longer than max size, but less than hardMax size\n\t-mismatch\tcreate file of pairs with bad orientation of ends\n\t-orphan\t\tcreate file of unmatched end sequences\n");
      return 1;
    }
  VERBOSE = optionExists("verbose");
  SLOP = optionExists("slop");
  SHORT = optionExists("short");
  LONG = optionExists("long");
  MISMATCH = optionExists("mismatch");
  ORPHAN = optionExists("orphan");
  NORANDOM = optionExists("noRandom");
  NOBIN = optionExists("noBin");
  MIN = optionInt("min",32000);
  MAX = optionInt("max",47000);
  TINSERT = optionInt("tInsert",500);
  HARDMAX = optionInt("hardMax",75000);
  if (SLOP)
      SLOPVAL = optionInt("slopval",5000);
  else
      SLOPVAL = 0;
  NEARTOP = optionFloat("nearTop",0.001);
  NEARTOP = 1 - NEARTOP;
  MIN_ID = optionFloat("minId",0.96);
  MIN_ORPHAN_ID = optionFloat("minOrphanId",0.96);

  pf = pslFileOpen(argv[1]);
  prf = lineFileOpen(argv[2],TRUE);
  pslTable = cloneString(argv[3]);

  sprintf(filename, "%s.pairs", argv[4]);
  of = mustOpen(filename, "w");
  if (SLOP)
    {
      sprintf(filename, "%s.slop", argv[4]);
      sf = mustOpen(filename, "w");
    }
  if (ORPHAN)
    {
      sprintf(filename, "%s.orphan", argv[4]);
      orf = mustOpen(filename, "w");
    }
  if (MISMATCH)
    {
      sprintf(filename, "%s.mismatch", argv[4]);
      mf = mustOpen(filename, "w");
    }
  if (SHORT)
    {
      sprintf(filename, "%s.short", argv[4]);
      esf = mustOpen(filename, "w");
    }
  if (LONG)
    {
      sprintf(filename, "%s.long", argv[4]);
      elf = mustOpen(filename, "w");
    }
      
  if (VERBOSE)
    printf("Reading pair file\n");
  clones = newHash(23);
  leftNames = newHash(23);
  rightNames = newHash(23);
  readPairFile(prf);
  if (VERBOSE)
    printf("Reading psl file\n");
  readPslFile(pf);
  if (VERBOSE)
    printf("Creating Pairs\n");
  pslPairs();
  if (VERBOSE) 
    printf("Writing to files\n");
  printOut(pslTable);
  
  lineFileClose(&pf);
  lineFileClose(&prf);
  fclose(of);
  if (ORPHAN)
    fclose(orf);
  if (SLOP)
    fclose(sf);
  if (MISMATCH)
    fclose(mf);
  if (SHORT)
    fclose(esf);
  if (LONG)
    fclose(elf);
 
  return 0;
}



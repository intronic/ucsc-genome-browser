#include "common.h"
#include "errabort.h"
#include "linefile.h"
#include "hdb.h"
#include "hash.h"
#include "bed.h"
#include "psl.h"
#include "options.h"
#include "phyloTree.h"
#include "element.h"
#include "dystring.h"

static char const rcsid[] = "$Id: atomString.c,v 1.1 2007/04/09 16:39:53 braney Exp $";

void usage()
/* Explain usage and exit. */
{
errAbort(
  "atomString - string together atoms\n"
  "usage:\n"
  "   atomString in.atom in.seq out.atom out.seq\n"
  "arguments:\n"
  "   in.atom        list of atoms to string\n"
  "   in.seq         list of sequence (atom orderings)\n"
  "   out.atom       list of atoms to string\n"
  "   in.seq         list of sequence (atom orderings)\n"
  "options:\n"
  "   -minLen=N       minimum size of atom to consider\n"
  );
}

static struct optionSpec options[] = {
   {"minLen", OPTION_INT},
   {NULL, 0},
};

int maxGap = 100000;
int minLen = 1;
boolean noOverlap = FALSE;
char *stringsFile = NULL;

#define NUMSTARTS 10000
int instanceStarts[NUMSTARTS];

struct instance;

struct atom
{
struct atom *next;
char *name;
int numInstances;
int length;
struct instance *instances;
int stringNum;
};

struct instance
{
struct instance *next; /* in atom */
int num; /* in atom */
char *species;
char *chrom;
int start;
int end;
char strand;
struct instance **seqPos; /* in sequence */
struct atom *atom;
};

struct sequence
{
struct sequence *next;
char *name;
int length;
struct instance **data;
};


struct atomString
{
struct atomString *next;
int num;
int length;
int count;
//struct atom *atoms;
struct instance *instances;
};

char *bigWords[10*10*1024];

char *getShareString(char *name)
{
static struct hash *stringHash = NULL;

if (stringHash == NULL)
    stringHash = newHash(4);

return hashStoreName(stringHash, name);
}

/*
void calcLens(struct atomString *strings)
{
for(; strings; strings = strings->next)
    {
    //slReverse(&strings->atoms);

    struct atom *atoms = strings->atoms;
    for(; atoms ; atoms = atoms->next)
	{
	strings->count++;
	strings->length += atoms->length;
	}
    }
}
*/

/*
int atomCmp(const void *va, const void *vb)
{
const struct atom *a = *((struct atom **)va);
const struct atom *b = *((struct atom **)vb);

return a->order - b->order;
}
*/

/*
int stringCmp(const void *va, const void *vb)
{
const struct atomString *a = *((struct atomString **)va);
const struct atomString *b = *((struct atomString **)vb);
int dif;
dif = b->length - a->length;

return dif;
}
*/

boolean checkContig(struct atom *a1, struct atom *a2)
{
struct instance *i1 = a1->instances;
struct instance *i2 = a2->instances;

//printf("in atoms %s %s\n",a1->name,a2->name);
for(; (i1 != NULL) && (i2 != NULL); i1 = i1->next, i2 = i2->next)
    {
    if ((i1->strand == i2->strand) 
	&& (i1->species == i2->species)
	&& (i1->chrom == i2->chrom))
	{
	if (i1->strand == '+')
	    {
	    //printf("atoms %s %s prev end %d next start %d\n",a1->name,a2->name,i1->end,i2->start);
	    if (i1->end > i2->start)
		return FALSE;
	    if (i1->end + maxGap < i2->start)
		return FALSE;
	    }
	else
	    {
	    if (i2->end > i1->start)
		return FALSE;
	    if (i2->end + maxGap < i1->start)
		return FALSE;
	    }
	}
    else
	return FALSE;
    }

//printf("returning %d\n", (i1 == NULL) && (i2 == NULL));
return (i1 == NULL) && (i2 == NULL);
}

void buildStrings(struct atomString **strings, struct sequence *seq)
{
struct instance **data = &seq->data[1];
struct instance **last = &seq->data[seq->length + 1];
static int stringNum = 1;

for(; data < last; )
    {
    struct atom *atom = (*data)->atom;

    data++;

    if (atom->stringNum != 0)
	continue;

    struct atomString *string = NULL;

    AllocVar(string);
    string->num = stringNum++;
    //string->count++;
    slAddHead(strings, string);
    atom->stringNum = string->num;

    int count = 0;
    struct instance *instance;
    for( instance = atom->instances; instance;  instance = instance->next)
	{
	if (instance->strand == '+')
	    instanceStarts[count++] = instance->start;
	else
	    instanceStarts[count++] = instance->end;
	}

    //printf("this atom %s\n",atom->name);
    for(;;)
	{
	struct atom *firstNextAtom = NULL;

	for( instance = atom->instances; instance;  instance = instance->next)
	    {
	    struct instance *nextInSeq ;
	    
	    if (instance->strand == '+')
		nextInSeq = *(instance->seqPos + 1);
	    else
		nextInSeq = *(instance->seqPos - 1);

	    if (nextInSeq == NULL)
		break;

	    struct atom *nextAtom = nextInSeq->atom;
	    //printf("next atom %s\n",nextAtom->name);

	    if (nextAtom->stringNum)
		break;

	    if (firstNextAtom == NULL)
		firstNextAtom = nextAtom;
	    else if (nextAtom != firstNextAtom)
		break;
	    }
	if (instance != NULL)
	    break;

	struct atom *firstPrevAtom = NULL;

	for( instance = firstNextAtom->instances; instance;  instance = instance->next)
	    {
	    struct instance *prevInSeq;

	    if (instance->strand == '+')
		prevInSeq = *(instance->seqPos - 1);
	    else
		prevInSeq = *(instance->seqPos + 1);

	    if (prevInSeq == NULL)
		break;

	    struct atom *prevAtom = prevInSeq->atom;
	    //printf("prev atom %s\n",prevAtom->name);

	    //if (prevAtom->stringNum)
		//break;

	    if (firstPrevAtom == NULL)
		firstPrevAtom = prevAtom;
	    else if (prevAtom != firstPrevAtom)
		break;
	    }

	if (instance != NULL)
	    break;

	//printf("adding %s\n",firstNextAtom->name);
	firstNextAtom->stringNum = string->num;
	//string->count++;
	atom = (*data)->atom;
	data++;
	}

    //lastAtom = (firstNextAtom == NULL) ? atom1 : firstNextAtom;
    count = 0;
    for( instance = atom->instances;  instance; instance = instance->next)
	{
	struct instance *newInt;
	int length;

	AllocVar(newInt);
	*newInt = *instance;
	slAddHead(&string->instances, newInt);
	if (instance->strand == '+') 
	    newInt->start = instanceStarts[count];
	else
	    newInt->end = instanceStarts[count];
	count++;
	length = newInt->end - newInt->start;

	if (length > string->length)
	    string->length = length;
	}

    string->count = count;
    slReverse(&string->instances);
    }
}

struct atom *getAtoms(char *atomsName, struct hash *atomHash)
{
struct lineFile *lf = lineFileOpen(atomsName, TRUE);
struct atom *atoms = NULL;
int wordsRead;
struct atom *anAtom = NULL;
int intCount = 0;
int count = 0;

while( (wordsRead = lineFileChopNext(lf, bigWords, sizeof(bigWords)/sizeof(char *)) ))
    {
    if (wordsRead == 0)
	continue;

    if (*bigWords[0] == '>')
	{
	int num;

	if (anAtom != NULL)
	    slReverse(&anAtom->instances);
	if (wordsRead != 3)
	    errAbort("need 3 fields on atom def line");

	intCount = atoi(bigWords[2]);
	if ((num = atoi(bigWords[1])) >= minLen)
	    {
	    AllocVar(anAtom);
	    slAddHead(&atoms, anAtom);
	    anAtom->name = cloneString(&bigWords[0][1]);
	    //printf("adding %s to hash\n",anAtom->name);
	    hashAdd(atomHash, anAtom->name, anAtom);
	    anAtom->length = num;
	    anAtom->numInstances = intCount;
	    if (anAtom->numInstances >= NUMSTARTS)
		errAbort("exceeded static max number dups %d\n",NUMSTARTS);
	    //printf("defined atom %s\n",anAtom->name);
	    }
	else
	    anAtom = NULL;
	count = 0;
	}
    else
	{
	struct instance *instance;
	char *ptr, *chrom, *start;

	intCount--;
	if (intCount < 0)
	    errAbort("bad atom: too many instances");
	if (wordsRead != 2)
	    errAbort("need 2 fields on instance def line");
	if (anAtom == NULL)
	    continue;

	AllocVar(instance);
	instance->num = count++;
	slAddHead(&anAtom->instances, instance);

	ptr = strchr(bigWords[0], '.');
	*ptr++ = 0;
	chrom = ptr;
	instance->species = getShareString(bigWords[0]);

	ptr = strchr(chrom, ':');
	*ptr++ = 0;
	start = ptr;
	instance->chrom = getShareString(chrom);

	ptr = strchr(start, '-');
	*ptr++ = 0;
	instance->start = atoi(start);
	instance->end = atoi(ptr);
	instance->strand = *bigWords[1];
	
	//printf("got instance %s %s %d %d %c\n",instance->species,instance->chrom, instance->start, instance->end, instance->strand);
	}
    }
if (anAtom != NULL)
    slReverse(&anAtom->instances);
lineFileClose(&lf);
//slReverse(&atoms);

return atoms;
}

#ifdef NOTNOW
void sortAtoms(struct atom *atoms, struct sequence *sequence,
	 struct hash *atomHash)
{
struct atom *anAtom = atoms;
char **ptr = sequence->data;
char **last = &sequence->data[sequence->length];
int orderNum = 1;

for(; anAtom; anAtom = anAtom->next)
    anAtom->order = 0;

for(; ptr < last; ptr++)
    {
    boolean isNeg = **ptr == '-';
    char *name = (isNeg) ? *ptr + 1 : *ptr;
    struct atom *anAtom = hashFindVal(atomHash, name);

    if (anAtom == NULL)
	continue;

    if (anAtom->order == 0)
	anAtom->order = orderNum++;
    else if (anAtom->stringNum == 0)
	errAbort("not supporting dups now");
    }

slSort(&atoms, atomCmp);
}
#endif

#ifdef NOTNOW
void outStrings(struct atomString *strings, char *stringName)
{
FILE *outF = mustOpen(stringName, "w");

for(; strings; strings = strings->next)
    {
    struct atom *atom = strings->atoms, *last = NULL;
    struct instance *instance;
    int count = 0;

    fprintf(outF, "%d\t%d\t%d\t",strings->num,strings->length,strings->count);

    for( instance = atom->instances; instance;  instance = instance->next)
	{
	if (instance->strand == '+')
	    starts[count++] = instance->start;
	else
	    starts[count++] = instance->end;
	}

    assert(count <= 4);

    for(last=atom; atom ; last = atom, atom = atom->next)
	;

    count = 0;
    for( instance = last->instances;  instance; instance = instance->next)
	{
	if (instance->strand == '+') 
	//if (starts[count] < instance->end)
	    fprintf(outF,"%s.%s:%d-%d.%c,",instance->species,instance->chrom,
		starts[count], instance->end, instance->strand);
	else
	    fprintf(outF,"%s.%s:%d-%d.%c,",instance->species,instance->chrom,
		instance->start, starts[count], instance->strand);
	count++;
	}
    fprintf(outF,"\t");


    atom = strings->atoms;
    for(; atom ; atom = atom->next)
	fprintf(outF, "%s,",atom->name);
    fprintf(outF, "\n");

    fflush(outF);
    }

}
#endif

struct sequence *getSequences(char *inName, struct hash *atomHash)
{
struct lineFile *lf = lineFileOpen(inName, TRUE);
int wordsRead, count = 0;
struct sequence *seqHead = NULL;
struct sequence *sequence = NULL;

while( (wordsRead = lineFileChopNext(lf, bigWords, 
		sizeof(bigWords)/sizeof(char *)) ))
    {
    if (*bigWords[0] == '>')
	{
	AllocVar(sequence);
	slAddHead(&seqHead, sequence);
	sequence->name = getShareString(&bigWords[0][1]);
	sequence->length = atoi(bigWords[1]);
	sequence->data = needLargeMem((sequence->length + 2) * sizeof(struct instance *));
	count = 0;
	sequence->data[count++] = NULL;
	}
    else
	{
	int ii;

	if (count + wordsRead - 1 > sequence->length)
	    errAbort("too many words for sequence");

	for(ii=0; ii < wordsRead; ii++)
	    {
	    char *ptr = strchr(bigWords[ii], '.');
	    char *name = bigWords[ii];

	    *ptr++ = 0;
	    if (*name == '-')
		name++;
	    struct atom *atom = hashFindVal(atomHash, name);

	    if (atom == NULL)
		{
		sequence->length--;
		continue;
		}

	    int num = atoi(ptr);

	    struct instance *instance = atom->instances;
	    for(; (num > 0) && instance; instance = instance->next)
		num--;

	    instance->seqPos = &sequence->data[count];
	    instance->atom = atom;
	    sequence->data[count] = instance;

	    count++;
	    }
	}
    }

lineFileClose(&lf);

slReverse(&seqHead);
return seqHead;
}

void outAtoms(struct atomString *strings, char *outName)
{
FILE *outF = mustOpen(outName, "w");

for(; strings; strings = strings->next)
    {
    struct instance *instance;

    fprintf(outF, ">%d %d %d\n",strings->num, strings->length, strings->count);
    //fprintf(outF, "%d\t%d\t%d\t",strings->num,strings->length,strings->count);

    for( instance = strings->instances;  instance; instance = instance->next)
	{
	fprintf(outF,"%s.%s:%d-%d %c\n",instance->species,instance->chrom,
	    instance->start, instance->end, instance->strand);
	}
    }
fclose(outF);
}

void outSequence(struct sequence *sequences, char *outName)
{
FILE *outF = mustOpen(outName, "w");

for(; sequences; sequences = sequences->next)
    {
    struct instance **data = &sequences->data[1];
    struct instance **lastData = &sequences->data[sequences->length + 1];
    struct atom *anAtom;
    int lastNum = 0;
    int count = 0;


    for(; data < lastData; data++)
	{
	struct instance *thisInstance = *data;
	
	anAtom = thisInstance->atom;

	if (anAtom != NULL)
	    {
	    int val = anAtom->stringNum;

	    if (val && (val != lastNum))
		count++;
	    lastNum = val;
	    }
	}

    fprintf(outF, ">%s %d\n",sequences->name,count);

    data = &sequences->data[1];
    lastNum = 0;
    count = 0;
    for(; data < lastData; data++)
	{
	struct instance *thisInstance = *data;
	boolean isNeg = thisInstance->strand == '-';
	
	anAtom = thisInstance->atom;

	if (anAtom != NULL)
	    {
	    int val = anAtom->stringNum;

	    if (val && (val != lastNum))
		{
		if (isNeg)
		    fprintf(outF,"%d.%d ",-val,thisInstance->num);
		else
		    fprintf(outF,"%d.%d ",val, thisInstance->num);
		count++;
		if ((count & 0x7) == 0x7)
		    fprintf(outF,"\n");
		lastNum = val;
		}
	    }
	}
    if (!((count & 0x7) == 0x7))
	fprintf(outF,"\n");
    }
}

void atomString(char *inAtomName, char *inSeqName, 
    char *outAtomName, char *outSeqName)
{
struct atomString *strings = NULL;
struct sequence *seq;
struct hash *atomHash = newHash(20);
struct atom *atoms = getAtoms(inAtomName, atomHash);
struct sequence *sequences = getSequences(inSeqName, atomHash);

for(;atoms; atoms = atoms->next)
    {
    struct instance *instance = atoms->instances;

    for(; instance; instance = instance->next)
	if (instance->atom == NULL) 
	    errAbort("atom NULL for instance in species %s, atom %s\n",instance->species, atoms->name);
    }

for(seq = sequences ; seq ; seq = seq->next)
    {
    printf("building strings with %s\n",seq->name);
    buildStrings(&strings, seq);
    }

slReverse(&strings);

outAtoms(strings, outAtomName);
outSequence(sequences, outSeqName);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();

minLen = optionInt("minLen", minLen);

atomString(argv[1],argv[2], argv[3], argv[4]);
return 0;
}

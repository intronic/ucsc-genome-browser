/* blast0To8 - convert blast 0 output to tabbed format. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "blastTab.h"

int lineCount = 0;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "blast0To8 - convert blast 0 output to tabbed format without gaps\n"
  "usage:\n"
  "   blast0To8 XXX\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void
myErr(char *string)
{
    char buffer[1000];
    sprintf(buffer,  "%s at %d",string,lineCount);
    errAbort(buffer);
}

void blast0To8(char *inFile, char *outTab)
/* blast0To8 - convert blast 0 output to tabbed format. */
{
struct lineFile *in = lineFileOpen(inFile, TRUE);
FILE *out = mustOpen(outTab, "w");
char *words[50];
int wordCount;
char *lastPtr, *ptr;
int qStart, qEnd, qPos;
int qGaps;
int tIncr;
int tStart, tEnd, tPos;
struct blastTab bt;
char *qString,*qPtr, *tPtr;
#define SAFE_OUT	1
#define NEED_QUERY	2
#define GOT_QUERY	3
#define GOT_SCORE	4
#define GOT_TARGET	5
#define GOT_IDENT	6
#define NEED_TARGET	7
#define NEED_TARGET_CONT 8
int state = SAFE_OUT;
boolean pending = FALSE;

lineCount = 0;
while(wordCount = lineFileChopNext(in, words, 50))
    {
	lineCount++;
	if (sameString(words[0], "Reference:"))
	{

	    if (state != SAFE_OUT)
		myErr("boop");
	    state = NEED_QUERY;
	}
	else if ((wordCount == 5) && sameString(words[0], "*****"))
	{
	    state = SAFE_OUT;
	}
	else if ((wordCount == 8) && sameString(words[0], "Score"))
	{
	    if (!((state == SAFE_OUT) || (state == GOT_TARGET)))
		myErr("need target or safeout before score");
	    if (pending)
	    {
			bt.qEnd = qStart - 1;
			bt.tEnd = tStart;// - tIncr;
			bt.aliLength = bt.qEnd - bt.qStart + 1;
		blastTabTabOut(&bt, out);
	    }
	    state = GOT_SCORE;
	    bt.bitScore = atof(words[2]);
	    bt.eValue = atof(words[7]);
	//    printf("bitscore %g eValue %g\n",bt.bitScore, bt.eValue);
	}
	else if (((wordCount == 12) || (wordCount == 8)) && sameString(words[0], "Identities"))
	{
	    int top, bottom;

	    if (state != GOT_SCORE)
		myErr("Ident without score");

	    top = atoi(words[2]);
	    if ((ptr = strchr(words[2], '/')) != NULL)
		bottom = atoi(ptr+1);
	    else 
		myErr("beep");

	    state = GOT_IDENT;
	    bt.identity = (100.0 * top) / bottom;

	    bt.gapOpen = 0;
	    qGaps = 0;
	    if (wordCount == 12)
		qGaps = atoi(words[10]);
	    bt.mismatch = bottom - top - qGaps;
	 //   printf("identity %g \n",bt.identity);
	}
	else if ((wordCount == 3) && sameString(words[0], "Frame"))
	{
	    if (words[2][0]== '+')
		tIncr = 3;
	    else
		tIncr = -3;
	}
	else if ((wordCount == 2) && sameString(words[0], "Query="))
	{
	    if (state != NEED_QUERY)
		myErr("got Query= and not ready");
	    bt.query = cloneString(words[1]);
	  //  printf("query %s\n",bt.query);
	    state = GOT_QUERY;
	}
	else if ((wordCount == 1) && startsWith(">",words[0] ))
	{
	    if (state != GOT_QUERY)
		myErr("no query at '>'");
	    bt.target = cloneString(&words[0][1]);
	   // printf("target %s\n",bt.target);
	   state = GOT_TARGET;
	}
	else if ((wordCount ==4) && sameString(words[0], "Query:"))
	{
	    if (!((state == GOT_IDENT) || (state == SAFE_OUT)))
		myErr("got second query");
	    qStart = atoi(words[1]);
	    if (state == SAFE_OUT)
	    {
		//prinf("%d %d\n",qEnd, qStart);
		if (qStart != 1 + qEnd)
		    myErr("query continue");
		state = NEED_TARGET_CONT;
	    }
	    else
		state = NEED_TARGET;

	    qString = cloneString(words[2]);
	    qEnd = atoi(words[3]);
	}
	else if ((wordCount ==4) && sameString(words[0], "Sbjct:"))
	{
	    if (!((state == NEED_TARGET) || (state == NEED_TARGET_CONT)))
		myErr("don't need target");
	    if (state == NEED_TARGET_CONT)
	    {
		//if (tStart != 1 + tEnd)
		 //   myErr("target continue");
		bt.qEnd = qEnd;
		bt.tEnd = tEnd;
		if (tStart != atoi(words[1]))
		{printf("%d %d\n",tStart,atoi(words[1]));
		    myErr("cont target not right");
		}
		tEnd = atoi(words[3]);
	    }
	    else
	    {
	    tStart = atoi(words[1]);
	    tEnd = atoi(words[3]);
		bt.qStart = qStart;
		bt.qEnd = qEnd;
		bt.tStart = tStart;
		bt.tEnd = tEnd;
	    }
	    qPtr = qString;
	    tPtr = words[2];
	    if (strlen(qString) != strlen(words[2]))
		myErr("q and t string not same length");

	    while(*qPtr)
	    {
		if ((*qPtr == '-') || (*tPtr == '-'))
		{
		    if (pending)
		    {
			bt.qEnd = qStart - 1;
			bt.tEnd = tStart;// - tIncr;
			bt.aliLength = bt.qEnd - bt.qStart+1;
			blastTabTabOut(&bt, out);
		    //bt.qStart = qStart;
		    //bt.tStart = tStart;
		    }
		    pending = FALSE;
		}
		else if (!pending && ((*qPtr != '-') && (*tPtr != '-')))
		{
		    pending = TRUE;
		    bt.qStart = qStart;
		    bt.tStart = tStart;
		}

		if (*qPtr != '-')
		    qStart++;
		if (*tPtr != '-')
		    tStart += tIncr;
		qPtr++;
		tPtr++;
	    }
	    state = SAFE_OUT;
//	    printf("query: %d %s\n",qStart, qString);
//	    printf("sbjct: %d %s\n",tStart, tString);
	}
    }
if (pending)
		blastTabTabOut(&bt, out);
	/*
	    {
	    qPos = queryStart;
	    for (lastPtr=ptr= words[2]; ptr =strchr(ptr, '-');lastPtr = ptr)
	    {
		*ptr++ = 0;
		if ((ptr - lastPtr) > 1)
		{
		    printf("chop %d %s\n",qPos, lastPtr);
		    qPos += ptr - lastPtr - 1;
		}
	    }
	    if(lastPtr != ptr)
		    printf("chop %d %s\n",qPos, lastPtr);
	    }
	    */

}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
blast0To8(argv[1], argv[2]);
return 0;
}

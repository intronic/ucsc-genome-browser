/* doTracks.c draw various tracks for Proteome Browser */
#include "common.h"
#include "hCommon.h"
#include "portable.h"
#include "memalloc.h"
#include "jksql.h"
#include "vGfx.h"
#include "memgfx.h"
#include "htmshell.h"
#include "cart.h"
#include "hdb.h"
#include "web.h"
#include "cheapcgi.h"
#include "hgColors.h"
#include "pbTracks.h"

void calxy(int xin, int yin, int *outxp, int *outyp)
/* calxy() converts a logical drawing coordinate into an actual
   drawing coordinate with scaling and minor adjustments */
{
*outxp = xin*pbScale + 120;
//*outxp = xin*pbScale + 120;
*outyp = yin         + currentYoffset;
}

void doAnomalies(char *aa, int len, int *yOffp)
/* draw the AA Anomalies track */
{
char res;
int index;

struct sqlConnection *conn;
char query[56];
struct sqlResult *sr;
char **row;
    
int xx, yy;
int h;
int i, j;
	
float sum;
char *chp;
int aaResFound;
int totalResCnt;
int aaResCnt[20];
double aaResFreqDouble[20];
int abnormal;
int ia = -1;

// count frequency for each residue for current protein
chp = aa;
for (j=0; j<20; j++) aaResCnt[j] = 0;
   
for (i=0; i<len; i++)
    {
    for (j=0; j<20; j++)
        {
        if (*chp == aaChar[j])
            {
            aaResCnt[j] ++;
            break;
	    }
        }
    chp++;
    }
for (j=0; j<20; j++)
    {
    aaResFreqDouble[j] = ((double)aaResCnt[j])/((double)len);
    }

currentYoffset = *yOffp;
    
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-4, 10, 10, MG_BLACK, g_font, "AA Anomalies");
    
for (index=0; index < len; index++)
    {
    res = aa[index];
    for (j=0; j<20; j++)
	{
	if (res == aaChar[j])
	    {
	    ia = j;
	    break;
	    }
	}
    calxy(index, *yOffp, &xx, &yy);
    
    abnormal = chkAnomaly(aaResFreqDouble[ia], avg[ia], stddev[ia]);
    if (abnormal > 0)
	{
	vgBox(g_vg, xx, yy-5, 1*pbScale, 5, MG_RED);
	}
    else
	{
	if (abnormal < 0)
	    {
	    vgBox(g_vg, xx, yy, 1*pbScale, 5, MG_BLUE);
	    }
	}
    vgBox(g_vg, xx, yy, 1*pbScale, 1, MG_BLACK);
    }

// update y offset
*yOffp = *yOffp + 15;
}

void doCharge(char *aa, int len, int *yOffp)
// draw polarity track
{
char res;
int index;
    
int xx, yy;
	
currentYoffset = *yOffp;
    
calxy(0, *yOffp, &xx, &yy);

vgTextRight(g_vg, xx-25, yy-4, 10, 10, MG_BLACK, g_font, "Polarity");
vgTextRight(g_vg, xx-14, yy-10, 10, 10, MG_RED,  g_font, "+");
vgTextRight(g_vg, xx-14, yy, 10, 10, MG_BLUE, g_font, "-");
    
for (index=0; index < len; index++)
    {
    res = aa[index];
    calxy(index, *yOffp, &xx, &yy);

    if (aa_attrib[(int)res] == CHARGE_POS)
	{
	vgBox(g_vg, xx, yy-9, 1*pbScale, 9, MG_RED);
	}
    else
    if (aa_attrib[(int)res] == CHARGE_NEG)
	{
	vgBox(g_vg, xx, yy, 1*pbScale, 9, MG_BLUE);
	}
    else
    if (aa_attrib[(int)res] == POLAR)
	{
	vgBox(g_vg, xx, yy, 1*pbScale, 5, MG_BLUE);
	}
    else
    if (aa_attrib[(int)res] == NEUTRAL)
	{
	vgBox(g_vg, xx, yy, 1*pbScale, 1, MG_BLUE);
	}
    }

*yOffp = *yOffp + 15;
}

void doHydrophobicity(char *aa, int len, int *yOffp)
// draw Hydrophobicity track
{
char res;
int index;
    
int xx, yy;
int h;
int i, i0, i9, j;
int l;
    
int iw = 5;
float sum, avg;
	
currentYoffset = *yOffp;
    
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-7, 10, 10, MG_BLACK, g_font, "Hydrophobicity");
    
for (index=0; index < len; index++)
    {
    res = aa[index];
    calxy(index, *yOffp, &xx, &yy);
	{
	sum = 0;
	i=index;
		
	i0 = index - iw;
	if (i0 < 0) i0 = 0;
	i9 = index + iw;
	if (i9 >= len) i9 = len -1;

	l = 0;
	for (i=i0; i <= i9; i++)
	    {
	    sum = sum + aa_hydro[(int)aa[i]];
	    l++;
	    }
	
	avg = sum/(float)l;
		
	if (avg> 0.0)
	    {
	    h = 5 * 9 * (100.0 * avg / 9.0) / 100;
	    vgBox(g_vg, xx, yy-h, 1*pbScale, h, MG_BLUE);
	    }
	else
	    {
	    h = - 5 * 9 * (100.0 * avg / 9.0) / 100;
	    vgBox(g_vg, xx, yy, 1*pbScale, h, MG_RED);
	    }
	}
    }

*yOffp = *yOffp + 15;
}

void doCysteines(char *aa, int len, int *yOffp)
// draw track for Cysteines and Glycosylation
{
char res;
int index;
    
int xx, yy;
int h;
int iw = 5;
float sum;
	
currentYoffset = *yOffp;
    
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-8, 10, 10, MG_RED, g_font, "Cysteines");
    
vgTextRight(g_vg, xx-25, yy, 10, 10, MG_BLUE, g_font, "Glycosylation");
vgTextRight(g_vg, xx-25, yy+10, 10, 10, MG_BLUE, g_font, "(potential)");
    
for (index=0; index < len; index++)
    {
    res = aa[index];
    calxy(index, *yOffp, &xx, &yy);
    if (res == 'C')
	{
	vgBox(g_vg, xx, yy-9+1, 1*pbScale, 9, MG_RED);
	}
    else
	{
	vgBox(g_vg, xx, yy-0, 1, 1, MG_BLACK);
	}
    }

for (index=1; index < (len-1); index++)
    {
    calxy(index, *yOffp, &xx, &yy);
    if (aa[index-1] == 'N')
	{
	if ( (aa[index+1] == 'T') || (aa[index+1] == 'S') )
	    {
	    if (aa[index] != 'P')
		{
		vgBox(g_vg, xx-1, yy, 3*pbScale, 9, MG_BLUE);
		}
	    }
	}
    }
    
*yOffp = *yOffp + 15;
}

void doAAScale(int len, int *yOffp, int top_bottom)
// draw the track to show AA scale
{
char res;
int index;
   
int tb;	// top or bottom flag
  
int xx, yy;
int h;
int i, i0, i9, j;
int imax;
int interval;
   
char scale_str[20];
int iw = 5;
float sum;

tb = 0;
if (top_bottom < 0) tb = 1;
   
currentYoffset = *yOffp;
    
imax = len/100 * 100;
if ((len % 100) != 0) imax = imax + 100;
    
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-9*tb, 10, 10, MG_BLACK, g_font, "AA Scale");
   
calxy(1, *yOffp, &xx, &yy);
vgBox(g_vg, xx-pbScale/2, yy-tb, (len-1)*pbScale+pbScale/2, 1, MG_BLACK);

interval = 50;
if (pbScale >= 18) interval = 10;    
for (i=0; i<len; i++)
    {
    index = i+1;
    if ((index % interval) == 1)
	{
	if (((index % (interval*2)) == 1) || (index == len)) 
	    {
	    calxy(index, *yOffp, &xx, &yy);
	    vgBox(g_vg, xx-pbScale/2, yy-9*tb, 1, 9, MG_BLACK);
	    }
	else
	    {
	    calxy(index, *yOffp, &xx, &yy);
	    vgBox(g_vg, xx-pbScale/2, yy-5*tb, 1, 5, MG_BLACK);
	    }
    		
	sprintf(scale_str, "%d", index);
	vgText(g_vg, xx-pbScale/2+4, yy+4-12*tb, MG_BLACK, g_font, scale_str);
	}
    }

*yOffp = *yOffp + 12;
}

void mapBoxExon(int x, int y, int width, int height, char *mrnaID, 
		int exonNum, char *chrom, int exonGenomeStartPos, int exonGenomeEndPos)
{
hPrintf("<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\" ", x-1, y-1, x+width+1, y+height+1);
hPrintf("HREF=\"../cgi-bin/hgTracks?db=%s&position=%s:%d-%d\"" 
	,database, chrom, exonGenomeStartPos-1, exonGenomeEndPos+3);
//hPrintf("HREF=\"http://hgwdev-markd.cse.ucsc.edu/cgi-bin/hgTracks?db=%s&position=%s:%d-%d&%s=full" 
//	,database, chrom, exonGenomeStartPos-1, exonGenomeEndPos+3, kgProtMapTableName);
//hPrintf("&knownGene=full&mrna=full\"");
hPrintf(" target=_blank ALT=\"Exon %d\">\n", exonNum);
}

void mapBoxPrevGB(int x, int y, int width, int height, char *posStr)
{
hPrintf("<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\" ", x-1, y-1, x+width+1, y+height+1);
hPrintf("HREF=\"../cgi-bin/hgTracks?db=%s&position=%s\"", database, posStr);
hPrintf(" target=_blank ALT=\"UCSC Genome Browser %s\">\n", posStr);
}

void doPrevGB(int exonCount, char *chrom, char strand, int aaLen, int *yOffp, char *proteinID, char *mrnaID)
// draw the previous Genome Browser position range
{
int xx, yy;
int i, j;
char prevPosMessage[200];
char exonNumStr[10];
int mrnaLen;
Color color;

int exonStartPos, exonEndPos;
int exonGenomeStartPos, exonGenomeEndPos;
int exonNumber;
int printedExonNumber = -1;
int exonColor[2];
int currentPos;
int currentPBPos;
int jPrevStart, jPrevEnd;
int jcnt = 0;

int defaultColor;
defaultColor = vgFindColorIx(g_vg, 170, 170, 170);

// The imaginary mRNA length is 3 times of aaLen
mrnaLen = aaLen * 3;

exonColor[0] = MG_BLUE;
exonColor[1] = vgFindColorIx(g_vg, 0, 180, 0);

jPrevStart = mrnaLen-1;
jPrevEnd   = 0;

exonNumber = 1;

exonStartPos 	   = blockStartPositive[exonNumber-1]; 
exonEndPos 	   = blockEndPositive[exonNumber-1];
exonGenomeStartPos = blockGenomeStartPositive[exonNumber-1];
exonGenomeEndPos   = blockGenomeEndPositive[exonNumber-1];

currentYoffset = *yOffp;
    
for (j = 0; j < mrnaLen; j++)
    {
    color = defaultColor;
    calxy(j/3, *yOffp, &xx, &yy);
    if (j > exonEndPos)
	{
	if (printedExonNumber != exonNumber)
	    {
            if ((exonEndPos - exonStartPos)*pbScale/3 > 12) 
	    	{
	    	sprintf(exonNumStr, "%d", exonNumber);
	    	}
 	    printedExonNumber = exonNumber;
	    }

	if (exonNumber < exonCount)
    	    {
	    exonNumber++;
	    exonStartPos       = blockStartPositive[exonNumber-1]; 
	    exonEndPos 	       = blockEndPositive[exonNumber-1];
	    exonGenomeStartPos = blockGenomeStartPositive[exonNumber-1];
	    exonGenomeEndPos   = blockGenomeEndPositive[exonNumber-1];
	    }
    	}

    if ((j >= exonStartPos) && (j <= exonEndPos))
	{
	color = exonColor[(exonNumber-1) % 2];
	}
    if (strand == '-')
	{
    	currentPos = blockGenomeStartPositive[exonNumber-1] + (blockEndPositive[exonNumber-1]-j)+1;
    	}
    else
	{
    	currentPos = blockGenomeStartPositive[exonNumber-1]+(j - blockStartPositive[exonNumber-1])+1;
    	}

    if ((currentPos >= prevGBStartPos) && (currentPos <= prevGBEndPos))
	{
	//printf("<br>j=%d currentPos=%d prevGBStartPos=%d prevGBEndPos=%d\n", j, currentPos, prevGBStartPos, prevGBEndPos);fflush(stdout);
	jcnt++;
	if (j < jPrevStart) jPrevStart = j;
	if (j > jPrevEnd)   jPrevEnd   = j;
	}
    }

positionStr = strdup(cartOptionalString(cart, "position"));
if (jcnt > 0)
    {
    calxy(jPrevStart/3, *yOffp, &xx, &yy);
    if (pbScale > 6)
    	{
    	vgBox(g_vg,  xx+(jPrevStart%3)*6, yy-2, (jPrevEnd-jPrevStart+1)*pbScale/3, 2, MG_BLACK);
    	}
    else
    	{
    	vgBox(g_vg,  xx, yy-2, (jPrevEnd-jPrevStart+1)*pbScale/3, 2, MG_BLACK);
    	}

    mapBoxPrevGB(xx+(jPrevStart%3)*6, yy-2, (jPrevEnd-jPrevStart+1)*pbScale/3, 2, positionStr);
    sprintf(prevPosMessage, "You were at: %s", positionStr);
    if (jPrevStart < (mrnaLen/2))
   	{
   	vgText(g_vg, xx+(jPrevStart%3)*pbScale/3, yy-10, MG_BLACK, g_font, prevPosMessage);
   	}
    else
   	{
   	calxy(jPrevEnd/3, *yOffp, &xx, &yy);
   	vgTextRight(g_vg, xx-6, yy-10, 10, 10, MG_BLACK, g_font, prevPosMessage);
   	}
    }
else
    {
    sprintf(prevPosMessage, "You were at: %s (inside an intron)", positionStr);
    calxy(0, *yOffp, &xx, &yy);
    //vgText(g_vg, xx, yy-8, MG_BLACK, g_font, prevPosMessage);
    vgTextRight(g_vg, xx+6*strlen(prevPosMessage)-10, yy-8, 10, 10, MG_BLACK, g_font, prevPosMessage);
    }
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-8, 10, 10, MG_BLACK, g_font, "Genome Browser");
*yOffp = *yOffp + 7;
}

void doExon(int exonCount, char *chrom, int aaLen, int *yOffp, char *proteinID, char *mrnaID)
// draw the track for exons
{
int xx, yy;
int i, j;
char exonNumStr[10];
int mrnaLen;
Color color;

int exonStartPos, exonEndPos;
int exonGenomeStartPos, exonGenomeEndPos;
int exonNumber;
int printedExonNumber = -1;
int exonColor[2];

int defaultColor;
defaultColor = vgFindColorIx(g_vg, 170, 170, 170);

// The imaginary mRNA length is 3 times of aaLen
mrnaLen = aaLen * 3;

exonColor[0] = MG_BLUE;
exonColor[1] = vgFindColorIx(g_vg, 0, 180, 0);

exonNumber = 1;

exonStartPos 	   = blockStartPositive[exonNumber-1]; 
exonEndPos 	   = blockEndPositive[exonNumber-1];
exonGenomeStartPos = blockGenomeStartPositive[exonNumber-1];
exonGenomeEndPos   = blockGenomeEndPositive[exonNumber-1];

currentYoffset = *yOffp;
    
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-9, 10, 10, MG_BLACK, g_font, "Exons");

for (j = 0; j < mrnaLen; j++)
    {
    color = defaultColor;
    calxy(j/3, *yOffp, &xx, &yy);
    if (j > exonEndPos)
	{
	if (printedExonNumber != exonNumber)
	    {
            if ((exonEndPos - exonStartPos)*pbScale/3 > 12) 
	    	{
	    	sprintf(exonNumStr, "%d", exonNumber);
            	vgTextRight(g_vg, xx-(exonEndPos - exonStartPos)*pbScale/3/2 - 4,
                                  yy-9, 10, 10, MG_WHITE, g_font, exonNumStr);
	    	}
            mapBoxExon(xx - (exonEndPos - exonStartPos)*pbScale/3, yy-9, 
		       	(exonEndPos - exonStartPos)*pbScale/3, 9, mrnaID, 
		       	exonNumber, chrom, 
		 	blockGenomeStartPositive[exonNumber-1], 
		       	blockGenomeEndPositive[exonNumber-1]);
	    printedExonNumber = exonNumber;
	    }

	if (exonNumber < exonCount)
    	    {
	    exonNumber++;
	    exonStartPos       = blockStartPositive[exonNumber-1]; 
	    exonEndPos 	       = blockEndPositive[exonNumber-1];
	    exonGenomeStartPos = blockGenomeStartPositive[exonNumber-1];
	    exonGenomeEndPos   = blockGenomeEndPositive[exonNumber-1];
	    }
    	}

    if ((j >= exonStartPos) && (j <= exonEndPos))
	{
	color = exonColor[(exonNumber-1) % 2];
	}
    vgBox(g_vg, xx, yy-9+3*(j-(j/3)*3), pbScale, 3, color);
    }

if ((exonEndPos - exonStartPos)*pbScale/3 > 12)
    {
    sprintf(exonNumStr, "%d", exonNumber);
    vgTextRight(g_vg, xx-(exonEndPos - exonStartPos)*pbScale/3/2 - 5,
                      yy-9, 10, 10, MG_WHITE, g_font, exonNumStr);
    }

mapBoxExon(xx - (exonEndPos - exonStartPos)*pbScale/3, yy-9,    
	   (exonEndPos - exonStartPos)*pbScale/3, 9, mrnaID, 
	   exonNumber, chrom, 
	   blockGenomeStartPositive[exonNumber-1], 
	   blockGenomeEndPositive[exonNumber-1]);

*yOffp = *yOffp + 10;
}

#define MAX_SF 200
#define MAXNAMELEN 256

int sfId[MAX_SF];
int sfStart[MAX_SF], sfEnd[MAX_SF];
char superfam_name[MAX_SF][256];

struct sqlConnection *conn, *conn2;

int getSuperfamilies(char *proteinID)
{
char *before, *after = "", *s;
char startString[64], endString[64];

struct sqlConnection *conn, *conn2;
char query[MAXNAMELEN], query2[MAXNAMELEN];
struct sqlResult *sr, *sr2;
char **row, **row2;

char cond_str[255];

char *genomeID, *seqID, *modelID, *start, *end, *eValue, *sfID, *sfDesc;

char *name, *chrom, *strand, *txStart, *txEnd, *cdsStart, *cdsEnd,
     *exonCount, *exonStarts, *exonEnds;
char *region;
int  done;

char *gene_name;
char *ensPep;
char *transcriptName;

char *chp, *chp2;
int  i,l;
int  ii = 0;
int  int_start, int_end;
    
conn  = hAllocConn();
conn2 = hAllocConn();

// two steps query needed because the recent Ensembl gene_xref 11/2003 table does not have 
// valid translation_name
sprintf(cond_str, "external_name='%s'", protDisplayID);
transcriptName = sqlGetField(conn, database, "ensGeneXref", "transcript_name", cond_str);
if (transcriptName == NULL)
    {
    return(0); 
    }
else
    {
    sprintf(cond_str, "transcript_name='%s';", transcriptName);
    ensPep = sqlGetField(conn, database, "ensTranscript", "translation_name", cond_str);
    if (ensPep == NULL) 
	{
	hFreeConn(&conn);
    	return(0); 
    	}
    }

ensPepName = ensPep;

sprintf(query, "select * from %s.sfAssign where seqID='%s' and evalue <= 0.02;", database, ensPep);
sr = sqlMustGetResult(conn, query);
row = sqlNextRow(sr);
if (row == NULL) return(0);
    
while (row != NULL)
    {      
    genomeID = row[0];
    seqID    = row[1];
    modelID  = row[2];
    region   = row[3];
    eValue   = row[4];
    sfID     = row[5];
    //sfDesc   = row[6];
    // !!! the recent Suprefamily sfAssign table does not have valid sf description
    sprintf(cond_str, "id=%s;", sfID);
    sfDesc = sqlGetField(conn2, database, "sfDes", "description", cond_str);

    //!!! refine logic here later to be defensive against illegal syntax
    chp = region;
    done = 0;
    while (!done)
	{
	chp2  = strstr(chp, "-");
	*chp2 = '\0';
	chp2++;

	sscanf(chp, "%d", &int_start);
			
	chp = chp2;
	chp2  = strstr(chp, ",");
	if (chp2 != NULL) 
	    {
	    *chp2 = '\0';
	    }
	else
	    {
	    done = 1;
	    }
	chp2++;
	sscanf(chp, "%d", &int_end);

 	sfId[ii]    = atoi(sfID);
	sfStart[ii] = int_start;
	sfEnd[ii]   = int_end;
	strncpy(superfam_name[ii], sfDesc, MAXNAMELEN-1);
	ii++;
	chp = chp2;
	}

    row = sqlNextRow(sr);
    }

hFreeConn(&conn);
hFreeConn(&conn2);
sqlFreeResult(&sr);
  
return(ii);
}
    

void mapBoxSuperfamily(int x, int y, int width, int height, char *sf_name, int sfID)
{
hPrintf("<AREA SHAPE=RECT COORDS=\"%d,%d,%d,%d\" ", x-1, y-1, x+width+1, y+height+1);

hPrintf("HREF=\"%s?sunid=%d\"",
	"http://supfam.org/SUPERFAMILY/cgi-bin/scop.cgi", sfID);
hPrintf(" target=_blank ALT=\"%s\">\n", sf_name);
}

void vgDrawBox(struct vGfx *vg, int x, int y, int width, int height, Color color)
{
vgBox(g_vg, x, y, width, height, color);
vgBox(vg, x-1, 	y-1, 		width+2, 	1, 	MG_BLACK);
vgBox(vg, x-1, 	y+height, 	width+2, 	1, 	MG_BLACK);
vgBox(vg, x-1, 	y-1, 		1, 		height+2, MG_BLACK);
vgBox(vg, x+width, 	y-1, 		1, 		height+2, MG_BLACK);
}

void doSuperfamily(char *pepName, int sf_cnt, int *yOffp)
// draw the Superfamily track
{
int xx, yy;
int h;
int i, ii, jj, i0, i9, j;
char exonNumStr[10];
int len;
int sf_len, name_len;
int show_name;
    
Color color2;
    
color2 = vgFindColorIx(g_vg, 0, 180, 0);
   
currentYoffset = *yOffp;
   
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy-9, 10, 10, MG_BLACK, g_font, "Superfamily/SCOP");
    
jj = 0;
for (ii=0; ii<sf_cnt; ii++)
    {
    if (sfEnd[ii] != 0)
	{
	jj++;
	sprintf(exonNumStr, "%d", jj);
	calxy(sfStart[ii], *yOffp, &xx, &yy);

	sf_len   = sfEnd[ii] - sfStart[ii];
	name_len = strlen(superfam_name[ii]);
	if (sf_len*pbScale < name_len*6) 
	    {
	    show_name = 0;
	    }
	else
	    {
	    show_name = 1;
	    }

	len = strlen(superfam_name[ii]);
	vgDrawBox(g_vg, xx, yy-9+(jj%3)*4, (sfEnd[ii] - sfStart[ii])*pbScale, 9, MG_YELLOW);
	mapBoxSuperfamily(xx, yy-9+(jj%3)*4, 
			  (sfEnd[ii] - sfStart[ii])*pbScale, 9,
		 	  superfam_name[ii], sfId[ii]);
    	if (show_name) vgTextRight(g_vg, 
	    			   //xx+(sfEnd[ii] - sfStart[ii])*pbScale/2 + (len/2)*5 - 5, 
	    			   xx+(sfEnd[ii] - sfStart[ii])*pbScale/2 + (len/2)*5, 
				   yy-9+(jj%3)*4, 10, 10, MG_BLACK, g_font, superfam_name[ii]);
	}
    }

*yOffp = *yOffp + 20;
}

void doResidues(char *aa, int len, int *yOffp)
// draw track for AA residue
{
char res;
int index;
    
int xx, yy;
int h;
int i, i0, i9, j;

char res_str[2];
    
int iw = 5;
float sum;
	
currentYoffset = *yOffp;
    
calxy(0, *yOffp, &xx, &yy);
vgTextRight(g_vg, xx-25, yy, 10, 10, MG_BLACK, g_font, "Sequence");
    
res_str[1] = '\0';
for (index=0; index < len; index++)
    {
    res_str[0] = aa[index];
    calxy(index+1, *yOffp, &xx, &yy);
	
    //vgTextRight(g_vg, xx-3-6, yy, 10, 10, MG_BLACK, g_font, res_str);
    if (pbScale >= 18)
	{
	vgTextRight(g_vg, xx-3-16, yy, 10, 10, MG_BLACK, g_font, res_str);
    	}
    else
	{
        vgTextRight(g_vg, xx-3-6, yy, 10, 10, MG_BLACK, g_font, res_str);
    	}

    }
    
*yOffp = *yOffp + 12;
}

void doDnaTrack(char *chrom, char strand, int exonCount, int len, int *yOffp)
// draw track for AA residue
{
char res;
int index;

int xx, yy;
int h;
int i, i0, i9, j;
int mrnaLen;
char exonNumStr[10];
                       
int exonStartPos, exonEndPos;
int exonGenomeStartPos, exonGenomeEndPos;
int exonNumber;
int printedExonNumber = -1;
int exonColor[2];
int color;
int k;
struct dnaSeq *dna;

char res_str[2];
char base[2];
char baseComp[2];
int iw = 5;
float sum;
int dnaLen;

int defaultColor;
defaultColor = vgFindColorIx(g_vg, 170, 170, 170);

exonColor[0] = MG_BLUE;
exonColor[1] = vgFindColorIx(g_vg, 0, 180, 0);

base[1] = '\0';
baseComp[1] = '\0';
currentYoffset = *yOffp;

calxy(0, *yOffp, &xx, &yy);

// The imaginary mRNA length is 3 times of aaLen
mrnaLen = len * 3;
            
exonNumber = 1;

exonStartPos       = blockStartPositive[exonNumber-1];
exonEndPos         = blockEndPositive[exonNumber-1];
exonGenomeStartPos = blockGenomeStartPositive[exonNumber-1];
exonGenomeEndPos   = blockGenomeEndPositive[exonNumber-1];
dna = hChromSeq(chrom, exonGenomeStartPos, exonGenomeEndPos+1);
dnaLen = strlen(dna->dna);

vgTextRight(g_vg, xx-25, yy, 10, 10, MG_BLACK, g_font, "DNA Sequence");
if (strand == '-') vgTextRight(g_vg, xx-25, yy+9, 10, 10, MG_BLACK, g_font, "& complement");

k=0;
for (j = 0; j < mrnaLen; j++)
    {
    if (j > exonEndPos)
        {

        if (printedExonNumber != exonNumber)
            {
            printedExonNumber = exonNumber;
            }

        if (exonNumber < exonCount)
            {
            exonNumber++;
            exonStartPos       = blockStartPositive[exonNumber-1];
            exonEndPos         = blockEndPositive[exonNumber-1];
            exonGenomeStartPos = blockGenomeStartPositive[exonNumber-1];
            exonGenomeEndPos   = blockGenomeEndPositive[exonNumber-1];
	    dna = hChromSeq(chrom, exonGenomeStartPos, exonGenomeEndPos+1);
    	    dnaLen = strlen(dna->dna);
            k=0;
	    }
        }

    if ((j >= exonStartPos) && (j <= exonEndPos))
        {
	if (strand == '+')
	    {
	    base[0] = toupper(*(dna->dna + k));
	    }
	else
	    {
	    base[0]     = toupper(ntCompTable[*(dna->dna + dnaLen - k -1 )]);
	    baseComp[0] = toupper(*(dna->dna + dnaLen - k -1 ));
	    }

	k++;
        color = exonColor[(exonNumber-1) % 2];
        calxy(j/3, *yOffp, &xx, &yy);
        vgTextRight(g_vg, xx-3+(j%3)*6, yy, 10, 10, color, g_font, base);
        if (strand == '-') vgTextRight(g_vg, xx-3+(j%3)*6, yy+9, 10, 10, color, g_font, baseComp);
        }
    color = MG_BLUE;
    }
    
if (strand == '-')
    {
    *yOffp = *yOffp + 20;
    }
else
    {
    *yOffp = *yOffp + 12;
    }
}

void doTracks(char *proteinID, char *mrnaID, char *aa, struct vGfx *vg, int *yOffp)
/* draw various protein tracks */
{
int i,j,l;
char *exonNumStr;
int exonNum;

double pI, aaLen;
double exonCount;
char *chp;
int len;
int cCnt;

int xPosition;
int yPosition;

int aaResCnt[30];
double aaResFreqDouble[30];
int aaResFound;
int totalResCnt;
int hasResFreq;

char *aap;
double molWeight, hydroSum;
struct pbStamp *stampDataPtr;
char *chrom;
char strand;

Color bkgColor;

// initialize AA properties
aaPropertyInit(&hasResFreq);

g_vg = vg;
g_font = mgSmallFont();

dnaUtilOpen();

l=strlen(aa);
doAAScale(l, yOffp, 1);
if (mrnaID != NULL)
    {
    getExonInfo(proteinID, &exCount, &chrom, &strand);
    }

if (pbScale >= 6)  doResidues(aa, l, yOffp);
if (pbScale >= 18) doDnaTrack(chrom, strand, exCount, l, yOffp);

if (mrnaID != NULL)
    {
    doPrevGB(exCount, chrom, strand, l, yOffp, proteinID, mrnaID);
    }

if (mrnaID != NULL)
    {
    doExon(exCount, chrom, l, yOffp, proteinID, mrnaID);
    }

doCharge(aa, l, yOffp);

doHydrophobicity(aa, l, yOffp);

doCysteines(aa, l, yOffp);

if (sfCount > 0) doSuperfamily(ensPepName, sfCount, yOffp); 

if (hasResFreq) doAnomalies(aa, l, yOffp);

doAAScale(l, yOffp, -1);
}


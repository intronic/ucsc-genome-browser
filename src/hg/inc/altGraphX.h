/* altGraphX.h was originally generated by the autoSql program, which also 
 * generated altGraphX.c and altGraphX.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef ALTGRAPHX_H
#define ALTGRAPHX_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#ifndef BED_H
#include "bed.h"
#endif

#ifndef GENEGRAPH_H
#include "geneGraph.h"
#endif

#ifndef SPACESAVER_H
#include "spaceSaver.h"
#endif

#ifndef VGFX_H
#include "vGfx.h"
#endif

struct evidence
/* List of mRNA/ests supporting a given edge */
    {
    struct evidence *next;  /* Next in singly linked list. */
    int evCount;	/* number of ests evidence */
    int *mrnaIds;	/* ids of mrna evidence, indexes into altGraphx->mrnaRefs */
    };

struct evidence *evidenceCommaIn(char **pS, struct evidence *ret);
/* Create a evidence out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new evidence */

void evidenceFree(struct evidence **pEl);
/* Free a single dynamically allocated evidence such as created
 * with evidenceLoad(). */

void evidenceFreeList(struct evidence **pList);
/* Free a list of dynamically allocated evidence's */

void evidenceOutput(struct evidence *el, FILE *f, char sep, char lastSep);
/* Print out evidence.  Separate fields with sep. Follow last field with lastSep. */

#define evidenceTabOut(el,f) evidenceOutput(el,f,'\t','\n');
/* Print out evidence as a line in a tab-separated file. */

#define evidenceCommaOut(el,f) evidenceOutput(el,f,',',',');
/* Print out evidence as a comma separated list including final comma. */

struct altGraphX
/* An alternatively spliced gene graph. */
    {
    struct altGraphX *next;  /* Next in singly linked list. */
    char *tName;	/* name of target sequence, often a chrom. */
    int tStart;	/* First bac touched by graph. */
    int tEnd;	/* Start position in first bac. */
    char *name;	/* Human readable name. */
    unsigned id;	/* Unique ID. */
    char strand[3];	/* + or - strand. */
    unsigned vertexCount;	/* Number of vertices in graph. */
    unsigned char *vTypes;	/* Type for each vertex. */
    int *vPositions;	/* Position in target for each vertex. */
    unsigned edgeCount;	/* Number of edges in graph. */
    int *edgeStarts;	/* Array with start vertex of edges. */
    int *edgeEnds;	/* Array with end vertex of edges. */
    struct evidence *evidence;	/* array of evidence tables containing references to mRNAs that support a particular edge. */
    int *edgeTypes;	/* Type for each edge, ggExon, ggIntron, etc. */
    int mrnaRefCount;	/* Number of supporting mRNAs. */
    char **mrnaRefs;	/* Ids of mrnas supporting this. */
    int *mrnaTissues;	/* Ids of tissues that mrnas come from, indexes into tissue table */
    int *mrnaLibs;	/* Ids of libraries that mrnas come from, indexes into library table */
    };

struct altGraphX *altGraphXLoad(char **row);
/* Load a altGraphX from row fetched with select * from altGraphX
 * from database.  Dispose of this with altGraphXFree(). */

struct altGraphX *altGraphXLoadAll(char *fileName);
/* Load all altGraphX from a tab-separated file.
 * Dispose of this with altGraphXFreeList(). */

struct altGraphX *altGraphXLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all altGraphX from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with altGraphXFreeList(). */

void altGraphXSaveToDb(struct sqlConnection *conn, struct altGraphX *el, char *tableName, int updateSize);
/* Save altGraphX as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Note that strings must be escaped to allow insertion into the database.
 * For example "autosql's features include" --> "autosql\'s features include" 
 * If worried about this use altGraphXSaveToDbEscaped() */

void altGraphXSaveToDbEscaped(struct sqlConnection *conn, struct altGraphX *el, char *tableName, int updateSize);
/* Save altGraphX as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size.
 * of a string that would contain the entire query. Automatically 
 * escapes all simple strings (not arrays of string) but may be slower than altGraphXSaveToDb().
 * For example automatically copies and converts: 
 * "autosql's features include" --> "autosql\'s features include" 
 * before inserting into database. */ 

struct altGraphX *altGraphXCommaIn(char **pS, struct altGraphX *ret);
/* Create a altGraphX out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new altGraphX */

void altGraphXFree(struct altGraphX **pEl);
/* Free a single dynamically allocated altGraphX such as created
 * with altGraphXLoad(). */

void altGraphXFreeList(struct altGraphX **pList);
/* Free a list of dynamically allocated altGraphX's */

void altGraphXOutput(struct altGraphX *el, FILE *f, char sep, char lastSep);
/* Print out altGraphX.  Separate fields with sep. Follow last field with lastSep. */

#define altGraphXTabOut(el,f) altGraphXOutput(el,f,'\t','\n');
/* Print out altGraphX as a line in a tab-separated file. */

#define altGraphXCommaOut(el,f) altGraphXOutput(el,f,',',',');
/* Print out altGraphX as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct spliceEdge 
/* Structure to hold information about one edge in 
   a splicing graph. */
{
    struct spliceEdge *next;    /* Next in list. */
    int type;                   /* Type of edge: ggExon, ggIntron, ggSJ, ggCassette. */
    int start;                  /* Chrom start. */
    int end;                    /* End. */
    int v1;                     /* Vertex 1 in graph. */
    int v2;                     /* Vertex 2 in graph. */
    int itemNumber;             /* Number of altGraphX record derived from in list. */
    int row;                    /* Row that exon is stored in. */
    double conf;                /* Confidence. */
};

int spliceEdgeTypeConfCmp(const void *va, const void *vb);
/* Compare to sort based on type, confidence, and then start. */

struct spliceEdge *altGraphXToEdges(struct altGraphX *ag);
/* Return a list of splice edges based on data in altGraphX. */

void altGraphXoffset(struct altGraphX *ag, int offset);
/* Add offset to all coordinates of altGraphX. */

struct altGraphX *altGraphXClone(struct altGraphX *ag);
/* Make a clone of a current altGraphX structure. Free with altGraphXFree() */

int altGraphXNumAltSplices(struct altGraphX *ag);
/* Count number of times that exons have more than one edge through them, i.e. 
 * number of possible splices seen. */

void altGraphXTopologicalSort(struct altGraphX *ag);
/* Do a topological sort on vertices in altGraphX, basic 
 * algorithm from "Computer Algorithms" Sara Baase and Allen Van Gelder
 * 3rd Edition 2000, pp 345-353 */

void altGraphXVertPosSort(struct altGraphX *ag);
/* Sort an altGraphX's vertices by position on tName. */

bool **altGraphXCreateEdgeMatrix(struct altGraphX *ag);
/* Create an edgematix from an altGraphX, free this with 
 * altGraphFreeEdgeMatrix. */

void altGraphXFreeEdgeMatrix(bool ***pEm, int vertCount);
/* Free an edge matrix. */



float altGraphCassetteConfForEdge(struct altGraphX *ag, int eIx, float prior);
/* Return the score for this cassette exon. Want to have cassette exons
   that are present in multiple transcripts and that are not present in multiple
   exons. We want to see both forms of the cassette exon, we don't want to have
   one outlier be chosen. Thus we count the times that the exon is seen, we
   count the times that the exon isn't seen and we calculate a final score by:
   (seen + notseen + prior)/(abs(seen - notSeen+prior) + 1) . Thus larger scores are better. 
*/

struct bed *altGraphGetExonCassette(struct altGraphX *ag, int eIx);
/* Get a bed which corresponds to the exons involved in a cassette exon. */

int altGraphConfidenceForEdge(struct altGraphX *ag, int eIx);
/* count how many unique libraries or tissues contain a given edge */

struct bed *altGraphXToBed(struct altGraphX *ag);
/* Merge all overlapping exons to form bed datatype. Free with bedFree().*/

void altGraphXLayout(struct altGraphX *agList, int regionStart, int regionEnd, 
		     int regionWidth, double scale, int maxRows,
		     struct spaceSaver **ssList, struct hash **heightHash, 
		     int *rowCount);
/** Layout a list of altGraphX's in a space width wide. Return a list
    of one spaceSaver per altGraphX record, a hash with the row layout
    offset of the exons, and the number of rows required to layout. */

void altGraphXDrawPack(struct altGraphX *agList, struct spaceSaver *ssList, 
		       int xOff, int yOff, int width,  int heightPer, int lineHeight,
		       int regionStart, int regionEnd,
		       double scale, int baseWidth, 
		       struct vGfx *vg, MgFont *font, Color color, Color *shades, char *drawName,
		       void (*mapItem)(char *tableName, struct altGraphX *ag, int start, int end,
				       int x, int y, int width, int height));
/** Draw a splicing graph for each altGraphX in the agList where the
    exons don't overlap as they have been laid out in the spaceSaver
    list. */

enum ggEdgeType altGraphXEdgeVertexType(struct altGraphX *ag, int v1, int v2);
/* Return edge type. */

enum ggEdgeType getSpliceEdgeType(struct altGraphX *ag, int edge);
/* Return edge type. */

void altGraphXReverseComplement(struct altGraphX *ag);
/* Switch an altGraphX record around so it looks like the
   chromosomal coordinates were reverse complemented. */

int altGraphXGetEdgeNum(struct altGraphX *ag, int v1, int v2);
/** Find the edge index that corresponds to v1 and v2 */

#endif /* ALTGRAPHX_H */


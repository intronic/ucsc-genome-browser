/*****************************************************************************
 * Copyright (C) 2000 Jim Kent.  This source code may be freely used         *
 * for personal, academic, and non-profit purposes.  Commercial use          *
 * permitted only by explicit agreement with Jim Kent (jim_kent@pacbell.net) *
 *****************************************************************************/
/* gff.h Parse a GFF or GTF file. */

#ifndef GFF_H
#define GFF_H

struct gffLine
/* A parsed line in a GFF file. */
    {
    struct gffLine *next;  /* Next line in file */
    char *seq;      /* Name of sequence. */
    char *source;   /* Program that made this line.  Not allocated here. */
    char *feature;  /* Type field. (Intron, CDS, etc). Not allocated here. */
    int start;      /* Start of feature in sequence. Starts with 0, not 1 */
    int end;        /* End of feature in sequence. End is not included. */
    double score;   /* Score. */
    char strand;    /* Strand of sequence feature is on. + or - or .*/
    char frame;     /* Frame feature is in. 1, 2, 3, or . */
    char *group;    /* Group line is in. Not allocated here.  Corresponds to transcript_id in GTF */
    char *geneId;    /* gene_id in GTF, NULL in GFF.  Not allocated here. */
    char *exonId;       /* exon_id in GTF, NULL in GFF. Not allocated here. */
    int exonNumber; /* O in GFF or if missing in GTF.  Otherwise exon number. */
    };

struct gffGroup
/* A group of lines in a GFF file (all that share the same group field). */
    {
    struct gffGroup *next;   /* Next group in file. */
    char *name;     /* Name of group. Not allocated here. */
    char *seq;      /* Name of sequence. Not allocated here. */
    char *source;      /* Name of source program. Not allocated here. */
    /* The next three fields are only valid after call to gffGroupLines() */
    int start;      /* Start of feature in sequence. Starts with 0, not 1 */
    int end;        /* End of feature in sequence. End is not included. */
    char strand;    /* Strand of sequence. */
    struct gffLine *lineList;  /* List of lines in group. */
    };

struct gffSource
/* A list of sources. */
    {
    struct gffSource *next; /* Next in list. */
    char *name;	  /* Name, not allocated here. */
    unsigned int id;   /* Database ID (or just 0) */
    };

struct gffFeature
/* A list of types in GFF file. */
    {
    struct gffFeature *next; /* Next in list. */
    char *name;	  /* Name, not allocated here. */
    };

struct gffSeqName
/* A list of sequence. */
    {
    struct gffSeqName *next;  /* Next in list. */
    char *name;   /* Name, not allocated here. */
    };

struct gffGeneId
/* A list of genes. */
    {
    struct gffGeneId *next;  /* Next in list. */
    char *name;   /* Name, not allocated here. */
    };

struct gffFile
/* This keeps information on a fully parsed GFF file. */
    {
    struct gffFile *next;
    char *fileName;             /* Name of file (allocated here) */
    struct hash *seqHash;	/* A name only hash of the sequence. */
    struct hash *sourceHash;	/* A name only hash of gff sources. */
    struct hash *featureHash;   /* A name only hash of gff types. */
    struct hash *groupHash;	/* Associates group names and gffGroups. */
    struct hash *geneIdHash;    /* Hash of all geneIds. */
    struct hash *exonHash;       /* Hash of all exonIds. */
    struct gffLine *lineList;   /* List of lines - lines may be in groupList instead. */
    struct gffSeqName *seqList; /* List of sequences in file. */
    struct gffSource *sourceList; /* List of all sources in file. */
    struct gffFeature *featureList; /* List of all types in file. */
    struct gffGroup *groupList; /* A list of groups. */
    struct gffGeneId *geneIdList;  /* List of all gene ID's. */
    bool isGtf;			/* Is this a GTF file? */
    bool typeKnown;		/* Is 'isGtf' known? */
    };

void gffGroupFree(struct gffGroup **pGroup);
/* Free up a gffGroup including lineList. */

void gffGroupFreeList(struct gffGroup **pList);
/* Free up a list of gffGroups. */

void gffGroupLines(struct gffFile *gff);
/* Group lines of gff file together, in process mofing
 * gff->lineList to gffGroup->lineList. */

struct gffFile *gffRead(char *fileName);
/* Create a gffFile structure from a GFF file. */

struct gffFile *gffFileNew(char *fileName);
/* Create a new gffFile structure. */

void gffFileAdd(struct gffFile *gff, char *fileName, int baseOffset);
/* Add file to gffFile. */

void gffFileAddRow(struct gffFile *gff, int baseOffset, char *words[], int wordCount, 
		char *fileName, int lineIx);
/* Process one row of GFF file (a non-comment line parsed by tabs normally). */

void gffFileFree(struct gffFile **pGff);
/* Free up a gff file. */

int gffLineCmp(const void *va, const void *vb);
/* Compare two gffLines (for use in slSort, etc.) . */


#endif /* GFF_H */


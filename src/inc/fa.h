/*****************************************************************************
 * Copyright (C) 2000 Jim Kent.  This source code may be freely used         *
 * for personal, academic, and non-profit purposes.  Commercial use          *
 * permitted only by explicit agreement with Jim Kent (jim_kent@pacbell.net) *
 *****************************************************************************/
#ifndef FA_H
#define FA_H

#ifndef DNASEQ_H
#include "dnaseq.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif

struct dnaSeq *faReadDna(char *fileName);
/* Open fa file and read a single sequence from it. */

struct dnaSeq *faReadAllDna(char *fileName);
/* Return list of all sequences in FA file. */

struct dnaSeq *faReadOneDnaSeq(FILE *f, char *name, boolean mustStartWithSign);
/* Read one sequence from FA file. Assumes positioned at or before
 * the '>' at start of sequence. */  

boolean faReadNext(FILE *f, char *defaultName, boolean mustStartWithComment, 
    char **retCommentLine, struct dnaSeq **retSeq);
/* Read next sequence from .fa file. Return sequence in retSeq.  If retCommentLine is non-null
 * return the '>' line in retCommentLine.   The whole thing returns FALSE at end of file. 
 * Assumes positioned at or before the '>' at start of sequence.  File must have been
 * opened in binary mode! */

struct dnaSeq *faFromMemText(char *text);
/* Return a sequence from a .fa file that's been read into
 * a string in memory. This cannabalizes text, which should
 * be allocated with needMem.  This buffer becomes part of
 * the returned dnaSeq, which may be freed normally with
 * freeDnaSeq. */

boolean faFastReadNext(FILE *f, DNA **retDna, int *retSize, char **retName);
/* Read in next FA entry as fast as we can. Return FALSE at EOF. 
 * The returned DNA and name will be overwritten by the next call
 * to this function. */

boolean faSpeedReadNext(struct lineFile *lf, DNA **retDna, int *retSize, char **retName);
/* Read in next FA entry as fast as we can. Faster than that old,
 * pokey faFastReadNext. Return FALSE at EOF. 
 * The returned DNA and name will be overwritten by the next call
 * to this function. */

void faFreeFastBuf();
/* Free up buffers used in fa fast and speedreading. */

void faWrite(char *fileName, char *startLine, DNA *dna, int dnaSize);
/* Write out FA file or die trying. */

void faWriteNext(FILE *f, char *startLine, DNA *dna, int dnaSize);
/* Write next sequence to fa file. */

#endif /* FA_H */

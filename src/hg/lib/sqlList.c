/*****************************************************************************
 * Copyright (C) 2000 Jim Kent.  This source code may be freely used         *
 * for personal, academic, and non-profit purposes.  Commercial use          *
 * permitted only by explicit agreement with Jim Kent (jim_kent@pacbell.net) *
 *****************************************************************************/
/* Stuff for processing comma separated lists - a little long so
 * in a separate module from jksql.c though interface is still
 # in jksql.c. */

#include "common.h"
#include "jksql.h"

int sqlByteArray(char *s, signed char *array, int arraySize)
/* Convert comma separated list of numbers to an array.  Pass in 
 * array an max size of array. */
{
unsigned count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == arraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = sqlSigned(s);
    s = e;
    }
return count;
}

void sqlByteStaticArray(char *s, signed char **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static signed char *array = NULL;
static unsigned alloc = 0;
unsigned count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = sqlSigned(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlByteDynamicArray(char *s, signed char **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
signed char *sArray, *dArray = NULL;
unsigned size;

sqlByteStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlUbyteArray(char *s, unsigned char *array, int arraySize)
/* Convert comma separated list of numbers to an array.  Pass in 
 * array an max size of array. */
{
unsigned count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == arraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = sqlUnsigned(s);
    s = e;
    }
return count;
}

void sqlUbyteStaticArray(char *s, unsigned char **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static unsigned char *array = NULL;
static unsigned alloc = 0;
unsigned count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = sqlUnsigned(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlUbyteDynamicArray(char *s, unsigned char **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
unsigned char *sArray, *dArray = NULL;
unsigned size;

sqlUbyteStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlShortArray(char *s, short *array, int arraySize)
/* Convert comma separated list of numbers to an array.  Pass in 
 * array an max size of array. */
{
unsigned count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == arraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = sqlSigned(s);
    s = e;
    }
return count;
}

void sqlShortStaticArray(char *s, short **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static short *array = NULL;
static unsigned alloc = 0;
unsigned count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = sqlSigned(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlShortDynamicArray(char *s, short **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
short *sArray, *dArray = NULL;
unsigned size;

sqlShortStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlUshortArray(char *s, unsigned short *array, int arraySize)
/* Convert comma separated list of numbers to an array.  Pass in 
 * array an max size of array. */
{
unsigned count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == arraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = sqlUnsigned(s);
    s = e;
    }
return count;
}

void sqlUshortStaticArray(char *s, unsigned short **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static unsigned short *array = NULL;
static unsigned alloc = 0;
unsigned count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = sqlUnsigned(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlUshortDynamicArray(char *s, unsigned short **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
unsigned short *sArray, *dArray = NULL;
unsigned size;

sqlUshortStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlFloatArray(char *s, float *array, int maxArraySize)
/* Convert comma separated list of floating point numbers to an array.  
 * Pass in array and max size of array. */
{
unsigned count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == maxArraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = atof(s);
    s = e;
    }
return count;
}

void sqlFloatStaticArray(char *s, float **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static float *array = NULL;
static unsigned alloc = 0;
unsigned count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = atof(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlFloatDynamicArray(char *s, float **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
float *sArray, *dArray = NULL;
unsigned size;

sqlFloatStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlUnsignedArray(char *s, unsigned *array, int arraySize)
/* Convert comma separated list of numbers to an array.  Pass in 
 * array and max size of array. */
{
unsigned count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == arraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = sqlUnsigned(s);
    s = e;
    }
return count;
}

void sqlUnsignedStaticArray(char *s, unsigned **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static unsigned *array = NULL;
static unsigned alloc = 0;
unsigned count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = sqlUnsigned(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlUnsignedDynamicArray(char *s, unsigned **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
unsigned *sArray, *dArray = NULL;
unsigned size;

sqlUnsignedStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlSignedArray(char *s, int *array, int arraySize)
/* Convert comma separated list of numbers to an array.  Pass in 
 * array an max size of array. */
{
int count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == arraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = sqlSigned(s);
    s = e;
    }
return count;
}

void sqlSignedStaticArray(char *s, int **retArray, int *retSize)
/* Convert comma separated list of numbers to an array which will be
 * overwritten next call to this function, but need not be freed. */
{
static int *array = NULL;
static int alloc = 0;
int count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = sqlSigned(s);
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlSignedDynamicArray(char *s, int **retArray, int *retSize)
/* Convert comma separated list of numbers to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
int *sArray, *dArray = NULL;
int size;

sqlSignedStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

/*-------------------------*/

int sqlStringArray(char *s, char **array, int maxArraySize)
/* Convert comma separated list of strings to an array.  Pass in 
 * array and max size of array.  Returns actual size*/
{
int count = 0;
for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0 || count == maxArraySize)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    array[count++] = s;
    s = e;
    }
return count;
}

void sqlStringStaticArray(char *s, char  ***retArray, int *retSize)
/* Convert comma separated list of strings to an array which will be
 * overwritten next call to this function or to sqlUnsignedDynamicArray,
 * but need not be freed. */
{
static char **array = NULL;
static int alloc = 0;
int count = 0;

for (;;)
    {
    char *e;
    if (s == NULL || s[0] == 0)
	break;
    e = strchr(s, ',');
    if (e != NULL)
	*e++ = 0;
    if (count >= alloc)
	{
	if (alloc == 0)
	    alloc = 64;
	else
	    alloc <<= 1;
	ExpandArray(array, count, alloc);
	}
    array[count++] = s;
    s = e;
    }
*retSize = count;
*retArray = array;
}

void sqlStringDynamicArray(char *s, char ***retArray, int *retSize)
/* Convert comma separated list of strings to an dynamically allocated
 * array, which should be freeMem()'d when done. */
{
char **sArray, **dArray = NULL;
int size;
int i;

s = cloneString(s);
sqlStringStaticArray(s, &sArray, &size);
if (size > 0)
    {
    AllocArray(dArray,size);
    CopyArray(sArray, dArray, size);
    }
*retArray = dArray;
*retSize = size;
}

void sqlStringFreeDynamicArray(char ***pArray)
/* Free up a dynamic array (ends up freeing array and first string on it.) */
{
char **array;
if ((array = *pArray) != NULL)
    {
    freeMem(array[0]);
    freez(pArray);
    }
}

int sqlUnsignedComma(char **pS)
/* Return signed number at *pS.  Advance *pS past comma at end */
{
char *s = *pS;
char *e = strchr(s, ',');
unsigned ret;

*e++ = 0;
*pS = e;
ret = sqlUnsigned(s);
return ret;
}

int sqlSignedComma(char **pS)
/* Return signed number at *pS.  Advance *pS past comma at end */
{
char *s = *pS;
char *e = strchr(s, ',');
int ret;

*e++ = 0;
*pS = e;
ret = sqlSigned(s);
return ret;
}

static char *findStringEnd(char *start, char endC)
/* Return end of string. */
{
char c;
char *s = start;

for (;;)
    {
    c = *s;
    if (c == endC)
	return s;
    else if (c == 0)
	errAbort("Unterminated string");
    ++s;
    }
}

static char *sqlGetOptQuoteString(char **pS)
/* Return string at *pS.  (Either quoted or not.)  Advance *pS. */
{
char *s = *pS;
char *e;
char c = *s;

if (c  == '"' || c == '\'')
    {
    s += 1;
    e = findStringEnd(s, c);
    *e++ = 0;
    if (*e++ != ',')
	errAbort("Expecting comma after string");
    }
else
    {
    e = strchr(s, ',');
    *e++ = 0;
    }
*pS = e;
return s;
}

char *sqlStringComma(char **pS)
/* Return string at *pS.  (Either quoted or not.)  Advance *pS. */
{
return cloneString(sqlGetOptQuoteString(pS));
}

void sqlFixedStringComma(char **pS, char *buf, int bufSize)
/* Copy string at *pS to buf.  Advance *pS. */
{
strncpy(buf, sqlGetOptQuoteString(pS), bufSize);
}

char *sqlEatChar(char *s, char c)
/* Make sure next character is 'c'.  Return past next char */
{
if (*s++ != c)
    errAbort("Expecting %c got %c (%d) in database", c, s[-1], s[-1]);
return s;
}


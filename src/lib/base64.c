#include "common.h"
#include "base64.h"

char *base64Encode(char *input)
/* Use base64 to encode a string.  Returns one long encoded
 * string which need to be freeMem'd. Note: big-endian algorithm.
 * For some applications you may need to break the base64 output
 * of this function into lines no longer than 76 chars.
 */
{
char b64[] = B64CHARS;
int inplen = strlen(input);
int words = (inplen+2)/3;
int remains = inplen % 3;
char *result = (char *)needMem(4*words+1);
int i=0, j=0;
int word = 0;
char *p = input;
for(i=0; i<words; i++)
    {
    word = 0;
    word |= *p++;
    word <<= 8;
    word |= *p++;
    word <<= 8;
    word |= *p++;
    result[j++]=b64[word >> 18 & 0x3F];
    result[j++]=b64[word >> 12 & 0x3F];
    result[j++]=b64[word >> 6 & 0x3F];
    result[j++]=b64[word & 0x3F];
    }
result[j] = 0;    
if (remains >0) result[j-1] = '=';    
if (remains==1) result[j-2] = '=';    
return result;
}


char *base64Decode(char *input)
/* Use base64 to decode a string.  Return decoded
 * string which will be freeMem'd. Note: big-endian algorithm.
 * Call eraseWhiteSpace() and check for invalid input 
 * before passing in input if needed.  
 */
{
static int *map=NULL;
char b64[] = B64CHARS;
int inplen = strlen(input);
int words = (inplen+3)/4;
char *result = (char *)needMem(3*words+1);
int i=0, j=0;
int word = 0;
char *p = input;

if (!map)
    {
    int i = 0;
    map = needMem(256*sizeof(int));
    for (i = 0; i < 256; ++i)
	{
	map[i]=0;
	}
    for (i = 0; i < 64; ++i)
	{
	map[(int)b64[i]]=i;
	}
    }
for(i=0; i<words; i++)
    {
    word = 0;
    word |= map[(int)*p++];
    word <<= 6;
    word |= map[(int)*p++];
    word <<= 6;
    word |= map[(int)*p++];
    word <<= 6;
    word |= map[(int)*p++];
    result[j++]=word >> 16 & 0xFF;
    result[j++]=word >> 8 & 0xFF;
    result[j++]=word & 0xFF;
    }
result[j] = 0;    
     
return result;
}


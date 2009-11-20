/* udc - url data cache - a caching system that keeps blocks of data fetched from URLs in
 * sparse local files for quick use the next time the data is needed. 
 *
 * This cache is enormously simplified by there being no local _write_ to the cache,
 * just reads.  
 *
 * The overall strategy of the implementation is to have a root cache directory
 * with a subdir for each file being cached.  The directory for a single cached file
 * contains two files - "bitmap" and "sparseData" that contains information on which
 * parts of the URL are cached and the actual cached data respectively. The subdirectory name
 * associated with the file is constructed from the URL in a straightforward manner.
 *     http://genome.ucsc.edu/cgi-bin/hgGateway
 * gets mapped to:
 *     rootCacheDir/http/genome.ucsc.edu/cgi-bin/hgGateway/
 * The URL protocol is the first directory under the root, and the remainder of the
 * URL, with some necessary escaping, is used to define the rest of the cache directory
 * structure, with each '/' after the protocol line translating into another directory
 * level.
 *    
 * The bitmap file contains time stamp and size data as well as an array with one bit
 * for each block of the file that has been fetched.  Currently the block size is 8K. */

#include <sys/file.h>
#include "common.h"
#include "hash.h"
#include "obscure.h"
#include "bits.h"
#include "portable.h"
#include "sig.h"
#include "net.h"
#include "cheapcgi.h"
#include "udc.h"

static char const rcsid[] = "$Id: udc.c,v 1.30 2009/11/20 17:45:32 angie Exp $";

#define udcBlockSize (8*1024)
/* All fetch requests are rounded up to block size. */

#define udcMaxBytesPerRemoteFetch (udcBlockSize * 32)
/* Very large remote reads are broken down into chunks this size. */

struct connInfo
/* Socket descriptor and associated info, for keeping net connections open. */
    {
    int socket;                 /* Socket descriptor for data connection (or 0). */
    bits64 offset;		/* Current file offset of socket. */
    int ctrlSocket;             /* (FTP only) Control socket descriptor or 0. */
    };

typedef int (*UdcDataCallback)(char *url, bits64 offset, int size, void *buffer,
			       struct connInfo *ci);
/* Type for callback function that fetches file data. */

struct udcRemoteFileInfo
/* Information about a remote file. */
    {
    bits64 updateTime;	/* Last update in seconds since 1970 */
    bits64 size;	/* Remote file size */
    struct connInfo ci; /* Connection info for open net connection */
    };

typedef boolean (*UdcInfoCallback)(char *url, struct udcRemoteFileInfo *retInfo);
/* Type for callback function that fetches file timestamp and size. */

struct udcProtocol
/* Something to handle a communications protocol like http, https, ftp, local file i/o, etc. */
    {
    struct udcProtocol *next;	/* Next in list */
    UdcDataCallback fetchData;	/* Data fetcher */
    UdcInfoCallback fetchInfo;	/* Timestamp & size fetcher */
    };

struct udcFile
/* A file handle for our caching system. */
    {
    struct udcFile *next;	/* Next in list. */
    char *url;			/* Name of file - includes protocol */
    char *protocol;		/* The URL up to the first colon.  http: etc. */
    struct udcProtocol *prot;	/* Protocol specific data and methods. */
    time_t updateTime;		/* Last modified timestamp. */
    bits64 size;		/* Size of file. */
    bits64 offset;		/* Current offset in file. */
    char *cacheDir;		/* Directory for cached file parts. */
    char *bitmapFileName;	/* Name of bitmap file. */
    char *sparseFileName;	/* Name of sparse data file. */
    FILE *fSparse;		/* File handle for sparse data file. */
    bits64 startData;		/* Start of area in file we know to have data. */
    bits64 endData;		/* End of area in file we know to have data. */
    bits32 bitmapVersion;	/* Version of associated bitmap we were opened with. */
    struct connInfo connInfo;   /* Connection info for open net connection. */
    };

struct udcBitmap
/* The control structure including the bitmap of blocks that are cached. */
    {
    struct udcBitmap *next;	/* Next in list. */
    bits32 blockSize;		/* Number of bytes per block of file. */
    bits64 remoteUpdate;	/* Remote last update time. */
    bits64 fileSize;		/* File size */
    bits32 version;		/* Version - increments each time cache is stale. */
    bits64 localUpdate;		/* Time we last fetched new data into cache. */
    bits64 localAccess;		/* Time we last accessed data. */
    boolean isSwapped;		/* If true need to swap all bytes on read. */
    FILE *f;			/* File handle for file with current block. */
    };
static char *bitmapName = "bitmap";
static char *sparseDataName = "sparseData";
#define udcBitmapHeaderSize (64)
static int cacheTimeout = 0;

static int connInfoGetSocket(struct connInfo *ci, char *url, bits64 offset, int size)
/* If ci has an open socket and the given offset matches ci's current offset,
 * reuse ci->socket.  Otherwise close the socket, open a new one, and update ci. */
{
if (ci != NULL && ci->socket != 0 && ci->offset != offset)
    {
    verbose(2, "Offset mismatch (ci %lld != new %lld), reopening.\n", ci->offset, offset);
    close(ci->socket);
    if (ci->ctrlSocket != 0)
	close(ci->ctrlSocket);
    ZeroVar(ci);
    }
int sd;
if (ci == NULL || ci->socket == 0)
    {
    char rangeUrl[2048];
    if (ci == NULL)
	{
	safef(rangeUrl, sizeof(rangeUrl), "%s;byterange=%lld-%lld",
	      url, offset, (offset + size - 1));
	sd = netUrlOpen(rangeUrl);
	}
    else
	{
	safef(rangeUrl, sizeof(rangeUrl), "%s;byterange=%lld-", url, offset);
	sd = ci->socket = netUrlOpenSockets(rangeUrl, &(ci->ctrlSocket));
	ci->offset = offset;
	}
    if (startsWith("http", url))
	{
	char *newUrl = NULL;
	int newSd = 0;
	if (!netSkipHttpHeaderLinesHandlingRedirect(sd, url, &newSd, &newUrl))
	    errAbort("Couldn't open %s", url);   // do we really want errAbort here?
	if (newUrl)  // not sure redirection will work with byte ranges as it is now
	    {
	    freeMem(newUrl); 
	    sd = newSd;
	    if (ci != NULL)
		ci->socket = newSd;
	    }
	}
    }
else
    sd = ci->socket;
if (sd < 0)
    errnoAbort("Couldn't open %s", url);   // do we really want errAbort here?
return sd;
}

/********* Section for local file protocol **********/

static char *assertLocalUrl(char *url)
/* Make sure that url is local and return bits past the protocol. */
{
if (startsWith("local:", url))
    url += 6;
if (url[0] != '/')
    errAbort("Local urls must start at /");
if (stringIn("..", url) || stringIn("~", url) || stringIn("//", url) ||
    stringIn("/./", url) || endsWith("/.", url))
    {
    errAbort("relative paths not allowed in local urls (%s)", url);
    }
return url;
}

static int udcDataViaLocal(char *url, bits64 offset, int size, void *buffer, struct connInfo *ci)
/* Fetch a block of data of given size into buffer using the http: protocol.
* Returns number of bytes actually read.  Does an errAbort on
* error.  Typically will be called with size in the 8k - 64k range. */
{
/* Need to check time stamp here. */
verbose(2, "reading remote data - %d bytes at %lld - on %s\n", size, offset, url);
url = assertLocalUrl(url);
FILE *f = mustOpen(url, "rb");
fseek(f, offset, SEEK_SET);
int sizeRead = fread(buffer, 1, size, f);
if (ferror(f))
    {
    warn("udcDataViaLocal failed to fetch %d bytes at %lld", size, offset);
    errnoAbort("file %s", url);
    }
carefulClose(&f);
return sizeRead;
}

static boolean udcInfoViaLocal(char *url, struct udcRemoteFileInfo *retInfo)
/* Fill in *retTime with last modified time for file specified in url.
 * Return FALSE if file does not even exist. */
{
verbose(2, "checking remote info on %s\n", url);
url = assertLocalUrl(url);
struct stat status;
int ret = stat(url, &status);
if (ret < 0)
    return FALSE;
retInfo->updateTime = status.st_mtime;
retInfo->size = status.st_size;
return TRUE;
}

/********* Section for transparent file protocol **********/

static int udcDataViaTransparent(char *url, bits64 offset, int size, void *buffer,
				 struct connInfo *ci)
/* Fetch a block of data of given size into buffer using the http: protocol.
* Returns number of bytes actually read.  Does an errAbort on
* error.  Typically will be called with size in the 8k - 64k range. */
{
internalErr();	/* Should not get here. */
return size;
}

static boolean udcInfoViaTransparent(char *url, struct udcRemoteFileInfo *retInfo)
/* Fill in *retTime with last modified time for file specified in url.
 * Return FALSE if file does not even exist. */
{
internalErr();	/* Should not get here. */
return FALSE;
}

/********* Section for slow local file protocol - simulates network... **********/

static int udcDataViaSlow(char *url, bits64 offset, int size, void *buffer, struct connInfo *ci)
/* Fetch a block of data of given size into buffer using the http: protocol.
* Returns number of bytes actually read.  Does an errAbort on
* error.  Typically will be called with size in the 8k - 64k range. */
{
verbose(2, "slow reading remote data - %d bytes at %lld - on %s\n", size, offset, url);
sleep1000(500);
char *fileName = url + 5;  /* skip over 'slow:' */
FILE *f = mustOpen(fileName, "rb");
fseek(f, offset, SEEK_SET);
char *pt = buffer;
int i, step=1024;
int sizeRead = 0;
for (i=0; i<size; i += step)
    {
    sleep1000(250);
    int readChunk = size - i;
    if (readChunk > step)
        readChunk = step;
    int oneReadSize = fread(pt, 1, readChunk, f);
    verbose(2, "slowly read %d bytes\n", oneReadSize);
    if (ferror(f))
	{
	warn("udcDataViaSlow failed to fetch %d bytes at %lld", size, offset);
	errnoAbort("file %s", fileName);
	}
    pt += step;
    sizeRead += oneReadSize;
    }
carefulClose(&f);
return sizeRead;
}

static boolean udcInfoViaSlow(char *url, struct udcRemoteFileInfo *retInfo)
/* Fill in *retTime with last modified time for file specified in url.
 * Return FALSE if file does not even exist. */
{
char *fileName = url + 5;  /* skip over 'slow:' */
verbose(2, "slow checking remote info on %s\n", url);
sleep1000(500);
struct stat status;
int ret = stat(fileName, &status);
if (ret < 0)
    return FALSE;
retInfo->updateTime = status.st_mtime;
retInfo->size = status.st_size;
return TRUE;
}

/********* Section for http protocol **********/

int udcDataViaHttpOrFtp(char *url, bits64 offset, int size, void *buffer, struct connInfo *ci)
/* Fetch a block of data of given size into buffer using url's protocol,
 * which must be http, https or ftp.  Returns number of bytes actually read.
 * Does an errAbort on error.
 * Typically will be called with size in the 8k-64k range. */
{
if (startsWith("http://",url) || startsWith("https://",url) || startsWith("ftp://",url))
    verbose(2, "reading http/https/ftp data - %d bytes at %lld - on %s\n", size, offset, url);
else
    errAbort("Invalid protocol in url [%s] in udcDataViaFtp, only http, https, or ftp supported",
	     url); 
int sd = connInfoGetSocket(ci, url, offset, size);
int rd = 0, total = 0, remaining = size;
char *buf = (char *)buffer;
while ((remaining > 0) && ((rd = read(sd, buf, remaining)) > 0))
    {
    total += rd;
    buf += rd;
    remaining -= rd;
    }
if (rd == -1)
    errnoAbort("udcDataViaHttpOrFtp: error reading socket");
if (ci == NULL)
    close(sd);
else
    ci->offset += total;
return total;
}

boolean udcInfoViaHttp(char *url, struct udcRemoteFileInfo *retInfo)
/* Gets size and last modified time of URL
 * and returns status of HEAD GET. */
{
verbose(2, "checking http remote info on %s\n", url);
struct hash *hash = newHash(0);
int status = netUrlHead(url, hash);
if (status != 200) // && status != 302 && status != 301)
    return FALSE;
char *sizeString = hashFindValUpperCase(hash, "Content-Length:");
if (sizeString == NULL)
    {
    hashFree(&hash);
    errAbort("No Content-Length: returned in header for %s, can't proceed, sorry", url);
    }
retInfo->size = atoll(sizeString);

char *lastModString = hashFindValUpperCase(hash, "Last-Modified:");
if (lastModString == NULL)
    {
    // Date is a poor substitute!  It will always appear that the cache is stale.
    // But at least we can read files from dropbox.com.
    lastModString = hashFindValUpperCase(hash, "Date:");
    if (lastModString == NULL)
	{
	hashFree(&hash);
	errAbort("No Last-Modified: or Date: returned in header for %s, can't proceed, sorry", url);
	}
    }
struct tm tm;
time_t t;
// Last-Modified: Wed, 15 Nov 1995 04:58:08 GMT
// TODO: it's very likely that there are other date string patterns
//  out there that might be encountered.
if (strptime(lastModString, "%a, %d %b %Y %H:%M:%S %Z", &tm) == NULL)
    { /* Handle error */;
    hashFree(&hash);
    errAbort("unable to parse last-modified string [%s]", lastModString);
    }
// Not set by strptime(); tells mktime() to determine whether daylight saving time is in effect:
tm.tm_isdst = -1;
t = mktime(&tm);
if (t == -1)
    { /* Handle error */;
    hashFree(&hash);
    errAbort("mktime failed while parsing last-modified string [%s]", lastModString);
    }
retInfo->updateTime = t;

hashFree(&hash);
return status;
}


/********* Section for ftp protocol **********/

// fetchData method: See udcDataViaHttpOrFtp above.

boolean udcInfoViaFtp(char *url, struct udcRemoteFileInfo *retInfo)
/* Gets size and last modified time of FTP URL */
{
verbose(2, "checking ftp remote info on %s\n", url);
long long size = 0;
time_t t;
// TODO: would be nice to add int *retCtrlSocket to netGetFtpInfo so we can stash 
// in retInfo->connInfo and keep socket open.
boolean ok = netGetFtpInfo(url, &size, &t);
if (!ok)
    return FALSE;
retInfo->size = size;
retInfo->updateTime = t;
return TRUE;
}


/********* Non-protocol-specific bits **********/


static char *fileNameInCacheDir(struct udcFile *file, char *fileName)
/* Return the name of a file in the cache dir, from the cache root directory on down.
 * Do a freeMem on this when done. */
{
int dirLen = strlen(file->cacheDir);
int nameLen = strlen(fileName);
char *path = needMem(dirLen + nameLen + 2);
memcpy(path, file->cacheDir, dirLen);
path[dirLen] = '/';
memcpy(path+dirLen+1, fileName, nameLen);
return path;
}

static void udcNewCreateBitmapAndSparse(struct udcFile *file, 
	bits64 remoteUpdate, bits64 remoteSize, bits32 version)
/* Create a new bitmap file around the given remoteUpdate time. */
{
FILE *f = mustOpen(file->bitmapFileName, "wb");
bits32 sig = udcBitmapSig;
bits32 blockSize = udcBlockSize;
bits64 reserved64 = 0;
bits32 reserved32 = 0;
int blockCount = (remoteSize + udcBlockSize - 1)/udcBlockSize;
int bitmapSize = bitToByteSize(blockCount);
int i;

/* Write out fixed part of header. */
writeOne(f, sig);
writeOne(f, blockSize);
writeOne(f, remoteUpdate);
writeOne(f, remoteSize);
writeOne(f, version);
writeOne(f, reserved32);
writeOne(f, reserved64);
writeOne(f, reserved64);
writeOne(f, reserved64);
writeOne(f, reserved64);
if (ftell(f) != udcBitmapHeaderSize)
    errAbort("ftell(f=%s) is %lld, not expected udcBitmapHeaderSize %d",
	     file->bitmapFileName, (long long)ftell(f), udcBitmapHeaderSize);

/* Write out initial all-zero bitmap. */
for (i=0; i<bitmapSize; ++i)
    putc(0, f);

/* Clean up bitmap file and name. */
carefulClose(&f);

/* Create an empty data file. */
f = mustOpen(file->sparseFileName, "wb");
carefulClose(&f);
}

static struct udcBitmap *udcBitmapOpen(char *fileName)
/* Open up a bitmap file and read and verify header.  Return NULL if file doesn't
 * exist, abort on error. */
{
/* Open file, returning NULL if can't. */
FILE *f = fopen(fileName, "r+b");
if (f == NULL)
    return NULL;

/* Get status info from file. */
struct stat status;
fstat(fileno(f), &status);

/* Read signature and decide if byte-swapping is needed. */
bits32 magic;
boolean isSwapped = FALSE;
mustReadOne(f, magic);
if (magic != udcBitmapSig)
    {
    magic = byteSwap32(magic);
    isSwapped = TRUE;
    if (magic != udcBitmapSig)
       errAbort("%s is not a udcBitmap file", fileName);
    }

/* Allocate bitmap object, fill it in, and return it. */
bits32 reserved32;
bits64 reserved64;
struct udcBitmap *bits;
AllocVar(bits);
bits->blockSize = readBits32(f, isSwapped);
bits->remoteUpdate = readBits64(f, isSwapped);
bits->fileSize = readBits64(f, isSwapped);
bits->version = readBits32(f, isSwapped);
reserved32 = readBits32(f, isSwapped);
reserved64 = readBits64(f, isSwapped);
reserved64 = readBits64(f, isSwapped);
reserved64 = readBits64(f, isSwapped);
reserved64 = readBits64(f, isSwapped);
bits->localUpdate = status.st_mtime;
bits->localAccess = status.st_atime;
bits->isSwapped = isSwapped;
bits->f = f;

return bits;
}

static void udcBitmapClose(struct udcBitmap **pBits)
/* Free up resources associated with udcBitmap. */
{
struct udcBitmap *bits = *pBits;
if (bits != NULL)
    {
    carefulClose(&bits->f);
    freez(pBits);
    }
}

static struct udcProtocol *udcProtocolNew(char *upToColon)
/* Build up a new protocol around a string such as "http" or "local" */
{
struct udcProtocol *prot;
AllocVar(prot);
if (sameString(upToColon, "local"))
    {
    prot->fetchData = udcDataViaLocal;
    prot->fetchInfo = udcInfoViaLocal;
    }
else if (sameString(upToColon, "slow"))
    {
    prot->fetchData = udcDataViaSlow;
    prot->fetchInfo = udcInfoViaSlow;
    }
else if (sameString(upToColon, "http") || sameString(upToColon, "https"))
    {
    prot->fetchData = udcDataViaHttpOrFtp;
    prot->fetchInfo = udcInfoViaHttp;
    }
else if (sameString(upToColon, "ftp"))
    {
    prot->fetchData = udcDataViaHttpOrFtp;
    prot->fetchInfo = udcInfoViaFtp;
    }
else if (sameString(upToColon, "transparent"))
    {
    prot->fetchData = udcDataViaTransparent;
    prot->fetchInfo = udcInfoViaTransparent;
    }
else
    {
    errAbort("Unrecognized protocol %s in udcProtNew", upToColon);
    }
return prot;
}

static void udcProtocolFree(struct udcProtocol **pProt)
/* Free up protocol resources. */
{
freez(pProt);
}

static void setInitialCachedDataBounds(struct udcFile *file)
/* Open up bitmap file and read a little bit of it to see if cache is stale,
 * and if not to see if the initial part is cached.  Sets the data members
 * startData, and endData.  If the case is stale it makes fresh empty
 * cacheDir/sparseData and cacheDir/bitmap files. */
{
bits32 version = 0;

/* Get existing bitmap, and if it's stale clean up. */
struct udcBitmap *bits = udcBitmapOpen(file->bitmapFileName);
if (bits != NULL)
    {
    version = bits->version;
    if (bits->remoteUpdate != file->updateTime || bits->fileSize != file->size)
	{
        udcBitmapClose(&bits);
	remove(file->bitmapFileName);
	remove(file->sparseFileName);
	++version;
	verbose(2, "removing stale version, new version %d\n", version);
	}
    }

/* If no bitmap, then create one, and also an empty sparse data file. */
if (bits == NULL)
    {
    udcNewCreateBitmapAndSparse(file, file->updateTime, file->size, version);
    bits = udcBitmapOpen(file->bitmapFileName);
    if (bits == NULL)
        internalErr();
    }

file->bitmapVersion = bits->version;

/* Read in a little bit from bitmap while we have it open to see if we have anything cached. */
if (file->size > 0)
    {
    Bits b = fgetc(bits->f);
    int endBlock = (file->size + udcBlockSize - 1)/udcBlockSize;
    if (endBlock > 8)
        endBlock = 8;
    int initialCachedBlocks = bitFindClear(&b, 0, endBlock);
    file->endData = initialCachedBlocks * udcBlockSize;
    }

udcBitmapClose(&bits);
}

static boolean qEscaped(char c)
/* Returns TRUE if character needs to be escaped in q-encoding. */
{
if (isalnum(c))
    return c == 'Q';
else
    return c != '_' && c != '-' && c != '/' && c != '.';
}

static char *qEncode(char *input)
/* Do a simple encoding to convert input string into "normal" characters.
 * Abnormal letters, and '!' get converted into Q followed by two hexadecimal digits. */
{
/* First go through and figure out encoded size. */
int size = 0;
char *s, *d, c;
s = input;
while ((c = *s++) != 0)
    {
    if (qEscaped(c))
	size += 3;
    else
	size += 1;
    }

/* Allocate and fill in output. */
char *output = needMem(size+1);
s = input;
d = output;
while ((c = *s++) != 0)
    {
    if (qEscaped(c))
        {
	sprintf(d, "Q%02X", (unsigned)c);
	d += 3;
	}
    else
        *d++ = c;
    }
return output;
}

void udcParseUrlFull(char *url, char **retProtocol, char **retAfterProtocol, char **retColon,
		     char **retAuth)
/* Parse the URL into components that udc treats separately.
 * *retAfterProtocol is Q-encoded to keep special chars out of filenames.  
 * Free all *ret's except *retColon when done. */
{
char *protocol, *afterProtocol;
char *colon = strchr(url, ':');
if (!colon)
    {
    *retColon = NULL;
    return;
    }
int colonPos = colon - url;
protocol = cloneStringZ(url, colonPos);
afterProtocol = url + colonPos + 1;
while (afterProtocol[0] == '/')
   afterProtocol += 1;
char *userPwd = strchr(afterProtocol, '@');
if (userPwd)
    {
    if (retAuth)
	{
	char auth[1024];
	safencpy(auth, sizeof(auth), afterProtocol, userPwd+1-afterProtocol);
	*retAuth = qEncode(auth);
	}
    char *afterHost = strchr(afterProtocol, '/');
    if (!afterHost)
	{
	afterHost = afterProtocol+strlen(afterProtocol);
	}
    if (userPwd < afterHost)
	afterProtocol = userPwd + 1;
    }
else if (retAuth)
    *retAuth = NULL;
afterProtocol = qEncode(afterProtocol);
*retProtocol = protocol;
*retAfterProtocol = afterProtocol;
*retColon = colon;
}

void udcParseUrl(char *url, char **retProtocol, char **retAfterProtocol, char **retColon)
/* Parse the URL into components that udc treats separately.
 * *retAfterProtocol is Q-encoded to keep special chars out of filenames.  
 * Free  *retProtocol and *retAfterProtocol but not *retColon when done. */
{
udcParseUrlFull(url, retProtocol, retAfterProtocol, retColon, NULL);
}

void udcPathAndFileNames(struct udcFile *file, char *cacheDir, char *protocol, char *afterProtocol)
/* Initialize udcFile path and names */
{
int len = strlen(cacheDir) + 1 + strlen(protocol) + 1 + strlen(afterProtocol) + 1;
file->cacheDir = needMem(len);
safef(file->cacheDir, len, "%s/%s/%s", cacheDir, protocol, afterProtocol);

/* Create file names for bitmap and data portions. */
file->bitmapFileName = fileNameInCacheDir(file, bitmapName);
file->sparseFileName = fileNameInCacheDir(file, sparseDataName);
}

static long long int udcSizeFromBitmap(char *bitmapFileName)
/* Look up the file size from the local cache bitmap file, or -1 if there
 * is no cache for url. */
{
long long int ret = -1;
struct udcBitmap *bits = udcBitmapOpen(bitmapFileName);
if (bits != NULL)
    ret = bits->fileSize;
else
    warn("Can't open bitmap file %s: %s\n", bitmapFileName, strerror(errno));
udcBitmapClose(&bits);
return ret;
}

struct udcFile *udcFileMayOpen(char *url, char *cacheDir)
/* Open up a cached file. cacheDir may be null in which case udcDefaultDir() will be
 * used.  Return NULL if file doesn't exist. */
{
if (cacheDir == NULL)
    cacheDir = udcDefaultDir();
verbose(2, "udcfileOpen(%s, %s)\n", url, cacheDir);
/* Parse out protocol.  Make it "transparent" if none specified. */
char *protocol = NULL, *afterProtocol = NULL, *colon;
boolean isTransparent = FALSE;
udcParseUrl(url, &protocol, &afterProtocol, &colon);
if (!colon)
    {
    freeMem(protocol);
    protocol = cloneString("transparent");
    freeMem(afterProtocol);
    afterProtocol = cloneString(url);
    isTransparent = TRUE;
    }
struct udcProtocol *prot;
prot = udcProtocolNew(protocol);

/* Figure out if anything exists. */
boolean useCacheInfo = (udcCacheAge(url, cacheDir) < udcCacheTimeout());
struct udcRemoteFileInfo info;
ZeroVar(&info);
if (!isTransparent && !useCacheInfo)
    {
    if (!prot->fetchInfo(url, &info))
	{
	udcProtocolFree(&prot);
	freeMem(protocol);
	freeMem(afterProtocol);
	return NULL;
	}
    }

/* Allocate file object and start filling it in. */
struct udcFile *file;
AllocVar(file);
file->url = cloneString(url);
file->protocol = protocol;
file->prot = prot;
if (isTransparent)
    {
    /* If transparent dummy up things so that the "sparse" file pointer is actually
     * the file itself, which appears to be completely loaded in cache. */
    FILE *f = file->fSparse = mustOpen(url, "rb");
    struct stat status;
    fstat(fileno(f), &status);
    file->startData = 0;
    file->endData = file->size = status.st_size;
    }
else
    {
    udcPathAndFileNames(file, cacheDir, protocol, afterProtocol);
    if (useCacheInfo)
	{
	file->updateTime = fileModTime(file->bitmapFileName);
	file->size = udcSizeFromBitmap(file->bitmapFileName);
	}
    else
	{
	file->updateTime = info.updateTime;
	file->size = info.size;
	memcpy(&(file->connInfo), &(info.ci), sizeof(struct connInfo));
	}

    /* Make directory. */
    makeDirsOnPath(file->cacheDir);

    /* Figure out a little bit about the extent of the good cached data if any. */
    setInitialCachedDataBounds(file);
    file->fSparse = mustOpen(file->sparseFileName, "rb+");
    }
freeMem(afterProtocol);
return file;
}

struct udcFile *udcFileOpen(char *url, char *cacheDir)
/* Open up a cached file.  cacheDir may be null in which case udcDefaultDir() will be
 * used.  Abort if if file doesn't exist. */
{
struct udcFile *udcFile = udcFileMayOpen(url, cacheDir);
if (udcFile == NULL)
    errAbort("Couldn't open %s", url);
return udcFile;
}


struct slName *udcFileCacheFiles(char *url, char *cacheDir)
/* Return low-level list of files used in cache. */
{
char *protocol, *afterProtocol, *colon;
struct udcFile *file;
udcParseUrl(url, &protocol, &afterProtocol, &colon);
if (colon == NULL)
    return NULL;
AllocVar(file);
udcPathAndFileNames(file, cacheDir, protocol, afterProtocol);
struct slName *list = NULL;
slAddHead(&list, slNameNew(file->bitmapFileName));
slAddHead(&list, slNameNew(file->sparseFileName));
slReverse(&list);
freeMem(file->cacheDir);
freeMem(file->bitmapFileName);
freeMem(file->sparseFileName);
freeMem(file);
freeMem(protocol);
freeMem(afterProtocol);
return list;
}

void udcFileClose(struct udcFile **pFile)
/* Close down cached file. */
{
struct udcFile *file = *pFile;
if (file != NULL)
    {
    if (file->connInfo.socket != 0)
	close(file->connInfo.socket);
    if (file->connInfo.ctrlSocket != 0)
	close(file->connInfo.ctrlSocket);
    freeMem(file->url);
    freeMem(file->protocol);
    udcProtocolFree(&file->prot);
    freeMem(file->cacheDir);
    freeMem(file->bitmapFileName);
    freeMem(file->sparseFileName);
    carefulClose(&file->fSparse);
    }
freez(pFile);
}

static void qDecode(const char *input, char *buf, size_t size)
/* Reverse the qEncode performed on afterProcotol above into buf or abort. */
{
safecpy(buf, size, input);
char c, *r = buf, *w = buf;
while ((c = *r++) != '\0')
    {
    if (c == 'Q')
	{
	int q;
	if (sscanf(r, "%02X", &q))
	    {
	    *w++ = (char)q;
	    r += 2;
	    }
	else
	    errAbort("qDecode: input \"%s\" does not appear to be properly formatted "
		     "starting at \"%s\"", input, r);
	}
    else
	*w++ = c;
    }
*w = '\0';
}

char *udcPathToUrl(const char *path, char *buf, size_t size, char *cacheDir)
/* Translate path into an URL, store in buf, return pointer to buf if successful
 * and NULL if not. */
{
if (cacheDir == NULL)
    cacheDir = udcDefaultDir();
int offset = 0;
if (startsWith(cacheDir, (char *)path))
    offset = strlen(cacheDir);
if (path[offset] == '/')
    offset++;
char protocol[16];
strncpy(protocol, path+offset, sizeof(protocol));
protocol[ArraySize(protocol)-1] = '\0';
char *p = strchr(protocol, '/');
if (p == NULL)
    {
    errAbort("unable to parse protocol (first non-'%s' directory) out of path '%s'\n",
	     cacheDir, path);
    return NULL;
    }
*p++ = '\0';
char afterProtocol[4096];
qDecode(path+1+strlen(protocol)+1, afterProtocol, sizeof(afterProtocol));
safef(buf, size, "%s://%s", protocol, afterProtocol);
return buf;
}

long long int udcSizeFromCache(char *url, char *cacheDir)
/* Look up the file size from the local cache bitmap file, or -1 if there
 * is no cache for url. */
{
long long int ret = -1;
if (cacheDir == NULL)
    cacheDir = udcDefaultDir();
struct slName *sl, *slList = udcFileCacheFiles(url, cacheDir);
for (sl = slList;  sl != NULL;  sl = sl->next)
    if (endsWith(sl->name, bitmapName))
	{
	ret = udcSizeFromBitmap(sl->name);
	break;
	}
slNameFreeList(&slList);
return ret;
}

unsigned long udcCacheAge(char *url, char *cacheDir)
/* Return the age in seconds of the oldest cache file.  If a cache file is
 * missing, return the current time (seconds since the epoch). */
{
unsigned long now = clock1(), oldestTime = now;
if (cacheDir == NULL)
    cacheDir = udcDefaultDir();
struct slName *sl, *slList = udcFileCacheFiles(url, cacheDir);
if (slList == NULL)
    return now;
for (sl = slList;  sl != NULL;  sl = sl->next)
    if (fileExists(sl->name))
	oldestTime = min(fileModTime(sl->name), oldestTime);
    else
	return now;
return (now - oldestTime);
}

static void readBitsIntoBuf(FILE *f, int headerSize, int bitStart, int bitEnd,
	Bits **retBits, int *retPartOffset)
/* Do some bit-to-byte offset conversions and read in all the bytes that
 * have information in the bits we're interested in. */
{
int byteStart = bitStart/8;
int byteEnd = bitToByteSize(bitEnd);
int byteSize = byteEnd - byteStart;
Bits *bits = needLargeMem(byteSize);
fseek(f, headerSize + byteStart, SEEK_SET);
mustRead(f, bits, byteSize);
*retBits = bits;
*retPartOffset = byteStart*8;
}

static boolean allBitsSetInFile(FILE *f, int headerSize, int bitStart, int bitEnd)
/* Return TRUE if all bits in file between start and end are set. */
{
int partOffset;
Bits *bits;

readBitsIntoBuf(f, headerSize, bitStart, bitEnd, &bits, &partOffset);

int partBitStart = bitStart - partOffset;
int partBitEnd = bitEnd - partOffset;
int bitSize = bitEnd - bitStart;
int nextClearBit = bitFindClear(bits, partBitStart, bitSize);
boolean allSet = (nextClearBit >= partBitEnd);

freeMem(bits);
return allSet;
}

static void fetchMissingBlocks(struct udcFile *file, struct udcBitmap *bits, 
	int startBlock, int blockCount, int blockSize)
/* Fetch missing blocks from remote and put them into file.  errAbort if trouble. */
{
bits64 startPos = (bits64)startBlock * blockSize;
bits64 endPos = startPos + (bits64)blockCount * blockSize;
if (endPos > file->size)
    endPos = file->size;
if (endPos > startPos)
    {
    bits64 readSize = endPos - startPos;
    void *buf = needLargeMem(readSize);
    int actualSize = file->prot->fetchData(file->url, startPos, readSize, buf, &(file->connInfo));
    if (actualSize != readSize)
	errAbort("unable to fetch %lld bytes from %s @%lld (got %d bytes)",
		 readSize, file->url, startPos, actualSize);
    fseek(file->fSparse, startPos, SEEK_SET);
    mustWrite(file->fSparse, buf, readSize);
    freez(&buf);
    }
}

static void fetchMissingBits(struct udcFile *file, struct udcBitmap *bits,
	bits64 start, bits64 end, bits64 *retFetchedStart, bits64 *retFetchedEnd)
/* Scan through relevant parts of bitmap, fetching blocks we don't already have. */
{
/* Fetch relevant part of bitmap into memory */
int partOffset;
Bits *b;
int startBlock = start / bits->blockSize;
int endBlock = (end + bits->blockSize - 1) / bits->blockSize;
readBitsIntoBuf(bits->f, udcBitmapHeaderSize, startBlock, endBlock, &b, &partOffset);

/* Loop around first skipping set bits, then fetching clear bits. */
boolean dirty = FALSE;
int s = startBlock - partOffset;
int e = endBlock - partOffset;
for (;;)
    {
    int nextClearBit = bitFindClear(b, s, e);
    if (nextClearBit >= e)
        break;
    int nextSetBit = bitFindSet(b, nextClearBit, e);
    int clearSize =  nextSetBit - nextClearBit;

    fetchMissingBlocks(file, bits, nextClearBit + partOffset, clearSize, bits->blockSize);
    bitSetRange(b, nextClearBit, clearSize);

    dirty = TRUE;
    if (nextSetBit >= e)
        break;
    s = nextSetBit;
    }

if (dirty)
    {
    /* Update bitmap on disk.... */
    int byteStart = startBlock/8;
    int byteEnd = bitToByteSize(endBlock);
    int byteSize = byteEnd - byteStart;
    fseek(bits->f, byteStart + udcBitmapHeaderSize, SEEK_SET);
    mustWrite(bits->f, b, byteSize);
    }

freeMem(b);
*retFetchedStart = startBlock * bits->blockSize;
*retFetchedEnd = endBlock * bits->blockSize;
}

static boolean udcCacheContains(struct udcFile *file, struct udcBitmap *bits, 
	bits64 offset, int size)
/* Return TRUE if cache already contains region. */
{
bits64 endOffset = offset + size;
int startBlock = offset / bits->blockSize;
int endBlock = (endOffset + bits->blockSize - 1) / bits->blockSize;
return allBitsSetInFile(bits->f, udcBitmapHeaderSize, startBlock, endBlock);
}


static void udcFetchMissing(struct udcFile *file, struct udcBitmap *bits, bits64 start, bits64 end)
/* Fetch missing pieces of data from file */
{
/* Call lower level routine fetch remote data that is not already here. */
bits64 fetchedStart, fetchedEnd;
fetchMissingBits(file, bits, start, end, &fetchedStart, &fetchedEnd);

/* Update file startData/endData members to include new data (and old as well if
 * the new data overlaps the old). */
if (rangeIntersection(file->startData, file->endData, fetchedStart, fetchedEnd) >= 0)
    {
    if (fetchedStart > file->startData)
        fetchedStart = file->startData;
    if (fetchedEnd < file->endData)
        fetchedEnd = file->endData;
    }
file->startData = fetchedStart;
file->endData = fetchedEnd;
}

static boolean udcCachePreload(struct udcFile *file, bits64 offset, int size)
/* Make sure that given data is in cache - fetching it remotely if need be. 
 * Return TRUE on success. */
{
boolean ok = TRUE;
/* We'll break this operation into blocks of a reasonable size to allow
 * other processes to get cache access, since we have to lock the cache files. */
bits64 s,e, endPos=offset+size;
for (s = offset; s < endPos; s = e)
    {
    /* Figure out bounds of this section. */
    e = s + udcMaxBytesPerRemoteFetch;
    if (e > endPos)
	e = endPos;

    struct udcBitmap *bits = udcBitmapOpen(file->bitmapFileName);
    if (bits->version == file->bitmapVersion)
	{
	if (!udcCacheContains(file, bits, s, e-s))
	    udcFetchMissing(file, bits, s, e);
	}
    else
	{
	ok = FALSE;
	verbose(2, "udcCachePreload version check failed %d vs %d", 
		bits->version, file->bitmapVersion);
	}
    udcBitmapClose(&bits);
    if (!ok)
        break;
    }
return ok;
}

int udcRead(struct udcFile *file, void *buf, int size)
/* Read a block from file.  Return amount actually read. */
{
/* Figure out region of file we're going to read, and clip it against file size. */
bits64 start = file->offset;
if (start > file->size)
    return 0;
bits64 end = start + size;
if (end > file->size)
    end = file->size;
size = end - start;

/* If we're outside of the window of file we already know is good, then have to
 * consult cache on disk, and maybe even fetch data remotely! */
if (start < file->startData || end > file->endData)
    {
    if (!udcCachePreload(file, start, size))
	{
	verbose(2, "udcCachePreload failed");
	return 0;
	}
    /* Currently only need fseek here.  Would be safer, but possibly
     * slower to move fseek so it is always executed in front of read, in
     * case other code is moving around file pointer. */
    fseek(file->fSparse, start, SEEK_SET);
    }
mustRead(file->fSparse, buf, size);
file->offset += size;
return size;
}

void udcMustRead(struct udcFile *file, void *buf, int size)
/* Read a block from file.  Abort if any problem, including EOF before size is read. */
{
int sizeRead = udcRead(file, buf, size);
if (sizeRead < size)
    errAbort("udc couldn't read %d bytes from %s, did read %d", size, file->url, sizeRead);
}

int udcGetChar(struct udcFile *file)
/* Get next character from file or die trying. */
{
UBYTE b;
udcMustRead(file, &b, 1);
return b;
}

bits64 udcReadBits64(struct udcFile *file, boolean isSwapped)
/* Read and optionally byte-swap 64 bit entity. */
{
bits64 val;
udcMustRead(file, &val, sizeof(val));
if (isSwapped)
    val = byteSwap64(val);
return val;
}

bits32 udcReadBits32(struct udcFile *file, boolean isSwapped)
/* Read and optionally byte-swap 32 bit entity. */
{
bits32 val;
udcMustRead(file, &val, sizeof(val));
if (isSwapped)
    val = byteSwap32(val);
return val;
}

bits16 udcReadBits16(struct udcFile *file, boolean isSwapped)
/* Read and optionally byte-swap 16 bit entity. */
{
bits16 val;
udcMustRead(file, &val, sizeof(val));
if (isSwapped)
    val = byteSwap16(val);
return val;
}

float udcReadFloat(struct udcFile *file, boolean isSwapped)
/* Read and optionally byte-swap floating point number. */
{
float val;
udcMustRead(file, &val, sizeof(val));
if (isSwapped)
    val = byteSwapFloat(val);
return val;
}

double udcReadDouble(struct udcFile *file, boolean isSwapped)
/* Read and optionally byte-swap double-precision floating point number. */
{
double val;
udcMustRead(file, &val, sizeof(val));
if (isSwapped)
    val = byteSwapDouble(val);
return val;
}

char *udcReadStringAndZero(struct udcFile *file)
/* Read in zero terminated string from file.  Do a freeMem of result when done. */
{
char shortBuf[2], *longBuf = NULL, *buf = shortBuf;
int i, bufSize = sizeof(shortBuf);
for (i=0; ; ++i)
    {
    /* See if need to expand buffer, which is initially on stack, but if it gets big goes into 
     * heap. */
    if (i >= bufSize)
        {
	int newBufSize = bufSize*2;
	char *newBuf = needLargeMem(newBufSize);
	memcpy(newBuf, buf, bufSize);
	freeMem(longBuf);
	buf = longBuf = newBuf;
	bufSize = newBufSize;
	}
    char c = udcGetChar(file);
    buf[i] = c;
    if (c == 0)
        break;
    }
char *retString = cloneString(buf);
freeMem(longBuf);
return retString;
}



void udcSeek(struct udcFile *file, bits64 offset)
/* Seek to a particular position in file. */
{
file->offset = offset;
fseek(file->fSparse, offset, SEEK_SET);
}

bits64 udcTell(struct udcFile *file)
/* Return current file position. */
{
return file->offset;
}

static bits64 rCleanup(time_t deleteTime, boolean testOnly)
/* Delete any bitmap or sparseData files last accessed before deleteTime */
{
struct fileInfo *file, *fileList = listDirX(".", "*", FALSE);
bits64 results = 0;
for (file = fileList; file != NULL; file = file->next)
    {
    if (file->isDir)
        {
	setCurrentDir(file->name);
	bits64 oneResult = rCleanup(deleteTime, testOnly);
	setCurrentDir("..");
	if (oneResult > 0)
	    {
	    if (!testOnly)
		remove(file->name);
	    results += oneResult;
	    results += file->size;
	    }
	}
    else if (sameString(file->name, bitmapName))
        {
	if (file->lastAccess < deleteTime)
	    {
	    /* Remove all files when get bitmap, so that can ensure they are deleted in 
	     * right order. */
	    results += file->size;
	    if (!testOnly)
		{
		remove(bitmapName);
		remove(sparseDataName);
		}
	    }
	}
    else if (sameString(file->name, sparseDataName))
        {
	if (results > 0)
	    results += file->size;
	}
    }
return results;
}

bits64 udcCleanup(char *cacheDir, double maxDays, boolean testOnly)
/* Remove cached files older than maxDays old. If testOnly is set
 * no clean up is done, but the size of the files that would be
 * cleaned up is still. */

{
time_t maxSeconds = maxDays * 24 * 60 * 60;
char *curPath = cloneString(getCurrentDir());
setCurrentDir(cacheDir);
time_t deleteTime = time(NULL) - maxSeconds;
bits64 result = rCleanup(deleteTime, testOnly);
setCurrentDir(curPath);
return result;
}

static char *defaultDir = "/tmp/udcCache";

char *udcDefaultDir()
/* Get default directory for cache */
{
return defaultDir;
}

void udcSetDefaultDir(char *path)
/* Set default directory for cache */
{
defaultDir = path;
}


int udcCacheTimeout()
/* Get cache timeout (if local cache files are newer than this many seconds,
 * we won't ping the remote server to check the file size and update time). */
{
return cacheTimeout;
}

void udcSetCacheTimeout(int timeout)
/* Set cache timeout (if local cache files are newer than this many seconds,
 * we won't ping the remote server to check the file size and update time). */
{
cacheTimeout = timeout;
}

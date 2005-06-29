/* net.c some stuff to wrap around net communications. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "internet.h"
#include "errabort.h"
#include "hash.h"
#include "net.h"
#include "linefile.h"

static char const rcsid[] = "$Id: net.c,v 1.41 2005/06/29 22:29:15 galt Exp $";

/* Brought errno in to get more useful error messages */

extern int errno;

static int netStreamSocket()
/* Create a TCP/IP streaming socket.  Complain and return something
 * negative if can't */
{
int sd = socket(AF_INET, SOCK_STREAM, 0);
if (sd < 0)
    warn("Couldn't make AF_INET socket.");
return sd;
}


int netConnect(char *hostName, int port)
/* Start connection with a server. */
{
int sd, err;
struct sockaddr_in sai;		/* Some system socket info. */

if (hostName == NULL)
    {
    warn("NULL hostName in netConnect");
    return -1;
    }
if (!internetFillInAddress(hostName, port, &sai))
    return -1;
if ((sd = netStreamSocket()) < 0)
    return sd;
if ((err = connect(sd, (struct sockaddr*)&sai, sizeof(sai))) < 0)
   {
   warn("Couldn't connect to %s %d", hostName, port);
   close(sd);
   return err;
   }
return sd;
}

int netMustConnect(char *hostName, int port)
/* Start connection with server or die. */
{
int sd = netConnect(hostName, port);
if (sd < 0)
   noWarnAbort();
return sd;
}

int netMustConnectTo(char *hostName, char *portName)
/* Start connection with a server and a port that needs to be converted to integer */
{
if (!isdigit(portName[0]))
    errAbort("netConnectTo: ports must be numerical, not %s", portName);
return netMustConnect(hostName, atoi(portName));
}

int netAcceptingSocketFrom(int port, int queueSize, char *host)
/* Create a socket that can accept connections from a 
 * IP address on the current machine if the current machine
 * has multiple IP addresses. */
{
struct sockaddr_in sai;
int sd;
int flag = 1;
 int len;

netBlockBrokenPipes();
if ((sd = netStreamSocket()) < 0)
    return sd;
if (!internetFillInAddress(host, port, &sai))
    return -1;
if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
    return -1;
if (getsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &flag, &len))
    warn("getsockopt error\n");
if (bind(sd, (struct sockaddr*)&sai, sizeof(sai)) == -1)
    {
    warn("Couldn't bind socket to %d: %s", port, strerror(errno));
    close(sd);
    return -1;
    }
listen(sd, queueSize);
return sd;
}

int netAcceptingSocket(int port, int queueSize)
/* Create a socket that can accept connections from
 * anywhere. */
{
return netAcceptingSocketFrom(port, queueSize, NULL);
}

int netAccept(int sd)
/* Accept incoming connection from socket descriptor. */
{
int fromLen;
return accept(sd, NULL, &fromLen);
}

int netAcceptFrom(int acceptor, unsigned char subnet[4])
/* Wait for incoming connection from socket descriptor
 * from IP address in subnet.  Subnet is something
 * returned from netParseSubnet or internetParseDottedQuad. 
 * Subnet may be NULL. */
{
struct sockaddr_in sai;		/* Some system socket info. */
ZeroVar(&sai);
sai.sin_family = AF_INET;
for (;;)
    {
    int addrSize = sizeof(sai);
    int sd = accept(acceptor, (struct sockaddr *)&sai, &addrSize);
    if (sd >= 0)
	{
	if (subnet == NULL)
	    return sd;
	else
	    {
	    unsigned char unpacked[4]; 
	    internetUnpackIp(ntohl(sai.sin_addr.s_addr), unpacked);
	    if (internetIpInSubnet(unpacked, subnet))
		{
		return sd;
		}
	    else
		{
		close(sd);
		}
	    }
	}
    }
}

FILE *netFileFromSocket(int socket)
/* Wrap a FILE around socket.  This should be fclose'd
 * and separately the socket close'd. */
{
FILE *f;
if ((socket = dup(socket)) < 0)
   errnoAbort("Couldn't dupe socket in netFileFromSocket");
f = fdopen(socket, "r+");
if (f == NULL)
   errnoAbort("Couldn't fdopen socket in netFileFromSocket");
return f;
}

static boolean plumberInstalled = FALSE;

void netBlockBrokenPipes()
/* Make it so a broken pipe doesn't kill us. */
{
if (!plumberInstalled)
    {
    signal(SIGPIPE, SIG_IGN);       /* Block broken pipe signals. */
    plumberInstalled = TRUE;
    }
}

int netReadAll(int sd, void *vBuf, size_t size)
/* Read given number of bytes into buffer.
 * Don't give up on first read! */
{
char *buf = vBuf;
size_t totalRead = 0;
int oneRead;

if (!plumberInstalled)
    netBlockBrokenPipes();
while (totalRead < size)
    {
    oneRead = read(sd, buf + totalRead, size - totalRead);
    if (oneRead < 0)
	return oneRead;
    if (oneRead == 0)
        break;
    totalRead += oneRead;
    }
return totalRead;
}

int netMustReadAll(int sd, void *vBuf, size_t size)
/* Read given number of bytes into buffer or die.
 * Don't give up if first read is short! */
{
int ret = netReadAll(sd, vBuf, size);
if (ret < 0)
    errnoAbort("Couldn't finish netReadAll");
return ret;
}

static void notGoodSubnet(char *sns)
/* Complain about subnet format. */
{
errAbort("'%s' is not a properly formatted subnet.  Subnets must consist of\n"
         "one to three dot-separated numbers between 0 and 255\n", sns);
}

void netParseSubnet(char *in, unsigned char out[4])
/* Parse subnet, which is a prefix of a normal dotted quad form.
 * Out will contain 255's for the don't care bits. */
{
out[0] = out[1] = out[2] = out[3] = 255;
if (in != NULL)
    {
    char *snsCopy = strdup(in);
    char *words[5];
    int wordCount, i;
    wordCount = chopString(snsCopy, ".", words, ArraySize(words));
    if (wordCount > 3 || wordCount < 1)
        notGoodSubnet(in);
    for (i=0; i<wordCount; ++i)
	{
	char *s = words[i];
	int x;
	if (!isdigit(s[0]))
	    notGoodSubnet(in);
	x = atoi(s);
	if (x > 255)
	    notGoodSubnet(in);
	out[i] = x;
	}
    freez(&snsCopy);
    }
}

void netParseUrl(char *url, struct netParsedUrl *parsed)
/* Parse a URL into components.   A full URL is made up as so:
 *   http://hostName:port/file
 * This is set up so that the http:// and the port are optional. 
 */
{
char *s, *t, *u;
char buf[1024];

/* Make local copy of URL. */
if (strlen(url) >= sizeof(buf))
    errAbort("Url too long: '%s'", url);
strcpy(buf, url);
url = buf;

/* Find out protocol - default to http. */
s = trimSpaces(url);
s = stringIn("://", url);
if (s == NULL)
    {
    strcpy(parsed->protocol, "http");
    s = url;
    }
else
    {
    *s = 0;
    tolowers(url);
    strncpy(parsed->protocol, url, sizeof(parsed->protocol));
    s += 3;
    }

/* Split off file part. */
u = strchr(s, '/');
if (u == NULL)
   strcpy(parsed->file, "/");
else
   {
   strncpy(parsed->file, u, sizeof(parsed->file));
   *u = 0;
   }

/* Save port if it's there.  If not default to 80. */
t = strchr(s, ':');
if (t == NULL)
   {
   if (sameWord(parsed->protocol,"http"))
      strcpy(parsed->port, "80");
   if (sameWord(parsed->protocol,"ftp"))
      strcpy(parsed->port, "21");
   }
else
   {
   *t++ = 0;
   if (!isdigit(t[0]))
      errAbort("Non-numeric port name %s", t);
   strncpy(parsed->port, t, sizeof(parsed->port));
   }

/* What's left is the host. */
strncpy(parsed->host, s, sizeof(parsed->host));
}



/* this was cloned from rudp.c - move it later for sharing */
static boolean readReadyWait(int sd, int microseconds)
/* Wait for descriptor to have some data to read, up to
 * given number of microseconds. */
{
struct timeval tv;
fd_set set;
int readyCount;

for (;;)
    {
    if (microseconds > 1000000)
	{
	tv.tv_sec = microseconds/1000000;
	tv.tv_usec = microseconds%1000000;
	}
    else
	{
	tv.tv_sec = 0;
	tv.tv_usec = microseconds;
	}
    FD_ZERO(&set);
    FD_SET(sd, &set);
    readyCount = select(sd+1, &set, NULL, NULL, &tv);
    if (readyCount < 0) 
	{
	if (errno == EINTR)	/* Select interrupted, not timed out. */
	    continue;
    	else 
    	    warn("select failure in rudp: %s", strerror(errno));
    	}
    else
	{
    	return readyCount > 0;	/* Zero readyCount indicates time out */
	}
    }
}

struct dyString *sendFtpCommand(int sd, char *cmd, boolean seeResult, boolean noTimeoutError)
/* send command to ftp server and check resulting reply code, 
   give error if not desired reply */
{   
struct dyString *rs = NULL;
int reply = 0;
char buf[4*1024];
int readSize;
char *startLastLine = NULL;
long timeOut = 1000000; /* wait in microsec */

write(sd, cmd, strlen(cmd));

rs = newDyString(4*1024);
while (1)
    {
    while (1)
	{
	if (!readReadyWait(sd, timeOut))
	    {
	    if (!noTimeoutError)
		errAbort("ftp server response timed out > %ld microsec",timeOut);
	    return rs;
	    }
	if ((readSize = read(sd, buf, sizeof(buf))) == 0)
	    break;

	dyStringAppendN(rs, buf, readSize);
	if (endsWith(rs->string,"\n"))
	    break;
	}
	
    /* find the start of the last line in the buffer */
    startLastLine = rs->string+strlen(rs->string)-1;
    if (startLastLine >= rs->string)
	if (*startLastLine == '\n') 
	    --startLastLine;
    while ((startLastLine >= rs->string) && (*startLastLine != '\n'))
	--startLastLine;
    ++startLastLine;
	
    if (strlen(startLastLine)>4)
      if (
	isdigit(startLastLine[0]) &&
	isdigit(startLastLine[1]) &&
	isdigit(startLastLine[2]) &&
	startLastLine[3]==' ')
	break;
	
    /* must be some text info we can't use, ignore it till we get status code */

    }

reply = atoi(startLastLine);

if ((reply < 200) || (reply > 399))
    errAbort("ftp server error on cmd=[%s] response=[%s]\n",cmd,rs->string);
if (!seeResult) dyStringFree(&rs);

return rs;
}


int parsePasvPort(char *rs)
/* parse PASV reply to get the port and return it */
{
char *rsCopy = strdup(rs);
char *words[7];
int wordCount;
char *rsStart = strchr(rs,'(');
char *rsEnd = strchr(rs,')');
int result = 0;
rsStart++;
*rsEnd=0;
wordCount = chopString(rsStart, ",", words, ArraySize(words));
if (wordCount != 6)
    errAbort("PASV reply does not parse correctly");
result = atoi(words[4])*256+atoi(words[5]);    
freez(&rsCopy);
return result;
}    


int netGetOpenFtp(char *url)
/* Return a file handle that will read the url. */
{
struct netParsedUrl npu;
struct dyString *dy = newDyString(512);
struct dyString *rs = NULL;
int sd;
long timeOut = 1000000; /* wait in microsec */

/* Parse the URL and connect. */
netParseUrl(url, &npu);
if (!sameString(npu.protocol, "ftp"))
    errAbort("Sorry, can only netOpen ftp's currently");
sd = netMustConnect(npu.host, atoi(npu.port));

/* Ask remote ftp server for a file. */

/* don't send a command, just read the welcome msg */
if (readReadyWait(sd, timeOut))
    sendFtpCommand(sd, "", FALSE, FALSE);

sendFtpCommand(sd, "USER anonymous\r\n", FALSE, FALSE);

sendFtpCommand(sd, "PASS x@genome.ucsc.edu\r\n", FALSE, FALSE);

rs = sendFtpCommand(sd, "PASV\r\n", TRUE, FALSE);
/* 227 Entering Passive Mode (128,231,210,81,222,250) */

dyStringPrintf(dy, "RETR %s\r\n", npu.file);
/* we can't wait for reply because 
   we need to start the next fetch connect 
   but then if there is an error e.g. missing file,
   then we don't see the err msg because we
   already closed the port and are waiting.
   And our timeout is long - indefinitely so?
*/
sendFtpCommand(sd, dy->string, FALSE, TRUE);  

close(sd);

sd = netMustConnect(npu.host, parsePasvPort(rs->string));

/* Clean up and return handle. */
dyStringFree(&dy);
dyStringFree(&rs);
return sd;
}



int netOpenHttpExt(char *url, char *method, boolean end)
/* Return a file handle that will read the url.  If end is not
 * set then can send cookies and other info to returned file 
 * handle before reading. */
{
struct netParsedUrl npu;
struct dyString *dy = newDyString(512);
int sd;

/* Parse the URL and connect. */
netParseUrl(url, &npu);
if (!sameString(npu.protocol, "http"))
    errAbort("Sorry, can only netOpen http's currently");
sd = netMustConnect(npu.host, atoi(npu.port));

/* Ask remote server for a file. */
dyStringPrintf(dy, "%s %s HTTP/1.0\r\n", method, npu.file);
dyStringPrintf(dy, "User-Agent: genome.ucsc.edu/net.c\r\n");
dyStringPrintf(dy, "Host: %s:%s\r\n", npu.host, npu.port);
dyStringPrintf(dy, "Accept: */*\r\n");
if (end)
    dyStringPrintf(dy, "\r\n", npu.host, npu.port);
write(sd, dy->string, dy->stringSize);

/* Clean up and return handle. */
dyStringFree(&dy);
return sd;
}

static int netGetOpenHttp(char *url)
/* Return a file handle that will read the url.  */
{
return netOpenHttpExt(url, "GET", TRUE);
}

int netUrlHead(char *url, struct hash *hash)
/* Go get head and return status.  Return negative number if
 * can't get head. If hash is non-null, fill it with header
 * lines, including hopefully Content-Type: */
{
int sd = netOpenHttpExt(url, "HEAD", TRUE);
int status = EIO;
if (sd >= 0)
    {
    char *line, *word;
    struct lineFile *lf = lineFileAttach(url, TRUE, sd);

    if (lineFileNext(lf, &line, NULL))
	{
	if (startsWith("HTTP/", line))
	    {
	    word = nextWord(&line);
	    word = nextWord(&line);
	    if (word != NULL && isdigit(word[0]))
	        {
		status = atoi(word);
		if (hash != NULL)
		    {
		    while (lineFileNext(lf, &line, NULL))
		        {
			word = nextWord(&line);
			if (word == NULL)
			    break;
			hashAdd(hash, word, cloneString(skipLeadingSpaces(line)));
			}
		    }
		}
	    }
	}
    lineFileClose(&lf);
    }
else
    status = errno;
return status;
}

int netUrlOpen(char *url)
/* Return unix low-level file handle for url. 
 * Just close(result) when done. */
{
if (startsWith("http://",url) || (stringIn("://", url) == NULL))
    return netGetOpenHttp(url);
else if (startsWith("ftp://",url))
    return netGetOpenFtp(url);
else    
    errAbort("Sorry, can only netOpen http and ftp currently");
return -1;    
}

struct dyString *netSlurpFile(int sd)
/* Slurp file into dynamic string and return. */
{
char buf[4*1024];
int readSize;
struct dyString *dy = newDyString(4*1024);

/* Slurp file into dy and return. */
while ((readSize = read(sd, buf, sizeof(buf))) > 0)
    dyStringAppendN(dy, buf, readSize);
return dy;
}

struct dyString *netSlurpUrl(char *url)
/* Go grab all of URL and return it as dynamic string. */
{
int sd = netUrlOpen(url);
struct dyString *dy = netSlurpFile(sd);
close(sd);
return dy;
}

static boolean netSkipHttpHeaderLines(struct lineFile *lf)
/* Skip http header lines. Return FALSE if there's a problem */
{
char *line;
if (lineFileNext(lf, &line, NULL))
    {
    if (startsWith("HTTP/", line))
        {
	char *version, *code;
	version = nextWord(&line);
	code = nextWord(&line);
	if (code == NULL)
	    {
	    warn("Strange http header on %s\n", lf->fileName);
	    return FALSE;
	    }
	if (startsWith("30", code) && isdigit(code[2]) && code[3] == 0)
	    {
	    warn("Your URL \"%s\" resulted in a redirect message "
		 "(HTTP status code %s %s).  <BR>\n"
		 "Sorry, redirects are not supported.  "
		 "Please use the new location of your URL, which you "
		 "should be able to find by viewing it in your browser: "
		 "<A HREF=\"%s\" TARGET=_BLANK>click here to view URL</A>.",
		 lf->fileName, code, line, lf->fileName);
	    return FALSE;
	    }
	else if (!sameString(code, "200"))
	    {
	    warn("%s: %s %s\n", lf->fileName, code, line);
	    return FALSE;
	    }
	while (lineFileNext(lf, &line, NULL))
	    {
	    if ((line[0] == '\r' && line[1] == 0) || line[0] == 0)
	        break;
	    }
	}
    else
        lineFileReuse(lf);
    }
lf->nlType = nlt_undet;  /* reset it so the body of the response can figure it out independent of header */    
return TRUE;
}

struct lineFile *netLineFileMayOpen(char *url)
/* Return a lineFile attatched to url. Skipp
 * http header.  Return NULL if there's a problem. */
{
int sd = netUrlOpen(url);
if (sd < 0)
    {
    warn("Couldn't open %s", url);
    return NULL;
    }
else
    {
    struct lineFile *lf = lineFileAttatch(url, TRUE, sd);
    if (startsWith("http://",url))
	{
	if (!netSkipHttpHeaderLines(lf))
	    lineFileClose(&lf);
	}
    return lf;
    }
}

struct lineFile *netLineFileOpen(char *url)
/* Return a lineFile attatched to url.  This one
 * will skip any headers.   Free this with
 * lineFileClose(). */
{
struct lineFile *lf = netLineFileMayOpen(url);
if (lf == NULL)
    noWarnAbort();
return lf;
}

boolean netSendString(int sd, char *s)
/* Send a string down a socket - length byte first. */
{
int length = strlen(s);
UBYTE len;

if (length > 255)
    errAbort("Trying to send a string longer than 255 bytes (%d bytes)", length);
len = length;
if (write(sd, &len, 1)<0)
    {
    warn("Couldn't send string to socket");
    return FALSE;
    }
if (write(sd, s, length)<0)
    {
    warn("Couldn't send string to socket");
    return FALSE;
    }
return TRUE;
}

boolean netSendLongString(int sd, char *s)
/* Send a long string down socket: two bytes for length. */
{
unsigned length = strlen(s);
UBYTE b[2];

if (length >= 64*1024)
    {
    warn("Trying to send a string longer than 64k bytes (%d bytes)", length);
    return FALSE;
    }
b[0] = (length>>8);
b[1] = (length&0xff);
if (write(sd, b, 2) < 0)
    {
    warn("Couldn't send long string to socket");
    return FALSE;
    }
if (write(sd, s, length)<0)
    {
    warn("Couldn't send long string to socket");
    return FALSE;
    }
return TRUE;
}

boolean netSendHugeString(int sd, char *s)
/* Send a long string down socket: four bytes for length. */
{
unsigned long length = strlen(s);
unsigned long l = length;
UBYTE b[4];
int i;
for (i=3; i>=0; --i)
    {
    b[i] = l & 0xff;
    l >>= 8;
    }
if (write(sd, b, 4) < 0)
    {
    warn("Couldn't send huge string to socket");
    return FALSE;
    }
if (write(sd, s, length) < 0)
    {
    warn("Couldn't send huge string to socket");
    return FALSE;
    }
return TRUE;
}


char *netGetString(int sd, char buf[256])
/* Read string into buf and return it.  If buf is NULL
 * an internal buffer will be used. Print warning message
 * and return NULL if any problem. */
{
static char sbuf[256];
UBYTE len = 0;
int length;
int sz;
if (buf == NULL) buf = sbuf;
sz = netReadAll(sd, &len, 1);
if (sz == 0)
    return NULL;
if (sz < 0)
    {
    warn("Couldn't read string length");
    return NULL;
    }
length = len;
if (length > 0)
    if (netReadAll(sd, buf, length) < 0)
	{
	warn("Couldn't read string body");
	return NULL;
	}
buf[length] = 0;
return buf;
}

char *netGetLongString(int sd)
/* Read string and return it.  freeMem
 * the result when done. */
{
UBYTE b[2];
char *s = NULL;
int length = 0;
int sz;
b[0] = b[1] = 0;
sz = netReadAll(sd, b, 2);
if (sz == 0)
    return NULL;
if (sz < 0)
    {
    warn("Couldn't read long string length");
    return NULL;
    }
length = (b[0]<<8) + b[1];
s = needMem(length+1);
if (length > 0)
    if (netReadAll(sd, s, length) < 0)
	{
	warn("Couldn't read long string body");
	return NULL;
	}
s[length] = 0;
return s;
}

char *netGetHugeString(int sd)
/* Read string and return it.  freeMem
 * the result when done. */
{
UBYTE b[4];
char *s = NULL;
unsigned long length = 0;
int sz, i;
sz = netReadAll(sd, b, 4);
if (sz == 0)
    return NULL;
if (sz < 4)
    {
    warn("Couldn't read huge string length");
    return NULL;
    }
for (i=0; i<4; ++i)
    {
    length <<= 8;
    length += b[i];
    }
s = needMem(length+1);
if (length > 0)
    {
    if (netReadAll(sd, s, length) < 0)
	{
	warn("Couldn't read huge string body");
	return NULL;
	}
    }
s[length] = 0;
return s;
}


char *netRecieveString(int sd, char buf[256])
/* Read string into buf and return it.  If buf is NULL
 * an internal buffer will be used. Abort if any problem. */
{
char *s = netGetString(sd, buf);
if (s == NULL)
     noWarnAbort();   
return s;
}

char *netRecieveLongString(int sd)
/* Read string and return it.  freeMem
 * the result when done. Abort if any problem*/
{
char *s = netGetLongString(sd);
if (s == NULL)
     noWarnAbort();   
return s;
}

char *netRecieveHugeString(int sd)
/* Read string and return it.  freeMem
 * the result when done. Abort if any problem*/
{
char *s = netGetHugeString(sd);
if (s == NULL)
     noWarnAbort();   
return s;
}


struct lineFile *netHttpLineFileMayOpen(char *url, struct netParsedUrl **npu)
/* Parse URL and open an HTTP socket for it but don't send a request yet. */
{
int sd;
struct lineFile *lf;

/* Parse the URL and try to connect. */
AllocVar(*npu);
netParseUrl(url, *npu);
if (!sameString((*npu)->protocol, "http"))
    errAbort("Sorry, can only netOpen http's currently");
sd = netConnect((*npu)->host, atoi((*npu)->port));
if (sd < 0)
    return NULL;

/* Return handle. */
lf = lineFileAttatch(url, TRUE, sd);
return lf;
} /* netHttpLineFileMayOpen */


void netHttpGet(struct lineFile *lf, struct netParsedUrl *npu,
		boolean keepAlive)
/* Send a GET request, possibly with Keep-Alive. */
{
struct dyString *dy = newDyString(512);

/* Ask remote server for the file/query. */
dyStringPrintf(dy, "GET %s HTTP/1.1\r\n", npu->file);
dyStringPrintf(dy, "User-Agent: genome.ucsc.edu/net.c\r\n");
dyStringPrintf(dy, "Host: %s:%s\r\n", npu->host, npu->port);
dyStringAppend(dy, "Accept: */*\r\n");
if (keepAlive)
  {
    dyStringAppend(dy, "Connection: Keep-Alive\r\n");
    dyStringAppend(dy, "Connection: Persist\r\n");
  }
else
    dyStringAppend(dy, "Connection: close\r\n");
dyStringAppend(dy, "\r\n");
write(lf->fd, dy->string, dy->stringSize);
/* Clean up. */
dyStringFree(&dy);
} /* netHttpGet */

int netHttpGetMultiple(char *url, struct slName *queries, void *userData,
		       void (*responseCB)(void *userData, char *req,
					  char *hdr, struct dyString *body))
/* Given an URL which is the base of all requests to be made, and a 
 * linked list of queries to be appended to that base and sent in as 
 * requests, send the requests as a batch and read the HTTP response 
 * headers and bodies.  If not all the requests get responses (i.e. if 
 * the server is ignoring Keep-Alive or is imposing a limit), try again 
 * until we can't connect or until all requests have been served. 
 * For each HTTP response, do a callback. */
{
  struct slName *qStart;
  struct slName *qPtr;
  struct lineFile *lf;
  struct netParsedUrl *npu;
  struct dyString *dyQ    = newDyString(512);
  struct dyString *body;
  char *base;
  char *hdr;
  int qCount;
  int qTotal;
  int numParseFailures;
  int contentLength;
  boolean chunked;
  boolean done;
  boolean keepAlive;

  /* Find out how many queries we'll need to do so we know how many times 
   * it's OK to run into end of file in case server ignores Keep-Alive. */
  qTotal = 0;
  for (qPtr = queries;  qPtr != NULL;  qPtr = qPtr->next)
    {
      qTotal++;
    }

  done = FALSE;
  qCount = 0;
  numParseFailures = 0;
  qStart = queries;
  while ((! done) && (qStart != NULL))
    {
      lf = netHttpLineFileMayOpen(url, &npu);
      if (lf == NULL)
	{
	  done = TRUE;
	  break;
	}
      base = cloneString(npu->file);
      /* Send all remaining requests with keep-alive. */
      for (qPtr = qStart;  qPtr != NULL;  qPtr = qPtr->next)
	{
	  dyStringClear(dyQ);
	  dyStringAppend(dyQ, base);
	  dyStringAppend(dyQ, qPtr->name);
	  strcpy(npu->file, dyQ->string);
	  keepAlive = (qPtr->next == NULL) ? FALSE : TRUE;
	  netHttpGet(lf, npu, keepAlive);
	}
      /* Get as many responses as we can; call responseCB() and 
       * advance qStart for each. */
      for (qPtr = qStart;  qPtr != NULL;  qPtr = qPtr->next)
        {
	  if (lineFileParseHttpHeader(lf, &hdr, &chunked, &contentLength))
	    {
	      body = lineFileSlurpHttpBody(lf, chunked, contentLength);
	      dyStringClear(dyQ);
	      dyStringAppend(dyQ, base);
	      dyStringAppend(dyQ, qPtr->name);
	      responseCB(userData, dyQ->string, hdr, body);
	      qStart = qStart->next;
	      qCount++;
	    }
	  else
	    {
	      if (numParseFailures++ > qTotal) {
		done = TRUE;
	      }
	      break;
	    }
	}
    }

  return qCount;
} /* netHttpMultipleQueries */



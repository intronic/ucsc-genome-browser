/* netC - net client. */
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include "common.h"
#include "options.h"
#include "obscure.h"

char signature[] = "0d2f";

void usage()
/* Explain usage and exit. */
{
errAbort("netC - net client.\n"
         "usage:\n"
	 "    netC messages(s)\n"
	 "options:\n"
	 "    -file - parameters are files to send, not text to send\n");

}


void netC(boolean isFile, int argc, char *argv[])
/* netC - a net client. */
{
int sd;
struct sockaddr_in sai;
int fromlen;
int childCount = 0;
struct hostent *host;
char *alias;
UBYTE *addr;
int i;
int port = 0x46DC;
char *command = argv[0];
int optVal = 1;
int optLen = sizeof(optVal);

/* Set up broadcast connection. */
sai.sin_family = AF_INET;
sai.sin_port = htons(port);
sai.sin_addr.s_addr = htonl(INADDR_BROADCAST);
sd = socket(AF_INET, SOCK_DGRAM, 0);
if (setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (char *)&optVal, optLen) != 0)
    {
    close(sd);
    errAbort("Can't set broadcast option on socket\n");
    }

for (i=0; i<argc; ++i)
    {
    char *s = argv[i];
    size_t size = strlen(s);
    if (isFile)
        readInGulp(s, &s, &size);
    if (sendto(sd, s, size+1, 0, (struct sockaddr *)&sai, sizeof(sai)) < 0)
	errnoAbort("Couldn't sendto size %d", size+1);
    if (isFile)
        freez(&s);
    }

close(sd);
}


int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc < 2)
    usage();
netC(optionExists("file"), argc-1, argv+1);
return 0;
}



/* netC - net client. */
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "common.h"
#include "options.h"
#include "obscure.h"


void usage()
/* Explain usage and exit. */
{
errAbort("netC - net client.\n"
         "usage:\n"
	 "    netC host count messages\n"
	 "options:\n"
	 "    -file - parameters are files to send, not text to send\n");

}

void fillInAddress(char *hostName, int port, struct sockaddr_in *address)
/* Fill in address. Return FALSE if can't.  */
{
struct hostent *hostent;
ZeroVar(address);
address->sin_family = AF_INET;
address->sin_port = htons(port);
hostent = gethostbyname(hostName);
if (hostent == NULL)
    errnoAbort("Couldn't find host %s. h_errno %d", hostName, h_errno);
memcpy(&address->sin_addr.s_addr, hostent->h_addr_list[0], sizeof(address->sin_addr.s_addr));
}

void netC(char *host, int count, char *message, boolean isFile)
/* netC - a net client. */
{
int sd;
struct sockaddr_in sai;
int i, size;
int port = 0x46DC;
char *s = message;
char buf[1024];

fillInAddress(host, port, &sai);
sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
if (isFile)
    readInGulp(s, &s, &size);

for (i=0; i<count; ++i)
    {
    size_t size;
    size = snprintf(buf, sizeof(buf), "%s %d", s, i);
    if (sendto(sd, buf, size+1, 0, (struct sockaddr *)&sai, sizeof(sai)) < 0)
	errnoAbort("Couldn't sendto size %d", size+1);
    }
if (isFile)
    freez(&s);
close(sd);
}


int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc < 4)
    usage();
netC(argv[1], atoi(argv[2]), argv[3], optionExists("file"));
return 0;
}



/* parasol - Parasol program - for launching programs in parallel on a computer cluster. */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "common.h"
#include "net.h"
#include "linefile.h"
#include "errabort.h"
#include "dystring.h"
#include "hash.h"
#include "options.h"
#include "paraLib.h"

int hubFd;	/* Network connection to paraHub. */

void closeHubFd()
/* Close connection to hub if it's open. */
{
if (hubFd != 0)
    {
    close(hubFd);
    hubFd = 0;
    }
}

void usage()
/* Explain usage and exit. */
{
errAbort(
  "parasol - Parasol program - for launching programs in parallel on a computer cluster\n"
  "usage:\n"
  "   parasol add machine machineName localTempDir\n"
  "   parasol remove machine machineName\n"
  "   parasol [options] add job command-line\n"
  "         options: -out=out -in=in -dir=dir -results=file\n"
  "   parasol [options] qsub command-line\n"
  "         options: -out=out -in=in -dir=dir -results=file\n"
  "   parasol remove job id\n"
  "   parasol close (close down system - kill paraHub, but not paraNodes on remote machines)\n"
  "   parasol ping [count]\n"
  "   parasol remove user name [jobPattern]\n"
  "   parasol list machines\n"
  "   parasol list jobs\n"
  "   parasol add spoke\n"
  "   parasol qstat\n"
  "   parasol status\n"
  );
}

void commandHub(char *command)
/* Send a command to hub. */
{
int sigSize = strlen(paraSig);
if (write(hubFd, paraSig, sigSize) < sigSize)
    errnoAbort("Couldn't write signature to hub");
netSendLongString(hubFd, command);
}

char *hubCommandGetReciept(char *command)
/* Send command to hub,  wait for one line respons and
 * return it.  freeMem return value when done. */
{
commandHub(command);
return netRecieveLongString(hubFd);
}

void hubCommandCheckReciept(char *command)
/* Send command to hub, and wait for one line response which should
 * be 'ok'. */
{
char *line = hubCommandGetReciept(command);
if (line == NULL || !sameString(line, "ok"))
    errAbort("Hub didn't acknowledge %s", command);
freeMem(line);
}

void hubCommandAndPrint(char *command)
/* Send command to hub, and print response until blank line. */
{
char *line = NULL;
struct slRef *list = NULL, *ref;

/* Issue command and suck down response quickly into memory. */
commandHub(command);
for (;;)
    {
    line = netRecieveLongString(hubFd);
    if (line[0] == 0)
        break;
    refAdd(&list, line);
    }
slReverse(&list);

/* Print results. */
for (ref = list; ref != NULL; ref = ref->next)
    {
    line = ref->val;
    puts(line);
    }
}


void addMachine(char *machine, char *tempDir)
/* Tell hub about a new machine. */
{
char buf[512];
sprintf(buf, "%s %s %s", "addMachine", machine, tempDir);
commandHub(buf);
}

void addJob(int argc, char *argv[], boolean printId)
/* Tell hub about a new job. */
{
struct dyString *dy = newDyString(1024);
char curDir[512];
char defaultResults[512];
char *in = optionVal("in", "/dev/null"), 
     *out = optionVal("out", "/dev/null"), 
     *dir = optionVal("dir", curDir),
     *results = optionVal("results", defaultResults);
char *jobIdString;
int i;

getcwd(curDir, sizeof(curDir));
sprintf(defaultResults, "%s/results", curDir);
dyStringPrintf(dy, "addJob %s %s %s %s %s", getlogin(), dir, in, out, results);
for (i=0; i<argc; ++i)
    dyStringPrintf(dy, " %s", argv[i]);
jobIdString = hubCommandGetReciept(dy->string);
dyStringFree(&dy);
if (printId)
    {
    printf("your job %s (\"%s", jobIdString, argv[0]);
    for (i=1; i<argc; ++i)
	 printf(" %s", argv[i]);
    printf("\") has been submitted\n");
    }
freez(&jobIdString);
}

void addSpoke()
/* Tell hub to add a spoke. */
{
commandHub("addSpoke");
}

void removeMachine(char *machine)
/* Tell hub to get rid of machine. */
{
char buf[512];
sprintf(buf, "%s %s", "removeMachine", machine);
commandHub(buf);
}

void removeJob(char *job)
/* Tell hub to remove job. */
{
char buf[512];
if (!isdigit(job[0]))
    errAbort("remove job requires a numerical job id");
sprintf(buf, "%s %s", "removeJob", job);
commandHub(buf);
}

void dehub()
/* Tell hub to die. */
{
commandHub("quit");
}

struct jobInfo
/* Info on a job. */
    {
    struct jobInfo *next;   /* Next in list. */
    char *id;		    /* Job id. */
    char *machine;	    /* Machine name (or NULL) */
    char *user;		    /* Job user. */
    char *command;	    /* Job command line. */
    };

void jobInfoFree(struct jobInfo **pJob)
/* Free up a job info. */
{
struct jobInfo *job;
if ((job = *pJob) != NULL)
    {
    freeMem(job->id);
    freeMem(job->machine);
    freeMem(job->user);
    freeMem(job->command);
    freez(pJob);
    }
}

void jobInfoFreeList(struct jobInfo **pList)
/* Free up list of jobInfo */
{
struct jobInfo *el, *next;
for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    jobInfoFree(&el);
    }
*pList = NULL;
}

struct jobInfo *jobInfoParse(char *line)
/* Create a job info from parsing line.  Eats line.  Returns
 * NULL if there's a problem. */
{
char *id, *user, *machine, *command;
struct jobInfo *job;
if ((id = nextWord(&line)) == NULL) return NULL;
if ((machine = nextWord(&line)) == NULL) return NULL;
if ((user = nextWord(&line)) == NULL) return NULL;
line = skipLeadingSpaces(line);
if (line == NULL || line[0] == 0) return NULL;
command = line;
AllocVar(job);
job->id = cloneString(id);
job->user = cloneString(user);
job->machine = cloneString(machine);
job->command = cloneString(command);
return job;
}

struct jobInfo *jobInfoMustParse(char *line)
/* Create job info from parsing line.  Eats line.  Aborts
 * with error message if a problem. */
{
struct jobInfo *job = jobInfoParse(line);
if (job == NULL)
    errAbort("Couldn't parse job %s", line);
return job;
}

void reopenHub()
/* Close hub if it's already open.  Then reopen it. */
{
closeHubFd();
hubFd = netMustConnect(getHost(), paraPort);
}

struct jobInfo *getJobList()
/* Read job list from server. */
{
char *line;
struct jobInfo *jobList = NULL, *job;
commandHub("listJobs");
for (;;)
    {
    line = netRecieveLongString(hubFd);
    if (line[0] == 0)
        break;
    job = jobInfoMustParse(line);
    slAddHead(&jobList, job);
    freeMem(line);
    }
slReverse(&jobList);
return jobList;
}

void removeUserJobs(char *user, int argc, char *argv[])
/* Remove jobs associated with user. */
{
struct jobInfo *jobList = getJobList(), *job;
char *wildCard = NULL;
if (argc > 0) wildCard = argv[0];
for (job = jobList; job != NULL; job = job->next)
    {
    if (sameString(user, job->user) && 
    	(wildCard == NULL || wildMatch(wildCard, job->command)))
	{
	reopenHub();
        removeJob(job->id);
	}
    }
jobInfoFreeList(&jobList);
}

void listJobs()
/* Send list job command to hub and print. */
{
hubCommandAndPrint("listJobs");
}

void listMachines()
/* Send list machines command to hub and print. */
{
hubCommandAndPrint("listMachines");
}

void status()
/* Send status command to hub and print. */
{
hubCommandAndPrint("status");
}

void qstat()
/* Send qstat to hub and print result. */
{
hubCommandAndPrint("qstat");
}

void pstat()
/* Send pstat to hub and print result. */
{
hubCommandAndPrint("pstat");
}

void ping(int count)
/* Ping hub server given number of times. */
{
int i;
for (i=0; i<count; ++i)
    {
    if (i != 0)
	reopenHub();
    hubCommandCheckReciept("ping");
    }
printf("Pinged hub %d times\n", count);
}


void parasol(char *command, int argc, char *argv[])
/* parasol - Parasol program - for launching programs in parallel on a computer cluster. */
{
char *subType = argv[0];
atexit(closeHubFd);
reopenHub();
if (sameString(command, "qsub"))
    {
    if (argc < 1)
	usage;
    addJob(argc, argv, TRUE);
    }
else if (sameString(command, "add"))
    {
    if (argc < 1)
        usage();
    if (sameString(subType, "machine"))
	{
	if (argc != 3)
	    usage();
	addMachine(argv[1], argv[2]);
	}
    else if (sameString(subType, "job"))
	{
	if (argc < 2)
	    usage;
        addJob(argc-1, argv+1, FALSE);
	}
    else if (sameString(subType, "spoke"))
        addSpoke();
    else
        usage();
    }
else if (sameString(command, "remove"))
    {
    if (argc < 2)
        usage();
    if (sameString(subType, "machine"))
        removeMachine(argv[1]);
    else if (sameString(subType, "job"))
        removeJob(argv[1]);
    else if (sameString(subType, "user"))
        removeUserJobs(argv[1], argc-2, argv+2);
    else
        usage();
    }
else if (sameString(command, "list"))
    {
    if (argc != 1)
        usage();
    if (sameString(subType, "machine") || sameString(subType, "machines"))
        listMachines();
    else if (sameString(subType, "job") || sameString(subType, "jobs"))
        listJobs();
    else
        usage();
    }
else if (sameString(command, "ping"))
    {
    int count = 1;
    if (argc >= 1)
        {
	if (!isdigit(argv[0][0]))
	    usage();
	count = atoi(argv[0]);
	}
    ping(count);
    }
else if (sameString(command, "status"))
    status();
else if (sameString(command, "qstat"))
    qstat();
else if (sameString(command, "pstat"))
    pstat();
else if (sameString(command, "close"))
    dehub();
else
    usage();
closeHubFd();
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHashSome(&argc, argv, TRUE);
if (argc < 2)
    usage();
parasol(argv[1], argc-2, argv+2);
return 0;
}

/* paraNode - parasol node server. */
#include <signal.h>
#include <sys/wait.h>
#include "common.h"
#include "errabort.h"
#include "dystring.h"
#include "hash.h"
#include "options.h"
#include "paraLib.h"
#include "net.h"

int socketHandle;		/* Handle to socket to hub. */
int connectionHandle;	        /* Connection to singel hub request. */
char execCommand[16*1024];          /* Name of command currently executing. */
char *hostName;			/* Name of this host. */

boolean busy = FALSE;           /* True if executing a command. */
boolean gotZombie = FALSE;      /* True if command finished. */
int childId;                    /* PID of child. */
int grandId;			/* PID of grandchild. */

void usage()
/* Explain usage and exit. */
{
errAbort("paraNode - parasol node serve.\n"
         "usage:\n"
	 "    paraNode start\n"
	 "options:\n"
	 "    log=file - file may be 'stdout' to go to console");
}

FILE *logFile = NULL;

void vLogIt(char *format, va_list args)
/* Virtual logit. */
{
if (logFile != NULL)
    {
    vfprintf(logFile, format, args);
    fflush(logFile);
    }
}

void logIt(char *format, ...)
/* Print message to log file. */
{
if (logFile != NULL)
    {
    va_list args;
    va_start(args, format);
    vLogIt(format, args);
    va_end(args);
    }
}

void childSignalHandler(int x)
/* Handle child died signal. */
{
gotZombie = TRUE;
}

void clearZombie()
/* Wait on any leftover zombie. */
{
if (gotZombie)
    {
    int pid, status;
    pid = wait(&status);
    if (pid == childId)
	{
	childId = 0;
	busy = FALSE;
	gotZombie = FALSE;
	execCommand[0] = 0;
	}
    }
}

void handleTerm(int x)
/* Handle SIGTERM for manager process - kill grandchild. */
{
if (grandId != 0)
    {
    int id = grandId;
    grandId = 0;
    kill(id, SIGTERM);
    }
exit(-1);
}

void manageExec(char *managingHost, char *jobId, char *user,
	char *dir, char *in, char *out, char *err,
	char *exe, char **params)
/* This routine is the child process of doExec.
 * It spawns a grandchild that actually does the
 * work and waits on it.  It then lets the hub
 * know when the exec is done. */
{
close(connectionHandle);
if ((grandId = fork()) == 0)
    {
    int newStdin, newStdout, newStderr;

    /* Get rid of excess old file handles. */
    close(socketHandle);

    /* Change to given dir. */
    chdir(dir);

    /* Redirect standard io.  You'd think there'd be a less
     * cryptic way to do this. */
    newStdin = open(in, O_RDONLY);
    close(0);
    dup(newStdin);
    newStdout = open(out, O_WRONLY | O_CREAT, 0666);
    close(1);
    dup(newStdout);
    newStderr = open(err, O_WRONLY | O_CREAT, 0666);
    close(2);
    dup(newStderr);
    close(newStdin);
    close(newStdout);
    close(newStderr);

    if (execvp(exe, params) < 0)
	{
	perror("");
	errAbort("Error execlp'ing %s %s", exe, params);
	}
    // Never gets here because of execlp.
    errAbort("Must have skipped execlp!");
    }
else
    {
    /* Wait on executed job and send jobID and status back to whoever
     * started the job. */
    int status;
    int sd;
    char buf[128];
    signal(SIGTERM, handleTerm);
    wait(&status);
    // sleep(1);	/* Help keep from overloading server. I wish there were a better way. */
    sd = netConnect(managingHost, paraPort);
    if (sd > 0)
        {
	sprintf(buf, "jobDone %s %d", jobId, status);
	write(sd, paraSig, strlen(paraSig));
	netSendLongString(sd, buf);
	close(sd);
	}
    exit(0);
    }
}

void doCheck(char *line)
/* Send back check result */
{
char *managingHost = nextWord(&line);
if (managingHost != NULL)
    {
    int sd = netConnect(managingHost, paraPort);
    char *status = (busy ? "busy" : "free");
    char buf[256];
    sprintf(buf, "checkIn %s %s", hostName, status);
    write(sd, paraSig, strlen(paraSig));
    netSendLongString(sd, buf);
    close(sd);
    }
}

void doResurrect(char *line)
/* Send back I'm alive message */
{
char *managingHost = nextWord(&line);
if (managingHost != NULL)
    {
    int sd = netConnect(managingHost, paraPort);
    char buf[256];
    sprintf(buf, "alive %s", hostName);
    write(sd, paraSig, strlen(paraSig));
    netSendLongString(sd, buf);
    close(sd);
    }
}

void doRun(char *line)
/* Execute command. */
{
static char *args[1024];
char *managingHost, *jobId, *user, *dir, *in, *out, *err, *cmd;
int argCount;
int commandOffset;
if (line == NULL)
    warn("Executing nothing...");
else if (!busy || gotZombie)
    {
    char *exe;
    if (gotZombie)
	clearZombie();
    strcpy(execCommand, line);
    argCount = chopLine(line, args);
    if (argCount >= ArraySize(args))
        errAbort("Too many arguments");
    args[argCount] = NULL;
    managingHost = args[0];
    jobId = args[1];
    user = args[2];
    dir = args[3];
    in = args[4];
    out = args[5];
    err = args[6];
    commandOffset = 7;
    signal(SIGCHLD, childSignalHandler);
    if ((childId = fork()) == 0)
	{
	manageExec(managingHost, jobId, user, dir, in, out, err,
		args[commandOffset], args+commandOffset);
	}
    else
	{
	busy = TRUE;
	}
    }
else
    {
    warn("Trying to run when busy.");
    }
}

void doKill()
/* Kill current job if any. */
{
if (childId != 0)
    {
    logIt("Killing %s\n", execCommand);
    kill(childId, SIGTERM);
    clearZombie();
    }
else
    {
    warn("Nothing to kill\n");
    }
}


void doStatus()
/* Report status. */
{
char *status = (busy ? "busy" : "free");
struct dyString *dy = newDyString(256);
dyStringAppend(dy, status);
if (busy)
    dyStringPrintf(dy, " %s", execCommand);
write(connectionHandle, dy->string, dy->stringSize);
}

void paraNode()
/* paraNode - a net server. */
{
char *buf = NULL, *line;
int fromLen, readSize;
int childCount = 0;
char *command;
char signature[20];
int sigLen = strlen(paraSig);

/* We have to know who we are... */
hostName = getHost();

/* Precompute some signature stuff. */
assert(sigLen < sizeof(signature));
signature[sigLen] = 0;

/* Set up socket and self to listen to it. */
socketHandle = netAcceptingSocket(paraPort, 10);
if (socketHandle < 0)
    errAbort("I'm dead without my socket, sorry");

/* Event loop. */
for (;;)
    {
    connectionHandle = netAccept(socketHandle);
    if (netMustReadAll(connectionHandle, signature, sigLen))
	{
	if (sameString(paraSig, signature))
	    {
	    line = buf = netGetLongString(connectionHandle);
	    logIt("node  %s: %s\n", hostName, line);
	    clearZombie();
	    ++childCount;
	    command = nextWord(&line);
	    if (sameString("quit", command))
		break;
	    else if (sameString("run", command))
		doRun(line);
	    else if (sameString("status", command))
		doStatus();
	    else if (sameString("kill", command))
		doKill();
	    else if (sameString("check", command))
	        doCheck(line);
	    else if (sameString("resurrect", command))
	        doResurrect(line);
	    freez(&buf);
	    }
	}
    if (connectionHandle != 0)
	{
	close(connectionHandle);
	connectionHandle = 0;
	}
    }
}

void paraFork()
/* Fork off real handler and exit */
{
if (fork() == 0)
    {
    /* Set up log handler. */
    char *log = optionVal("log", NULL);
    if (log != NULL)
        logFile = mustOpen(log, "w");
    pushWarnHandler(vLogIt);

    /* Close standard file handles. */
    close(0);
    if (log == NULL || !sameString(log, "stdout"))
	close(1);
    close(2);

    /* Execute daemon. */
    paraNode();
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 2)
    usage();
paraFork();
return 0;
}



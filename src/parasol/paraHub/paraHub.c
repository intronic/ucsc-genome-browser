/* paraHub - parasol hub server. */
#include <time.h>
#include "common.h"
#include "options.h"
#include "linefile.h"
#include "hash.h"
#include "errabort.h"
#include "dystring.h"
#include "dlist.h"
#include "net.h"
#include "paraLib.h"
#include "paraHub.h"
#include "machSpec.h"

/* Some command-line configurable quantities and their defaults. */
int jobCheckPeriod = 10;	/* Minutes between checking running jobs. */
int machineCheckPeriod = 20;	/* Minutes between checking dead machines. */
int initialSpokes = 50;		/* Number of spokes to start with. */

void usage()
/* Explain usage and exit. */
{
errAbort("paraHub - parasol hub server.\n"
         "usage:\n"
	 "    paraHub machineList\n"
	 "Where machine list is a file with machine names in the\n"
	 "first column, and number of CPUs in the second column.\n"
	 "options:\n"
	 "    spokes=N  Number of processes that feed jobs to nodes - default %d\n"
	 "    jobCheckPeriod=N  Minutes between checking on job - default %d\n"
	 "    machineCheckPeriod=N Seconds between checking on machine - default %d\n"
	 "    log=logFile Write a log to logFile. Use 'stdout' here for console\n"
	               ,
	 initialSpokes, jobCheckPeriod, machineCheckPeriod
	 );
}

struct spoke *spokeList;	/* List of all spokes. */
struct dlList *freeSpokes;      /* List of free spokes. */
struct dlList *busySpokes;	/* List of busy spokes. */
struct dlList *deadSpokes;	/* List of dead spokes. */

struct machine *machineList; /* List of all machines. */
struct dlList *freeMachines;     /* List of machines ready for jobs. */
struct dlList *busyMachines;     /* List of machines running jobs. */
struct dlList *deadMachines;     /* List of machines that aren't running. */

struct dlList *pendingJobs;     /* Jobs still to do. */
struct dlList *runningJobs;     /* Jobs that are running. */
int nextJobId = 0;		/* Next free job id. */

struct resultQueue *resultQueues; /* Result files. */
int finishedJobCount = 0;		/* Number of finished jobs. */
int crashedJobCount = 0;		/* Number of crashed jobs. */

char *hubHost;	/* Name of machine running this. */

void removeJobId(int id);
/* Remove job with given ID. */

void setupLists()
/* Make up data structure to keep track of each machine. 
 * Try to get sockets on all of them. */
{
freeMachines = newDlList();
busyMachines = newDlList();
deadMachines = newDlList();
pendingJobs = newDlList();
runningJobs = newDlList();
freeSpokes = newDlList();
busySpokes = newDlList();
deadSpokes = newDlList();
}

boolean runNextJob()
/* Assign next job in pending queue if any to a machine. */
{
if (!dlEmpty(freeMachines) && !dlEmpty(freeSpokes) && !dlEmpty(pendingJobs))
    {
    struct dlNode *mNode, *jNode, *sNode;
    struct spoke *spoke;
    struct job *job;
    struct machine *machine;
    time_t now = time(NULL);

    /* Get free resources from free list and move them to busy lists. */
    mNode = dlPopHead(freeMachines);
    dlAddTail(busyMachines, mNode);
    machine = mNode->val;
    machine->lastChecked = now;
    jNode = dlPopHead(pendingJobs);
    dlAddTail(runningJobs, jNode);
    job = jNode->val;
    sNode = dlPopHead(freeSpokes);
    dlAddTail(busySpokes, sNode);
    spoke = sNode->val;
    spoke->lastPinged = now;
    spoke->pingCount = 0;

    /* Tell machine, job, and spoke about each other. */
    machine->job = job;
    job->machine = machine;
    job->startTime = now;
    spokeSendJob(spoke, machine, job);
    return TRUE;
    }
else
    return FALSE;
}

void runner(int count)
/* Try to run a couple of jobs. */
{
while (--count >= 0)
    if (!runNextJob())
        break;
}

struct machine *machineNew(char *name, char *tempDir)
/* Create a new machine structure. */
{
struct machine *mach;
AllocVar(mach);
mach->name = cloneString(name);
mach->tempDir = cloneString(tempDir);
AllocVar(mach->node);
mach->node->val = mach;
return mach;
}

void machineFree(struct machine **pMach)
/* Delete machine structure. */
{
struct machine *mach = *pMach;
if (mach != NULL)
    {
    freeMem(mach->node);
    freeMem(mach->name);
    freeMem(mach->tempDir);
    freez(pMach);
    }
}

void doAddMachine(char *name, char *tempDir)
/* Add machine to pool. */
{
struct machine *mach;
mach = machineNew(name, tempDir);
dlAddTail(freeMachines, mach->node);
slAddHead(&machineList, mach);
}

void addMachine(char *line)
/* Process message to add machine to pool. */
{
char *name = nextWord(&line);
char *tempDir = nextWord(&line);
if (tempDir != NULL)
    {
    doAddMachine(name, tempDir);
    runner(1);
    }
}

struct machine *findMachine(char *name)
/* Find named machine. */
{
struct machine *mach;
for (mach = machineList; mach != NULL; mach = mach->next)
     {
     if (sameString(mach->name, name))
         return mach;
     }
return NULL;
}

struct machine *findMachineOnDlList(char *name, struct dlList *list)
/* Find machine on list, or return NULL */
{
struct dlNode *node;
struct machine *mach;
for (node = list->head; !dlEnd(node); node = node->next)
    {
    mach = node->val;
    if (sameString(mach->name, name))
        return mach;
    }
return NULL;
}

struct machine *findMachineWithJob(char *name, int jobId)
/* Find named machine that is running job.  If jobId is
 * 0, find it regardless of job it's running. */
{
struct machine *mach;
for (mach = machineList; mach != NULL; mach = mach->next)
     {
     if (sameString(mach->name, name))
	 {
	 struct job *job = mach->job;
	 if (jobId == 0)
	     return mach;
	 if (job != NULL && job->id == jobId)
	     return mach;
	 }
     }
return NULL;
}


struct job *jobFind(struct dlList *list, int id)
/* Find node of job with given id on list.  Return NULL if
 * not found. */
{
struct dlNode *el;
struct job *job;
for (el = list->head; !dlEnd(el); el = el->next)
    {
    job = el->val;
    if (job->id == id)
        return job;
    }
return NULL;
}

void removeMachine(char *name)
/* Remove machine form pool. */
{
struct machine *mach;
name = trimSpaces(name);
if ((mach = findMachine(name)) != NULL)
    {
    if (mach->job != NULL)
	removeJobId(mach->job->id);
    dlRemove(mach->node);
    slRemoveEl(&machineList, mach);
    machineFree(&mach);
    }
}

void nodeDown(char *line)
/* Deal with a node going down - move it to dead list and
 * put job back on head of job list. */
{
struct machine *mach;
char *machName = nextWord(&line);
char *jobIdString = nextWord(&line);
int jobId;
struct job *job;

if (jobIdString == NULL)
    return;
jobId = atoi(jobIdString);
if ((mach = findMachineWithJob(machName, jobId)) != NULL)
    {
    if ((job = mach->job) != NULL)
        {
	mach->job = NULL;
	job->machine = NULL;
	dlRemove(job->node);
	dlAddHead(pendingJobs, job->node);
	}
    dlRemove(mach->node);
    mach->lastChecked = time(NULL);
    mach->isDead = TRUE;
    dlAddTail(deadMachines, mach->node);
    }
runner(1);
}

char *exeFromCommand(char *cmd)
/* Return executable name (without path) given command line. */
{
static char exe[128];
char *s,*e,*x;
int i, size;
int lastSlash = -1;

/* Isolate first space-delimited word between s and e. */
s = skipLeadingSpaces(cmd);
e = skipToSpaces(cmd);
if (e == NULL) 
    e = s + strlen(s);
size = e - s;

/* Find last '/' in this word if any, and reposition s after it. */
for (i=0; i<size; ++i)
    {
    if (s[i] == '/')
        lastSlash = i;
    }
if (lastSlash > 0)
    s += lastSlash + 1;

/* Copy whats left to string to return . */
size = e - s;
if (size >= sizeof(exe))
    size = sizeof(exe)-1;
memcpy(exe, s, size);
exe[size] = 0;
return exe;
}

struct job *jobNew(char *cmd, char *user, char *dir, char *in, char *out, char *results)
/* Create a new job structure */
{
struct job *job;
AllocVar(job);
AllocVar(job->node);
job->node->val = job;
job->id = ++nextJobId;
job->exe = cloneString(exeFromCommand(cmd));
job->cmd = cloneString(cmd);
job->user = cloneString(user);
job->dir = cloneString(dir);
job->in = cloneString(in);
job->out = cloneString(out);
job->results = cloneString(results);
return job;
}

void jobFree(struct job **pJob)
/* Free up a job. */
{
struct job *job = *pJob;
if (job != NULL)
    {
    freeMem(job->node);
    freeMem(job->cmd);
    freeMem(job->user);
    freeMem(job->dir);
    freeMem(job->in);
    freeMem(job->out);
    freeMem(job->err);
    freeMem(job->results);
    freez(pJob);
    }
}

boolean sendViaSpoke(struct machine *machine, char *message)
/* Send a message to machine via spoke. */
{
struct dlNode *node = dlPopHead(freeSpokes);
struct spoke *spoke;
if (node == NULL)
    return FALSE;
dlAddTail(busySpokes, node);
spoke = node->val;
spoke->lastPinged = time(NULL);
spokeSendMessage(spoke, machine, message);
return TRUE;
}

void checkPeriodically(struct dlList *machList, int period, char *checkMessage,
	int spokesToUse, time_t now)
/* Periodically send checkup messages to machines on list. */
{
struct dlNode *mNode;
struct machine *machine;
char message[512];
int i;

sprintf(message, "%s %s", checkMessage, hubHost);
for (i=0; i<spokesToUse; ++i)
    {
    /* If we have some free spokes and some busy machines, and
     * the busy machines haven't been checked for a while, go
     * check them. */
    if (dlEmpty(freeSpokes) || dlEmpty(machList))
        break;
    machine = machList->head->val;
    if (now - machine->lastChecked < period)
        break;
    machine->lastChecked = now;
    mNode = dlPopHead(machList);
    dlAddTail(machList, mNode);
    sendViaSpoke(machine, message);
    }
}

void hangman(int spokesToUse, time_t now)
/* Check that busy nodes aren't dead.  Also send message for 
 * busy nodes to check in, in case we missed one of their earlier
 * jobDone messages. */
{
int i, period = jobCheckPeriod*60;
struct dlNode *mNode;
struct machine *machine;
struct job *job;

for (i=0; i<spokesToUse; ++i)
    {
    if (dlEmpty(freeSpokes) || dlEmpty(busyMachines))
        break;
    machine = busyMachines->head->val;
    if (now - machine->lastChecked < period)
        break;
    machine->lastChecked = now;
    mNode = dlPopHead(busyMachines);
    dlAddTail(busyMachines, mNode);
    job = machine->job;
    if (job != NULL)
        {
	char message[512];
	sprintf(message, "check %s %d", hubHost, job->id);
	sendViaSpoke(machine, message);
	}
    }
}

void graveDigger(int spokesToUse, time_t now)
/* Check out dead nodes.  Try and resurrect them periodically. */
{
checkPeriodically(deadMachines, 60 * machineCheckPeriod, "resurrect", 
	spokesToUse, now);
}

void straightenSpokes(time_t now)
/* Move spokes that have been busy too long back to
 * free spoke list under the assumption that we
 * missed a recycleSpoke message. */
{
struct dlNode *node, *next;
struct spoke *spoke;

for (node = busySpokes->head; !dlEnd(node); node = next)
    {
    next = node->next;
    spoke = node->val;
    if (now - spoke->lastPinged > 30)
	{
	if (spoke->pingCount < 10)
	    {
	    ++spoke->pingCount;
	    spoke->lastPinged = now;
	    spokePing(spoke);
	    }
	else
	    {
	    dlRemove(node);
	    dlAddTail(deadSpokes, node);
	    }
        }
    }
}

void writeJobResults(struct job *job, time_t now, char *status,
	char *uTime, char *sTime)
/* Write out job results to output queue.  This
 * will create the output queue if it doesn't yet
 * exist. */
{
struct resultQueue *rq;
for (rq = resultQueues; rq != NULL; rq = rq->next)
    if (sameString(job->results, rq->name))
        break;
if (rq == NULL)
    {
    AllocVar(rq);
    slAddHead(&resultQueues, rq);
    rq->name = cloneString(job->results);
    rq->f = fopen(rq->name, "a");
    rq->lastUsed = now;
    }
if (rq->f != NULL)
    {
    fprintf(rq->f, "%s %s %d %s %s %s %lu %lu %lu %s %s '%s'\n",
        status, job->machine->name, job->id, job->exe, 
	uTime, sTime, 
	job->submitTime, job->startTime, now,
	job->user, job->err, job->cmd);
    fflush(rq->f);
    if (sameString(status, "0"))
        ++finishedJobCount;
    else
        ++crashedJobCount;
    rq->lastUsed = now;
    }
}

void resultQueueFree(struct resultQueue **pRq)
/* Free up a results queue, closing file if open. */
{
struct resultQueue *rq = *pRq;
if (rq != NULL)
    {
    carefulClose(&rq->f);
    freeMem(rq->name);
    freez(pRq);
    }
}

void sweepResults(time_t now)
/* Get rid of result queues that haven't been accessed for
 * a while. */
{
struct resultQueue *newList = NULL, *rq, *next;
for (rq = resultQueues; rq != NULL; rq = next)
    {
    next = rq->next;
    if (now - rq->lastUsed > 5*60)
	{
	logIt("hub: closing results file %s\n", rq->name);
        resultQueueFree(&rq);
	}
    else
        {
	slAddHead(&newList, rq);
	}
    }
slReverse(&newList);
resultQueues = newList;
}

void processHeartbeat()
/* Check that system is ok.  See if we can do anything useful. */
{
int spokesToUse;
time_t now = time(NULL);
runner(50);
straightenSpokes(now);
spokesToUse = dlCount(freeSpokes);
if (spokesToUse > 0)
    {
    spokesToUse >>= 1;
    spokesToUse -= 1;
    if (spokesToUse < 1) spokesToUse = 1;
    graveDigger(spokesToUse, now);
    hangman(spokesToUse, now);
    sweepResults(now);
    }
}

void nodeAlive(char *name)
/* Deal with message from node that says it's alive.
 * Move it from dead to free list. */
{
struct machine *mach;
struct dlNode *node;
for (node = deadMachines->head; !dlEnd(node); node = node->next)
    {
    mach = node->val;
    if (sameString(mach->name, name))
        {
	dlRemove(node);
	dlAddTail(freeMachines, node);
	mach->isDead = FALSE;
	runner(1);
	break;
	}
    }
}

void recycleMachine(struct machine *mach)
/* Recycle machine into free list. */
{
mach->job = NULL;
dlRemove(mach->node);
dlAddTail(freeMachines, mach->node);
}

void recycleJob(struct job *job)
/* Remove job from lists and free up memory associated with it. */
{
dlRemove(job->node);
jobFree(&job);
}

void nodeCheckIn(char *line)
/* Deal with check in message from node. */
{
char *machine = nextWord(&line);
char *jobIdString = nextWord(&line);
char *status = nextWord(&line);
int jobId = atoi(jobIdString);
if (status != NULL && sameString(status, "free"))
    {
    struct job *job = jobFind(runningJobs, jobId);
    if (job != NULL)
         {
	 struct machine *mach = job->machine;
	 if (mach != NULL)
	     recycleMachine(mach);
	 recycleJob(job);
	 logIt("hub:  >>>RECYCLING MACHINE IN NODE CHECK IN<<<<\n");
	 }
    }
}

void sendOk(boolean ok, boolean connectionHandle)
/* Send ok (or error) to handle. */
{
netSendLongString(connectionHandle, (ok ? "ok" : "error"));
}

void recycleSpoke(char *spokeName, int connectionHandle)
/* Try to find spoke and put it back on free list. */
{
struct dlNode *node;
struct spoke *spoke;
boolean foundSpoke = FALSE;
for (node = busySpokes->head; !dlEnd(node); node = node->next)
    {
    spoke = node->val;
    if (sameString(spoke->socketName, spokeName))
        {
	dlRemove(spoke->node);
	freez(&spoke->machine);
	dlAddTail(freeSpokes, spoke->node);
	foundSpoke = TRUE;
	break;
	}
    }
sendOk(foundSpoke, connectionHandle);	/* Do we really need this handshake?
                                         * Other end in spoke.c would need 
					 * to be deleted too. */
if (!foundSpoke)
    warn("Couldn't free spoke %s", spokeName);
else
    runner(1);
}

int addJob(char *line)
/* Add job to queues. */
{
char *user, *dir, *in, *out, *results, *command;
struct job *job;

if ((user = nextWord(&line)) == NULL)
    return 0;
if ((dir = nextWord(&line)) == NULL)
    return 0;
if ((in = nextWord(&line)) == NULL)
    return 0;
if ((out = nextWord(&line)) == NULL)
    return 0;
if ((results = nextWord(&line)) == NULL)
    return 0;
if (line == NULL || line[0] == 0)
    return 0;
command = line;
job = jobNew(command, user, dir, in, out, results);
job->submitTime = time(NULL);
dlAddTail(pendingJobs, job->node);
return job->id;
}

void addJobAcknowledge(char *line, int connectionHandle)
/* Add job.  Line format is <user> <dir> <stdin> <stdout> <results> <command> 
 * Returns job ID or 0 if a problem.  Send jobId back to client. */
{
int id = addJob(line);
char jobIdString[16];
sprintf(jobIdString, "%d", id);
netSendLongString(connectionHandle, jobIdString);
runner(1);
}

void respondToPing(int connectionHandle)
/* Someone want's to know we're alive. */
{
netSendLongString(connectionHandle, "ok");
processHeartbeat();
}

void sendKillJobMessage(struct machine *machine, struct job *job)
/* Send message to compute node to kill job there. */
{
char message[64];
sprintf(message, "kill %d", job->id);
sendViaSpoke(machine, message);
     /* There's a race condition here that can cause jobs not
      * to be killed if there are no spokes free to handle
      * this message.  I'm trying to think of a simple solution
      * to this. The damage isn't too serious though,
      * since the jobs will at least stay on the main job list.
      * if they are not killed.  User can curse us and try to
      * kill jobs again.  At least the node won't end up 
      * double-booked.  */
}

void finishJob(struct job *job)
/* Recycle job memory and the machine it's running on. */
{
struct machine *mach = job->machine;
if (mach != NULL)
     recycleMachine(mach);
recycleJob(job);
runner(1);
}

void removeJob(struct job *job)
/* Remove job - if it's running kill it,  remove from job list. */
{
if (job->machine != NULL)
    sendKillJobMessage(job->machine, job);
finishJob(job);
}

void removeJobId(int id)
/* Remove job of a given id. */
{
struct job *job = jobFind(runningJobs, id);
if (job == NULL)
   job = jobFind(pendingJobs, id);
if (job != NULL)
    removeJob(job);
}

void removeJobName(char *name)
/* Remove job of a given name. */
{
name = trimSpaces(name);
removeJobId(atoi(name));
}

void jobDone(char *line)
/* Handle job is done message. */
{
struct job *job;
char *id, *status, *uTime, *sTime, *tTime;
id = nextWord(&line);
status = nextWord(&line);
uTime = nextWord(&line);
sTime = nextWord(&line);
if (sTime != NULL)
    {
    job = jobFind(runningJobs, atoi(id));
    if (job != NULL)
	{
	time_t now = time(NULL);
	job->machine->lastChecked = now;
	writeJobResults(job, now, status, uTime, sTime);
	finishJob(job);
	}
    }
}

void listMachines(int fd)
/* Write list of machines to fd.  Format is one machine per line
 * followed by a blank line. */
{
struct dyString *dy = newDyString(256);
struct machine *mach;
struct job *job;
for (mach = machineList; mach != NULL; mach = mach->next)
    {
    dyStringClear(dy);
    dyStringPrintf(dy, "%-10s ", mach->name);
    job = mach->job;
    if (job != NULL)
        dyStringPrintf(dy, "%-10s %s", job->user, job->cmd);
    else
	{
	if (mach->isDead)
	    dyStringPrintf(dy, "dead");
	else
	    dyStringPrintf(dy, "idle");
	}
    netSendLongString(fd, dy->string);
    }
netSendLongString(fd, "");
freeDyString(&dy);
}

void appendLocalTime(struct dyString *dy, time_t t)
/* Append time t converted to day/time format to dy. */
{
struct tm *tm;
tm = localtime(&t);
dyStringPrintf(dy, "%02d/%02d/%d %02d:%02d:%02d",
   tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

char *upToFirstDot(char *s, bool dotQ)
/* Return string up to first dot. */
{
static char ret[128];
int size;
char *e = strchr(s, '.');
if (e == NULL)
    size = strlen(s);
else
    size = e - s;
if (size >= sizeof(ret)-2)	/* Leave room for .q */
    size = sizeof(ret)-3;
memcpy(ret, s, size);
ret[size] = 0;
if (dotQ)
    strcat(ret, ".q");
return ret;
}

void oneJobList(int fd, struct dlList *list, struct dyString *dy, boolean sinceStart)
/* Write out one job list. */
{
struct dlNode *el;
struct job *job;
char *machName;
for (el = list->head; !dlEnd(el); el = el->next)
    {
    job = el->val;
    if (job->machine != NULL)
        machName = upToFirstDot(job->machine->name, FALSE);
    else
	machName = "none";
    dyStringClear(dy);
    dyStringPrintf(dy, "%-4d %-10s %-10s ", job->id, machName, job->user);
    if (sinceStart)
        appendLocalTime(dy, job->startTime);
    else
        appendLocalTime(dy, job->submitTime);
    dyStringPrintf(dy, " %s", job->cmd);
    netSendLongString(fd, dy->string);
    }
}

void listJobs(int fd)
/* Write list of jobs to fd. Format is one job per line
 * followed by a blank line. */
{
struct dyString *dy = newDyString(256);

oneJobList(fd, runningJobs, dy, TRUE);
oneJobList(fd, pendingJobs, dy, FALSE);
netSendLongString(fd, "");
freeDyString(&dy);
}

void oneQstatList(int fd, struct dlList *list, boolean running, struct dyString *dy)
/* Write out one job list in qstat format. */
{
struct dlNode *node;
struct job *job;
time_t t;
char *machName;
char *s;
char *state = (running ? "r" : "qw");

for (node = list->head; !dlEnd(node); node = node->next)
    {
    job = node->val;
    if (job->machine != NULL)
        machName = upToFirstDot(job->machine->name, TRUE);
    else
        machName = "none";
    if (running)
        t = job->startTime;
    else
        t = job->submitTime;
    dyStringClear(dy);
    dyStringPrintf(dy, "%-7d -100 %-10s %-12s %-5s ", job->id, job->exe, job->user, state);
    appendLocalTime(dy, t);
    if (running)
	dyStringPrintf(dy, " %-10s MASTER", machName);
    netSendLongString(fd, dy->string);
    }
}

void qstat(int fd)
/* Write list of jobs in qstat format. */
{
if (!dlEmpty(runningJobs) || !dlEmpty(pendingJobs))
    {
    struct dyString *dy = newDyString(256);
    netSendLongString(fd, "job-ID prior name       user         state submit/start at     queue      master");
    netSendLongString(fd, "--------------------------------------------------------------------------------");
    oneQstatList(fd, runningJobs, TRUE, dy);
    oneQstatList(fd, pendingJobs, FALSE, dy);
    freeDyString(&dy);
    }
netSendLongString(fd, "");
}

void onePstatList(int fd, struct dlList *list, boolean running, struct dyString *dy)
/* Write out one job list in qstat format. */
{
struct dlNode *node;
struct job *job;
time_t t;
char *machName;
char *s;
char *state = (running ? "r" : "q");

for (node = list->head; !dlEnd(node); node = node->next)
    {
    job = node->val;
    if (job->machine != NULL)
	machName = job->machine->name;
    else
        machName = "none";
    if (running)
        t = job->startTime;
    else
        t = job->submitTime;
    dyStringClear(dy);
    dyStringPrintf(dy, "%s %d %s %s %lu", 
        state, job->id, job->user, job->exe, t);
    netSendLongString(fd, dy->string);
    }
}

void pstat(int fd)
/* Write list of jobs in pstat format. */
{
struct dyString *dy = newDyString(0);
onePstatList(fd, runningJobs, TRUE, dy);
onePstatList(fd, pendingJobs, FALSE, dy);
netSendLongString(fd, "");
freeDyString(&dy);
}

void status(int fd)
/* Write summary status to fd.  Format is lines of text
 * followed by a blank line. */
{
char buf[256];
sprintf(buf, "CPUs free: %d", dlCount(freeMachines));
netSendLongString(fd, buf);
sprintf(buf, "CPUs busy: %d", dlCount(busyMachines));
netSendLongString(fd, buf);
sprintf(buf, "CPUs dead: %d", dlCount(deadMachines));
netSendLongString(fd, buf);
sprintf(buf, "Jobs running:  %d", dlCount(runningJobs));
netSendLongString(fd, buf);
sprintf(buf, "Jobs waiting:  %d", dlCount(pendingJobs));
netSendLongString(fd, buf);
sprintf(buf, "Jobs finished: %d", finishedJobCount);
netSendLongString(fd, buf);
sprintf(buf, "Jobs crashed:  %d", crashedJobCount);
netSendLongString(fd, buf);
sprintf(buf, "Spokes free: %d", dlCount(freeSpokes));
netSendLongString(fd, buf);
sprintf(buf, "Spokes busy: %d", dlCount(busySpokes));
netSendLongString(fd, buf);
sprintf(buf, "Spokes dead: %d", dlCount(deadSpokes));
netSendLongString(fd, buf);
netSendLongString(fd, "");
}

void addSpoke(int socketHandle, int connectionHandle)
/* Start up a new spoke and add it to free list. */
{
struct spoke *spoke;
static int closeList[4];
closeList[0] = connectionHandle;
closeList[1] = socketHandle;
closeList[2] = -1;
spoke = spokeNew(closeList);
if (spoke != NULL)
    {
    slAddHead(&spokeList, spoke);
    dlAddTail(freeSpokes, spoke->node);
    }
}

void killSpokes()
/* Kill all spokes. */
{
struct spoke *spoke, *next;
for (spoke = spokeList; spoke != NULL; spoke = next)
    {
    next = spoke->next;
    dlRemove(spoke->node);
    spokeFree(&spoke);
    }
}

int hubConnect()
/* Return connection to hub socket - with paraSig already written. */
{
int hubFd;
int sigSize = strlen(paraSig);
hubFd  = netConnect(hubHost, paraPort);
if (hubFd > 0)
    write(hubFd, paraSig, sigSize);
return hubFd;
}

void startSpokes()
/* Start default number of spokes. */
{
int i;
for (i=0; i<initialSpokes; ++i)
    addSpoke(-1, -1);
}

void startMachines(char *fileName)
/* If they give us a beginning machine list use it here. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];
while (lineFileRow(lf, row))
    {
    struct machSpec ms;
    int i;
    machSpecStaticLoad(row, &ms);
    for (i=0; i<ms.cpus; ++i)
	doAddMachine(ms.name, ms.tempDir);
    }
lineFileClose(&lf);
}

void startHub(char *machineList)
/* Do hub daemon - set up socket, and loop around on it until we get a quit. */
{
int socketHandle = 0, connectionHandle = 0;
char sig[20], *line, *command;
int sigLen = strlen(paraSig);
char *buf = NULL;

/* Find name and IP address of our machine. */
hubHost = getHost();
logIt("Starting paraHub on %s\n", hubHost);

/* Set up various lists. */
setupLists();
startMachines(machineList);
assert(sigLen < sizeof(sig));
sig[sigLen] = 0;

/* Set up socket.  Get ready to listen to it. */
socketHandle = netAcceptingSocket(paraPort, 100);
if (socketHandle < 0)
    errAbort("Can't set up socket.  Urk!  I'm dead.");

/* Move output to log file. */
setupDaemonLog(optionVal("log", NULL));

/* Do some initialization. */
startSpokes();
startHeartbeat();

/* Main event loop. */
for (;;)
    {
    int connectionHandle = netAccept(socketHandle);
    if (connectionHandle < 0)
        continue;
    if (!netMustReadAll(connectionHandle, sig, sigLen) || !sameString(sig, paraSig))
	{
	close(connectionHandle);
        continue;
	}
    line = buf = netGetLongString(connectionHandle);
    if (line == NULL)
        {
	close(connectionHandle);
        continue;
	}
    logIt("hub: %s\n", line);
    command = nextWord(&line);
    if (sameWord(command, "jobDone"))
         jobDone(line);
    else if (sameWord(command, "recycleSpoke"))
         recycleSpoke(line, connectionHandle);
    else if (sameWord(command, "heartbeat"))
         processHeartbeat();
    else if (sameWord(command, "addJob"))
         addJobAcknowledge(line, connectionHandle);
    else if (sameWord(command, "nodeDown"))
         nodeDown(line);
    else if (sameWord(command, "alive"))
         nodeAlive(line);
    else if (sameWord(command, "checkIn"))
         nodeCheckIn(line);
    else if (sameWord(command, "removeJob"))
         removeJobName(line);
    else if (sameWord(command, "ping"))
         respondToPing(connectionHandle);
    else if (sameWord(command, "addMachine"))
         addMachine(line);
    else if (sameWord(command, "removeMachine"))
         removeMachine(line);
    else if (sameWord(command, "listJobs"))
         listJobs(connectionHandle);
    else if (sameWord(command, "listMachines"))
         listMachines(connectionHandle);
    else if (sameWord(command, "status"))
         status(connectionHandle);
    else if (sameWord(command, "qstat"))
         qstat(connectionHandle);
    else if (sameWord(command, "pstat"))
         pstat(connectionHandle);
    else if (sameWord(command, "addSpoke"))
         addSpoke(socketHandle, connectionHandle);
    if (sameWord(command, "quit"))
         break;
    close(connectionHandle);
    freez(&buf);
    }
endHeartbeat();
killSpokes();
close(socketHandle);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc < 2)
    usage();
jobCheckPeriod = optionInt("jobCheckPeriod", jobCheckPeriod);
machineCheckPeriod = optionInt("machineCheckPeriod", machineCheckPeriod);
initialSpokes = optionInt("spokes",  initialSpokes);
startHub(argv[1]);
return 0;
}



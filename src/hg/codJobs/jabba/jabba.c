/* jabba - A program to launch,  monitor, and restart jobs via Codine. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "cheapcgi.h"
#include "jksql.h"
#include "jobDb.h"
#include "portable.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "jabba - A program to launch,  monitor, and restart jobs via Codine\n"
  "usage:\n"
  "   jabba command batch.hut [command-specific arguments]\n"
  "The commands are:\n"
  "jabba make batch.hut jobList\n"
  "   This makes the job-tracking database from a text file with the\n"
  "   command line for each job on a separate line\n"
  "jabba check batch.hut\n"
  "   This checks on the progress of the jobs.\n"
  "jabba push batch.hut\n"
  "   This pushes forward the batch of jobs by submitting jobs to codine\n"
  "   It will try and keep the codine queue a size that is efficient for\n"
  "   codine, and retry failed jobs\n"
  "   options:\n"
  "      -retries=N   Number of retries per job - default 3.\n"
  "      -maxQueue=N  Number of jobs to allow on codine queue - default 10000\n"
  "      -minPush=N  Minimum number of jobs to queue - default 1.  Overrides maxQueue\n"
  "      -maxPush=N  Maximum numer of jobs to queue - default 10000\n"
  "      -warnTime=N Number of minutes job can run before hang warning - default 4320 (3 days)\n"
  "      -killTime=N Number of minutes job can run before push kills it - default 20160 (2 weeks)\n"
  "jabba stop batch.hut\n"
  "   This stops all the jobs in the batch\n"
  );
}

/* Variables that can be set from command line. */

int retries = 3;
int maxQueue = 10000;
int minPush = 1;
int maxPush = 10000;
int warnTime = 3*24*60;
int killTime = 14*24*60;

/* Some variable we might want to move to a config file someday. */
char *tempName = "jabba.tmp";	/* Name for temp files. */
char *submitCommand = "/cluster/gridware/bin/glinux/qsub -cwd -o out -e err";
char *statusCommand = "/cluster/gridware/bin/glinux/qstat";
char *runJobCommand = "/cluster/bin/scripts/runJob";

/* Places that can be checked. */
char *checkWhens[] = {"in", "out"};

/* Types of checks. */
char *checkTypes[] = {"exists", "nonzero", "line", "line+"};

struct job *jobFromLine(struct lineFile *lf, char *line)
/* Parse out the beginnings of a job from input line. 
 * Parse out and keep checks. */
{
struct check *check;
char *pattern = "{check";
char *s, *e, *z;
struct dyString *dy = dyStringNew(1024);
struct job *job;

AllocVar(job);
s = line;
for (;;)
    {
    e = stringIn(pattern, s);
    if (e == NULL)
	{
	dyStringAppend(dy, s);
	break;
	}
    else
        {
	char *parts[5];
	int partCount;
	dyStringAppendN(dy, s, e-s);
	z = strchr(e, '}');
	if (z == NULL)
	    errAbort("{check without } line %d of %s", lf->lineIx, lf->fileName);
	*z = 0;
	partCount = chopLine(e, parts);
	if (partCount != 4)
	    errAbort("Badly formatted check line %d of %s", lf->lineIx, lf->fileName);
	AllocVar(check);
	slAddHead(&job->checkList, check);
	job->checkCount += 1;
	if (stringIx(parts[1], checkWhens) < 0)
	    errAbort("Unrecognized word '%s' in check line %d of %s", 
	    	parts[1], lf->lineIx, lf->fileName);
	check->when = cloneString(parts[1]);
	if (stringIx(parts[2], checkTypes) < 0)
	    errAbort("Unrecognized word '%s' in check line %d of %s", 
	    	parts[2], lf->lineIx, lf->fileName);
	check->what = cloneString(parts[2]);
	check->file = cloneString(parts[3]);
	dyStringAppend(dy, check->file);
	s = z+1;
	}
    }
job->command = cloneString(dy->string);
slReverse(&job->checkList);
dyStringFree(&dy);
return job;
}

struct fileStatus
/* Some info on a file. */
    {
    bool exists;	/* TRUE if file exists. */
    bool hasData;	/* TRUE if nonempty. */
    bool completeLastLine;	/* TRUE if last line ends with <lf> */
    bool reported;	/* TRUE if reported error. */
    };

struct fileStatus *getFileStatus(char *file)
/* Get information on file. */
{
struct fileStatus *fi;
FILE *f;

AllocVar(fi);
if ((f = fopen(file, "rb")) != NULL)
    {
    fi->exists = TRUE;
    if (fseek(f, -1, SEEK_END) == 0)
        {
	int c = fgetc(f);
	if (c >= 0)
	    {
	    fi->hasData = TRUE;
	    if (c == '\n')
	        fi->completeLastLine = TRUE;
	    }
	}
    fclose(f);
    }
return fi;
}

void doChecks(struct jobDb *db, char *when)
/* Do checks on files where check->when matches when. */
{
int errCount = 0;
struct job *job;
struct check *check;
struct hash *hash = newHash(0);
struct fileStatus *fi;

for (job = db->jobList; job != NULL; job = job->next)
    {
    for (check = job->checkList; check != NULL; check = check->next)
        {
	if (sameWord(when, check->when))
	    {
	    char *file = check->file;
	    char *what = check->what;
	    if ((fi = hashFindVal(hash, file)) == NULL)
	        {
		fi = getFileStatus(file);
		hashAdd(hash, file, fi);
		}
	    if (!fi->reported)
	        {
		if (!fi->exists)
		    {
		    warn("%s does not exist", file);
		    fi->reported = TRUE;
		    ++errCount;
		    continue;
		    }
		if (sameWord(what, "nonzero"))
		    {
		    if (!fi->hasData)
		        {
			warn("%s is empty", file);
			fi->reported = TRUE;
			++errCount;
			}
		    }
		else if (sameWord(what, "line"))
		    {
		    if (fi->hasData && !fi->completeLastLine)
		        {
			warn("%s has an incomplete last line", file);
			fi->reported = TRUE;
			++errCount;
			}
		    }
		else if (sameWord(what, "line+"))
		    {
		    if (!fi->hasData)
		        {
			warn("%s is empty", file);
			fi->reported = TRUE;
			++errCount;
			}
		    else if (!fi->completeLastLine)
		        {
			warn("%s has an incomplete last line", file);
			fi->reported = TRUE;
			++errCount;
			}
		    }
		else if (sameString(what, "exists"))
		    {
		    /* Check already made. */
		    }
		else
		    {
		    warn("Unknown check '%s'", what);
		    }
		}
	    }
	}
    }
if (errCount > 0)
    errAbort("%d total errors in file check", errCount);
freeHashAndVals(&hash);
}

void writeBatch(struct jobDb *db, char *fileName)
/* Write out batch file. */
{
FILE *f = mustOpen(fileName, "w");
struct job *job;
for (job = db->jobList; job != NULL; job = job->next)
    {
    jobCommaOut(job, f);
    fprintf(f, "\n");
    }
carefulClose(&f);
}

struct jobDb *readBatch(char *batch)
/* Read a batch file. */
{
struct jobDb *db;
struct job *job;
struct lineFile *lf = lineFileOpen(batch, TRUE);
char *line;

AllocVar(db);
while (lineFileNext(lf, &line, NULL))
    {
    line = skipLeadingSpaces(line);
    if (line[0] == '#' || line[0] == 0)
       continue;
    job = jobCommaIn(&line, NULL);
    slAddHead(&db->jobList, job);
    ++db->jobCount;
    }
lineFileClose(&lf);
slReverse(&db->jobList);
return db;
}


void jabbaMake(char *batch, char *jobList)
/* Make a batch database from a job list. */
{
struct lineFile *lf = lineFileOpen(jobList, TRUE);
char *line;
int jobCount = 0;
struct jobDb *db;
struct job *job;

AllocVar(db);
while (lineFileNext(lf, &line, NULL))
    {
    line = trimSpaces(line);
    if (line[0] == '#')
        continue;
    ++db->jobCount;
    job = jobFromLine(lf, line);
    slAddHead(&db->jobList, job);
    }
lineFileClose(&lf);
slReverse(&db->jobList);

doChecks(db, "in");
writeBatch(db, batch);
printf("%d jobs written to %s\n", db->jobCount, batch);
}

void fillInSub(char *fileName, struct submission *sub)
/* Fill in submission from output file produced by qsub. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line, *words[8];
int wordCount;
char buf[256];

if (!lineFileNext(lf, &line, NULL))
    errAbort("Empty qsub output, sorry can't cope.");
wordCount = chopLine(line, words);
if (wordCount < 3 || !sameString("your", words[0]) || !isdigit(words[2][0]))
    errAbort("qsub output seems to have changed, you'll have to update fillInSub");
sub->id = cloneString(words[2]);
sprintf(buf, "out/runJob.o%s", sub->id);
sub->outFile = cloneString(buf);
sprintf(buf, "err/runJob.e%s", sub->id);
sub->errFile = cloneString(buf);
lineFileClose(&lf);
}

void submitJob(struct job *job)
/* Attempt to submit job. */
{
struct dyString *cmd = dyStringNew(1024);
struct submission *sub;
int err;

dyStringAppend(cmd, submitCommand);
dyStringAppend(cmd, " ");
dyStringAppend(cmd, runJobCommand);
dyStringAppend(cmd, " ");
dyStringAppend(cmd, job->command);
dyStringPrintf(cmd, " > %s", tempName);

err = system(cmd->string);
AllocVar(sub);
slAddHead(&job->submissionList, sub);
job->submissionCount += 1;

if (err != 0)
    {
    sub->submitError = TRUE;
    sub->id = cloneString("n/a");
    sub->errFile = cloneString("n/a");
    sub->outFile = cloneString("n/a");
    }
else
    {
    fillInSub(tempName, sub);
    }
dyStringFree(&cmd);
}

void statusOutputChanged()
/* Complain about status output format change and die. */
{
errAbort("%s output format changed, please update markQueuedJobs in jabba.c", 
	statusCommand);
}

int markQueuedJobs(struct jobDb *db)
/* Mark jobs that are queued up. Return total number of jobs in queue. */
{
struct dyString *cmd = dyStringNew(1024);
int err;
struct lineFile *lf;
struct hash *hash = newHash(0);
struct job *job;
struct submission *sub;
char *line, *words[10];
int wordCount;
int queueSize = 0;

/* Execute qstat system call. */
dyStringAppend(cmd, statusCommand);
dyStringPrintf(cmd, " > %s", tempName);
err = system(cmd->string);
if (err != 0)
    errAbort("Couldn't execute '%s'", cmd->string);

/* Make hash of submissions based on id and clear flags. */
for (job = db->jobList; job != NULL; job = job->next)
    {
    for (sub = job->submissionList; sub != NULL; sub = sub->next)
        {
	hashAdd(hash, sub->id, sub);
	sub->running = FALSE;
	sub->inQueue = FALSE;
	}
    }

/* Read status output. */
lf = lineFileOpen(tempName, TRUE);
if (lineFileNext(lf, &line, NULL))	/* Empty is ok. */
    {
    if (!startsWith("job-ID", line))
	statusOutputChanged();
    if (!lineFileNext(lf, &line, NULL) || !startsWith("-----", line))
	statusOutputChanged();
    while (lineFileNext(lf, &line, NULL))
        {
	wordCount = chopLine(line, words);
	if (wordCount < 7)
	    statusOutputChanged();
	++queueSize;
	if ((sub = hashFindVal(hash, words[0])) != NULL)
	    {
	    char *state = words[4];
	    if (state[0] == 'E')
		sub->queueError = TRUE;
	    else 
	        {
		if (sameString(state, "r") || sameString(state, "t"))
		    {
		    sub->running = TRUE;
		    }
		else
		    {
		    sub->inQueue = TRUE;
		    }
		}
	    }
	}
    }
lineFileClose(&lf);
freeHash(&hash);
dyStringFree(&cmd);
return queueSize;
}

long dateToSeconds(char *date)
/* Convert from format like:
 *   'Wed Nov 7 13:35:11 PST 2001' to seconds since Jan. 1 2001.
 * This should be in a library somewhere, but I can't find it. 
 * This function is not totally perfect.  It'll add a leap year in 2200
 * when it shouldn't for instance. */
{
char *dupe = cloneString(skipLeadingSpaces(date));
char *words[8], *parts[4];
static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static int daysInMonths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int wordCount;
int leapDiv = 4;
int x;
int leapCount;
long secondsInDay = 24*60*60;
int year, month, day, hour, minute, second;
long dayCount;
long result;

/* Parse string into various integer variables. */
wordCount = chopLine(dupe, words);
if (wordCount < 6)
    errAbort("Badly formated date '%s'", date);
year = atoi(words[5]);
if ((month = stringIx(words[1], months)) < 0)
    errAbort("Unrecognized month '%s'", date);
day = atoi(words[2]);
wordCount = chopString(words[3], ":", parts, ArraySize(parts));
if (wordCount != 3)
    errAbort("Badly formated time in '%s'", date);
hour = atoi(parts[0]);
minute = atoi(parts[1]);
second = atoi(parts[2]);
freez(&dupe);

/* Figure out elapsed days with leap-years. */
x = year - 1 - 2000;	/* 1972 is nearest leap year. */
leapCount = x/4 + 1;
dayCount = (year - 2001) * 365 + leapCount;
for (x=0; x<month; ++x)
    dayCount += daysInMonths[x];
if (year%4 == 0 && month >= 2)
    ++dayCount;
result = secondsInDay*dayCount + hour*3600 + minute*60 + second;
return result;
}

long nowInSeconds()
/* Return current date in above format. */
{
time_t timer;
timer = time(NULL);
return dateToSeconds(ctime(&timer));
}

void parseRunJobOutput(char *fileName, int *retStartTime, int *retEndTime, float *retCpuTime,
	int *retRet, boolean *gotRet, boolean *retTrackingError)
/* Parse a run job output file.  Might have trouble if the program output
 * is horribly complex. */
{
struct lineFile *lf;
char *line, *words[20], *s;
int wordCount;
char *startPattern = "Start time: ";
char *endPattern = "Finish time: ";
char *returnPattern = "Return value = ";
boolean gotStart = FALSE, gotEnd = FALSE;
boolean gotCpu = FALSE, gotReturn = FALSE;

/* Set up default return values. */
*retCpuTime = *retStartTime = *retEndTime = *retRet = *gotRet = *retTrackingError = 0;

lf = lineFileMayOpen(fileName, TRUE);
if (lf == NULL)
    {
    *retTrackingError = TRUE;
    return;
    }
while (lineFileNext(lf, &line, NULL))
    {
    if (startsWith(startPattern, line))
        {
	line += strlen(startPattern);
	*retStartTime = dateToSeconds(line);
	gotStart = TRUE;
	}
    else if (startsWith(endPattern, line))
	{
	line += strlen(endPattern);
	*retEndTime = dateToSeconds(line);
	gotEnd = TRUE;
	break;
	}
    else if (isdigit(line[0]) )
	{
	wordCount = chopLine(line, words);
	if (wordCount >= 3 && lastChar(words[0]) == 'u'
	    && lastChar(words[1]) == 's' && isdigit(words[1][0]))
	    {
	    *retCpuTime = atof(words[0]) + atof(words[1]);
	    gotCpu = TRUE;
	    }
	}
    else if (startsWith(returnPattern, line))
	{
	line += strlen(returnPattern);
	line = skipLeadingSpaces(line);
	*retRet = atoi(line);
	*gotRet = TRUE;
	gotReturn = TRUE;
	}
    }
if (!gotStart)
    {
    *retTrackingError = TRUE;
    }
if (gotEnd)
    {
    if (!gotCpu || !gotReturn)
       errAbort("%s is not in a runJob format jabba can parse", fileName);
    }
lineFileClose(&lf);
}

void markRunJobStatus(struct jobDb *db)
/* Mark jobs based on runJob output file. */
{
struct job *job;
struct submission *sub;
char *line, *words[10];
int wordCount;
boolean gotRet, trackingError;
long killSeconds = killTime*60;
long warnSeconds = warnTime*60;
long duration;

for (job=db->jobList; job != NULL; job = job->next)
    {
    if ((sub = job->submissionList) != NULL)
        {
	/* Look for hitherto unclassified jobs that are either running or
	 * possibly finished. */
	if (!sub->queueError && !sub->inQueue && !sub->crashed && !sub->ranOk)
	    {
	    parseRunJobOutput(sub->outFile, &sub->startTime, &sub->endTime, 
	        &sub->cpuTime, &sub->retVal, &gotRet, &trackingError);
	    sub->gotRetVal = gotRet;
	    sub->trackingError = trackingError;
	    if (gotRet)
	        {
		if (sub->retVal == 0)	/* Put checks on output here. */
		    sub->ranOk = TRUE;
		else
		    sub->crashed = TRUE;
		}
	    else
	        {
		if (!sub->trackingError)
		    {
		    duration = nowInSeconds() - sub->startTime;
		    if (duration >= killSeconds)
		        sub->hung = TRUE;
		    else if (duration >= warnSeconds)
		        sub->slow = TRUE;
		    }
		}
	    }
	}
    }
}

void reportOnJobs(struct jobDb *db)
/* Report on status of jobs. */
{
int submitError = 0, inQueue = 0, queueError = 0, trackingError = 0, running = 0, crashed = 0,
    slow = 0, hung = 0, ranOk = 0, jobCount = 0, unsubmitted = 0, total = 0;
struct job *job;
struct submission *sub;

for (job = db->jobList; job != NULL; job = job->next)
    {
    if ((sub = job->submissionList) != NULL)	/* Get most recent submission if any. */
        {
	if (sub->submitError) ++submitError;
	if (sub->queueError) ++queueError;
	if (sub->trackingError) ++trackingError;
	if (sub->inQueue) ++inQueue;
	if (sub->crashed) ++crashed;
	if (sub->slow) ++slow;
	if (sub->hung) ++hung;
	if (sub->running) ++running;
	if (sub->ranOk) ++ranOk;
	}
    else
        ++unsubmitted;
    ++total;
    }
if (unsubmitted > 0)
   printf("unsubmitted jobs: %d\n", unsubmitted);
if (submitError > 0)
   printf("submission errors: %d\n", submitError);
if (queueError > 0)
   printf("queue errors: %d\n", queueError);
if (trackingError > 0)
   printf("tracking errors: %d\n", trackingError);
if (inQueue > 0)
   printf("in queue: %d\n", inQueue);
if (crashed > 0)
   printf("crashed: %d\n", crashed);
if (slow > 0)
   printf("slow (> %d minutes): %d\n", warnTime, slow);
if (hung > 0)
   printf("hung (> %d minutes): %d\n", killTime, hung);
if (running > 0)
   printf("running: %d\n", running);
if (ranOk > 0)
   printf("ranOk: %d\n", ranOk);
printf("total jobs in batch: %d\n", total);
}

void jabbaPush(char *batch)
/* Push a batch of jobs forward - submit jobs. */
{
struct jobDb *db = readBatch(batch);
struct job *job;
int queueSize;

makeDir("err");
makeDir("out");
if ((job = db->jobList) != NULL)
    submitJob(job);    

queueSize = markQueuedJobs(db);
printf("jobs (everybody's) in Codine queue: %d\n", queueSize);
markRunJobStatus(db);
reportOnJobs(db);

writeBatch(db, batch);
}

void jabbaCheck(char *batch)
/* Check on progress of a batch. */
{
struct jobDb *db = readBatch(batch);
int queueSize;

queueSize = markQueuedJobs(db);
printf("jobs (everybody's) in Codine queue: %d\n", queueSize);
markRunJobStatus(db);
reportOnJobs(db);

writeBatch(db, batch);
}

void jabbaStop(char *batch)
/* Stop batch of jobs. */
{
}

int main(int argc, char *argv[])
/* Process command line. */
{
char *command;
char *batch;

cgiSpoof(&argc, argv);
if (argc < 3)
    usage();
retries = cgiUsualInt("retries",  retries);
maxQueue = cgiUsualInt("maxQueue",  maxQueue);
minPush = cgiUsualInt("minPush",  minPush);
maxPush = cgiUsualInt("maxPush",  maxPush);
command = argv[1];
batch = argv[2];
if (sameString(command, "make"))
    {
    if (argc != 4)
        usage();
    jabbaMake(batch, argv[3]);
    }
else if (sameString(command, "check"))
    {
    jabbaCheck(batch);
    }
else if (sameString(command, "push"))
    {
    jabbaPush(batch);
    }
else if (sameString(command, "stop"))
    {
    jabbaStop(batch);
    }
else
    {
    errAbort("Unrecognized command '%s'.  Run jabba with no arguments for usage summary", 
    	command);
    }
return 0;
}

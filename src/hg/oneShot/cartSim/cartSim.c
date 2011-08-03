/* cartSim - Simulate cart usage by genome browser users.. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"
#include "portable.h"
#include "hgRelate.h"

static char const rcsid[] = "$Id: newProg.c,v 1.30 2010/03/24 21:18:33 hiram Exp $";

int userCount = 10;
int randSeed = 0;
int cgiDelay = 100;
int hitDelay = 100;
int iterations = 100;
double newRatio = 0.1;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "cartSim - Simulate cart usage by genome browser users.\n"
  "usage:\n"
  "   cartSim host user password database\n"
  "options:\n"
  "   -userCount=N number of users simulating.  Default %d\n"
  "   -randSeed=N random number generator seed.  Defaults to pid\n"
  "   -cgiDelay=N number of milliseconds to pretend cgi is crunching. Default is %d\n"
  "   -hitDelay=N number of milliseconds to delay between hits. Default is %d\n"
  "   -iterations=N number of iterations to hit cart.  Default is %d\n"
  "   -newRatio=0.N proportion of hits that are from new users. Default is %g\n"
  "   -verbose=N level of diagnostic output verbosity.  Default is 1.\n"
  "   -create=N create database if it doesn't exist with given number of dummy records\n"
  "   -clone=source clone existing database\n"
  , userCount, cgiDelay, hitDelay, iterations, newRatio
  );
}

static struct optionSpec options[] = {
   {"userCount", OPTION_INT},
   {"randSeed", OPTION_INT},
   {"cgiDelay", OPTION_INT},
   {"hitDelay", OPTION_INT},
   {"iterations", OPTION_INT},
   {"create", OPTION_INT},
   {"clone", OPTION_STRING},
   {"newRatio", OPTION_DOUBLE},
   {NULL, 0},
};

char *prefixes[] = {"pre", "post", "ex", ""};
char *vars[] = {"avocado", "bean", "almond", "peach", "pea", "oat", "artichoke", "lettuce",
   "apple", "beet", "pumpkin", "potato", "strawberry", "raspberry", "sage", "wheat"};
char *vals[] = { "owl", "hawk", "dove", "robin", "bluebird", "raven", "crow", "sparrow",
    "swallow", "falcon", "pigeon", "finch", "thrush", "woodpecker", "starling", };


#define fakePrefix "FAKE "
#define cartNumFields 6

struct dyString *fakeCart(int size)
/* Return a this=that string with so many elements */
{
struct dyString *dy = dyStringNew(0);
int i;
dyStringAppend(dy, fakePrefix);
for (i=0; i<size; ++i)
    {
    if (i != 0)
       dyStringAppendC(dy, '&');
    dyStringPrintf(dy, "%s%s=%s%d",
    	prefixes[rand()%ArraySize(prefixes)],
	vars[rand()%ArraySize(vars)],
	vals[rand()%ArraySize(vals)],
	rand()%10);
    }
return dy;
}

#define userTable "userDb"
#define sessionTable "sessionDb"

void checkNotRealCartTable(struct sqlConnection *conn, char *database, char *table)
/* Abort unless either table doesn't exist, or table exists and is in fake prefix format. */
{
if (!sqlTableExists(conn, table))
    return;
int contentsFieldIx = sqlFieldIndex(conn, table, "contents");
if (contentsFieldIx < 0)
    errAbort("%s.%s doesn't have a contents field", database, table);
char query[256];
safef(query, sizeof(query), "select contents from %s limit 1", table);
char *firstContents = sqlQuickString(conn, query);
if (firstContents != NULL && !startsWith(fakePrefix, firstContents))
    errAbort("%s.%s.contents doesn't start with '%s'", database, table, firstContents);
freez(&firstContents);
}

void checkFakeCartTable(struct sqlConnection *conn, char *database, char *table)
/* Abort unless either table doesn't exist, or table exists and is in fake prefix format. */
{
if (!sqlTableExists(conn, table))
    errAbort("no good test database %s: table %s doesn't exist", 
       database, table);
int contentsFieldIx = sqlFieldIndex(conn, table, "contents");
if (contentsFieldIx < 0)
    errAbort("%s.%s doesn't have a contents field", database, table);
char query[256];
safef(query, sizeof(query), "select contents from %s limit 1", table);
char *firstContents = sqlQuickString(conn, query);
if (firstContents == NULL)
    errAbort("no good test database %s: table %s is empty", 
       database, table);
if (!startsWith(fakePrefix, firstContents))
    errAbort("no goot test database %s: %s.contents doesn't start with '%s'", 
    	database, table, firstContents);
freez(&firstContents);
}

void checkNotRealDatabase(char *host, char *user, char *password, char *database)
/* Make sure that database does not contain real looking user and session tables. */
{
struct sqlConnection *conn = sqlMayConnectRemote(host, user, password, database);
if (conn != NULL)
    {
    checkNotRealCartTable(conn, database, userTable);
    checkNotRealCartTable(conn, database, sessionTable);
    sqlDisconnect(&conn);
    }
}

void checkEmptyOrFakeDatabase(char *host, char *user, char *password, char *database)
/* Make sure that either database doesn't exist, or that it does exist and
 * has fake tables. */
{
struct sqlConnection *conn = sqlMayConnectRemote(host, user, password, database);
if (conn != NULL)
    {
    checkFakeCartTable(conn, database, userTable);
    checkFakeCartTable(conn, database, sessionTable);
    sqlDisconnect(&conn);
    }
}

boolean databaseExists(char *host, char *user, char *password, char *database)
/* Return TRUE if database exists. */
{
struct sqlConnection *conn = sqlMayConnectRemote(host, user, password, database);
if (conn == NULL)
    return FALSE;
sqlDisconnect(&conn);
return TRUE;
}

void createEmptyDatabase(char *host, char *user, char *password, char *database)
/* Create a new database with no tables. */
{
struct sqlConnection *conn = sqlConnectRemote(host, user, password, NULL);
char query[512];
safef(query, sizeof(query), "create database %s", database);
sqlUpdate(conn, query);
sqlDisconnect(&conn);
}

void dropDatabase(char *host, char *user, char *password, char *database)
/* Drop database if it exists. */
{
if (databaseExists(host, user, password, database))
    {
    struct sqlConnection *conn = sqlConnectRemote(host, user, password, NULL);
    char query[512];
    safef(query, sizeof(query), "drop database %s", database);
    sqlUpdate(conn, query);
    sqlDisconnect(&conn);
    }
}

void createNewFakeDatabase(char *host, char *user, char *password, char *database)
/* Create a fake database with two empty fake tables. */
{
dropDatabase(host, user, password, database);
createEmptyDatabase(host, user, password, database);
struct sqlConnection *conn = sqlConnectRemote(host, user, password, database);
sqlUpdate(conn, 
"CREATE TABLE userDb (\n"
"    id integer unsigned not null auto_increment,	# Cart ID\n"
"    contents longblob not null,	# Contents - encoded variables\n"
"    reserved tinyint not null,	# True if a user (rather than session) cart\n"
"    firstUse DATETIME not null,	# First time this was used\n"
"    lastUse DATETIME not null,	# Last time this was used\n"
"    useCount int not null,	# Number of times used\n"
"              #Indices\n"
"    PRIMARY KEY(id)\n"
")\n" );
sqlUpdate(conn, 
"CREATE TABLE sessionDb (\n"
"    id integer unsigned not null auto_increment,	# Cart ID\n"
"    contents longblob not null,	# Contents - encoded variables\n"
"    reserved tinyint not null,	# True if a user (rather than session) cart\n"
"    firstUse DATETIME not null,	# First time this was used\n"
"    lastUse DATETIME not null,	# Last time this was used\n"
"    useCount int not null,	# Number of times used\n"
"              #Indices\n"
"    PRIMARY KEY(id)\n"
")\n");
sqlDisconnect(&conn);
}


void createFakeEntry(struct sqlConnection *conn, int size)
/* Create a fake entry of the given size. */
{
struct dyString *contents = fakeCart(size);
struct dyString *query = dyStringNew(0);
dyStringPrintf(query, "INSERT %s VALUES(0,'%s',0,now(),now(),0)", userTable, contents->string);
sqlUpdate(conn, query->string);
dyStringClear(query);
dyStringPrintf(query, "INSERT %s VALUES(0,'%s',0,now(),now(),0)", sessionTable, contents->string);
sqlUpdate(conn, query->string);
dyStringFree(&query);
dyStringFree(&contents);
}

int randomFakeSize()
/* Return a fake size of cart entry */
{
if (rand()%3 == 0)
    {
    double a = 0.0001 * (rand()%10000 + 1);
    return (int)(a*a*a*a*a*a*a*10000) + 10;
    }
else
    return rand()%20 + 1;
}

void createFakeEntries(char *host, char *user, char *password, char *database, int count)
/* create count new fake entries in database. */
{
struct sqlConnection *conn = sqlConnectRemote(host, user, password, database);
int i;
for (i=0; i<count; ++i)
    createFakeEntry(conn, randomFakeSize());
sqlDisconnect(&conn);
}

void fakeCloneOldTable(struct sqlConnection *oldConn, struct sqlConnection *newConn, char *table)
/* Clone sessionDb or userDb table in newConn from oldConn. Add fake prefix to
 * contents field to help mark it as fake. */
{
char query[256];
safef(query, sizeof(query), "select * from %s", table);
struct sqlResult *sr = sqlGetResult(oldConn, query);
char **row;
FILE *f = hgCreateTabFile(NULL, table);
while ((row = sqlNextRow(sr)) != NULL)
    {
    int i;
    for (i=0; i<cartNumFields; ++i)
        {
	if (i != 0)
	    fprintf(f, "\t");
	if (i == 1)
	    fprintf(f, "%s", fakePrefix);
	fprintf(f, "%s", row[i]);
	}
    fprintf(f, "\n");
    }
hgLoadTabFile(newConn, NULL, table, &f);
hgUnlinkTabFile(NULL, table);
}

void cloneOldDatabase(char *host, char *user, char *password, char *newDatabase, char *oldDatabase)
/* Write out old database and read in new one. */
{
struct sqlConnection *oldConn = sqlConnectRemote(host, user, password, oldDatabase);
struct sqlConnection *newConn = sqlConnectRemote(host, user, password, newDatabase);
fakeCloneOldTable(oldConn, newConn, userTable);
fakeCloneOldTable(oldConn, newConn, sessionTable);
}

int *getSomeInts(struct sqlConnection *conn, char *table, char *field, int limit)
/* Return an array of ints from field in table. */
{
int *result, i;
char query[512];
safef(query, sizeof(query), "select %s from %s limit %d", field, table, limit);
struct sqlResult *sr = sqlGetResult(conn, query);
AllocArray(result, limit);
for (i=0; i<limit; ++i)
    {
    char **row = sqlNextRow(sr);
    if (row == NULL)
        errAbort("Less than %d rows in %s", limit, table);
    result[i] = sqlSigned(row[0]);
    }
sqlFreeResult(&sr);
return result;
}

void updateOne(struct sqlConnection *conn, char *table, char *contents, int id, int useCount)
/* Update one of cart tables with new contents. */
{
struct dyString *dy = dyStringNew(0);
dyStringPrintf(dy, "UPDATE %s SET contents='", table);
dyStringAppend(dy, contents);
dyStringPrintf(dy, "',lastUse=now(),useCount=%d ", useCount+1);
dyStringPrintf(dy, " where id=%u", id);
sqlUpdate(conn, dy->string);
dyStringFree(&dy);
}

int dummyQuery(struct sqlConnection *conn, char *table, int id, char **retContents)
/* Ask database for useCount and contents. Just return useCount, fill in *retContents  */
{
char *contents = "";
char query[256];
safef(query, sizeof(query), "select useCount,contents from sessionDb where id=%d", id);
struct sqlResult *sr = sqlGetResult(conn, query);
char **row = sqlNextRow(sr);
int useCount = 0;
if (row != NULL)
    {
    contents = row[1];
    useCount = sqlUnsigned(row[0]);
    }
*retContents = cloneString(contents);
sqlFreeResult(&sr);
return useCount;
}

int dummyInsert(struct sqlConnection *conn, char *table)
/* Insert new row into cart table and return ID */
{
char query[256];
safef(query, sizeof(query), "INSERT %s VALUES(0,\"\",0,now(),now(),0)",
      table);
sqlUpdate(conn, query);
return sqlLastAutoId(conn);
}

boolean randomBitFromProb(double prob)
/* Where prob is between 0 and 1,  return TRUE with at given probability,
 * FALSE otherwise. */
{
return (rand() % 1000000 <= prob*1000000);
}

void cartSimulate(char *host, char *user, char *password, char *database)
/* Simulate action of various UCSC Genome Browser CGIs on cart. */
{
/* Figure out size of tables. */
struct sqlConnection *conn = sqlConnectRemote(host, user, password, database);
int userDbSize = sqlQuickNum(conn, "select count(*) from userDb");
int sessionDbSize = sqlQuickNum(conn, "select count(*) from sessionDb");
int sampleSize = min(userDbSize, sessionDbSize);
int maxSampleSize = 1024*1024;
sampleSize = min(sampleSize, maxSampleSize);
verbose(2, "# userDb has %d rows,  sessionDb has %d rows, sampling %d\n"
	, userDbSize, sessionDbSize, sampleSize);

/* Get sample of user id's. */
int *userIds = getSomeInts(conn, "userDb", "id", sampleSize);
int *sessionIds = getSomeInts(conn, "sessionDb", "id", sampleSize);

/* Get userCount random indexes. */
int *randomIxArray, ix;
AllocArray(randomIxArray, userCount);
verbose(2, "random user ix:\n");
for (ix=0; ix<userCount; ++ix)
    {
    randomIxArray[ix] = rand() % sampleSize;
    verbose(2, "%d ", randomIxArray[ix]);
    }
verbose(2, "\n");

sqlDisconnect(&conn);

int iteration = 0;
for (;;)
    {
    for (ix = 0; ix < userCount; ++ix)
	{
	int randomIx = rand()%sampleSize;
	boolean doNew = randomBitFromProb(newRatio);
	long startTime = clock1000();
	struct sqlConnection *conn = sqlConnectRemote(host, user, password, database);
	long connectTime = clock1000();
	struct dyString *contents = fakeCart(randomFakeSize());

	char *userContents = NULL;
	int userId = userIds[randomIx];
	if (doNew)
	    userId = userIds[randomIx] = dummyInsert(conn, userTable);
	int userUseCount = dummyQuery(conn, userTable, userId, &userContents);
	long userReadTime = clock1000();

	char *sessionContents = NULL;
	int sessionId = sessionIds[randomIx];
	if (doNew)
	    sessionId = sessionIds[randomIx] = dummyInsert(conn, sessionTable);
	int sessionUseCount = dummyQuery(conn, sessionTable, sessionId, &sessionContents);
	long sessionReadTime = clock1000();

	updateOne(conn, userTable, contents->string, userId, userUseCount);
	long userWriteTime = clock1000();

	updateOne(conn, sessionTable, contents->string, sessionId, sessionUseCount);
	long sessionWriteTime = clock1000();

	sqlDisconnect(&conn);
	long disconnectTime = clock1000();

	printf("%ld total, %ld oldSize, %ld newSize, %ld connect, %ld userRead, %ld sessionRead, %ld userWrite, %ld sessionWrite\n",
		disconnectTime - startTime,
		(long) strlen(userContents) + strlen(sessionContents),
		(long)contents->stringSize,
		connectTime - startTime,
		userReadTime - connectTime,
		sessionReadTime - userReadTime,
		userWriteTime - sessionReadTime,
		sessionWriteTime - userReadTime);

	dyStringFree(&contents);
	freez(&sessionContents);
	freez(&userContents);

	sleep1000(hitDelay);
	if (++iteration >= iterations)
	    return;
	}
    }

errAbort("cartSimulate(%s %s %s %s) not implemented", host, user, password, database);
}

void cartSim(char *host, char *user, char *password, char *database)
/* cartSim - Simulate cart usage by genome browser users. */
{
char *create = optionVal("create", NULL);
char *clone = optionVal("clone", NULL);
if (create != NULL)
    {
    int createSize = sqlUnsigned(create);
    checkNotRealDatabase(host, user, password, database);
    checkEmptyOrFakeDatabase(host, user, password, database);
    createNewFakeDatabase(host, user, password, database);
    createFakeEntries(host, user, password, database, createSize);
    }
else if (clone != NULL)
    {
    checkNotRealDatabase(host, user, password, database);
    checkEmptyOrFakeDatabase(host, user, password, database);
    createNewFakeDatabase(host, user, password, database);
    cloneOldDatabase(host, user, password, database, clone);
    }
else
    {
    cartSimulate(host, user, password, database);
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();
userCount = optionInt("userCount", userCount);
randSeed = optionInt("randSeed", getpid());
srand(randSeed);
cgiDelay = optionInt("cgiDelay", cgiDelay);
hitDelay = optionInt("hitDelay", hitDelay);
iterations = optionInt("iterations", iterations);
newRatio = optionDouble("newRatio", newRatio);
cartSim(argv[1], argv[2], argv[3], argv[4]);
return 0;
}

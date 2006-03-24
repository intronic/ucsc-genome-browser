/* apacheMonitor -- check for error 500 */
#include "common.h"
#include "hdb.h"
#include "hgConfig.h"

char *host = NULL;
char *user = NULL;
char *password = NULL;
char *database = NULL;
int minutes = 0;

int total = 0;

int hgw1count = 0;
int hgw2count = 0;
int hgw3count = 0;
int hgw4count = 0;
int hgw5count = 0;
int hgw6count = 0;
int hgw7count = 0;
int hgw8count = 0;
int mgccount = 0;

int robotcount = 0;

int status200 = 0; 
int status206 = 0; 
int status301 = 0;
int status302 = 0;
int status304 = 0;
int status400 = 0;
int status403 = 0;
int status404 = 0;
int status405 = 0;
int status408 = 0;
int status414 = 0;
int status500 = 0;


void usage()
/* Explain usage and exit. */
{
errAbort(
    "apacheMonitor - check for error 500s in the last minutes\n"
    "usage:\n"
    "    apacheMonitor host database minutes\n");
}

static char* getCfgValue(char* envName, char* cfgName)
/* get a configuration value, from either the environment or the cfg file,
 * with the env take precedence.
*/
{
char *val = getenv(envName);
if (val == NULL)
    val = cfgOption(cfgName);
    return val;
}    

void logStatus(int status)
{
switch (status)
    {
    case 200: 
        status200++;
        break;
    case 206: 
        status206++;
        break;
    case 301: 
        status301++;
        break;
    case 302: 
        status302++;
        break;
    case 304: 
        status304++;
        break;
    case 400: 
        status400++;
        break;
    case 403: 
        status403++;
        break;
    case 404: 
        status404++;
        break;
    case 405: 
        status405++;
        break;
    case 408: 
        status408++;
        break;
    case 414: 
        status414++;
        break;
    case 500: 
        status500++;
        break;
    }
}

void logMachine(char *machine)
{
if (sameString(machine, "hgw1"))
    {
    hgw1count++;
    return;
    }
if (sameString(machine, "hgw2"))
    {
    hgw2count++;
    return;
    }
if (sameString(machine, "hgw3"))
    {
    hgw3count++;
    return;
    }
if (sameString(machine, "hgw4"))
    {
    hgw4count++;
    return;
    }
if (sameString(machine, "hgw5"))
    {
    hgw5count++;
    return;
    }
if (sameString(machine, "hgw6"))
    {
    hgw6count++;
    return;
    }
if (sameString(machine, "hgw7"))
    {
    hgw7count++;
    return;
    }
if (sameString(machine, "hgw8"))
    {
    hgw8count++;
    return;
    }
}


void printDatabaseTime()
{
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;

safef(query, sizeof(query), "select now()");
sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row == NULL)
    errAbort("couldn't get database time\n");
verbose(1, "\nCurrent date-time = %s\n\n", row[0]);
sqlFreeResult(&sr);
}

int getUnixTimeNow()
/* ask the database for it's unix time (seconds since Jan. 1, 1970) */
{
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
int ret = 0;

safef(query, sizeof(query), "select unix_timestamp(now())");
sr = sqlGetResult(conn, query);
row = sqlNextRow(sr);
if (row == NULL)
    errAbort("couldn't get current time\n");
ret = sqlUnsigned(row[0]);
sqlFreeResult(&sr);
return ret;
}

void readLogs(int secondsNow)
/* read access_log where time_stamp > startTime */
{
char query[512];
struct sqlConnection *conn = hAllocConn();
struct sqlResult *sr;
char **row;
int startTime = secondsNow - (minutes * 60);
int hits = 0;
int errors = 0;

safef(query, sizeof(query), "select remote_host, machine_id, status, time_stamp, request_line, request_method, referer, agent "
                            "from access_log where time_stamp > %d", startTime);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {  
    logStatus(sqlUnsigned(row[2]));
    logMachine(row[1]);
    if (sameString(row[6], "-")) robotcount++;
    total++;
    } 
sqlFreeResult(&sr);

}

void printMachines()
{
verbose(1, "Count by machine: \n");
verbose(1, "hgw1 = %d\n", hgw1count);
verbose(1, "hgw2 = %d\n", hgw2count);
verbose(1, "hgw3 = %d\n", hgw3count);
verbose(1, "hgw4 = %d\n", hgw4count);
verbose(1, "hgw5 = %d\n", hgw5count);
verbose(1, "hgw6 = %d\n", hgw6count);
verbose(1, "hgw7 = %d\n", hgw7count);
verbose(1, "hgw8 = %d\n\n", hgw8count);
}

void printStatus()
{
verbose(1, "Count by status: \n");
verbose(1, "200: %d\n", status200);
verbose(1, "206: %d\n", status206); 
verbose(1, "301: %d\n", status301);
verbose(1, "302: %d\n", status302);
verbose(1, "304: %d\n", status304);
verbose(1, "400: %d\n", status400);
verbose(1, "403: %d\n", status403);
verbose(1, "404: %d\n", status404);
verbose(1, "405: %d\n", status405);
verbose(1, "408: %d\n", status408);
verbose(1, "414: %d\n", status414);
verbose(1, "500: %d\n\n", status500);
}

int main(int argc, char *argv[])
/* Check args and call readLogs. */
{
int timeNow = 0;
if (argc != 4)
    usage();
host = argv[1];
database = argv[2];
minutes = atoi(argv[3]);

user = getCfgValue("HGDB_USER", "db.user");
password = getCfgValue("HGDB_PASSWORD", "db.password");

hSetDbConnect(host, database, user, password);

printDatabaseTime();
timeNow = getUnixTimeNow();
readLogs(timeNow);
verbose(1, "Total hits in the last %d minutes = %d\n", minutes, total);
verbose(1, "Hits from robots = %d\n\n", robotcount);
printMachines();
printStatus();
return 0;
}

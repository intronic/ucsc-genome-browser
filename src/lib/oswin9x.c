/*****************************************************************************
 * Copyright (C) 2000 Jim Kent.  This source code may be freely used         *
 * for personal, academic, and non-profit purposes.  Commercial use          *
 * permitted only by explicit agreement with Jim Kent (jim_kent@pacbell.net) *
 *****************************************************************************/
/* Stuff that's specific for Win95 goes here. */
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <direct.h>
#include "common.h"
#include "portable.h"

/* Return how long the named file is in bytes. 
 * Return -1 if no such file. */
long fileSize(char *fileName)
{
int fd;
long size;
fd = _open(fileName, _O_RDONLY, 0);
if (fd < 0)
    return -1;
size = _lseek(fd, 0L, SEEK_END);
_close(fd);
return size;
}

long clock1000()
/* A millisecond clock. */
{
return clock() /* 1000/CLOCKS_PER_SEC */;   /* CLOCKS_PER_SEC == 1000 for windows */
}

long clock1()
/* Second clock. */
{
return clock()/CLOCKS_PER_SEC;
}

void uglyfBreak()
/* Go into debugger. */
{
__asm { int 3 } /* uglyf */
}

char *getCurrentDir()
/* Return current directory. */
{
static char dir[_MAX_PATH];

if( _getcwd( dir, _MAX_PATH ) == NULL )
    {
    warn("No current directory");
    return NULL;
    }
return dir;
}

boolean setCurrentDir(char *newDir)
/* Set current directory.  Return FALSE if it fails. */
{
if (_chdir(newDir) != 0)
    {
    warn("Unable to set dir %s", newDir);
    return FALSE;
    }
return TRUE;
}

struct slName *listDir(char *dir, char *pattern)
/* Return an alphabetized list of all files that match 
 * the wildcard pattern in directory. */
{
long hFile;
struct _finddata_t fileInfo;
struct slName *list = NULL, *name;
boolean otherDir = FALSE;
char *currentDir;

if (dir == NULL || sameString(".", dir) || sameString("", dir))
    dir = "";
else
    {
    currentDir = getCurrentDir();
    setCurrentDir(dir);
    otherDir = TRUE;
    }

if( (hFile = _findfirst( pattern, &fileInfo)) == -1L )
    return NULL;

do
    {
    if (!sameString(".", fileInfo.name) && !sameString("..", fileInfo.name))
        {
        name = newSlName(fileInfo.name);
        slAddHead(&list, name);
        }
    }
while( _findnext( hFile, &fileInfo) == 0 );
_findclose( hFile );
if (otherDir)
    setCurrentDir(currentDir);
slNameSort(&list);
return list;
}

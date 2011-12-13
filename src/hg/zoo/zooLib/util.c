#include "util.h"

char *argv0;

/* print_argv0 ---------------------------------------- print name of program */
void print_argv0(void)
{
	if (argv0) {
	char *p = strrchr(argv0, '/');
	(void)fprintf(stderr, "%s: ", p ? p+1 : argv0);
	}
}

/* fatal ---------------------------------------------- print message and die */
void fatal(const char *msg)
{
	fatalf("%s", msg);
}

/* fatalf --------------------------------- format message, print it, and die */
void fatalf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fflush(stdout);
	print_argv0();
	(void)vfprintf(stderr, fmt, ap);
	(void)fputc('\n', stderr);
	va_end(ap);
	exit(1);
}

/* ckopen -------------------------------------- open file; check for success */
FILE *ckopen(const char *name, const char *mode)
{
	FILE *fp;

	if ((fp = fopen(name, mode)) == NULL)
		fatalf("Cannot open %s.", name);
	return fp;
}

/* ckalloc -------------------------------- allocate space; check for success */
void *ckalloc(size_t amount)
{
	void *p;

	if ((long)amount < 0)                                  /* was "<= 0" -CR */
		fatal("ckalloc: request for negative space."); 
	if (amount == 0)
		amount = 1; /* ANSI portability hack */
	if ((p = malloc(amount)) == NULL)
		fatalf("Ran out of memory trying to allocate %lu.",
			(unsigned long)amount);
	return p;
}

/* ckallocz -------------------- allocate space; zero fill; check for success */
void *ckallocz(size_t amount)
{
	void *p = ckalloc(amount);
	memset(p, 0, amount);
	return p;
}

/* same_string ------------------ determine whether two strings are identical */
bool same_string(const char *s, const char *t)
{
	return (strcmp(s, t) == 0);
}

/* starts ------------------------------ determine whether t is a prefix of s */
bool starts(const char *s, const char *t)
{
	return (strncmp(s, t, strlen(t)) == 0);
}

/* skip_ws ------------------- find the first non-whitespace char in a string */
char *skip_ws(char *s)
{
	while (isspace(*s))
		s++;
	return s;
}

/* copy_string ---------------------- save string s somewhere; return address */
char *copy_string(const char *s)
{
	char *p = ckalloc(strlen(s)+1);    /* +1 to hold '\0' */
	return strcpy(p, s);
}

/* copy_substring ------------ save first n chars of string s; return address */
char *copy_substring(const char *s, int n)
{
	char *p = ckalloc((size_t)n+1);    /* +1 to hold '\0' */
	memcpy(p, s, (size_t)n);
	p[n] = 0;
	return p;
}

void ckfree(void *p)
{
	if (p) free(p);
}


unsigned int roundup(unsigned int n, unsigned int m)
{
    return ((n+(m-1))/m) * m;
}

void fatalfr(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        fflush(stdout);
        print_argv0();
        (void)vfprintf(stderr, fmt, ap);
        (void)fprintf(stderr, ": %s\n", strerror(errno));
        va_end(ap);
        exit(1);
}


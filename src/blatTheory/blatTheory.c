/* blatTheory - Calculate some theoretical blat statistics. */
#include "common.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "blatTheory - Calculate some theoretical blat statistics\n"
  "usage:\n"
  "   blatTheory XXX\n");
}

double factorial(int n)
{
int i;
double tot = 1.0;

for (i=1; i<=n; ++i)
    tot *= i;
return tot;
}

double choose(int m, int n)
{
return factorial(n)/(factorial(m) * factorial(n-m));
}

double power(double x, int y)
{
int i;
double tot = 1.0;

for (i=1; i<=y; ++i)
    tot *= x;
return tot;
}

double blatSensitivity(double m, int w, int k, int n)
/* Probability that blat finds a hit. */
{
int t = w/k;
double p = power(m, k);
int i;
double pn;
double pTotal = 0;

for (i = n; i<= t; ++i)
    {
    pn = choose(i, t) * power(p, i) * power(1.0-p, t-i);
    pTotal += pn;
    }
return pTotal;
}

double blatSpecificity(int k, int n, double genomeSize, double querySize, int alphabetSize)
{
double a = 1.0/alphabetSize;
double p = power(a, k);
double f1 = p*querySize*genomeSize/k;
double total = f1;
int i;

for (i=1; i<n; ++i)
    total *= 100.0/k*p;
return total;
}

double exonerateSensitivity(double m, double w, int k)
/* Probability that exonerate finds a hit. */
{
int t = w/k;
double p = power(m, k) + k*power(m, k-1)*(1.0-m);
double pTotal = 1.0 - power((1.0 - p), t);
return pTotal;
}

double exonerateSpecificity(int k, double genomeSize, double querySize, double alphabetSize)
/* Return number of false positives. */
{
double a = 1.0/alphabetSize;
double p = power(a, k) + k*power(a, k-1)*(1.0-a);
return p*genomeSize*querySize/k;
}


void printBlat(double m, int w, int k, int n)
/* Print blat probability .*/
{
double pTotal = blatSensitivity(m, w, k, n);
printf("blat sensitivity %f (m=%f, w=%d, k=%d, n=%d)\n",
	pTotal, m, w, k, n);
}

void printExonerate(double m, int w, int k)
/* Print exonerate hit probability given parameters. */
{
double pTotal = exonerateSensitivity(m, w, k);
printf("exonerate sensitivity %f (m=%f, w=%d, k=%d)\n",
	pTotal, m, w, k);
}


void oldBlatTheory()
/* blatTheory - Calculate some theoretical blat statistics. */
{
int k;
int n;
int w;
double m;
double blatSens, blatSpec, exonSens, exonSpec;
double dnaConservation = 0.862, aaConservation = 0.89;
int blatNum = 2, exonerateWin = 14;

printf("homology     exonerate            blat      \n");
printf("  size    hit%%     false pos   hit%%     false pos\n");
for (w=10; w<=150; w += 10)
    {
    blatSens = blatSensitivity(aaConservation, w/3, 4, blatNum);
    blatSpec = blatSpecificity(4, blatNum, 6.0e9, 600/3, 20);
    exonSens = exonerateSensitivity(dnaConservation, w, exonerateWin);
    exonSpec = exonerateSpecificity(exonerateWin, 6.0e9, 600, 4);
    printf("%3d       %4.2f    %5.1f     %4.2f    %5.1f\n",
    	w, 100*exonSens, exonSpec, 100*blatSens, blatSpec);
    }
}

double p1(double m, int k, double h)
/* Probability of single tile perfect match. */
{
double t = (double)h/(double)k;
double p = pow(m, k);
return 1  - pow((1-p), t);
}

double f1(double q, double g, double k, double a)
/* Number of single tile false positives. */
{
return q*(g/k)*pow(a,-k);
}



void blatTheory2()
/* blatTheory - Calculate some theoretical blat statistics. */
{
int k,m,n;

 for (m=85; m<99; m += 1)
    {
//    printf("%d%% ", m);
    for (n=2; n<=3; ++n)
	for (k=3; k<=6; ++k)
//	    printf("%1.3f ", blatSensitivity(m*0.01, 100.0/3, k, n));
	printf("%1.0f ", blatSpecificity(k, n, 3e9, 500.0/3, 20));
    printf("\n");
    }
}

void blatTheory3()
/* blatTheory - Calculate some theoretical blat statistics. */
{
int w,k,m,n;
int s = 20, e = 150;

printf("5\t");
/* Probability of single tile perfect match. */
for (w=s; w<=e; w += 10)
    printf("%1.3f ", p1(0.89, 5, w/3));
printf("\n");
printf("2x4\t");
for (w=s; w<=e; w += 10)
    printf("%1.3f ", blatSensitivity(0.89, w/3, 4, 2));
printf("\n");
printf("12-1\t");
for (w=s; w<=e; w += 10)
    printf("%1.3f ", exonerateSensitivity(0.86, w, 12));
printf("\n");
printf("8-1\t");
for (w=s; w<=e; w += 10)
    printf("%1.3f ", exonerateSensitivity(0.89, w/3, 8));
printf("\n");
}

void blatTheory()
/* blatTheory - Calculate some theoretical blat statistics. */
{
int k;

for (k=5; k<12; ++k)
    printf("%4d ", k);
printf("\n");
for (k=5; k<12; ++k)
    printf("%1.3f ", blatSensitivity(0.86, 100.0, k, 2));
printf("\n");
for (k=5; k<12; ++k)
    printf("%4.0f ", blatSpecificity(k, 2, 6.0e9, 600, 4));
printf("\n");
}

int main(int argc, char *argv[])
/* Process command line. */
{
blatTheory();
return 0;
}

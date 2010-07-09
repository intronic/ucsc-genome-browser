/* hdb - human genome browser database. */

#ifndef DNAMARKOVSQL_H
#define DNAMARKOVSQL_H

#ifndef JKSQL_H
#include "jksql.h"
#endif 

boolean loadMark2(struct sqlConnection *conn, char *table, char *chrom, unsigned start, unsigned end, double mark2[5][5][5]);
// Load 2nd-order markov model from given table

#endif /* DNAMARKOVSQL_H */

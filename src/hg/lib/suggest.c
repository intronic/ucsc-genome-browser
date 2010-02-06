/* code to support suggesting genes given a prefix typed by the user. */

#include "suggest.h"

static char const rcsid[] = "$Id: ";

boolean connSupportsGeneSuggest(struct sqlConnection *conn)
// return true if this connection has tables to support gene autocompletion
{
return sqlTableExists(conn, "knownCanonical") || sqlTableExists(conn, "refGene");
}

boolean assemblySupportsGeneSuggest(char *database)
// return true if this assembly has tables to support gene autocompletion
{
struct sqlConnection *conn = hAllocConn(database);
boolean retval = connSupportsGeneSuggest(conn);
hFreeConn(&conn);
return retval;
}

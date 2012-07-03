/* Code to support geographic mirrors (e.g. euronode) */

#ifndef GEOMIRROR_H
#define GEOMIRROR_H

#include "hdb.h"

boolean geoMirrorEnabled();
// return TRUE if this site has geographic mirroring turned on

char *geoMirrorNode();
// return which geo mirror node this is (or NULL if geo mirroring is turned off)

int defaultNode(struct sqlConnection *centralConn, char *ipStr);
// return default node for given IP

#endif /* GEOMIRROR_H */

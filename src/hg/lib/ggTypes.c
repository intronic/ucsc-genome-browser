/* ggTypes - Type enumerations used by geneGraph and other modules. */

#include "common.h"
#include "ggTypes.h"


char *ggVertexTypeAsString(enum ggVertexType type)
/* Return string corresponding to vertex type. */
{
switch(type)
    {
    case ggSoftStart:
        return "(";
    case ggHardStart:
        return "[";
    case ggSoftEnd:
        return ")";
    case ggHardEnd:
        return "]";
    default:
        errAbort("Unknown type %d", type);
	return "unknown";
    }
}

enum ggVertexType ggVertexTypeFromString(char *s)
/* Return string corresponding to vertex type. */
{
switch(s[0])
    {
    case '(':
	return ggSoftStart;
    case '[':
	return ggHardStart;
    case ')':
	return ggSoftEnd;
    case ']':
	return ggHardEnd;
    default:
        errAbort("Unknown type %s", s);
	return ggUnused;
    }
}

char *ggEdgeTypeAsString(enum ggVertexType type)
/* Return string corresponding to edge type. */
{
switch(type)
    {
    case ggIntron:
        return "intron";
    case ggExon:
        return "exon";
    default:
        errAbort("Unknown type %d", type);
	return "unknown";
    }
}

enum ggVertexType ggEdgeTypeFromString(char *s)
/* Return string corresponding to edge type. */
{
if (sameString(s, "exon"))
    return ggExon;
else if (sameString(s, "intron"))
    return ggIntron;
else
    {
    errAbort("Unknown type %s", s);
    return ggIntron;	/* Because compiler wants us to return something. */
    }
}

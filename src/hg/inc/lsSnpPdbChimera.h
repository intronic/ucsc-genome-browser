/* lsSnpPdbChimera - Code for interfacing to LS-SNP and Chimera.  Generates
 * Chimera scripts to visualize proteins with SNPs annotated.  Takes an
 * external python script that defines methods used in the rendering.  The
 * generates a .chimerax file that contains the script as well as the code to
 * initialize Chimera for the specific PDB and SNP set.  This also includes
 * interfacing to LS-SNP/PDB, which is used to label Chimera images.
 */
#ifndef lsSnpPdbChimera_h
#define lsSnpPdbChimera_h
struct sqlConnection;
struct tempName;

void lsSnpPdbChimeraSnpAnn(struct sqlConnection *conn,
                           char *pdbId, char *primarySnpId,
                           struct tempName *outName);
/* Generate a chimerax file for the given pdb with all non-synonymous SNPs
 * that have been mapped to this protein.  If primarySnpId is not NULL, it is
 * colored differently than the other SNPs.  Fills in outName structure. */

struct slName *lsSnpPdbChimeraGetSnpPdbs(struct sqlConnection *conn,
                                         char *snpId);
/* get list of PDBs to which snpId is mapped.  */

char *lsSnpPdbChimeraGetStructType(struct sqlConnection *conn, char *pdbId);
/* Determine structure type of a PDB (NMR or X-Ray).  Constant result, don't
 * free. */

boolean lsSnpPdbHasPdb(struct sqlConnection *conn, char *pdbId);
/* determine if the specified PDB has any entries in LS-SNP */

char *lsSnpPdbGetUrlPdbSnp(char *pdbId, char *snpId);
/* get LS-SNP/PDB URL for a particular PDB and/or SNP.  One or the two
 * ids maybe null */

#endif

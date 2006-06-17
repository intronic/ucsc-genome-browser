/* geneCheckWidget - output HTML tables to display geneCheck or
 * geneCheckDetails with browser links */
#ifndef GENECHECKWIDGET_H
#define GENECHECKWIDGET_H

struct geneCheck;
struct cart;
struct geneCheckDetails;

void geneCheckWidgetSummary(struct geneCheck *gc, char *tblClass, char *caption);
/* write a table with result summary for one gene; caption maybe NULL */

void geneCheckWidgetDetails(struct cart *cart, struct geneCheck *gc,
                            struct geneCheckDetails *gcdList, char *tblClass,
                            char *caption, char *target);
/* display gene-check details; caption maybe NULL.  target is target of
 * links if not NULL.*/

#endif

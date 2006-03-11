/* dbLoadPartitions - get information about partitions to process */
#ifndef DBLOADPARTITIONS_H
#define DBLOADPARTITIONS_H
struct gbIndex;
struct dbLoadOptions;

struct gbSelect* dbLoadPartitionsGet(struct dbLoadOptions* options,
                                     struct gbIndex* index);
/* build a list of partitions to load based on the command line and
 * conf file options and whats in the index */
#endif

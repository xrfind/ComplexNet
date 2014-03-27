#ifndef CN_IINET_H
#define CN_IINET_H

#include "iilinefile.h"

struct iiNet{
	int maxId;
	int minId;
	int idNum;
	long edgesNum;
	long countMin;
	long countMax;
	long *count;
	int **edges;
};

void free_iiNet(struct iiNet *net);
struct iiNet *create_iiNet(const struct iiLineFile * const file);

void print_iiNet(struct iiNet *net, char *filename);

//thread routine, don't need arg, the routine will use net;
//of course, you can use this in the main thread.
void *verify_iiNet(void *arg);


#endif
#include "common.h"
#include "hashtable.h"
#include "iilinefile.h"
#include <assert.h>
#include <stdlib.h>

//elementNum
static void elementNumSumHT(struct HashTable *ht)
{
	if (!ht->sumSign) {
		int i;
		int ivalue=ht->elementNum[0];
		ht->elementNum[0]=0;
		for (i=1; i<ht->length; ++i) {
			int temp=ht->elementNum[i];
			ht->elementNum[i]=ivalue+ht->elementNum[i-1];
			ivalue=temp;
		}
		ht->elementSumNum = ht->elementNum[ht->length-1];
		struct HashElement *he=ht->he[ht->length-1];
		while(he) {
			++ht->elementSumNum;
			he=he->next;
		}
		ht->sumSign=1;
	}
}

static void elementNumBackHT(struct HashTable *ht)
{
	if (ht->sumSign) {
		int i;
		for (i=1; i<ht->length; ++i) {
			ht->elementNum[i-1]=ht->elementNum[i]-ht->elementNum[i-1];
		}
		ht->elementNum[ht->length-1]=0;
		struct HashElement *he=ht->he[ht->length-1];
		while(he) {
			++ht->elementNum[ht->length-1];
			he=he->next;
		}
		ht->sumSign=0;
	}
}


int getelementSumNumHT(struct HashTable *ht) {
	elementNumSumHT(ht);
	return ht->elementSumNum;
}

void setelementIndexHT(struct HashTable *ht) {
	if (!ht->indexSign) {
		elementNumSumHT(ht);
		long *index = malloc((ht->elementSumNum)*sizeof(long));
		assert(index != NULL);

		struct HashElement *he;
		int i, j=0;
		for (i=0; i<ht->length; ++i) {
			he = ht->he[i];
			while(he) {
				index[j++] = he->element;
				he=he->next;
				//if (j>=ht->elementSumNum) {
				//	printf("%d\n", j);fflush(stdout);
				//}
			}
		}
		ht->index = index;
		ht->indexSign = 1;
	}
}

long getelementValueHT(struct HashTable *ht, int index) {
	if (ht->indexSign) {
		return ht->index[index];
	}
	else {
		elementNumSumHT(ht);
		if (index>ht->elementSumNum) return -1;
		int i;
		for (i=0; i<ht->length; ++i) {
			if (ht->elementNum[i]<=index) {
				continue;
			}
			break;
		}
		int rowindex = index - ht->elementNum[--i];
		struct HashElement *he = ht->he[i];
		while(rowindex--) {
			he=he->next;
		}
		return he->element;
	}
}

//recursion free.
static void freeHashElement(struct HashElement *he)
{
	if (he != NULL && he->next !=NULL) {
		freeHashElement(he->next);
	}
	free(he);
	he=NULL;
}

void freeHashTable(struct HashTable *ht)
{
	elementNumBackHT(ht);
	int i;
	for (i=0; i<ht->length; ++i) {
		if (ht->elementNum[i]>0) {
			freeHashElement(ht->he[i]);
		}
	}
	free(ht->elementNum);
	free(ht->he);
	if (ht->indexSign) {
		free(ht->index);
	}
	free(ht);
}


struct HashTable *createHashTable(int length)
{
	struct HashTable *ht=calloc(1, sizeof(struct HashTable));
	assert(ht!=NULL);

	struct HashElement **he=calloc(length, sizeof(void *));
	assert(he!=NULL);

	int *elementNum=calloc(length, sizeof(int));
	assert(elementNum!=NULL);
	
	ht->he=he;
	ht->length=length;
	ht->elementNum=elementNum;
	//ht->sumSign=0;

	return ht;
}

int insertHEtoHT(struct HashTable *ht, long element)
{
	elementNumBackHT(ht);

	struct HashElement *he=calloc(1, sizeof(struct HashElement));
	assert(he!=NULL);

	int index=element%ht->length;
	he->element=element;

	struct HashElement *l;
	struct HashElement *r;

	if (ht->he[index]!=NULL) {
		//if element is smaller than the first element, insert.
		if (ht->he[index]->element > element) {
			he->next=ht->he[index];
			ht->he[index]=he;
			++ht->elementNum[index];
		} else if (ht->he[index]->element < element) {
			//if element is bigger than the first element, and there is only one element, insert.
			if (ht->he[index]->next == NULL) {
				he->next=NULL;
				ht->he[index]->next=he;
				++ht->elementNum[index];
			} else {
				l=ht->he[index];
				r=ht->he[index]->next;
				while(r) {
					//if element is bigger than the last element, insert.
					if (r->element < element) {
						if (r->next==NULL) {
							he->next=NULL;
							r->next=he;
							++ht->elementNum[index];
							break;
						//if element is bigger than any element but not the last one, next.
						} else {
							l=r;
							r=r->next;	
							continue;
						}
					//else if element is smaller than any element, insert.
					} else if (r->element > element) {
						he->next=l->next;
						l->next=he;
						++ht->elementNum[index];
						break;
					//if element existed, do nothing.
					} else {
						free(he);
						return -1;
					}
				}
			}
		} else {
			free(he);
			return -1;
		}
	} else {
		//if no element, insert.
		he->next=NULL;
		ht->he[index]=he;
		++ht->elementNum[index];
	}
	return 0;
}

void deleteHEfromHT(struct HashTable *ht, long element)
{
	elementNumBackHT(ht);
	int index=element%ht->length;
	struct HashElement *l;
	struct HashElement *r;
	if (ht->he[index]!=NULL) {
		if (ht->he[index]->element==element) {
			struct HashElement *temp=ht->he[index];
			ht->he[index]=ht->he[index]->next;
			free(temp);
			--ht->elementNum[index];
		} 
		else if (ht->he[index]->next!=NULL){
			l=ht->he[index];
			r=ht->he[index]->next;
			while(r) {
				if (r->element==element) {
					l->next=r->next;
					free(r);
					--ht->elementNum[index];
					break;
				}
				l=r;
				r=r->next;
			}
		}
	}
}

int getelementIndexHT(struct HashTable *ht, long element)
{
	elementNumSumHT(ht);
	int rindex=element%ht->length;
	struct HashElement *r=ht->he[rindex];
	int index=ht->elementNum[rindex];
	while(r) {
		if(r->element == element) {
			return index;
		}
		++index;
		r=r->next;
	}
	return -1;
}

void *writeContinuousiiLineFileHT(void * arg) {
	struct iiLineFile *file = arg;
	struct HashTable *ht = createHashTable(3000000);
	long i;
	for (i=0; i<file->linesNum; ++i) {
		insertHEtoHT(ht, file->lines[i].i1);
		insertHEtoHT(ht, file->lines[i].i2);
	}
	FILE *fp = fopen("data/continuousiiLineFile", "w");
	fileError(fp, "data/continuousiiLineFile");
	for (i=0; i<file->linesNum; ++i) {
		int i1 = getelementIndexHT(ht, file->lines[i].i1);
		int i2 = getelementIndexHT(ht, file->lines[i].i2);
		fprintf(fp, "%d\t%d\n", i1, i2);
	}
	fclose(fp);
	freeHashTable(ht);
	return (void *)0;
}
/*
void *writeContinuousi4LineFileHT(void * arg) {
	struct i4LineFile *file = arg;
	struct HashTable *ht = createHashTable(3000000);
	//struct HashTable *ht_i4 = createHashTable(300000);
	long i;

	for (i=0; i<file->linesNum; ++i) {
		insertHEtoHT(ht, file->lines[i].i1);
		insertHEtoHT(ht, file->lines[i].i2);
	//	insertHEtoHT(ht_i4,file->lines[i].i4);
	}
	FILE *fp = fopen("data/continuousi4LineFile", "w");
	fileError(fp, "data/continuousi4LineFile");
	for (i=0; i<file->linesNum; ++i) {
		int i1 = getelementIndexHT(ht, file->lines[i].i1);
		int i2 = getelementIndexHT(ht, file->lines[i].i2);
	//	int i4 = getelementIndexHT(ht_i4, file->lines[i].i4);
	//	fprintf(fp, "%d\t%d\t%d\t%d\n", i1, i2, file->lines[i].i3, i4);
		fprintf(fp, "%d\t%d\t%d\t%d\n", i1, i2, file->lines[i].i3, file->lines[i].i4);
	}
	fclose(fp);
	freeHashTable(ht);
	//freeHashTable(ht_i4);
	return (void *)0;
}
*/

void wc_ii_HT(struct iiLineFile *file, char *filename) {
	struct HashTable *i1 = createHashTable(3000000);
	struct HashTable *i2 = createHashTable(3000000);
	long i;
	for (i=0; i<file->linesNum; ++i) {
		insertHEtoHT(i1, file->lines[i].i1);
		insertHEtoHT(i2, file->lines[i].i2);
	}
	FILE *fp = fopen(filename, "w");
	fileError(fp, "wc_bip2_ii_HT");
	for (i=0; i<file->linesNum; ++i) {
		int ii1 = getelementIndexHT(i1, file->lines[i].i1);
		int ii2 = getelementIndexHT(i2, file->lines[i].i2);
		fprintf(fp, "%d\t%d\n", ii1, ii2);
	}
	fclose(fp);
	freeHashTable(i1);
	freeHashTable(i2);
}

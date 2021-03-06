/**
 * onion model knn implementation
 *
 * 1, read a biparte net, and divide it into two parts: trainset,testset.
 * 2, for each user in testset, first I use the first two step of mass diffusion, then 
 *    I will use the user-similarity to decide how many similar users I will use on the third step.
 *    actually, for one user, I have to do many calculations.
 *    for example, if one user has 100 similar users, I will need to calculate 100 times.
 *    	the first time: I use the top 1 user to calculate.
 *    	the second time: I use the top 2 users to calculate.
 *    	the third time: I use the top 3 users to calculate.
 *    	...
 *    	the 100 time: I use all 100 users.
 *    and this is only one user.
 *    the result is that: one user to one simliar users' number.
 * 3, use the new data to diffusion, get rankscore and all other things. and use mass diffusion too, get compared.
 *
 */

//#define NDEBUG  //for assert
#include <stdio.h>

#include <assert.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "iilinefile.h"
#include "iidlinefile.h"
#include "bip.h"
#include "error.h"
#include "mt_random.h"
#include "sort.h"

void set_RandomSeed(void) {
	time_t t=time(NULL);
	unsigned long init[4]={t, 0x234, 0x345, 0x456}, length=4;
	init_by_array(init, length);
}

void print_time(void) {
	time_t t=time(NULL); 
	printf("%s\n", ctime(&t)); 
	fflush(stdout);
}

void create_2dataset(struct Bipii *neti1, struct Bipii *neti2, double rate, struct Bipii **A1, struct Bipii **A2, struct Bipii **B1, struct Bipii **B2) {
	struct iiLineFile *first, *second;
	divide_Bipii(neti1, neti2, rate, &first, &second);
	//A is big train set. B is big test set.
	*A1 = create_Bipii(second, 1);
	*A2 = create_Bipii(second, 0);
	*B1 = create_Bipii(first, 1);
	*B2 = create_Bipii(first, 0);
	free_iiLineFile(first);
	free_iiLineFile(second);
}

void get_UserSimilarity(struct Bipii *traini1, struct Bipii *traini2, struct iidNet **userSim) {
	struct iidLineFile *userSimilarityfile = similarity_Bipii(traini1, traini2, 1);
	*userSim = create_iidNet(userSimilarityfile);
	free_iidLineFile(userSimilarityfile);
}

void get_ItemSimilarity(struct Bipii *traini1, struct Bipii *traini2, struct iidNet **itemSim) {
	struct iidLineFile *itemSimilarityfile = similarity_Bipii(traini1, traini2, 0);
	*itemSim = create_iidNet(itemSimilarityfile);
	free_iidLineFile(itemSimilarityfile);
}

void test_ArgcArgv(int argc, char **argv, char **netfilename, int *N) {
	if (argc == 1) {
		*netfilename = "data/movielens/movielens_2c";
		//*colK = 0.2;	
		*N = 2;
	}
	else if (argc == 3) {
		*netfilename = argv[1];
		char *p;
		*N = strtol(argv[2], &p, 10);

	}
	else {
		isError("wrong argc, argv.\n");
	}
}

int main(int argc, char **argv)
{
	print_time();
	//set_RandomSeed();

	int N = 2;
	char *netfilename;
	test_ArgcArgv(argc, argv, &netfilename, &N); 

	/*******************prepare A B C D from netfilename*********************************************************/
	struct iiLineFile *netfile = create_iiLineFile(netfilename);
	struct Bipii *neti1 = create_Bipii(netfile, 1);
	struct Bipii *neti2 = create_Bipii(netfile, 0);
	free_iiLineFile(netfile);

	struct Bipii *A1, *A2, *B1, *B2;
	create_2dataset(neti1, neti2, 0.1, &A1, &A2, &B1, &B2);
	free_Bipii(neti1);
	free_Bipii(neti2);
	struct iidNet *AuserSim, *AitemSim=NULL;
	get_UserSimilarity(A1, A2, &AuserSim);
	get_ItemSimilarity(A1, A2, &AitemSim);
	sort_desc_iidNet(AuserSim);

	//C is train set in A, D is test set in A.
	struct Bipii *C1, *C2, *D1, *D2;
	create_2dataset(A1, A2, 0.11, &C1, &C2, &D1, &D2);
	struct iidNet *CuserSim, *CitemSim=NULL;
	get_UserSimilarity(C1, C2, &CuserSim);
	get_ItemSimilarity(C1, C2, &CitemSim);
	sort_desc_iidNet(CuserSim);
	/***********************************************************************************************************/

	//TODO  lucky: A and C have same length! get one c&d, get one a&b, get a R; then get next c&d, next a&b, get a R.
	//TODO  get a user into c, at the same time, get a user into d/a/b.
	//TODO  decide divide into N parts, get a variables means how long a subset will be, divide C&D, A&B into cn&dn, an&bn.
	//TODO  A,B,C,D will keep unchange. I need to clone a A,B,C,D, and use this four as a,b,c,d.
	//TODO  decide N, decide L(length), sub a user from C to c, A to a, B to b, D to d.


	/***********************************************************************************************************/
	struct Bipii *a1 = clone_Bipii(A1);
	struct Bipii *a2 = clone_Bipii(A2);
	struct Bipii *b1 = clone_Bipii(B1);
	struct Bipii *b2 = clone_Bipii(B2);
	struct Bipii *c1 = clone_Bipii(C1);
	struct Bipii *c2 = clone_Bipii(C2);
	struct Bipii *d1 = clone_Bipii(D1);
	struct Bipii *d2 = clone_Bipii(D2);

	//A1->maxId == C1->maxId, always for sure.
	int *NACcount = malloc((A1->maxId + 1)*sizeof(int));
	int *NACuid = malloc((A1->maxId + 1)*sizeof(int));
	int i;
	for (i=0; i<A1->maxId + 1; ++i) {
		NACcount[i] = C1->count[i];
		NACuid[i] = i;
	}
	qsort_ii_desc(NACcount, 0, C1->maxId, NACuid);
	int Length = ceil((double)(C1->maxId + 1)/N);
	double RealR = 0;
	for (i=0; i<N; ++i) {
		int begin = Length*i;
		int end   = Length*(i+1);
		memset(c1->count, 0, (c1->maxId + 1)*sizeof(long));
		memset(d1->count, 0, (d1->maxId + 1)*sizeof(long));
		memset(a1->count, 0, (a1->maxId + 1)*sizeof(long));
		memset(b1->count, 0, (b1->maxId + 1)*sizeof(long));
		memset(c2->count, 0, (c2->maxId + 1)*sizeof(long));
		memset(d2->count, 0, (d2->maxId + 1)*sizeof(long));
		memset(a2->count, 0, (a2->maxId + 1)*sizeof(long));
		memset(b2->count, 0, (b2->maxId + 1)*sizeof(long));
		d1->edgesNum = 0;
		b1->edgesNum = 0;
		int j;
		for (j=begin; j<end && j<C1->maxId + 1; ++j) {
			int id = NACuid[j];
			c1->count[id] = C1->count[id];
			int k;
			for (k=0; k<c1->count[id]; ++k) {
				int itemid = c1->id[id][k];
				c2->id[itemid][c2->count[itemid]++] = id;
			}
			if (id < d1->maxId + 1) {
				d1->count[id] = D1->count[id];
				d1->edgesNum += D1->count[id];
			}
			a1->count[id] = A1->count[id];
			for (k=0; k<a1->count[id]; ++k) {
				int itemid = a1->id[id][k];
				a2->id[itemid][a2->count[itemid]++] = id;
			}
			if (id < b1->maxId + 1) {
				b1->count[id] = B1->count[id];
				b1->edgesNum += B1->count[id];
			}
		}
		struct iidNet *auserSim, *aitemSim=NULL;
		get_UserSimilarity(a1, a2, &auserSim);
		get_ItemSimilarity(a1, a2, &aitemSim);
		sort_desc_iidNet(auserSim);
		struct iidNet *cuserSim, *citemSim=NULL;
		get_UserSimilarity(c1, c2, &cuserSim);
		get_ItemSimilarity(c1, c2, &citemSim);
		sort_desc_iidNet(cuserSim);
		int cdbestTopR = mass_GetTopR_Bipii(c1, c2, d1, d2, cuserSim);
		struct Metrics_Bipii *topR_result = mass_topR_Bipii(a1, a2, b1, b2, aitemSim, auserSim, cdbestTopR);
		RealR += topR_result->R*b1->edgesNum;
		free_MetricsBipii(topR_result);
		printf("edgesNum: %ld\n", b1->edgesNum);
		free_iidNet(auserSim);
		free_iidNet(cuserSim);
		free_iidNet(aitemSim);
		free_iidNet(citemSim);
	}
	RealR /= B1->edgesNum;
	printf("N: %d, R: %f\n", N, RealR);
	/***********************************************************************************************************/


	/*
	struct Metrics_Bipii *topR_result;

	int CDbestTopR = mass_GetTopR_Bipii(C1, C2, D1, D2, CuserSim);
	topR_result = mass_topR_Bipii(A1, A2, B1, B2, AitemSim, AuserSim, CDbestTopR);
	printf("best topR AB ,using CDbest\tR: %f, PL: %f, IL: %f, HL: %f, NL: %f\n", topR_result->R, topR_result->PL, topR_result->IL, topR_result->HL, topR_result->NL);
	free_MetricsBipii(topR_result);

	int ABbestTopR = mass_GetTopR_Bipii(A1, A2, B1, B2, AuserSim);
	topR_result = mass_topR_Bipii(A1, A2, B1, B2, AitemSim, AuserSim, ABbestTopR);
	printf("best topR AB ,using ABbest\tR: %f, PL: %f, IL: %f, HL: %f, NL: %f\n", topR_result->R, topR_result->PL, topR_result->IL, topR_result->HL, topR_result->NL);
	free_MetricsBipii(topR_result);
	*/

	free(NACcount);
	free(NACuid);
	memcpy(c1->count, C1->count, (c1->maxId + 1)*sizeof(long));
	memcpy(d1->count, D1->count, (d1->maxId + 1)*sizeof(long));
	memcpy(a1->count, A1->count, (a1->maxId + 1)*sizeof(long));
	memcpy(b1->count, B1->count, (b1->maxId + 1)*sizeof(long));
	memcpy(c2->count, C2->count, (c2->maxId + 1)*sizeof(long));
	memcpy(d2->count, D2->count, (d2->maxId + 1)*sizeof(long));
	memcpy(a2->count, A2->count, (a2->maxId + 1)*sizeof(long));
	memcpy(b2->count, B2->count, (b2->maxId + 1)*sizeof(long));
	free_Bipii(A1);
	free_Bipii(A2);
	free_Bipii(B1);
	free_Bipii(B2);
	free_Bipii(C1);
	free_Bipii(C2);
	free_Bipii(D1);
	free_Bipii(D2);
	free_Bipii(a1);
	free_Bipii(a2);
	free_Bipii(b1);
	free_Bipii(b2);
	free_Bipii(c1);
	free_Bipii(c2);
	free_Bipii(d1);
	free_Bipii(d2);

	free_iidNet(AuserSim);
	free_iidNet(AitemSim);
	free_iidNet(CuserSim);
	free_iidNet(CitemSim);
	//free_L_Bip(mass_result);
	//free_L_Bip(simcut_result);
	
	print_time();
	return 0;
}

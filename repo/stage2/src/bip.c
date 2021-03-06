/**
 *
 */
#include "bip.h"
#include "common.h"
#include "mt_random.h"
#include "sort.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>


/************************************************************************************************************/
/************************* biparte network recommendation core functions. ***********************************/
/*** they can be used by bipii or bipiid. **********************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/

//actually Bipii is the most basic biparte.
//so here I just typedef to create Metrics_Bip and Bip.
typedef struct Metrics_Bipii Metrics_Bip;
typedef struct Bipii Bip;
static Metrics_Bip *create_MetricsBip(void) {
	return create_MetricsBipii();
}

struct Bip_recommend_param{
	int i1;

	int mass_topR;
	int mass_topR5;
	int mass_topR5RE;
	double mass_topS;
	double mass_corR;
	double mass_corS;
	double mass_expR;
	double mass_expRI_itemAveDreMax;
	double *mass_expRI_itemAveDre;
	int *mass_bestR;

	double HNBI_param;

	double RENBI_param;

	double hybrid_param;


	struct iidNet *userSim;
	struct iidNet *itemSim;

	Bip *traini1;
	Bip *traini2;
	Bip *testi1;

	double *i1source;
	double *i2source;
	int *i1id;
	int *i2id;
	double *i2sourceA;

};

//following is for recommendation.
//Warning: remeber the maxId in testset maybe smaller than the maxId in trainset.
//R is rankscore.
//PL is precision
//Warning: about unselected_list_length, I use bipii->idNum, not bipii->maxId. 
//	but I believe in linyuan's paper, she use the bipii->maxId. 
//	I think bipii->idNum make more sence.
static void metrics_R_PL_Bip(int i1, long *i1count, int i2idNum, Bip *testi1, int L, int *rank, double *R, double *PL) {
//static void metrics_Bip(int i1, long *i1count, int i2idNum, struct Bip_core_test *testi1, int L, int *rank, double *R, double *PL) {
	if (i1<testi1->maxId + 1 &&  testi1->count[i1]) {
		int unselected_list_length = i2idNum - i1count[i1];
		int rank_i1_j = 0;
		int DiL = 0;
		int j, id;
		for (j=0; j<testi1->count[i1]; ++j) {
			id = testi1->id[i1][j];
			rank_i1_j += rank[id];
			if (rank[id] <= L) {
				++DiL;
			}
		}
		*R += (double)rank_i1_j/(double)unselected_list_length;
		*PL += (double)DiL/(double)L;
	}
}
//IL is intrasimilarity
static double metrics_IL_Bip(int i1maxId, long *i1count, int i1idNum, int i2maxId, int L, int *Hij, struct iidNet *sim) {
	if (!sim) return -1;
	double *sign = calloc((i2maxId + 1), sizeof(double));
	assert(sign != NULL);
	int i, j;
	long k;
	double IL = 0;
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1count[i]) {
			int *tmp = Hij + i*L;
			for (j=0; j<L; ++j) {
				int id = tmp[j];
				memset(sign, 0, (i2maxId + 1)*sizeof(double));
				for (k=0; k<sim->count[id]; ++k) {
					sign[sim->edges[id][k]] = sim->d3[id][k];
				}
				for (k=j+1; k<L; ++k) {
					id = tmp[k];
					IL += sign[id];
				}
			}
		}
	}
	free(sign);
	IL /= L*(L-1)*i1idNum;
	return 2*IL;
}
//HL is hamming distance.
static double metrics_HL_Bip(int i1maxId, long *i1count, int i2maxId, int L, int *Hij) {
	int *sign = calloc((i2maxId + 1), sizeof(int));
	assert(sign != NULL);
	int i, j;
	long k;
	int cou = 0;
	int Cij = 0;
	double HL = 0;
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1count[i]) {
			memset(sign, 0, (i2maxId + 1)*sizeof(int));
			for (k=i*L; k<i*L+L; ++k) {
				sign[Hij[k]] = 1;
			}
			for (j=i+1; j<i1maxId + 1; ++j) {
				if (i1count[j]) {
					Cij = 0;
					for (k=j*L; k<j*L+L; ++k) {
						if (sign[Hij[k]]) {
							++Cij;
						}
					}
					HL += 1 - ((double)Cij)/(double)L;
					//printf("%d %d %d\n", i, j, Cij);
					++cou;
				}
			}
		}
	}
	free(sign);
	return HL/cou;
}
//NL is popularity.
static double metrics_NL_Bip(int i1maxId, long *i1count, int i1idNum, long *i2count, int L, int *Hij) {
	int i,j;
	long NL = 0;
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1count[i]) {
			int *tmp = Hij + i*L;
			for (j=0; j<L; ++j) {
				NL += i2count[tmp[j]];
			}
		}
	}
	NL /= L*i1idNum;
	return NL;
}

static inline void Bip_core_common_part(struct Bip_recommend_param *args, int *i2id, int *rank, int *topL_i1L, int L) {
	
	int uid = args->i1;
	long uidCount = args->traini1->count[uid];
	int *uidId = args->traini1->id[uid];
	int i2maxId = args->traini2->maxId;
	long *i2count = args->traini2->count;
	double *i2source = args->i2source;

	long i;
	//set selected item's source to -1
	for (i=0; i<uidCount; ++i) {
		i2source[uidId[i]] = -1;
		//i2source[uidId[i]] = 0;
	}
	//set i2id and rank.
	int j=0;
	double dtmp;
	for (i=0; i<i2maxId + 1; ++i) {
		i2id[i] = i;
		//set unexisted item's source to -2.
		if (!i2count[i]) {
			i2source[i] = -2;
		}

		if (i2source[i] > 0 ) {
			dtmp = i2source[j];
			i2source[j] = i2source[i];
			i2source[i] = dtmp;

			i2id[i] = i2id[j];
			i2id[j] = i;

			++j;
			
		}
	}
	//to this step, i2source contains four parts: 
	//1, nomral i2source[x] , which <0, 1].
	//2, i2source[x] = 0, which x isn't selected by anyone has similarity.
	//3, i2source[x] = -1, which x has been selected by i1.
	//4, i2source[x] = -2, which x is the hole, x isn't selected by anyone.
	//
	//after qsort_di_desc, the id of the item with most source will be in i2id[0];
	//qsort_di_desc(i2source, 0, i2maxId, i2id);
	//printf("%d\t%d\n", j, i2maxId);
	qsort_di_desc(i2source, 0, j-1, i2id);
	//copy the top L itemid into topL.
	memcpy(topL_i1L, i2id, L*sizeof(int));
	//get rank;
	for (i=0; i<i2maxId + 1; ++i) {
		rank[i2id[i]] = i+1;
	}
}

//three-step random walk of Probs
static void mass_recommend_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1source[i]) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
	}
}

//three-step random walk of Probs
static void mass_recommend_topR_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
    struct iidNet *userSim = args->userSim;
	int topR = args->mass_topR;

	double * i1source = args->i1source;
	double *i2source = args->i2source;

	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	for (k=0; k<userSim->count[i1]; ++k) {
		i = userSim->edges[i1][k];
		if (k < topR) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}
}

//three-step random walk of Probs
static void mass_recommend_topR5_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
    struct iidNet *userSim = args->userSim;
	int topR5 = args->mass_topR5;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int *i2id = args->i2id;
	int *i1id = args->i1id;

	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	for (k=0; k<userSim->count[i1]; ++k) {
		i = userSim->edges[i1][k];
		if (k < topR5) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}


	int id = 0;
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			i2source[id] = i2source[i];
			i2id[id] = i;
			++id;
		}
	}
	if (id<1) {
		memset(i2source, 0, (i2maxId+1)*sizeof(double));
		return;
	}

	qsort_di_desc(i2source, 0, id -1 , i2id);
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1 && i < topR5 && i<id; ++i) {
		id = i2id[i];
		degree = i2count[id];
		source = (double)i2source[i]/degree;
		for (j=0; j<degree; ++j) {
			neigh = i2ids[id][j];
			i1source[neigh] += source;
		}
	}
	id = 0;
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1source[i]) {
			i1source[id] = i1source[i];
			i1id[id] = i;
			++id;
		}
	}
	if (id<1) {
		memset(i2source, 0, (i2maxId+1)*sizeof(double));
		return;
	}

	qsort_di_desc(i1source, 0, id - 1, i1id);
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1 && i<topR5 && i<id; ++i) {
		id = i1id[i];
		degree = i1count[id];
		source = (double)i1source[i]/degree;
		for (j=0; j<degree; ++j) {
			neigh = i1ids[id][j];
			i2source[neigh] += source;
		}
	}
	/*
	*/
}

//three-step random walk of Probs
static void mass_recommend_topR5RE_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
    struct iidNet *userSim = args->userSim;
	int topR5RE = args->mass_topR5RE;
	double RENBI_param = args->RENBI_param;



	double * i1source = args->i1source;
	double *i2source = args->i2source;
	double *i2sourceA = args->i2sourceA;
	int *i2id = args->i2id;
	int *i1id = args->i1id;

	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	for (k=0; k<userSim->count[i1]; ++k) {
		i = userSim->edges[i1][k];
		if (k < topR5RE) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}

	memcpy(i2sourceA, i2source, (i2maxId + 1)*sizeof(double));

	int id = 0;
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			i2source[id] = i2source[i];
			i2id[id] = i;
			++id;
		}
	}
	if (id<1) {
		memset(i2source, 0, (i2maxId+1)*sizeof(double));
		return;
	}

	qsort_di_desc(i2source, 0, id -1 , i2id);
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1 && i < topR5RE && i<id; ++i) {
		id = i2id[i];
		degree = i2count[id];
		source = (double)i2source[i]/degree;
		for (j=0; j<degree; ++j) {
			neigh = i2ids[id][j];
			i1source[neigh] += source;
		}
	}
	id = 0;
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1source[i]) {
			i1source[id] = i1source[i];
			i1id[id] = i;
			++id;
		}
	}
	if (id<1) {
		memset(i2source, 0, (i2maxId+1)*sizeof(double));
		return;
	}

	qsort_di_desc(i1source, 0, id - 1, i1id);
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1 && i<topR5RE && i<id; ++i) {
		id = i1id[i];
		degree = i1count[id];
		source = (double)i1source[i]/degree;
		for (j=0; j<degree; ++j) {
			neigh = i1ids[id][j];
			i2source[neigh] += source;
		}
	}
	/*
	*/
	for (i=0; i<i2maxId + 1; ++i) {
		i2sourceA[i] += RENBI_param*i2source[i];
	}

	memcpy(i2source, i2sourceA, (i2maxId + 1)*sizeof(double));
}

//three-step random walk of Probs
static void mass_recommend_topS_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
    struct iidNet *userSim = args->userSim;
	double topS = args->mass_topS;

	double * i1source = args->i1source;
	double *i2source = args->i2source;

	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	double sim;
	for (k=0; k<userSim->count[i1]; ++k) {
		i = userSim->edges[i1][k];
		sim = userSim->d3[i1][k];
		if (sim > topS) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}
}

//three-step random walk of Probs
static void mass_recommend_corR_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
    struct iidNet *userSim = args->userSim;
	double corR = args->mass_corR;

	double * i1source = args->i1source;
	double *i2source = args->i2source;

	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	int KI = floor(userSim->count[i1]*corR);
	for (k=0; k<KI; ++k) {
		i = userSim->edges[i1][k];
		degree = i1count[i];
		source = (double)i1source[i]/(double)degree;
		for (j=0; j<degree; ++j) {
			neigh = i1ids[i][j];
			i2source[neigh] += source;
		}
	}
}

//three-step random walk of Probs
static void mass_recommend_corS_Bip(struct Bip_recommend_param *args) {

	int i1 = args->i1;
    struct iidNet *userSim = args->userSim;
	double corS = args->mass_corS;

	double * i1source = args->i1source;
	double *i2source = args->i2source;

	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	double sim;
	if (userSim->count[i1]) {
		double simS = userSim->d3[i1][0] * corS;
		for (k=0; k<userSim->count[i1]; ++k) {
			i = userSim->edges[i1][k];
			sim = userSim->d3[i1][k];
			if (sim > simS) {
				degree = i1count[i];
				source = (double)i1source[i]/(double)degree;
				for (j=0; j<degree; ++j) {
					neigh = i1ids[i][j];
					i2source[neigh] += source;
				}
			}
			else {
				break;
			}
		}
	}
}

//three-step random walk of Probs, bestk cut.
static void mass_recommend_expR_Bip(struct Bip_recommend_param *args) {
	int uid = args->i1;
    struct iidNet *userSim = args->userSim;
	//int mass_topR = args->mass_topR;
	double mass_expR= args->mass_expR;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[uid]; ++j) {
		neigh = i1ids[uid][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	//long KI= floor(args->traini1->idNum * (double)i1count[uid]/args->traini1->countMax*mass_expR);
	long KI= floor((args->traini1->maxId+1) * pow((double)i1count[uid]/((double)args->traini1->countMax), mass_expR));
	//long KI= floor(args->traini1->idNum * pow((double)i1count[uid]*mass_expR_itemAveDre[uid]/((double)args->traini1->countMax*mass_expR_itemAveDreMax), mass_expR));
	//long KI= floor(mass_topR * pow((double)i1count[uid]/((double)args->traini1->countMax), mass_expR));
	//printf("%ld\t%ld\t%f\n", args->traini1->countMax, i1count[uid], pow((double)i1count[uid]/args->traini1->countMax, mass_expR));
	for (k=0; k < userSim->count[uid]; ++k) {
		if ( k<KI) {
			i = userSim->edges[uid][k];
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}
}

//three-step random walk of Probs, bestk cut.
static void mass_recommend_expRI_Bip(struct Bip_recommend_param *args) {
	int uid = args->i1;
    struct iidNet *userSim = args->userSim;
	double mass_expR= args->mass_expR;
	double mass_expRI_itemAveDreMax = args->mass_expRI_itemAveDreMax;
	double *mass_expRI_itemAveDre = args->mass_expRI_itemAveDre;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[uid]; ++j) {
		neigh = i1ids[uid][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	long KI= floor(args->traini1->idNum * pow((double)i1count[uid]*mass_expRI_itemAveDre[uid]/((double)args->traini1->countMax*mass_expRI_itemAveDreMax), mass_expR));
	for (k=0; k < userSim->count[uid]; ++k) {
		if ( k<KI) {
			i = userSim->edges[uid][k];
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}
}

//three-step random walk of Probs, bestk cut.
static void mass_recommend_bestR_Bip(struct Bip_recommend_param *args) {
	int uid = args->i1;
    struct iidNet *userSim = args->userSim;
	int *mass_bestR= args->mass_bestR;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	//one 
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[uid]; ++j) {
		neigh = i1ids[uid][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	long k;
	long KI = mass_bestR[uid];
	if (KI == -1) {
		KI = userSim->count[uid];
	}
	for (k=0; k < userSim->count[uid]; ++k) {
		if ( k<KI) {
			i = userSim->edges[uid][k];
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
		else {
			break;
		}
	}
}

//three-step random walk of heats
static void heats_recommend_Bip(struct Bip_recommend_param *args) {
	int i1 = args->i1;
	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int neigh, i;
	long j;
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1;
	}
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1count[i]) {
			for (j=0; j<i1count[i]; ++j) {
				neigh = i1ids[i][j];
				i1source[i] += i2source[neigh];
			}
			i1source[i] /= i1count[i];
		}
	}
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2count[i]) {
			for (j=0; j<i2count[i]; ++j) {
				neigh = i2ids[i][j];
				i2source[i] += i1source[neigh];
			}
			i2source[i] /= i2count[i];
		}
	}
}

//three-step random walk of HNBI
static void HNBI_recommend_Bip(struct Bip_recommend_param *args) {
	int i1 = args->i1;
	double HNBI_param = args->HNBI_param;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0*pow(i2count[neigh], HNBI_param);
	}
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1source[i]) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
	}
}

//five-step random walk of RENBI
static void RENBI_recommend_Bip(struct Bip_recommend_param *args) {
	int i1 = args->i1;
	double RENBI_param = args->RENBI_param;
	double *i2sourceA = args->i2sourceA;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int i, j, neigh;
	long degree;
	double source;
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	//one
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1.0;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1source[i]) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
	}
	//save three steps result	
	memcpy(i2sourceA, i2source, (i2maxId + 1)*sizeof(double));
	//four
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			degree = i2count[i];
			source = i2source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += source;
			}
		}
	}
	//five
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i1maxId + 1; ++i) {
		if (i1source[i]) {
			degree = i1count[i];
			source = (double)i1source[i]/(double)degree;
			for (j=0; j<degree; ++j) {
				neigh = i1ids[i][j];
				i2source[neigh] += source;
			}
		}
	}
	//now, i2source save w*w result.
	//add: w+RENBI_param*w*w.
	for (i=0; i<i2maxId + 1; ++i) {
		i2sourceA[i] += RENBI_param*i2source[i];
	}

	memcpy(i2source, i2sourceA, (i2maxId + 1)*sizeof(double));
}

//three-step random walk of hybrid
static void hybrid_recommend_Bip(struct Bip_recommend_param *args) {
	int i1 = args->i1;
	double hybrid_param = args->hybrid_param;

	double * i1source = args->i1source;
	double *i2source = args->i2source;
	int **i1ids = args->traini1->id;
	int **i2ids = args->traini2->id; 
	int i1maxId = args->traini1->maxId;
	int i2maxId = args->traini2->maxId;
	long *i1count = args->traini1->count;
	long *i2count = args->traini2->count;

	int neigh, i;
	//double source;
	long j;
	//one
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (j=0; j<i1count[i1]; ++j) {
		neigh = i1ids[i1][j];
		i2source[neigh] = 1;
	}
	//two
	memset(i1source, 0, (i1maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2source[i]) {
			double powl = pow(i2count[i], hybrid_param);
			for (j=0; j<i2count[i]; ++j) {
				neigh = i2ids[i][j];
				i1source[neigh] += i2source[i]/powl;
			}
		}
	}
	//three
	memset(i2source, 0, (i2maxId+1)*sizeof(double));
	for (i=0; i<i2maxId + 1; ++i) {
		if (i2count[i]) {
			double powl = pow(i2count[i], 1-hybrid_param);
			for (j=0; j<i2count[i]; ++j) {
				neigh = i2ids[i][j];
				i2source[i] += i1source[neigh]/i1count[neigh];
			}
			i2source[i] /= powl;
		}
	}
}


/** 
 * core function of recommendation.
 * type :
 * 1  -- mass (NONE arg)
 * 2  -- mass_topR (int mass_topR)
 * 3  -- mass_corK (under development)
 * 4  -- heats (NONE arg)
 * 5  -- HNBI  (double HNBI_param)
 * 6  -- RENBI  (RENBI_param)
 * 7  -- hybrid (hybrid_param)
 *
 * all L is from this function. if you want to change, change the L below.
 */
static Metrics_Bip *recommend_Bip(void (*recommend_core)(struct Bip_recommend_param *), struct Bip_recommend_param *args) {

	int i1maxId      = args->traini1->maxId;
	int i2maxId      = args->traini2->maxId;
	int i1idNum      = args->traini1->idNum;
	int i2idNum      = args->traini2->idNum;
	long *i1count    = args->traini1->count;
	long *i2count    = args->traini2->count;

	struct iidNet *itemSim = args->itemSim;

 	// all L is from this function. if you want to change, change the L below.
	int L = 50;

	double R, PL, HL, IL, NL;
	R=PL=HL=IL=NL=0;

	double *i1source = malloc((i1maxId + 1)*sizeof(double));
	assert(i1source != NULL);
	double *i2source = malloc((i2maxId + 1)*sizeof(double));
	assert(i2source != NULL);
	args->i1source = i1source;
	args->i2source = i2source;

	double *i2sourceA = malloc((i2maxId + 1)*sizeof(double));
	assert(i2sourceA != NULL);
	args->i2sourceA = i2sourceA;

	int *rank = malloc((i2maxId + 1)*sizeof(int));
	assert(rank != NULL);
	int *i1id =  malloc((i1maxId + 1)*sizeof(int));
	assert(i1id != NULL);
	int *i2id =  malloc((i2maxId + 1)*sizeof(int));
	assert(i2id != NULL);
	args->i1id = i1id;
	args->i2id = i2id;

	int i;
	int *topL = calloc(L*(i1maxId + 1), sizeof(int));
	assert(topL != NULL);

	for (i = 0; i<i1maxId + 1; ++i) { //each user
		//if (i%1000 ==0) {printf("%d\n", i);fflush(stdout);}
		//only compute the i in both i1 and test.
		if (i1count[i]) {
			//get rank
			args->i1 = i;
			recommend_core(args);
			Bip_core_common_part(args, i2id, rank, topL + i*L, L);
			metrics_R_PL_Bip(i, i1count, i2idNum, args->testi1, L, rank, &R, &PL);

		}
		//printf("%d\t", i);fflush(stdout);
	}

	R /= args->testi1->edgesNum;
	PL /= args->testi1->idNum;
	HL = metrics_HL_Bip(i1maxId, i1count, i2maxId, L, topL);
	IL = metrics_IL_Bip(i1maxId, i1count, i1idNum, i2maxId, L, topL, itemSim);
	NL = metrics_NL_Bip(i1maxId, i1count, i1idNum, i2count, L, topL);
	
	Metrics_Bip *retn = create_MetricsBip();
	retn->R = R;
	retn->PL = PL;
	retn->HL = HL;
	retn->IL = IL;
	retn->NL = NL;
	retn->topL = topL;
	retn->L = L;

	//printf("hybrid:\tR: %f, PL: %f, IL: %f, HL: %f, NL: %f\n", R, PL, IL, HL, NL);
	free(i1source);
	free(i2source);
	free(i2sourceA);
	free(i1id);
	free(i2id);
	free(rank);
	return retn;
}

/************************************************************************************************************/
/************************* the following functions are related with BIP2. ***********************************/
/** include free, create, divide, verify, similarity. *******************************************************/
/** include 8 recommend algorithm. **************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/

struct Metrics_Bipii *create_MetricsBipii(void) {
	struct Metrics_Bipii *lp = malloc(sizeof(struct Metrics_Bipii));
	assert(lp != NULL);
	lp->R = 0;
	lp->PL = 0;
	lp->HL = 0;
	lp->IL = 0;
	lp->NL = 0;
	lp->L = 0;
	lp->topL = NULL;
	return lp;
}
//free(NULL) is ok.
void clean_MetricsBipii(struct Metrics_Bipii *lp) {
	lp->R = 0;
	lp->PL = 0;
	lp->HL = 0;
	lp->IL = 0;
	lp->NL = 0;
	lp->L = 0;
	free(lp->topL);
	lp->topL = NULL;
}
void free_MetricsBipii(struct Metrics_Bipii *lp) {
	free(lp->topL);
	free(lp);
}

void free_Bipii(struct Bipii *Bip) {
	int i=0;
	for(i=0; i<Bip->maxId+1; ++i) {
		if (Bip->count[i]>0) {
			free(Bip->id[i]);
		}
	}
	free(Bip->count);
	free(Bip->id);
	free(Bip);
}

//kind of simple, just create struct Bipii.
//i1toi2 is 1, i1 of struct iiLineFile will be the index.
//i1toi2 is 0, i2 of struct iiLineFile will be the index.
struct Bipii *create_Bipii(const struct iiLineFile * const file, int i1toi2) {
	//all elements of struct Bipii.
	int maxId;
	int minId;
	int idNum=0;
	long countMax=-1;
	long countMin=LONG_MAX;
	long *count;
	int **id;
	long edgesNum;

	//get maxId of i1 or i2.
	if (i1toi2) {
		maxId=file->i1Max;
		minId=file->i1Min;
	}
	else {
		maxId = file->i2Max;
		minId = file->i2Min;
	}


	//once get maxId, the four points can be assigned with memory.
	count=calloc(maxId+1, sizeof(long));
	assert(count!=NULL);
	id=malloc((maxId+1)*sizeof(void *));
	assert(id!=NULL);

	//fill the count.
	edgesNum=file->linesNum;
	struct iiLine *lines=file->lines;
	long i;
	if (i1toi2) {
		for(i=0; i<edgesNum; ++i) {
			++count[lines[i].i1];
		}
	}
	else {
		for(i=0; i<edgesNum; ++i) {
			++count[lines[i].i2];
		}
	}


	//once get count, the three points which point to point can be assigned with memory.
	int j;
	for(j=0; j<maxId+1; ++j) {
		if (count[j]>0) {
			countMax = countMax>count[j]?countMax:count[j];
			countMin = countMin<count[j]?countMin:count[j];
			++idNum;
			id[j]=malloc(count[j]*sizeof(int));
			assert(id[j]!=NULL);
		}
		else {
			id[j] = NULL;
		}
	}

	//fill id, i3, i4
	long *temp = calloc(maxId+1, sizeof(long));
	assert(temp!=NULL);
	if (i1toi2) {
		for(i=0; i<edgesNum; ++i) {
			int i1 =lines[i].i1;
			id[i1][temp[i1]]=lines[i].i2;
			++temp[i1];
		}
	}
	else {
		for(i=0; i<edgesNum; ++i) {
			int i2 =lines[i].i2;
			id[i2][temp[i2]]=lines[i].i1;
			++temp[i2];
		}
	}
	free(temp);

	//fill Bip and return.
	struct Bipii *Bip = malloc(sizeof(struct Bipii));
	assert(Bip != NULL);

	Bip->maxId=maxId;
	Bip->minId=minId;
	Bip->idNum=idNum;
	Bip->countMax = countMax;
	Bip->countMin = countMin;
	Bip->count=count;
	Bip->id = id;
	Bip->edgesNum=edgesNum;

	if (i1toi2) {
		printf("build i1->i2 net:\n\tMax: %d, Min: %d, Num: %d, countMax: %ld, countMin: %ld, edgesNum: %ld\n", maxId, minId, idNum, countMax, countMin, edgesNum); fflush(stdout);
	}
	else {
		printf("build i2->i1 net:\n\tMax: %d, Min: %d, Num: %d, countMax: %ld, countMin: %ld, edgesNum: %ld\n", maxId, minId, idNum, countMax, countMin, edgesNum); fflush(stdout);
	}

	return Bip;
}

//divide Bipii into two parts.
//return two struct iiLineFile. 
//the first one is always the small one.
//the second is always the large one.
void divide_Bipii(struct Bipii *bipi1, struct Bipii *bipi2, double rate, struct iiLineFile **small, struct iiLineFile **big) {
	if (rate <=0 || rate >= 1) {
		isError("divide_Bipii error: wrong rate.\n");
	}
	rate = rate>0.5?1-rate:rate;
	long l1, l2;
	if (bipi1->edgesNum > 100000) {
		l1 = (int)(bipi1->edgesNum*(rate+0.1));
		l2 = (int)(bipi1->edgesNum*(1-rate+0.1));
	}
	else {
		l2 = l1 = bipi1->edgesNum;
	}

	*small = malloc(sizeof(struct iiLineFile));
	assert(*small != NULL);
	*big = malloc(sizeof(struct iiLineFile));
	assert(*big != NULL);

	(*small)->lines = malloc(l1*sizeof(struct iiLine));
	assert((*small)->lines != NULL);
	(*big)->lines = malloc(l2*sizeof(struct iiLine));
	assert((*big)->lines != NULL);

	long line1=0, line2=0;
	int i1Max=-1; 
	int i2Max=-1;
    int i1Min=INT_MAX;
    int i2Min=INT_MAX;
	int _i1Max=-1; 
	int _i2Max=-1;
    int _i1Min=INT_MAX;
    int _i2Min=INT_MAX;

	char *i1sign = calloc(bipi1->maxId + 1, sizeof(char));
	assert(i1sign);
	char *i2sign = calloc(bipi2->maxId + 1, sizeof(char));
	assert(i2sign);

	long *counti1 = malloc((bipi1->maxId + 1)*sizeof(long));
	assert(counti1 != NULL);
	memcpy(counti1, bipi1->count, (bipi1->maxId + 1)*sizeof(long));
	long *counti2 = malloc((bipi2->maxId + 1)*sizeof(long));
	assert(counti2 != NULL);
	memcpy(counti2, bipi2->count, (bipi2->maxId + 1)*sizeof(long));

	int i, neigh;
	long j;
	for (i=0; i<bipi1->maxId + 1; ++i) {
		for (j=0; j<bipi1->count[i]; ++j) {
			neigh = bipi1->id[i][j];
			if (genrand_real1() < rate) {
				if ((counti1[i] == 1 && i1sign[i] == 0) || (counti2[neigh] == 1 && i2sign[neigh] == 0)) {
					(*big)->lines[line2].i1 = i;	
					(*big)->lines[line2].i2 = neigh;	
					--counti1[i];
					--counti2[neigh];
					i1sign[i] = 1;
					i2sign[neigh] = 1;
					_i1Max = _i1Max>i?_i1Max:i;
					_i2Max = _i2Max>neigh?_i2Max:neigh;
					_i1Min = _i1Min<i?_i1Min:i;
					_i2Min = _i2Min<neigh?_i2Min:neigh;
					++line2;
					continue;
				}
				(*small)->lines[line1].i1 = i;	
				(*small)->lines[line1].i2 = neigh;	
				--counti1[i];
				--counti2[neigh];
				i1Max = i1Max>i?i1Max:i;
				i2Max = i2Max>neigh?i2Max:neigh;
				i1Min = i1Min<i?i1Min:i;
				i2Min = i2Min<neigh?i2Min:neigh;
				++line1;
			}
			else {
				(*big)->lines[line2].i1 = i;	
				(*big)->lines[line2].i2 = neigh;	
				i1sign[i] = 1;
				i2sign[neigh] = 1;
				--counti1[i];
				--counti2[neigh];
				_i1Max = _i1Max>i?_i1Max:i;
				_i2Max = _i2Max>neigh?_i2Max:neigh;
				_i1Min = _i1Min<i?_i1Min:i;
				_i2Min = _i2Min<neigh?_i2Min:neigh;
				++line2;
			}
		}
	}
	if ((line1 > l1) && (line2 > l2)) {
		isError("divide_Bipii: l1 and l2 two small.\n");
	}

	free(i1sign);
	free(i2sign);
	free(counti1);
	free(counti2);

	(*small)->linesNum = line1;
	(*small)->i1Max = i1Max;
	(*small)->i2Max = i2Max;
	(*small)->i1Min = i1Min;
	(*small)->i2Min = i2Min;

	(*big)->linesNum = line2;
	(*big)->i1Max = _i1Max;
	(*big)->i2Max = _i2Max;
	(*big)->i1Min = _i1Min;
	(*big)->i2Min = _i2Min;
	printf("divide_Bipii done:\n\trate: %f\n\tfile1: linesNum: %ld, i1Max: %d, i1Min: %d, i2Max: %d, i2Min: %d\n\tfile2: linesNum: %ld, i1Max: %d, i1Min: %d, i2Max: %d, i2Min: %d\n", rate, line1, i1Max, i1Min, i2Max, i2Min, line2, _i1Max, _i1Min, _i2Max, _i2Min);fflush(stdout);
}

struct Bipii * clone_Bipii(struct Bipii *bip) {
	struct Bipii *new = malloc(sizeof(struct Bipii));
	new->count = malloc((bip->maxId + 1)*sizeof(long));
	new->id = malloc((bip->maxId + 1)*sizeof(void *));
	memcpy(new->count, bip->count, (bip->maxId + 1)*sizeof(long));
	int i;
	for (i=0; i<bip->maxId +1 ; ++i) {
		if (new->count[i]) {
			new->id[i] = malloc(new->count[i]*sizeof(int));
			memcpy(new->id[i], bip->id[i], new->count[i]*sizeof(int));
		}
		else {
			new->id[i] = NULL;
		}
	}

	new->maxId = bip->maxId;
	new->minId = bip->minId;
	new->idNum = bip->idNum;
	new->countMax = bip->countMax;
	new->countMin = bip->countMin;
	new->edgesNum = bip->edgesNum;

	return new;
}

void *verifyBipii(struct Bipii *bipi1, struct Bipii *bipi2) {
	long i;
	int j,k;
	int *place = malloc((bipi2->maxId+1)*sizeof(int));
	FILE *fp = fopen("data/duplicatePairsinNet", "w");
	fileError(fp, "data/duplicatePairsinNet");
	FILE *fp2 = fopen("data/NoDuplicatePairsNetFile", "w");
	fileError(fp2, "data/NoDuplicatePairsNetFile");
	fprintf(fp, "the following pairs are duplicate in the net file\n");
	char sign=0;
	for (j=0; j<bipi1->maxId+1; ++j) {
		if (bipi1->count[j]>0) {
			for (k=0; k<bipi2->maxId + 1; ++k) {
				place[k] = -1;
			}
			for (i=0; i<bipi1->count[j]; ++i) {
				int origin = bipi1->id[j][i];
				int next = place[origin];
				if (next == -1) {
					place[origin]=origin;
					fprintf(fp2, "%d\t%d\n", j,origin);
				}
				else {
					fprintf(fp, "%d\t%d\n", j, next);
					sign=1;
				}
			}
		}
		if (j%10000 == 0) {
			printf("%d\n", j);
			fflush(stdout);
		}
	}
	free(place);
	fclose(fp);
	fclose(fp2);
	if (sign == 1) {
		isError("the file has duplicate pairs, you can check data/duplicatePairsinNet.\nwe generate a net file named data/NoDuplicatePairsNetFile which doesn't contain any duplicate pairsr.\nyou should use this file instead the origin wrong one.\n");
	}
	else {
		printf("verifyBipii: perfect network.\n");
	}
	return (void *)0;
}

//if i1ori2 == 1, then calculate i1(user)'s similarity.
//if i1ori2 == 0, then calculate i2(item)'s similarity.
struct iidLineFile *similarity_Bipii(struct Bipii *bipi1, struct Bipii *bipi2, int i1ori2) {
	int idmax, idmax2;
	long *count;
	int **id;
	if (i1ori2 == 1) {
		idmax = bipi1->maxId;
		idmax2 = bipi2->maxId;
		count = bipi1->count;
		id = bipi1->id;
	}
	else if (i1ori2 == 0) {
		idmax = bipi2->maxId;
		idmax2 = bipi1->maxId;
		count = bipi2->count;
		id = bipi2->id;
	}
	else {
		isError("similarity_Bipii");
	}
	int *sign = calloc((idmax2 + 1),sizeof(int));
	assert(sign != NULL);

	struct iidLineFile *simfile = malloc(sizeof(struct iidLineFile));
	assert(simfile != NULL);

	int con = 1000000;
	struct iidLine *lines = malloc(con*sizeof(struct iidLine));
	assert(lines != NULL);
	long linesNum = 0;

	int i1Max=-1;
	int i1Min=INT_MAX;
	int i2Max=-1;
	int i2Min=INT_MAX;

	long k;
	int Sij;
	double soij;
	int i,j;
	for (i=0; i<idmax; ++i) {
		if (count[i]) {
			memset(sign, 0, (idmax2 + 1)*sizeof(int));
			for (k=0; k<count[i]; ++k) {
				sign[id[i][k]] = 1;
			}
			for (j = i+1; j<idmax + 1; ++j) {
				if (count[j]) {
					Sij = 0;
					for (k=0; k<count[j]; ++k) {
						Sij += sign[id[j][k]];
					}
					if (Sij) {
						soij = (double)Sij/pow(count[i] * count[j], 0.5);
						//fprintf(fp, "%d, %d, %.17f\n", i, j, soij);
						i1Min = i1Min<i?i1Min:i;
						i1Max = i;
						i2Min = i2Min<j?i2Min:j;
						i2Max = i2Max>j?i2Max:j;
						lines[linesNum].i1 = i;
						lines[linesNum].i2 = j;
						lines[linesNum].d3 = soij;
						++linesNum;
						if (linesNum == con) {
							con += 1000000;
							struct iidLine *temp = realloc(lines, con*sizeof(struct iidLine));
							assert(temp != NULL);
							lines = temp;
						}
					}
				}
			}
		}
	}

	free(sign);

	simfile->i1Max = i1Max;
	simfile->i2Max = i2Max;
	simfile->i1Min = i1Min;
	simfile->i2Min = i2Min;
	simfile->linesNum = linesNum;
	simfile->lines = lines;
	printf("calculate similarity done.\n");
	return simfile;
}

//if i1ori2 == 1, then calculate i1(user)'s similarity.
//if i1ori2 == 0, then calculate i2(item)'s similarity.
struct iidLineFile *mass_similarity_Bipii(struct Bipii *bipi1, struct Bipii *bipi2) {


	struct iidLineFile *simfile = malloc(sizeof(struct iidLineFile));
	int con = 1000000;
	struct iidLine *lines = malloc(con*sizeof(struct iidLine));
	long linesNum = 0;

	double *i2source = malloc((bipi2->maxId + 1)*sizeof(double));
	double *i1source = malloc((bipi1->maxId + 1)*sizeof(double));

	int i, j, neigh;
	long degree;
	double source;

	int i1Max=-1;
	int i1Min=INT_MAX;
	int i2Max=-1;
	int i2Min=INT_MAX;

	int k;
	for (k=0; k<bipi1->maxId + 1; ++k) {
		if (bipi1->count[k]) {
			memset(i2source, 0, (bipi2->maxId+1)*sizeof(double));
			for (j=0; j<bipi1->count[k]; ++j) {
				neigh = bipi1->id[k][j];
				i2source[neigh] = 1.0;
			}
			memset(i1source, 0, (bipi1->maxId+1)*sizeof(double));
			for (i=0; i<bipi2->maxId + 1; ++i) {
				if (i2source[i]) {
					degree = bipi2->count[i];
					source = i2source[i]/(double)degree;
					for (j=0; j<degree; ++j) {
						neigh = bipi2->id[i][j];
						i1source[neigh] += source;
					}
				}
			}
			for (i=0; i<bipi1->maxId + 1; ++i) {
				if (i1source[i] && i!=k) {
					i1Min = i1Min<k?i1Min:k;
					i1Max = k;

					i2Min = i2Min<i?i2Min:i;
					i2Max = i2Max>i?i2Max:i;

					lines[linesNum].i1 = k;
					lines[linesNum].i2 = i;
					lines[linesNum].d3 = i1source[i];
					++linesNum;

					if (linesNum == con) {
						con += 1000000;
						struct iidLine *temp = realloc(lines, con*sizeof(struct iidLine));
						assert(temp != NULL);
						lines = temp;
					}
				}
			}
		}
	}

	simfile->i1Max = i1Max;
	simfile->i2Max = i2Max;
	simfile->i1Min = i1Min;
	simfile->i2Min = i2Min;
	simfile->linesNum = linesNum;
	simfile->lines = lines;
	printf("calculate mass similarity done.\n");
	free(i1source);
	free(i2source);
	return simfile;
}

struct Metrics_Bipii *mass_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim) {
	struct Bip_recommend_param param;
	param.itemSim = itemSim;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_Bip, &param);
}

struct Metrics_Bipii *mass_topR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, int mass_topR) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_topR = mass_topR;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_topR_Bip, &param);
}

struct Metrics_Bipii *mass_topR5_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, int mass_topR5) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_topR5 = mass_topR5;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_topR5_Bip, &param);
}

struct Metrics_Bipii *mass_topR5RE_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, int mass_topR5RE, double RENBI_param) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_topR5RE = mass_topR5RE;
	param.RENBI_param = RENBI_param;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_topR5RE_Bip, &param);
}

struct Metrics_Bipii *mass_topS_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, double mass_topS) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_topS = mass_topS;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_topS_Bip, &param);
}

struct Metrics_Bipii *mass_corR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, double mass_corR) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_corR = mass_corR;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_corR_Bip, &param);
}

struct Metrics_Bipii *mass_corS_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, double mass_corS) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_corS = mass_corS;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_corS_Bip, &param);
}

struct Metrics_Bipii *mass_expR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, double mass_expR) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_expR= mass_expR;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_expR_Bip, &param);
}

struct Metrics_Bipii *mass_expRI_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, double mass_expR, double *mass_expRI_itemAveDre, double mass_expRI_itemAveDreMax) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_expR= mass_expR;
	param.mass_expRI_itemAveDreMax = mass_expRI_itemAveDreMax;
	param.mass_expRI_itemAveDre = mass_expRI_itemAveDre;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_expRI_Bip, &param);
}

struct Metrics_Bipii *mass_bestR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, struct iidNet *userSim, int *mass_bestR) {
	struct Bip_recommend_param param;
	param.userSim = userSim;
	param.itemSim = itemSim;
	param.mass_bestR= mass_bestR;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(mass_recommend_bestR_Bip, &param);
}

struct Metrics_Bipii *heats_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim) {
	struct Bip_recommend_param param;
	param.itemSim = itemSim;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(heats_recommend_Bip, &param);
}

struct Metrics_Bipii *HNBI_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, double HNBI_param) {
	struct Bip_recommend_param param;
	param.itemSim = itemSim;
	param.HNBI_param = HNBI_param;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(HNBI_recommend_Bip, &param);
}

struct Metrics_Bipii *RENBI_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, double RENBI_param) {
	struct Bip_recommend_param param;
	param.itemSim = itemSim;
	param.RENBI_param = RENBI_param;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(RENBI_recommend_Bip, &param);
}

struct Metrics_Bipii *hybrid_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *itemSim, double hybrid_param) {
	struct Bip_recommend_param param;
	param.itemSim = itemSim;
	param.hybrid_param = hybrid_param;

	param.traini1 = traini1;
	param.traini2 = traini2;
	param.testi1 = testi1;

	return recommend_Bip(hybrid_recommend_Bip, &param);
}

int *mass_GetBestR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *userSim) {
	printf("get bestR begin......");fflush(stdout);

	double *i1source = malloc((traini1->maxId + 1)*sizeof(double));
	double *i2source = malloc((traini2->maxId + 1)*sizeof(double));
	int *rank = malloc((traini2->maxId + 1)*sizeof(int));
	int *i2id =  malloc((traini2->maxId + 1)*sizeof(int));

	double R, PL;

	struct Bip_recommend_param args;
	args.traini1 = traini1;
	args.traini2 = traini2;
	args.testi1 = testi1;
	args.i1source = i1source;
	args.i2source = i2source;
	args.userSim = userSim;

	int i;
    int j;
	int L = 50;
	int *topL = calloc(L*(traini1->maxId + 1), sizeof(int));
	double bestR;
	int bestRK;
	int *bestK = malloc((traini1->maxId + 1)*sizeof(int));
	double realR = 0, realR2 = 0;
	for (i = 0; i<traini1->maxId + 1; ++i) { //each user
		if (i<testi1->maxId + 1 && testi1->count[i]) {
			//just to make sure bestR is enough big.
			bestR = LONG_MAX;
			bestRK = -1;
			//to make realR2 is same with mass.
			for (j=1; j<= userSim->count[i] || (j ==1 && userSim->count[i] == 0); ++j) {

				//probs_knn_Bip_core(i, &args, userSim, j);
				args.i1 = i;
				args.mass_topR = j;
				mass_recommend_topR_Bip(&args);

				
				Bip_core_common_part(&args, i2id, rank, topL + i*L, L);

				R=PL=0;
				metrics_R_PL_Bip(i, traini1->count, traini2->idNum, testi1, L, rank, &R, &PL);
				//R will never be 0, because i is in testi1.
				if (bestR > R) {
					bestR = R;
					bestRK = j;
				}
			}
			realR += bestR;
			realR2 += R;

			bestK[i] = bestRK;

		}
		else {
			bestK[i] = -1;
		}
	}
	printf("%f, %f\n", realR/testi1->edgesNum, realR2/testi1->edgesNum);fflush(stdout);

	free(i1source);
	free(i2source);
	free(rank);
	free(i2id);
	printf("get bestR done.\n\n");fflush(stdout);
	return bestK;
}

int *mass_GetNR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *userSim, int N) {
	printf("get NR begin......\n");fflush(stdout);

	int *NR = malloc((traini1->maxId + 1)*sizeof(int));

	/**********************************************************************************/
	int *NACcount = malloc((traini1->maxId + 1)*sizeof(int));
	int *NACuid = malloc((traini1->maxId + 1)*sizeof(int));
	int i;
	for (i=0; i<traini1->maxId + 1; ++i) {
		NACcount[i] = traini1->count[i];
		NACuid[i] = i;
	}
	qsort_ii_desc(NACcount, 0, traini1->maxId, NACuid);
	int Length = ceil((double)(traini1->maxId + 1)/N);
	/**********************************************************************************/


	/**********************************************************************************/
	double *i1source = malloc((traini1->maxId + 1)*sizeof(double));
	double *i2source = malloc((traini2->maxId + 1)*sizeof(double));
	int *rank = malloc((traini2->maxId + 1)*sizeof(int));
	int *i2id =  malloc((traini2->maxId + 1)*sizeof(int));
	double R, PL;
	struct Bip_recommend_param args;
	args.traini1 = traini1;
	args.traini2 = traini2;
	args.testi1 = testi1;
	args.i1source = i1source;
	args.i2source = i2source;
	args.userSim = userSim;
	/**********************************************************************************/

	int L = 50;
	int *topL = calloc(L*(traini1->maxId + 1), sizeof(int));

	int n,j,k;
	for (n=0; n<N; ++n) {

		int begin = Length*n;
		int end = Length*(n + 1);
		if (begin > traini1->maxId) break; 
		int edNum = 0;
		double bestR = LONG_MAX;
		int bestK = -1;

		for (j = 1; j< userSim->countMax; ++j ) {
			args.mass_topR = j;
			R=0;
			edNum = 0;
			for (k=begin; k<end && k<traini1->maxId + 1; ++k) {	
				i = NACuid[k];
				if (i<testi1->maxId + 1 && testi1->count[i]) {
					args.i1 = i;
					mass_recommend_topR_Bip(&args);
					Bip_core_common_part(&args, i2id, rank, topL + i*L, L);
					double r = 0;
					metrics_R_PL_Bip(i, traini1->count, traini2->idNum, testi1, L, rank, &r, &PL);
					R += r;
					edNum += testi1->count[i];
				}
			}
			//if R == 0, only two reason:
			//   1, traini1->count[i] = 0.
			//   2, testi1->count[i] = 0.
			if (R != 0 && bestR > R) {
				bestR = R;
				bestK = j;
			}
			//printf("%d, %f\n", j, R/testi1->edgesNum);fflush(stdout);
		}
		for (k=begin; k<end && k<traini1->maxId + 1; ++k) {	
			i = NACuid[k];
			NR[i] = bestK;
		}
		//NR[n] = bestK;
		if (bestK != -1) {
			printf("N:%d, User: %d, NR: %d, RS: %f,edNum: %d \n", n, NACuid[begin], bestK, bestR/edNum, edNum);fflush(stdout);
		}
	}
	
	free(i1source);
	free(i2source);
	free(rank);
	free(i2id);
	free(topL);
	free(NACcount);
	free(NACuid);

	printf("get NR done.\n\n");fflush(stdout);
	return NR;
}


int mass_GetTopR_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *userSim) {
	printf("get TopR begin......\n");fflush(stdout);

	double *i1source = malloc((traini1->maxId + 1)*sizeof(double));
	double *i2source = malloc((traini2->maxId + 1)*sizeof(double));
	int *rank = malloc((traini2->maxId + 1)*sizeof(int));
	int *i2id =  malloc((traini2->maxId + 1)*sizeof(int));

	double R, PL;

	struct Bip_recommend_param args;
	args.traini1 = traini1;
	args.traini2 = traini2;
	args.testi1 = testi1;
	args.i1source = i1source;
	args.i2source = i2source;
	args.userSim = userSim;

	int i;
    int j;
	int L = 50;
	double bestR = LONG_MAX;
	int bestK = -1;
	int *topL = calloc(L*(traini1->maxId + 1), sizeof(int));
	for (j = 1; j< userSim->countMax; ++j ) {
	//for (j = 1; j< 300; ++j ) {
		args.mass_topR = j;
		R=0;
		for (i = 0; i<traini1->maxId + 1; ++i) { //each user
			if (i<testi1->maxId + 1 && testi1->count[i]) {
				args.i1 = i;
				mass_recommend_topR_Bip(&args);
				Bip_core_common_part(&args, i2id, rank, topL + i*L, L);
				double r = 0;
				metrics_R_PL_Bip(i, traini1->count, traini2->idNum, testi1, L, rank, &r, &PL);
				R += r;
			}
		}
		if (R != 0 && bestR > R) {
			bestR = R;
			bestK = j;
		}

		//printf("%d, %f\n", j, R/testi1->edgesNum);fflush(stdout);
	}

	printf("best topR: %d, RS: %f\n", bestK, bestR/testi1->edgesNum);
	
	free(i1source);
	free(i2source);
	free(rank);
	free(i2id);
	free(topL);
	printf("get TopR done.\n\n");fflush(stdout);
	return bestK;
}

void experiment_knn_Bipii(struct Bipii *traini1, struct Bipii *traini2, struct Bipii *testi1, struct Bipii *testi2, struct iidNet *userSim) {
	printf("\nexperiment_knn_Bipii begin....\n");fflush(stdout);

	double *i1source = malloc((traini1->maxId + 1)*sizeof(double));
	double *i2source = malloc((traini2->maxId + 1)*sizeof(double));
	int *rank = malloc((traini2->maxId + 1)*sizeof(int));
	int *i2id =  malloc((traini2->maxId + 1)*sizeof(int));

	double R, PL;

	struct Bip_recommend_param args;
	args.traini1 = traini1;
	args.traini2 = traini2;
	args.testi1 = testi1;
	args.i1source = i1source;
	args.i2source = i2source;
	args.userSim = userSim;

	int i;
    int j;
	int L = 50;
	int *topL = calloc(L*(traini1->maxId + 1), sizeof(int));
	double bestR;
	int bestRK;
	double realR = 0, realR2 = 0;
	for (i = 0; i<traini1->maxId + 1; ++i) { //each user
		if (i<testi1->maxId + 1 && testi1->count[i]) {
			//just to make sure bestR is enough big.
			bestR = LONG_MAX;
			bestRK = -1;
			//to make realR2 is same with mass.
			for (j=1; j<= userSim->count[i] || (j ==1 && userSim->count[i] == 0); ++j) {

				//probs_knn_Bip_core(i, &args, userSim, j);
				args.i1 = i;
				args.mass_topR = j;
				mass_recommend_topR_Bip(&args);

				
				Bip_core_common_part(&args, i2id, rank, topL + i*L, L);

				R=PL=0;
				metrics_R_PL_Bip(i, traini1->count, traini2->idNum, testi1, L, rank, &R, &PL);
				//R will never be 0, because i is in testi1.
				if (bestR > R) {
					bestR = R;
					bestRK = j;
				}
			}
			realR += bestR;
			realR2 += R;
			//printf("%d\t%f\t%ld\t%ld\n", i, R, traini1->count[i], testi1->count[i]);fflush(stdout);

			//int k;
			//long aveitemdegree=0;
			//for (k=0; k<traini1->count[i]; ++k) {
			//	aveitemdegree += traini2->count[traini1->id[i][k]];	
			//}
			//double avei = (double)aveitemdegree/traini1->count[i];

			if (userSim->count[i] && i%1 == 0) {
				//printf("%d\t%d\t%ld\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", i, bestRK, userSim->count[i], bestR, R, R-bestR, userSim->d3[i][0], userSim->d3[i][bestRK-1], userSim->d3[i][j-2], userSim->d3[i][bestRK-1]/userSim->d3[i][0]);fflush(stdout);
				//printf("%d\t%d\t%ld\t%ld\t%f\t%f\t%f\t%f\t%f\n", i, bestRK, userSim->count[i], traini1->count[i], avei, bestR, R, bestR/traini1->count[i], R/traini1->count[i]);fflush(stdout);
				printf("%d\t%d\t%f\t%f\t%f\t%f\t%f\n", i, bestRK, userSim->d3[i][bestRK-1], (double)traini1->count[i]/traini1->countMax, userSim->d3[i][bestRK-1]/userSim->d3[i][0], (double)bestRK/userSim->count[i], userSim->d3[i][0]);fflush(stdout);
			}
			else {
			//	printf("xxxxxxxxxxxxx\n");
			}
		}
	}
	printf("%f, %f\n", realR/testi1->edgesNum, realR2/testi1->edgesNum);fflush(stdout);

	free(i1source);
	free(i2source);
	free(rank);
	free(i2id);
	printf("calculat best knn done.\n\n");fflush(stdout);
}

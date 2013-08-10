#ifndef COMPLEXNET_RANDOM_H
#define COMPLEXNET_RANDOM_H

//#include <stdio.h>
/* Period parameters */  
#define N_MersenneTwister 624
#define M_MersenneTwister 397
#define MATRIX_A_MersenneTwister 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK_MersenneTwister 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK_MersenneTwister 0x7fffffffUL /* least significant r bits */


void init_genrand_MersenneTwister(unsigned long s);
void init_by_array_MersenneTwister(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);
long genrand_int31(void);
double genrand_real1(void);
double genrand_real2(void);
double genrand_real3(void);
double genrand_res53(void);

#endif

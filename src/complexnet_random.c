//modified by rui. 2013.08.09
#include "../inc/complexnet_random.h"

static unsigned long mt_MersenneTwister[N_MersenneTwister]={
2147483648, 1827812183, 1371430253, 3559376401, 4152304030, 3484546413, 904688886, 2640105624, 3033298696, 3386595201, 369478181, 2269089429, 618892536, 1404079710, 2891471128, 2871169702, 1020385029, 3311824836, 923090660, 1921041923, 1544466474, 685797265, 2465597578, 2346973074, 630894856, 665182002, 769175936, 3017123521, 3508859727, 1803296144, 2059566241, 4053644259, 3993140148, 741891261, 54278865, 20043431, 2445648452, 1044381999, 1773042707, 2768959228, 1818875112, 2035786986, 3458871676, 4190322778, 1552412821, 3743407409, 921020090, 3028219647, 1811282874, 458436201, 2261805142, 3540504881, 55523394, 3406035835, 3643554219, 160478601, 3131812209, 1750246847, 310524801, 538838761, 4097216171, 1424467795, 1736964303, 3924877158, 4050066274, 1284818847, 997608279, 1743645731, 3861638089, 1979190831, 331968179, 67771207, 937697508, 3340056593, 752755706, 1581536688, 2292813118, 822270257, 3518209714, 3540339067, 1356302093, 564364160, 3439306709, 4156257291, 2902329296, 2676847202, 199377087, 2700614214, 4256495779, 1042379751, 1902843850, 541663347, 1105719791, 2055488122, 1772382387, 1590461887, 1096731286, 4039442187, 1175370089, 1659062698, 53134572, 2821322629, 3865246561, 1928684207, 2238726924, 3291877865, 2486983120, 2591476417, 2633030347, 4263927136, 3370274455, 3018969306, 3342536519, 1048809092, 3887576647, 1798999130, 1882276042, 2314104910, 3303817434, 2251178951, 1354109516, 2944211425, 542518603, 3545156308, 292048272, 1667455762, 1824881537, 2291719689, 2601744913, 1475382386, 2151236187, 3551355316, 207179492, 4025262836, 4201541508, 4185568575, 1371095840, 2533214650, 750598898, 3336127010, 1112746635, 2168833184, 1866663264, 1008882524, 705705152, 434584641, 3720585473, 1359539617, 1186019465, 2997504282, 3279906530, 4174236237, 1111392757, 1874728325, 3994133610, 855552173, 1490712571, 770322226, 3509743848, 3097776221, 1669989606, 2981948555, 3420675648, 763522569, 1854726402, 1446282084, 4210797998, 3497860296, 1500567652, 4170419592, 375681337, 1685467682, 3252917671, 204735328, 725724149, 86204629, 3407749819, 1497038849, 1240663743, 1497563373, 330463645, 678281313, 555587055, 3420977290, 3864978040, 2598258963, 88433849, 706471606, 2204253466, 1207243547, 479433004, 2525229949, 3011716495, 3622461821, 2931310287, 994916916, 3209595751, 3887340275, 1441571561, 2496624432, 3592967859, 2170622192, 3964102663, 1750936864, 4286251765, 2410873793, 4056435468, 3778832465, 3568403537, 3195560039, 2447945345, 2289664522, 3125246309, 3254932065, 3412281324, 1070323682, 1454196965, 1242606593, 2025763526, 2480282382, 1785523412, 1678120983, 3466302448, 913688339, 2335682621, 2870884017, 3943203012, 3376940353, 821708821, 5329814, 2050965739, 4012960695, 4026282590, 848005093, 2090115245, 1732838459, 1701934203, 2618985574, 2392637351, 3610308264, 1162509741, 3772926898, 4026399774, 2801305136, 2182020938, 2540720293, 4177612013, 3758227323, 2141839466, 3457019126, 690868322, 598123635, 4220136616, 2063722767, 534543515, 3995640444, 3697772838, 605496025, 194542699, 4283492525, 3414550867, 2944993929, 3138370086, 2273260021, 1368435547, 328762456, 2810028600, 3571321502, 1782649452, 4008389438, 3374096016, 2070297302, 3548106024, 406229528, 420474145, 1345435476, 1454548380, 2158312864, 1379619651, 1353546123, 3811653577, 882683258, 2752377205, 3614226931, 1500298293, 2610950419, 1253921430, 2107280447, 2276719522, 1296486334, 3787958895, 3076802280, 1113584194, 1511963190, 4162049235, 3993023547, 3616161164, 1305747926, 3972166487, 1967370814, 920085398, 3066264196, 3270634177, 116308049, 3500602755, 2095466291, 374997835, 4013819058, 294797265, 2531304952, 1915167960, 667114423, 33740368, 2659595205, 1992038463, 1788663246, 1032288339, 2725176344, 965149701, 2340916620, 2880875099, 90563045, 550093497, 2205859613, 4113574329, 2611773242, 6224964, 2216769577, 2724275975, 1825208654, 1095989352, 1353654145, 4130469231, 2872700633, 3968090132, 3983530071, 3891778552, 253511543, 3451123488, 1858275584, 2348219340, 4164791526, 1546492002, 2318605776, 1965910834, 4016688232, 1661025458, 2551621891, 2559017942, 2999442539, 2626132036, 3855956683, 889284004, 4228792998, 2508936804, 2464411834, 3126525029, 1845662917, 1099875911, 230942152, 1316977638, 4287590380, 3960960001, 2035696777, 2084606597, 4185117397, 272555796, 1438814277, 3792058704, 2373818687, 328461098, 1933025545, 478993165, 2186693443, 2504363491, 2844647904, 4178236473, 174820621, 3894751753, 3928736686, 691843130, 3709350628, 2757608178, 1391782179, 1043240547, 2220523142, 2016183159, 2906513762, 3465827756, 3911890461, 2425950392, 948237040, 3362307558, 826526964, 1022776727, 1602110624, 495555482, 3374275041, 1251359676, 1255187341, 2187964004, 2260892791, 2998084135, 1359605011, 1974780261, 150018524, 729982456, 538716187, 2451949310, 1404163987, 323166556, 3575751708, 912151627, 1062492739, 136196234, 1525463811, 3934317698, 4059639798, 1714397141, 3799270074, 3933323871, 1950899325, 2202877835, 512028145, 121304123, 1747320652, 4025527049, 3203512622, 370863179, 1383153679, 338573713, 843668046, 3786076594, 3761568863, 1943691400, 4239756727, 2866888284, 927687523, 2835168303, 3639022486, 4177667412, 2454774443, 3330293413, 1073705099, 489962519, 2122918676, 3091912613, 2087072392, 2474785306, 4196422017, 1950304138, 1417647053, 3170449575, 2871308877, 2834888397, 2515213887, 706488188, 2765291661, 3342253292, 946613144, 2657539519, 1870033541, 2568978334, 2381242604, 2390291123, 1426830313, 1033183417, 1865206616, 3909956281, 3229647106, 567876042, 2632601739, 1787091399, 1136358267, 1707922556, 3600383805, 83493536, 3720481151, 1860566046, 1457764290, 1087693608, 2574737982, 3472233887, 1946429651, 657205714, 1561347349, 3458704973, 658970395, 2666292445, 4010435611, 2923502084, 2919668932, 1447403386, 272213229, 2379902797, 856095986, 390957008, 316624381, 37938916, 1312588871, 3443811924, 3563720170, 681138701, 3713652810, 1229426900, 45899309, 3319920329, 3184643346, 1733808374, 3218014532, 4131837143, 2708745467, 1925606777, 1070300412, 1857066639, 231709688, 89357594, 2807557743, 4246955725, 2648639203, 4014481964, 3609309784, 3413370774, 3362710738, 4117303240, 2975382313, 4244728709, 676814731, 1401643182, 4193290992, 1880982784, 3412352641, 432923266, 3204217730, 3640767186, 2748687591, 3238328348, 1816348975, 3454683239, 1788280760, 2390076397, 583783115, 749971462, 2070386333, 3684914363, 2505473379, 3781484919, 3251603932, 1061410633, 2657465574, 1047386942, 598203491, 3802852895, 3892183081, 650367471, 1888164348, 3790664944, 3285152048, 1772040010, 2779620173, 262034140, 4011996451, 172805695, 3112617504, 3698026131, 2749738982, 1338645063, 2883589338, 1171912066, 1463032348, 2303251030, 1690714656, 87045526, 3931593098, 1973669665, 2196833368, 3131780013, 2099423072, 3143196216, 2732157577, 3060002362, 3279855788, 2424755944, 3186596011, 892164315, 1948474334, 1752877034, 3954052922, 2895910728, 2605843797, 41504255, 2282306723, 2818684947, 2301112141, 125160123, 16831279, 2001639563, 2988100720, 2804013521, 1917204704, 2217119131, 747297802, 57526359, 1150692435, 1127421285, 1638007393, 2357393576, 1363062508, 112227006, 1305326547, 2699963315, 3027707384, 2567899981, 539524824, 1322391149, 4115045373, 2610809222, 1511354791, 1747171732, 2430429081, 2427698197, 668479568, 1616806017, 77518867
}; /* modified by rui, not need to init anymore */
static int mti_MersenneTwister=624; /* modified by rui, not need to init anymore */

/////////////----------------if you want to use the follow two init function, maybe you should uncomment the following two static, and comment the above two static.
//static unsigned long mt_MersenneTwister[N_MersenneTwister]; /* the array for the state vector  */
//static int mti_MersenneTwister=N_MersenneTwister+1; /* mti_MersenneTwister==N_MersenneTwister+1 means mt_MersenneTwister[N_MersenneTwister] is not initialized */

//should not be used, if you don't want to different random number.
/* initializes mt_MersenneTwister[N_MersenneTwister] with a seed */
void init_genrand_MersenneTwister(unsigned long s)
{
    mt_MersenneTwister[0]= s & 0xffffffffUL;
    for (mti_MersenneTwister=1; mti_MersenneTwister<N_MersenneTwister; mti_MersenneTwister++) {
        mt_MersenneTwister[mti_MersenneTwister] = 
	    (1812433253UL * (mt_MersenneTwister[mti_MersenneTwister-1] ^ (mt_MersenneTwister[mti_MersenneTwister-1] >> 30)) + mti_MersenneTwister); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt_MersenneTwister[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt_MersenneTwister[mti_MersenneTwister] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array_MersenneTwister(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand_MersenneTwister(19650218UL);
    i=1; j=0;
    k = (N_MersenneTwister>key_length ? N_MersenneTwister : key_length);
    for (; k; k--) {
        mt_MersenneTwister[i] = (mt_MersenneTwister[i] ^ ((mt_MersenneTwister[i-1] ^ (mt_MersenneTwister[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt_MersenneTwister[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N_MersenneTwister) { mt_MersenneTwister[0] = mt_MersenneTwister[N_MersenneTwister-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N_MersenneTwister-1; k; k--) {
        mt_MersenneTwister[i] = (mt_MersenneTwister[i] ^ ((mt_MersenneTwister[i-1] ^ (mt_MersenneTwister[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt_MersenneTwister[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N_MersenneTwister) { mt_MersenneTwister[0] = mt_MersenneTwister[N_MersenneTwister-1]; i=1; }
    }

    mt_MersenneTwister[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 

	//printf("%lu\n", mti_MersenneTwister);
	//for(i=0; i<N_MersenneTwister; ++i) {
	//	printf("%lu, ", mt_MersenneTwister[i]);
	//}
	//printf("\n");
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A_MersenneTwister};
    /* mag01[x] = x * MATRIX_A_MersenneTwister  for x=0,1 */

    if (mti_MersenneTwister >= N_MersenneTwister) { /* generate N_MersenneTwister words at one time */
        int kk;

        if (mti_MersenneTwister == N_MersenneTwister+1)   /* if init_genrand_MersenneTwister() has not been called, */
            init_genrand_MersenneTwister(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_MersenneTwister-M_MersenneTwister;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+M_MersenneTwister] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_MersenneTwister-1;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+(M_MersenneTwister-N_MersenneTwister)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_MersenneTwister[N_MersenneTwister-1]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[0]&LOWER_MASK_MersenneTwister);
        mt_MersenneTwister[N_MersenneTwister-1] = mt_MersenneTwister[M_MersenneTwister-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_MersenneTwister = 0;
    }
  
    y = mt_MersenneTwister[mti_MersenneTwister++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A_MersenneTwister};
    /* mag01[x] = x * MATRIX_A_MersenneTwister  for x=0,1 */

    if (mti_MersenneTwister >= N_MersenneTwister) { /* generate N_MersenneTwister words at one time */
        int kk;

        if (mti_MersenneTwister == N_MersenneTwister+1)   /* if init_genrand_MersenneTwister() has not been called, */
            init_genrand_MersenneTwister(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_MersenneTwister-M_MersenneTwister;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+M_MersenneTwister] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_MersenneTwister-1;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+(M_MersenneTwister-N_MersenneTwister)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_MersenneTwister[N_MersenneTwister-1]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[0]&LOWER_MASK_MersenneTwister);
        mt_MersenneTwister[N_MersenneTwister-1] = mt_MersenneTwister[M_MersenneTwister-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_MersenneTwister = 0;
    }
  
    y = mt_MersenneTwister[mti_MersenneTwister++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return (long)(y>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A_MersenneTwister};
    /* mag01[x] = x * MATRIX_A_MersenneTwister  for x=0,1 */

    if (mti_MersenneTwister >= N_MersenneTwister) { /* generate N_MersenneTwister words at one time */
        int kk;

        if (mti_MersenneTwister == N_MersenneTwister+1)   /* if init_genrand_MersenneTwister() has not been called, */
            init_genrand_MersenneTwister(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_MersenneTwister-M_MersenneTwister;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+M_MersenneTwister] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_MersenneTwister-1;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+(M_MersenneTwister-N_MersenneTwister)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_MersenneTwister[N_MersenneTwister-1]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[0]&LOWER_MASK_MersenneTwister);
        mt_MersenneTwister[N_MersenneTwister-1] = mt_MersenneTwister[M_MersenneTwister-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_MersenneTwister = 0;
    }
  
    y = mt_MersenneTwister[mti_MersenneTwister++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y*(1.0/4294967295.0); 
    /* divided by 2^32-1 */ 
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A_MersenneTwister};
    /* mag01[x] = x * MATRIX_A_MersenneTwister  for x=0,1 */

    if (mti_MersenneTwister >= N_MersenneTwister) { /* generate N_MersenneTwister words at one time */
        int kk;

        if (mti_MersenneTwister == N_MersenneTwister+1)   /* if init_genrand_MersenneTwister() has not been called, */
            init_genrand_MersenneTwister(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_MersenneTwister-M_MersenneTwister;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+M_MersenneTwister] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_MersenneTwister-1;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+(M_MersenneTwister-N_MersenneTwister)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_MersenneTwister[N_MersenneTwister-1]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[0]&LOWER_MASK_MersenneTwister);
        mt_MersenneTwister[N_MersenneTwister-1] = mt_MersenneTwister[M_MersenneTwister-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_MersenneTwister = 0;
    }
  
    y = mt_MersenneTwister[mti_MersenneTwister++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A_MersenneTwister};
    /* mag01[x] = x * MATRIX_A_MersenneTwister  for x=0,1 */

    if (mti_MersenneTwister >= N_MersenneTwister) { /* generate N_MersenneTwister words at one time */
        int kk;

        if (mti_MersenneTwister == N_MersenneTwister+1)   /* if init_genrand_MersenneTwister() has not been called, */
            init_genrand_MersenneTwister(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_MersenneTwister-M_MersenneTwister;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+M_MersenneTwister] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_MersenneTwister-1;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+(M_MersenneTwister-N_MersenneTwister)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_MersenneTwister[N_MersenneTwister-1]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[0]&LOWER_MASK_MersenneTwister);
        mt_MersenneTwister[N_MersenneTwister-1] = mt_MersenneTwister[M_MersenneTwister-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_MersenneTwister = 0;
    }
  
    y = mt_MersenneTwister[mti_MersenneTwister++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return (((double)y) + 0.5)*(1.0/4294967296.0); 
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void) 
{ 
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A_MersenneTwister};
    /* mag01[x] = x * MATRIX_A_MersenneTwister  for x=0,1 */

    if (mti_MersenneTwister >= N_MersenneTwister) { /* generate N_MersenneTwister words at one time */
        int kk;

        if (mti_MersenneTwister == N_MersenneTwister+1)   /* if init_genrand_MersenneTwister() has not been called, */
            init_genrand_MersenneTwister(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N_MersenneTwister-M_MersenneTwister;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+M_MersenneTwister] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N_MersenneTwister-1;kk++) {
            y = (mt_MersenneTwister[kk]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[kk+1]&LOWER_MASK_MersenneTwister);
            mt_MersenneTwister[kk] = mt_MersenneTwister[kk+(M_MersenneTwister-N_MersenneTwister)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt_MersenneTwister[N_MersenneTwister-1]&UPPER_MASK_MersenneTwister)|(mt_MersenneTwister[0]&LOWER_MASK_MersenneTwister);
        mt_MersenneTwister[N_MersenneTwister-1] = mt_MersenneTwister[M_MersenneTwister-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti_MersenneTwister = 0;
    }
  
    y = mt_MersenneTwister[mti_MersenneTwister++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);


    unsigned long a=y>>5, b=y>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 
/* These real versions are due to Isaku Wada, 2002/01/09 added */

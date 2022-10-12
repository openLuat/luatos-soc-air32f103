#include <string.h>
#include <stdio.h>
#include "air_crypt.h"
#include "air_aes.h"
#include "air_bignum_tool.h"
#include "air_rand.h"
#include "debug.h"
#include "air_cephes.h"
#include <math.h>

double air_frequency(uint8_t *, uint32_t);
double air_block_frequency(uint8_t *, uint32_t);
double air_runs(uint8_t *, uint32_t);
double air_longest_run_of_ones(uint8_t *, uint32_t);

const int8_t air_freq_tab[] = {-8, -6, -6, -4, -6, -4, -4, -2, -6, -4, -4, -2, -4, -2, -2,  0, 
                                -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2, 
                                -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2, 
                                -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4, 
                                -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2, 
                                -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4, 
                                -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4, 
                                -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6, 
                                -6, -4, -4, -2, -4, -2, -2,  0, -4, -2, -2,  0, -2,  0,  0,  2, 
                                -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4, 
                                -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4, 
                                -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6, 
                                -4, -2, -2,  0, -2,  0,  0,  2, -2,  0,  0,  2,  0,  2,  2,  4, 
                                -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6, 
                                -2,  0,  0,  2,  0,  2,  2,  4,  0,  2,  2,  4,  2,  4,  4,  6, 
                                0,  2,  2,  4,  2,  4,  4,  6,  2,  4,  4,  6,  4,  6,  6,  8};

double air_frequency(uint8_t *s, uint32_t n)
{
    int     i;
    double  s_obs, p_value, sum, sqrt2 = 1.41421356237309504880;

    if (n * 8 < 100)
        return 0.0;
    
    sum = 0.0;
    for ( i=0; i<n; i++ )
        sum += air_freq_tab[s[i]];

    s_obs = (double)fabs(sum)/(double)sqrt(n * 8)/sqrt2;
    p_value = erfc(s_obs);

    return p_value;
}

const int8_t air_blk_freq_tab[] = {0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
                                    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
                                    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
                                    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
                                    4,  5,  5,  6,  5,  6,  6,  7,  5,  6,  6,  7,  6,  7,  7,  8};

double air_block_frequency(uint8_t *s, uint32_t n)
{
    int     i, j, N, M;
    double  p_value, sum, pi, v, x_obs;

    if (n * 8 < 100)
        return 0.0;

    //M ¡Ö 0.1n
    M = n * 8 / 10 / 8 * 8;
    //N = n / M
    N = n * 8 / M;

    sum = 0.0;

    for ( i=0; i<N; i++ ) 
    {
        pi = 0;
        for (j=0; j<M/8; j++)
            pi += (double)air_blk_freq_tab[s[i*(M/8)+j]];
        pi = pi/(double)M;
        v = pi-0.5;
        sum += v*v;
    }

    x_obs = 4.0 * M * sum;
    p_value = air_cephes_igamc(N/2.0, x_obs/2.0);

    return p_value;
}

const int8_t air_runs_tab[] = {0,  1,  2,  1,  2,  3,  2,  1,  2,  3,  4,  3,  2,  3,  2,  1,
                                2,  3,  4,  3,  4,  5,  4,  3,  2,  3,  4,  3,  2,  3,  2,  1,
                                2,  3,  4,  3,  4,  5,  4,  3,  4,  5,  6,  5,  4,  5,  4,  3,
                                2,  3,  4,  3,  4,  5,  4,  3,  2,  3,  4,  3,  2,  3,  2,  1,
                                2,  3,  4,  3,  4,  5,  4,  3,  4,  5,  6,  5,  4,  5,  4,  3,
                                4,  5,  6,  5,  6,  7,  6,  5,  4,  5,  6,  5,  4,  5,  4,  3,
                                2,  3,  4,  3,  4,  5,  4,  3,  4,  5,  6,  5,  4,  5,  4,  3,
                                2,  3,  4,  3,  4,  5,  4,  3,  2,  3,  4,  3,  2,  3,  2,  1,
                                1,  2,  3,  2,  3,  4,  3,  2,  3,  4,  5,  4,  3,  4,  3,  2,
                                3,  4,  5,  4,  5,  6,  5,  4,  3,  4,  5,  4,  3,  4,  3,  2,
                                3,  4,  5,  4,  5,  6,  5,  4,  5,  6,  7,  6,  5,  6,  5,  4,
                                3,  4,  5,  4,  5,  6,  5,  4,  3,  4,  5,  4,  3,  4,  3,  2,
                                1,  2,  3,  2,  3,  4,  3,  2,  3,  4,  5,  4,  3,  4,  3,  2,
                                3,  4,  5,  4,  5,  6,  5,  4,  3,  4,  5,  4,  3,  4,  3,  2,
                                1,  2,  3,  2,  3,  4,  3,  2,  3,  4,  5,  4,  3,  4,  3,  2,
                                1,  2,  3,  2,  3,  4,  3,  2,  1,  2,  3,  2,  1,  2,  1,  0};


double air_runs(uint8_t *s, uint32_t n)
{
    int     S, i, N = (n * 8);
    double  pi, V, x_obs, p_value;

    if (n * 8 < 100)
        return 0.0;
    
    S = 0;
    for ( i=0; i<n; i++ )
        S += air_blk_freq_tab[s[i]];
    pi = (double)S / (double)N;

    if ( fabs(pi - 0.5) > (2.0 / sqrt(N)) ) 
    {
        p_value = 0.0;
    }
    else 
    {
        V = 1;

        for ( i=0; i<n; i++ )
        {
            V += air_runs_tab[s[i]];
            if(i<n-1)
            {
                //big endian
                if(!(s[i] & 0x01)^(!(s[i+1] & 0x80)))
                {
                    V++;
                }
//                if(((s[i] & 0x80) && !(s[i+1] & 0x01)) || (!(s[i] & 0x80) && (s[i+1] & 0x01)))
//                    V++;
            }
        }
        x_obs = fabs(V - 2.0 * N * pi * (1-pi)) / (2.0 * pi * (1-pi) * sqrt(2*N));
        p_value = erfc(x_obs);
    }

    return p_value;
}

const int8_t air_longest_run_tab[] = {0,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                        1,  1,  1,  2,  1,  1,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                        1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                        2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  4,  4,  5,  6,
                                        1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                        1,  1,  1,  2,  1,  1,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                        2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2,  3,  4,
                                        3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  7,
                                        1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                        1,  1,  1,  2,  1,  1,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                        1,  1,  1,  2,  1,  1,  2,  3,  1,  1,  1,  2,  2,  2,  3,  4,
                                        2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  4,  4,  5,  6,
                                        2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2,  3,  4,
                                        2,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  3,  3,  4,  5,
                                        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,
                                        4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  7,  8};

double air_longest_run_of_ones(uint8_t *s, uint32_t N)
{
    double          p_value, chi2, pi[7];
    int             v_n_obs, n, i, j, K, M, V[7];
    unsigned int    nu[7] = { 0, 0, 0, 0, 0, 0, 0 };

    K = 3;
    M = 8;
    V[0] = 1; V[1] = 2; V[2] = 3; V[3] = 4;
    pi[0] = 0.21484375;
    pi[1] = 0.3671875;
    pi[2] = 0.23046875;
    pi[3] = 0.1875;

    n = N * M;

    if (6272 <= n || n < 128)
        return 0.0;

    for ( i=0; i<N; i++ ) {
        v_n_obs = air_longest_run_tab[s[i]];
        if ( v_n_obs < V[0] )
            nu[0]++;
        for ( j=0; j<=K; j++ )
        {
            if ( v_n_obs == V[j] )
                nu[j]++;
        }
        if ( v_n_obs > V[K] )
            nu[K]++;
    }

    chi2 = 0.0;
    for ( i=0; i<=K; i++ )
        chi2 += ((nu[i] - N * pi[i]) * (nu[i] - N * pi[i])) / (N * pi[i]);

    p_value = air_cephes_igamc((double)(K/2.0), chi2 / 2.0);

    return p_value;
}

void rand_self_test(void)
{
	uint32_t i;
	uint8_t rand[16]; 
	double ret;
//    uint8_t s[] = {0xb2, 0xd8, 0xe6, 0xcc,0xd7,0x10,0xd6,0x13,0x51,0x4d,0x02,0xe0,0x4c,0x6c,0x15,0xcc};
//    uint8_t s[] = {0x79, 0x20, 0xDE, 0x0E,0x6E,0x22,0xFD,0xB7,0x82,0x92,0x9C,0x12,0xA4,0x4A,0x0F,0x22};
    uint8_t s[] = {0x79, 0x20, 0xD3, 0x0A,0x6E,0x27,0xFD,0xB7,0x82,0x92,0x9C,0x12,0xA4,0x4A,0x0F,0x22};
    double (*air_rand_func[4])(uint8_t *s, uint32_t n) = {air_frequency, air_block_frequency, air_runs, air_longest_run_of_ones};
    DBG_PRINT("rand self test start\n");
    AIRRAND_Prand(rand, sizeof(rand));
    for(i = 0; i < 4; i++)
    {
//        ret = air_rand_func[i](s, sizeof(s));
//        DBG_PRINT("caculate val is: %f\n\n", ret);
//        if(ret < 0.01)
//        {
//            DBG_PRINT("rand self test error\n");
//            while(1);
//        }
        
        ret = air_rand_func[i](rand, sizeof(rand));
//        ret = air_frequency(rand, sizeof(rand));
        DBG_PRINT("caculate val is: %f\n\n", ret);
        if(ret < 0.01)
        {
            DBG_PRINT("rand self test error\n");
            for(i = 0; i < sizeof(rand); i++)
            {
                DBG_PRINT("%0x ", rand[i]);
            }
            DBG_PRINT("\n");
            while(1);
        }
    }
}

void rand_frequency_test(void)
{
	double r;
//	uint8_t s[] = {0x0c,0x90,0xfd,0xaa,0x22,0x16,0x8c,0x23,0x4c,0x4c,0x66,0x28,0xb8};
	uint8_t s[] = {0xb8,0x28,0x66,0x4c,0x4c,0x23,0x8c,0x16,0x22,0xaa,0xfd,0x90,0x0c};
	DBG_PRINT("rand_frequency_test\n");
	r = air_frequency(s, sizeof(s));
	DBG_PRINT("expect val is:   0.049860\n");
	DBG_PRINT("caculate val is: %f\n\n", r);

	
}

void rand_block_frequency_test(void)
{
	
	double r;
	uint8_t s[] = {0xb8,0x28,0x66,0x4c,0x4c,0x23,0x8c,0x16,0x22,0xaa,0xfd,0x90,0x0c};
//    uint8_t s[] ={0xcc,0xb2,0x75,0x9c,0x6d,0xbc,0xcb,0x56,0x67,0x9a,0x0b,0x4f,0x5e,0x29,0xba,0x37,
//                    0xe9,0x73,0x1a,0x28,0x29,0xde,0x62,0xcf,0xeb,0xd9,0x97,0x5d,0xd9,0x63,0xd2,0xf2,
//                    0x84,0x13,0x96,0x02,0xa5,0x0d,0xfc,0x53,0x91,0xa3,0xa2,0xb4,0x73,0x60,0x06,0x01,
//                    0xc8,0x94,0x81,0x8a,0xbb,0xd6,0x78,0xd0,0x52,0x4c,0x26,0x39,0x66,0x14,0x39,0xe8,
//                    0xdf,0x99,0xd0,0x9d,0xa6,0x58,0xd3,0x4b,0xa7,0x57,0xfb,0x83,0x4a,0x46,0x8c,0xec,
//                    0x06,0xfe,0xd1,0x20,0xe7,0x64,0xaa,0x85,0x73,0x30,0x31,0x64,0xee,0x2c,0x68,0x12,
//                    0x02,0xc4,0xd4,0xcd,0x27,0x33,0x67,0xcd,0x07,0x2d,0xbe,0x9a,0xc3,0x91,0xfa,0x98,
//                    0x26,0x81,0x6f,0x23,0xf9,0xa6,0xbe,0x16,0xda,0x6c,0x29,0x69,0x8f,0xdb,0xfe,0x9f,
//                    0xf7,0x39,0xc7,0x4b,0xba,0xab,0xa1,0x67,0x8d,0xdd,0x67,0xd6,0x6c,0x9c,0x74,0xd6,
//                    0x2e,0xc5,0x66,0xfe,0x9d,0xef,0x4a,0x62,0xc3,0x98,0x38,0x1f,0xe4,0xbb,0x69,0xad,
//                    0x52,0x74,0x3e,0x4c,0x06,0x5e,0x0f,0xe3,0x85,0xd0,0xf8,0xb8,0x8a,0x59,0xb3,0x7b,
//                    0x91,0xd7,0x99,0xa7,0x66,0x13,0x74,0x01,0x9a,0xf6,0x36,0xca,0x65,0x1d,0x69,0x21,
//                    0x7a,0xd9,0xe2,0x55,0x83,0x2b,0x16,0xf5,0x24,0xff,0x30,0x0a,0xb6,0x1a,0x29,0xb6,
//                    0x53,0x51,0xa2,0x74,0x78,0xc6,0x78,0x10,0x14,0x81,0x75,0xb2,0xf1,0x18,0x5a,0x6c,
//                    0xa1,0x6d,0x14,0x0f,0x03,0xd9,0x65,0xd5,0xbe,0x00,0x2e,0x6a,0x4f,0xfa,0x2d,0xff,
//                    0x5e,0xf1,0x8b,0xd8,0x48,0x18,0xc9,0x27,0x9f,0x36,0x26,0x34,0xea,0x0d,0xc8,0x03};
	DBG_PRINT("rand_block_frequency_test\n");
	r = air_block_frequency(s, sizeof(s));
	DBG_PRINT("expect val is:   0.307353\n");
	DBG_PRINT("caculate val is: %f\n\n", r);
}

void rand_runs_test(void)
{
	double r;
	uint8_t s[] = {0xb8,0x28,0x66,0x4c,0x4c,0x23,0x8c,0x16,0x22,0xaa,0xfd,0x90,0x0c};
	DBG_PRINT("rand_runs_test\n");
	r = air_runs(s, sizeof(s));
	DBG_PRINT("expect val is:   0.987501\n");
	DBG_PRINT("caculate val is: %f\n\n", r);
}

void rand_longest_run_of_ones_test(void)
{
	double r;
	uint8_t s[] = {0xb2, 0xd8, 0xe6, 0xcc,0xd7,0x10,0xd6,0x13,0x51,0x4d,0x02,0xe0,0x4c,0x6c,0x15,0xcc};
	DBG_PRINT("rand_longest_run_of_ones_test\n");
	r = air_longest_run_of_ones(s, sizeof(s));
	DBG_PRINT("expect val is:   0.180609\n");
	DBG_PRINT("caculate val is: %f\n\n", r);
}


/*
void test_rand_time(void)
{
	uint32_t dbg_t = 0;
	uint32_t fclk = 0;
	uint8_t rand[512]; 
	fclk = air_dbg_fclk_get();
	BEGIN_TEST();
	air_rand(rand, sizeof(rand));
	END_TEST();
	DBG_PRINT("%d bytes rand data use %dus\n", sizeof(rand), dbg_t/(fclk/1000000));
	
}
*/

typedef struct _PokerNi {
    unsigned int flag;
    unsigned int count;
} PokerNi;

unsigned char toByte(uint8_t *subEpsilon, int M)
{
    int i = 0;
    unsigned char result = 0;
    for (i = 0; i < M; ++i) {
        result |= (subEpsilon[i] << (M - i - 1));
    }
    return result;
}

int findIndex(PokerNi *tab, int tabSize, unsigned int flag)
{
    int i = 0;
    for (i = 0; i < tabSize; ++i) {
        if (tab[i].flag == flag) {
            return i;
        }
    }
    return -1;
}
/*
 *parameter: M is the subsequence length
 *           n is the sequence length
 *           M and n should apply correspond tables
 *           epsilon is the test buff
 *return value: 
 *            0  -- failed
 *            1  -- success
 */
int PokerDetect(int M, int n, uint8_t *epsilon)
{
    int ret = 0;
    int i = 0, j = 0, N = n*8 / M, index = 0;
    int maxElements = (int) pow(2, M);
    double p_value = 0.0, sum_ni = 0.0, mp = 0.0, V = 0.0;
    unsigned int flag = 0;
    PokerNi tab[256];
    // M =2, 4 ,8
    if (M > 8) {
        return 0;
    }
    for (i = 0; i < maxElements; ++i) {
        tab[i].flag = (unsigned int) i;
        tab[i].count = 0;
    }
    
//    for (i = 0, j = 0; j < N; ++j, i += M) {
//        flag = toByte(epsilon + i, M);
//        index = findIndex(tab, maxElements, flag);
//        if (-1 == index) {
//             return 0;
//        }
//        tab[index].count += 1;
//    }
    for (i = 0; i < n; i++)
    {
        for(j = 8/M - 1; j >= 0; j--)
        {
            flag = (epsilon[i]>>(j*M)) & ((1<<M)-1);
            index = findIndex(tab, maxElements, flag);
            if (-1 == index) 
            {
                return 0;
            }
            tab[index].count += 1;
        }
    }
    

    for (i = 0; i < maxElements; ++i)
    {
        sum_ni += pow(tab[i].count, 2);
    }
    mp = (double)maxElements / N;
    V = mp * sum_ni - N;

    p_value = air_cephes_igamc((double)(maxElements - 1) / 2, V / 2);
//    p_value = air_cephes_igamc(1.5, 21.2/2);
    DBG_PRINT("p_value is %lf\n", p_value);
    
    if (p_value < 0.01) {
        return 0;
    }

    ret = 1;
    return ret;
}

void rand_poker_test()
{
    uint8_t s0[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
                    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x00,
                    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x01,
                    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
                    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x01,
                    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
                    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x00,
                    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
                    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x01,
                    0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
                    0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0x00,
                    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
                    0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0x00,
                    0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
                    0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0x46,0x1b,0x75,0xd3,0x62,0x8d,0xac,0x01};
    uint8_t s[1280];
    uint8_t i = 0;
    for(i = 0; i < 5; i++)
    {
        memcpy(s + i*256, s0, 256);
    }                    
    PokerDetect(8, 1280, s);
}

void RAND_Test()
{
//    rand_poker_test();
    rand_frequency_test();
    rand_block_frequency_test();
    rand_runs_test();
    rand_longest_run_of_ones_test();
    rand_self_test();
}

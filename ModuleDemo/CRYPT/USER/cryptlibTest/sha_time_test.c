#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_sha.h"
#include "debug.h"
#include "air_rand.h"

typedef struct
{
	uint32_t 	u32Val;
    uint32_t    u32OutLen;
	char *		name;
}var_name_t;

#define SHA_TIME_TEST_DATA				(1024*4)
#define SHA_TIME_TEST_COUNT				10

void SHA_Time_Test(void)
{
    uint32_t i,j;
    var_name_t modes[3] =   {{AIRSHA1, 20, "SHA_160"},
                            {AIRSHA224, 28, "SHA_224"},
                            {AIRSHA256, 32, "SHA_256"}};
    
    uint8_t plain[SHA_TIME_TEST_DATA];
    uint8_t cipher[512 /8];
    
    TIME_MS_US time[5] = {0}, timeTotal[5] = {0};

	memset(plain, 0, sizeof(plain));
	AIRRAND_Prand(plain, sizeof(plain));

	DBG_PRINT("\n%d bytes data size\n", SHA_TIME_TEST_DATA);

    for (i = 0; i < 3; i++)
    {
        
        for(j = 0; j < SHA_TIME_TEST_COUNT; j++)
        {
            memset(cipher, 0, sizeof(cipher));
            timer_init_ms(1);
            AIRSHA_Sha((AIRSHA_MOD_TYPE)modes[i].u32Val, plain, sizeof(plain), cipher, modes[i].u32OutLen);
            get_time(&time[i]);
            time_add(&time[i], &timeTotal[i], &timeTotal[i]);
//            DBG_PRINT("%d ms %d us time %s Test\n", time[i].u32Ms, time[i].u32Us, modes[i].name);
        }
    }
    DBG_PRINT("\n");
    time_avg(timeTotal + 0, SHA_TIME_TEST_COUNT, time + 0);
    time_avg(timeTotal + 1, SHA_TIME_TEST_COUNT, time + 1);
    time_avg(timeTotal + 2, SHA_TIME_TEST_COUNT, time + 2);
    DBG_PRINT("Total %dms %dus avg %dms %dus SHA_160 time_sum\n", 
    timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us);
    DBG_PRINT("Total %dms %dus avg %dms %dus SHA_224 time_sum\n", 
    timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us);
    DBG_PRINT("Total %dms %dus avg %dms %dus SHA_256 time_sum\n", 
    timeTotal[2].u32Ms, timeTotal[2].u32Us,  time[2].u32Ms, time[2].u32Us);
    DBG_PRINT("\n"); 
}


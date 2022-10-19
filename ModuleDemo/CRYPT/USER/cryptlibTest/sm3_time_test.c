#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "debug.h"
#include "air_rand.h"
#include "air_sm3.h"

#define SM3_TIME_TEST_DATA				(1024*4)
#define SM3_TIME_TEST_COUNT				10

void SM3_Time_Test(void)
{
    uint32_t i;
//    AIRSM3_Context Sm3Ctx;
    TIME_MS_US time, timeTotal = { 0 };
    
    uint8_t plain[SM3_TIME_TEST_DATA];
    uint8_t cipher[32];

    memset(cipher, 0, sizeof(cipher));
    memset(plain, 0, sizeof(plain));
    AIRRAND_Prand(plain, sizeof(plain));

	DBG_PRINT("\n%d bytes data size\n", SM3_TIME_TEST_DATA);

    for(i = 0; i < SM3_TIME_TEST_COUNT; i++)
    {
        timer_init_ms(1);
        AIRSM3_Cal(cipher,plain,sizeof(plain));
//        AIRSM3_Starts(&Sm3Ctx);
//        AIRSM3_Update(&Sm3Ctx, plain, sizeof(plain));
//        AIRSM3_Finish(&Sm3Ctx, cipher);
        get_time(&time);
        time_add(&time, &timeTotal, &timeTotal);
//        DBG_PRINT("%dms %dus time SM3 Test\n", time.u32Ms, time.u32Us);
    }
    DBG_PRINT("\n");
    time_avg(&timeTotal, SM3_TIME_TEST_COUNT, &time);
    DBG_PRINT("Total %dms %dus time_sum SM3 Test\n", timeTotal.u32Ms, timeTotal.u32Us);
    DBG_PRINT("avg %dms %dus time_sum SM3 Test\n", time.u32Ms, time.u32Us);
    DBG_PRINT("\n");
}

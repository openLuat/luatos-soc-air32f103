#include <string.h>
#include <stdio.h>
#include "air_sm7.h"
#include "debug.h"
#include "air_rand.h"


#define SM7_TIME_TEST_DATA				(1024*4)
#define SM7_TIME_TEST_COUNT				10

typedef struct
{
	uint32_t 	val;
	char *		name;
}var_name_t;




void SM7_Time_Test(void)
{
    uint32_t i, j;
    var_name_t modes[2] =   {{AIR_SM7_OPT_BLK_ECB, "ECB"},
                             {AIR_SM7_OPT_BLK_CBC, "CBC"}};
    TIME_MS_US time[2], timeTotal[2] = { 0 };
    
    uint8_t plain[2][SM7_TIME_TEST_DATA];
    uint8_t cipher[SM7_TIME_TEST_DATA];
        
    uint8_t iv[] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};
    uint8_t au8IV[8] = {0};
    uint8_t key[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    AIR_SYM_CRYPT_CALL callSm7;

    memset(plain, 0, sizeof(plain));
    AIRRAND_Prand(plain[0],sizeof(plain[0]));
	i = 0xFFFFF;
	while(i--);

	DBG_PRINT("\n%d bytes data size\n", SM7_TIME_TEST_DATA);

    //pack mode ECB CBC
    for (i = 0; i < 2; i++)
    {
        memset(timeTotal,0,sizeof(timeTotal));
        for (j = 0; j < SM7_TIME_TEST_COUNT; j++)
        {
            memset(cipher, 0, sizeof(cipher));
            memset(plain[1], 0, sizeof(plain[1]));
            memcpy(au8IV, iv, sizeof(iv));  

            memset(&callSm7, 0, sizeof(callSm7));
            callSm7.pu8In = plain[0];
            callSm7.u32InLen = sizeof(plain[0]);
            callSm7.pu8Out = cipher;
            callSm7.u32OutLen = sizeof(cipher);
            callSm7.pu8IV  = au8IV;
            callSm7.pu8Key = key;
            callSm7.u32Crc = 0;
            callSm7.u16Opt = modes[i].val | AIR_SM7_OPT_MODE_ENCRYPT;
            callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);  
            timer_init_ms(1);
            AIRSM7_EncDec(&callSm7);
            get_time(time);
            time_add(time, timeTotal + 0, timeTotal + 0);
            DBG_PRINT("%d ms %dus SM7 %s encrypt time\n", time[0].u32Ms, time[0].u32Us, modes[i].name);
            
            
            memcpy(au8IV, iv, sizeof(iv));                                                                                                     
            callSm7.pu8In = cipher;
            callSm7.u32InLen = sizeof(cipher);
            callSm7.pu8Out = plain[1];
            callSm7.u32OutLen = sizeof(plain[1]);
            callSm7.pu8IV  = au8IV;
            callSm7.pu8Key = key;
            callSm7.u32Crc = 0;
            callSm7.u16Opt = modes[i].val | AIR_SM7_OPT_MODE_DECRYPT;
            callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);  
            timer_init_ms(1);     
            AIRSM7_EncDec(&callSm7);
            get_time(time + 1);
            time_add(time + 1, timeTotal + 1, timeTotal + 1);
//            DBG_PRINT("%d ms %dus SM7 %s decrypt time\n", time[1].u32Ms, time[1].u32Us, modes[i].name);            
            if(!memcmp(plain[0], plain[1], sizeof(plain[0])))
            {
                DBG_PRINT("SM7 %s Test Pass...\n", modes[i].name);
            }
            else
            {
                DBG_PRINT("SM7 %s Test Fail...\n", modes[i].name);
                while(1);
            }
        }
        DBG_PRINT("\n");
        time_avg(timeTotal + 0, SM7_TIME_TEST_COUNT, time + 0);
        time_avg(timeTotal + 1, SM7_TIME_TEST_COUNT, time + 1);
        DBG_PRINT("Total %dms %dus avg %dms %dus SM7 %s enc time_sum\n", 
        timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us, modes[i].name);
        DBG_PRINT("Total %dms %dus avg %dms %dus SM7 %s dec time_sum\n", 
        timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us, modes[i].name);
        DBG_PRINT("\n");  
    }
}


























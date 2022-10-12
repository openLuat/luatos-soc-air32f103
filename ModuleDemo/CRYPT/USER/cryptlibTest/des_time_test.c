#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_des.h"
#include "debug.h"
#include "air_rand.h"

#define DES_TIME_TEST_DATA				(1024*4)
#define DES_TIME_TEST_COUNT				10

typedef struct
{
	uint32_t 	val;
	char *		name;
}var_name_t;


void DES_Time_Test(void)
{
    uint32_t i, j;
    var_name_t modes[2] =   {{AIR_DES_OPT_BLK_ECB, "ECB"},
                             {AIR_DES_OPT_BLK_CBC, "CBC"}};
    TIME_MS_US time[2] = {0}, timeTotal[2] = {0};
    
    uint8_t plain[2][DES_TIME_TEST_DATA];
    uint8_t cipher[DES_TIME_TEST_DATA];
        
    uint8_t iv[] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};
    uint8_t au8IV[8] = {0};
    uint8_t key[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    AIR_SYM_CRYPT_CALL callDes;

    memset(plain, 0, sizeof(plain));
    AIRRAND_Prand(plain[0], sizeof(plain[0]));

	i = 0xFFFFF;
	while(i--);

	DBG_PRINT("\n%d bytes data size\n", DES_TIME_TEST_DATA);

    //pack mode ECB CBC
    for (i = 0; i < 2; i++)
    {
        memset(timeTotal,0,sizeof(timeTotal));
        for (j = 0; j < DES_TIME_TEST_COUNT; j++)
        {
            memset(cipher, 0, sizeof(cipher));
            memset(plain[1], 0, sizeof(plain[1]));
            memcpy(au8IV, iv, sizeof(iv));

            memset(&callDes, 0, sizeof(callDes));
            callDes.pu8In = plain[0];
            callDes.u32InLen = sizeof(plain[0]);
            callDes.pu8Out = cipher;
            callDes.u32OutLen = sizeof(cipher);
            callDes.pu8IV  = au8IV;
            callDes.pu8Key = key;
            callDes.u32Crc = 0;
            callDes.u16Opt = modes[i].val | AIR_DES_OPT_MODE_ENCRYPT;
            callDes.u32Crc = AIRCRC_CalcBuff(0xffff, &callDes, sizeof(AIR_SYM_CRYPT_CALL)-4);
            timer_init_ms(1);
            AIRDES_EncDec(&callDes);           
            get_time(time);
            time_add(time, timeTotal + 0, timeTotal + 0);
//            DBG_PRINT("%d ms %dus DES %s encrypt time\n", time[0].u32Ms, time[0].u32Us, modes[i].name);
            
            memcpy(au8IV, iv, sizeof(iv));
                                                                                                          
            callDes.pu8In = cipher;
            callDes.u32InLen = sizeof(cipher);
            callDes.pu8Out = plain[1];
            callDes.u32OutLen = sizeof(plain[1]);
            callDes.pu8IV  = au8IV;
            callDes.pu8Key = key;
            callDes.u32Crc = 0;
            callDes.u16Opt = modes[i].val | AIR_DES_OPT_MODE_DECRYPT;
            callDes.u32Crc = AIRCRC_CalcBuff(0xffff, &callDes, sizeof(AIR_SYM_CRYPT_CALL)-4);
            timer_init_ms(1); 
            AIRDES_EncDec(&callDes);
            get_time(time+1);
            time_add(time + 1, timeTotal + 1, timeTotal + 1);
//            DBG_PRINT("%d ms %dus DES %s decrypt time\n", time[1].u32Ms, time[1].u32Us, modes[i].name);
            
            if(!memcmp(plain[0], plain[1], sizeof(plain[0])))
            {
                DBG_PRINT("DES %s Test Pass...\n", modes[i].name);
            }
            else
            {
                DBG_PRINT("DES %s Test Fail...\n", modes[i].name);
                while(1);
            }
        }
        DBG_PRINT("\n");
        time_avg(timeTotal + 0, DES_TIME_TEST_COUNT, time + 0);
        time_avg(timeTotal + 1, DES_TIME_TEST_COUNT, time + 1);
        DBG_PRINT("Total %dms %dus avg %dms %dus DES %s enc time_sum\n", 
        timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us, modes[i].name);
        DBG_PRINT("Total %dms %dus avg %dms %dus DES %s dec time_sum\n", 
        timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us, modes[i].name);
        DBG_PRINT("\n");  
    }

}


void TDES_Time_Test(void)
{
    uint32_t i, j;
    TIME_MS_US time[2], timeTotal[2] = { 0 };
    var_name_t modes[2] =   {{AIR_DES_OPT_BLK_ECB, "ECB"},
                             {AIR_DES_OPT_BLK_CBC, "CBC"}};
    
    uint8_t plain[2][DES_TIME_TEST_DATA];
    uint8_t cipher[DES_TIME_TEST_DATA];

	uint8_t iv[] = {0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17};
    uint8_t au8IV[8] = {0};
    
	uint8_t key[3][8] = {{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
						 {0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01},
						 {0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23}};
    AIR_SYM_CRYPT_CALL callTdes;

    memset(plain, 0, sizeof(plain));
    AIRRAND_Prand(plain[0], sizeof(plain[0]));

	i = 0xFFFF;
	while(i--);

	DBG_PRINT("\n%d bytes data size\n", DES_TIME_TEST_DATA);

    //pack mode ECB CBC
    for (i = 0; i < 2; i++)
    {
        memset(timeTotal, 0, sizeof(timeTotal));
        for (j = 0; j < DES_TIME_TEST_COUNT; j++)
        {
            memset(cipher, 0, sizeof(cipher));
            memset(plain[1], 0, sizeof(plain[1]));
            memcpy(au8IV, iv, sizeof(iv));
            
            memset(&callTdes, 0, sizeof(callTdes));
            
            callTdes.pu8In = plain[0];
            callTdes.u32InLen = sizeof(plain[0]);
            callTdes.pu8Out = cipher;
            callTdes.u32OutLen = sizeof(cipher);
            callTdes.pu8IV  = au8IV;
            callTdes.pu8Key = (uint8_t *)key;
            callTdes.u32Crc = 0;
            callTdes.u16Opt = modes[i].val | AIR_DES_OPT_MODE_ENCRYPT;
            callTdes.u32Crc = AIRCRC_CalcBuff(0xffff, &callTdes, sizeof(AIR_SYM_CRYPT_CALL)-4);
            timer_init_ms(1);
            AIRTDES_EncDec(&callTdes);
            get_time(time);
            time_add(time, timeTotal + 0, timeTotal + 0);
//            DBG_PRINT("%d ms %dus TDES %s encrypt time\n", time[0].u32Ms, time[0].u32Us, modes[i].name);                       
            memcpy(au8IV, iv, sizeof(iv));
            
            callTdes.pu8In = cipher;
            callTdes.u32InLen = sizeof(cipher);
            callTdes.pu8Out = plain[1];
            callTdes.u32OutLen = sizeof(plain[1]);
            callTdes.pu8IV  = au8IV;
            callTdes.pu8Key = (uint8_t *)key;
            callTdes.u32Crc = 0;
            callTdes.u16Opt = modes[i].val | AIR_DES_OPT_MODE_DECRYPT;
            callTdes.u32Crc = AIRCRC_CalcBuff(0xffff, &callTdes, sizeof(AIR_SYM_CRYPT_CALL)-4);
            timer_init_ms(1);
            AIRTDES_EncDec(&callTdes);
            get_time(time +1 );
            time_add(time + 1, timeTotal + 1, timeTotal + 1);
//            DBG_PRINT("%d ms %dus DES %s decrypt time\n", time[1].u32Ms, time[1].u32Us, modes[i].name);

            if(!memcmp(plain[0], plain[1], sizeof(plain[0])))
            {
                DBG_PRINT("TDES %s Test Pass...\n", modes[i].name);
            }
            else
            {
                DBG_PRINT("TDES %s Test Fail...\n", modes[i].name);
                while(1);
            }
        }
        DBG_PRINT("\n"); 
        time_avg(timeTotal + 0, DES_TIME_TEST_COUNT, time);
        time_avg(timeTotal + 1, DES_TIME_TEST_COUNT, time + 1);
        DBG_PRINT("total %dms %dus TDES %s encrypt time\n", timeTotal[0].u32Ms, timeTotal[0].u32Us, modes[i].name);
        DBG_PRINT("avg %dms %dus TDES %s encrypt time\n", time[0].u32Ms, time[0].u32Us, modes[i].name);              
        DBG_PRINT("%dms %dus TDES %s decrypt time\n", timeTotal[1].u32Ms, timeTotal[1].u32Us, modes[i].name);
        DBG_PRINT("avg %dms %dus TDES %s decrypt time\n", time[1].u32Ms, time[1].u32Us, modes[i].name);
        DBG_PRINT("\n"); 
    }
}















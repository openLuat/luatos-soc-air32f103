#include <string.h>
#include <stdio.h>
#include "air_sm4.h"
#include "debug.h"
#include "air_rand.h"

typedef struct
{
	uint32_t 	val;
	char *		name;
}var_name_t;

#define SM4_TIME_TEST_DATA				(1024*4)
#define SM4_TIME_TEST_COUNT				10

void SM4_Time_Test(void)
{
    uint32_t i, j;
    TIME_MS_US time[2], timeTotal[2] = { 0 };
    var_name_t modes[2] =   {{AIR_SM4_OPT_BLK_ECB, "ECB"},
                             {AIR_SM4_OPT_BLK_CBC, "CBC"}};
    
    uint8_t plain[2][SM4_TIME_TEST_DATA];
    uint8_t cipher[SM4_TIME_TEST_DATA];

	uint8_t key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
	uint8_t iv[16] = {0xA3, 0xB1, 0xBA, 0xC6, 0x56, 0xAA, 0x33, 0x50, 0x67, 0x7D, 0x91, 0x97, 0xB2, 0x70, 0x22, 0xDC};
    uint8_t au8IV[16] = {0};
    AIR_SYM_CRYPT_CALL callSm4;

	memset(plain, 0, sizeof(plain));

	DBG_PRINT("\n%d bytes data size\n", SM4_TIME_TEST_DATA);

    //pack mode ECB CBC
    for (i = 0; i < 2; i++)
    {
        memset(timeTotal, 0, sizeof(timeTotal));
        for (j = 0; j < SM4_TIME_TEST_COUNT; j++)
        {
            memset(cipher, 0, sizeof(cipher));
            memset(plain[1], 0, sizeof(plain[1]));
            memcpy(au8IV, iv, sizeof(iv));
            memset(&callSm4, 0, sizeof(callSm4));
//            timer_init_ms(1);
            callSm4.pu8In = plain[0];
            callSm4.u32InLen = sizeof(plain[0]);
            callSm4.pu8Out = cipher;
            callSm4.u32OutLen = sizeof(cipher);
            callSm4.u32Crc = 0;
            callSm4.pu8IV = au8IV;
            callSm4.pu8Key = key;
            callSm4.u16Opt = modes[i].val | AIR_SM4_OPT_MODE_ENCRYPT ;
            callSm4.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm4, sizeof(AIR_SYM_CRYPT_CALL)-4);
            timer_init_ms(1);
            AIRSM4_EncDec(&callSm4);
            
            get_time(time);
            time_add(time + 0, timeTotal + 0, timeTotal + 0);
            
            memcpy(au8IV, iv, sizeof(iv));
//            timer_init_ms(1);
            callSm4.pu8In = cipher;
            callSm4.u32InLen = sizeof(cipher);
            callSm4.pu8Out = plain[1];
            callSm4.u32OutLen = sizeof(plain[1]);
            callSm4.u32Crc = 0;
            callSm4.pu8IV = au8IV;
            callSm4.pu8Key = key;
            callSm4.u16Opt = modes[i].val | AIR_SM4_OPT_MODE_DECRYPT;
            callSm4.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm4, sizeof(AIR_SYM_CRYPT_CALL)-4);
            timer_init_ms(1);
            AIRSM4_EncDec(&callSm4);
            
            get_time(time + 1);
            time_add(time + 1, timeTotal + 1, timeTotal + 1);
            
            if(!memcmp(plain[0], plain[1], sizeof(plain[0])))
            {
                DBG_PRINT("SM4 %s Test Pass...\n", modes[i].name);
            }
            else
            {
                DBG_PRINT("SM4 %s Test Fail...\n", modes[i].name);
                while (1);
            }
        }
        DBG_PRINT("\n");
        time_avg(timeTotal + 0, SM4_TIME_TEST_COUNT, time + 0);
        time_avg(timeTotal + 1, SM4_TIME_TEST_COUNT, time + 1);
        DBG_PRINT("Total %dms %dus avg %dms %dus SM4 %s encrypt time_sum\n", 
               timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us, modes[i].name);
        DBG_PRINT("Total %dms %dus avg %dms %dus SM4 %s decrypt time_sum\n", 
               timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us, modes[i].name);
        DBG_PRINT("\n");

	}
}

//#define MEM_ADDR1    ((uint8_t *)(0x20001800))
//#define MEM_ADDR2    ((uint8_t *)(0x20001000))
//void MemCopy_Test()
//{
//    uint8_t i, au8Data[1024] = {0};
//    uint32_t j;
//    TIME_MS_US time[2], timeTotal[2] = { 0 };
//    memset(MEM_ADDR2, 'a', sizeof(au8Data));
//    
//    for (i = 0; i < 2; i++)
//    {
//        memset(timeTotal, 0, sizeof(timeTotal));
//        for (j = 0; j < SM4_TIME_TEST_COUNT; j++)
//        {
//            timer_init_ms(1);
//            memcpy(MEM_ADDR1,MEM_ADDR2,1024);
////            memcpy(MEM_ADDR1,au8Data,1024);
//            get_time(time);
//            time_add(time + 0, timeTotal + 0, timeTotal + 0);
////            timer_init_ms(1);
////            memcpy(MEM_ADDR2,au8Data,1024);
////            get_time(time + 1);
//            time_add(time + 1, timeTotal + 1, timeTotal + 1);
//            DBG_PRINT("%dms %dus\n", time[0].u32Ms, time[0].u32Us);
////            DBG_PRINT("%dms %dus \n", time[1].u32Ms, time[1].u32Us);
//        }
//        DBG_PRINT("\n");
//        time_avg(timeTotal + 0, SM4_TIME_TEST_COUNT, time + 0);
////        time_avg(timeTotal + 1, SM4_TIME_TEST_COUNT, time + 1);
//        DBG_PRINT("Total %dms %dus avg %dms %dus \n", 
//               timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us);
////        DBG_PRINT("Total %dms %dus avg %dms %dus \n", 
////               timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us);
//        DBG_PRINT("\n");
//    }
//}

#include <string.h>
#include <stdio.h>
#include "air_sm1.h"
#include "debug.h"
#include "air_rand.h"

#define SM1_TIME_TEST_DATA              1024*4
#define SM1_TIME_TEST_COUNT             10

typedef struct
{
    uint8_t *   key;
    uint32_t    size;
    char *      name;
}sm1_key_t;

typedef struct
{
    uint32_t    val;
    char *      name;
}var_name_t;

void SM1_Time_Test(void)
{
    uint32_t i, j, k;
    uint32_t u32KeyLen;
    AIR_SYM_CRYPT_CALL callSm1;
    
    var_name_t modes[2] =   {{AIR_SM1_OPT_BLK_ECB, "ECB"},
                             {AIR_SM1_OPT_BLK_CBC, "CBC"}};
    TIME_MS_US time[2], timeTotal[2] = {0};

    sm1_key_t keys[3] =     {{NULL, AIR_SM1_OPT_KEY_128, "KEY_128"},
                             {NULL, AIR_SM1_OPT_KEY_192, "KEY_192"},
                             {NULL, AIR_SM1_OPT_KEY_256, "KEY_256"}};

    uint8_t plain[2][SM1_TIME_TEST_DATA];
    uint8_t cipher[SM1_TIME_TEST_DATA];

    uint8_t u8Skey128_8[16] = {0xa2,0x91,0xb3,0xeb,0xa4,0xed,0x22,0x5f,0x1c,0xea,0xa6,0x07,0x62,0x8f,0xb2,0x36};
    uint8_t u8Ekey128_8[16] = {0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef,0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};
    uint8_t u8Akey128_8[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        
    uint8_t IV[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t au8IV[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    uint8_t au8Key[32 + 16 + 16];
    
    memset(au8Key, 0, sizeof(au8Key));
    u32KeyLen = 0;
    memcpy(au8Key, u8Skey128_8, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, u8Ekey128_8, 16);

    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, u8Akey128_8, 16);
    
    keys[0].key = au8Key;
    keys[1].key = au8Key;
    keys[2].key = au8Key;
    
    memset(plain, 0, sizeof(plain));                    
                    
    DBG_PRINT("\n%d bytes data size\n", SM1_TIME_TEST_DATA);

    //key 128 192 256
    for (i = 0; i < 3; i++)
    {
        //pack mode ECB CBC
        for (j = 0; j < 2; j++)
        {
            memset(timeTotal,0,sizeof(timeTotal));
            for (k = 0; k < SM1_TIME_TEST_COUNT; k++)
            {
                memset(cipher, 0, sizeof(cipher));
                memset(plain[1], 0, sizeof(plain[1]));
                memcpy(au8IV, IV, sizeof(IV));
                
                memset(&callSm1, 0, sizeof(callSm1));
                //SM1 enc                               
                callSm1.pu8In = plain[0];
                callSm1.u32InLen = sizeof(plain[0]);
                callSm1.pu8Out = cipher;
                callSm1.u32OutLen = sizeof(cipher);
                callSm1.u32Crc = 0;
                callSm1.pu8IV = au8IV;
                callSm1.pu8Key = keys[i].key;
                callSm1.u16Opt = modes[j].val | AIR_SM1_OPT_MODE_ENCRYPT | keys[i].size | AIR_SM1_OPT_ROUND_8;
                callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
                timer_init_ms(1);
                AIRSM1_EncDec(&callSm1);                
                get_time(time);
                time_add(time, timeTotal + 0, timeTotal + 0);
//                DBG_PRINT("%dms %dus SM1 %s %s encrypt time\n", time[0].u32Ms, time[0].u32Us, modes[j].name, keys[i].name);
                
                
                memcpy(au8IV, IV, sizeof(IV));
                //SM1 dec              
                callSm1.pu8In = cipher;
                callSm1.u32InLen = sizeof(cipher);
                callSm1.pu8Out = plain[1];
                callSm1.u32OutLen = sizeof(plain[1]);
                callSm1.u32Crc = 0;
                callSm1.pu8IV = au8IV;
                callSm1.pu8Key = keys[i].key;
                callSm1.u16Opt = modes[j].val | AIR_SM1_OPT_MODE_DECRYPT | keys[i].size | AIR_SM1_OPT_ROUND_8;
                callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
                timer_init_ms(1);
                AIRSM1_EncDec(&callSm1);                
                get_time(time+1);
                time_add(time + 1, timeTotal + 1, timeTotal + 1);
                if(!memcmp(plain[0], plain[1], sizeof(plain[0])))
                {
                    DBG_PRINT("SM1 %s %s Test Pass...\n", modes[j].name, keys[i].name);
                }
                else
                {
                    DBG_PRINT("SM1 %s %s Test Fail...\n", modes[j].name, keys[i].name);
                    while (1);
                    
                }
            }
            DBG_PRINT("\n");
            time_avg(timeTotal + 0, SM1_TIME_TEST_COUNT, time + 0);
            time_avg(timeTotal + 1, SM1_TIME_TEST_COUNT, time + 1);
            DBG_PRINT("Total %dms %dus avg %dms %dus %s %s SM1 enc time_sum\n", 
            timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us, modes[j].name, keys[i].name);
            DBG_PRINT("Total %dms %dus avg %dms %dus  %s %s SM1 dec time_sum\n", 
            timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us, modes[j].name, keys[i].name);
            DBG_PRINT("\n");
        }
    }
}
















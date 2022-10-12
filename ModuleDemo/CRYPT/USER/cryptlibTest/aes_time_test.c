#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_aes.h"
#include "debug.h"
#include "air_rand.h"

#define AES_TIME_TEST_DATA              (1024 * 4)
#define AES_TIME_TEST_COUNT             10

typedef struct
{
    uint8_t     u8KLen;
    uint8_t *   pu8Key;
    uint32_t    u32Size;
    char *      pu8Name;
}AES_KEY_TYP;

typedef struct
{
    uint32_t    u32Val;
    char *      pu8Name;
}AES_MODE_TYP;

void AES_Time_Test(void)
{
    uint32_t i, j, k;
    AIR_SYM_CRYPT_CALL callAes;  
    AES_MODE_TYP ModeType[2] = {{AIR_AES_OPT_BLK_ECB, "ECB"},
                             {AIR_AES_OPT_BLK_CBC, "CBC"}};
    TIME_MS_US time[2], timeTotal[2] = {0};
    AES_KEY_TYP KeyType[3] = {{16, NULL, AIR_AES_OPT_KEY_128, "KEY_128"},
                           {24, NULL, AIR_AES_OPT_KEY_192, "KEY_192"},
                           {32, NULL, AIR_AES_OPT_KEY_256, "KEY_256"}};
    uint8_t au8Plain[2][AES_TIME_TEST_DATA];
    uint8_t au8Cipher[AES_TIME_TEST_DATA];
    uint8_t au8Iv[16] = {0};
    uint8_t u8Key128[128/8] = {0}; 
    uint8_t u8Key192[192/8] = {0};
    uint8_t u8Key256[256/8] = {0};
    KeyType[0].pu8Key = u8Key128;
    KeyType[1].pu8Key = u8Key192;
    KeyType[2].pu8Key = u8Key256;    
    DBG_PRINT("\n %d bytes data size\n", AES_TIME_TEST_DATA);
    //key 128 192 256
    for (i = 0; i < 3; i++)
    {
        //pack mode ECB CBC
        for (j = 0; j < 2; j++)
        {
            memset(timeTotal,0,sizeof(timeTotal));
            for (k = 0; k < AES_TIME_TEST_COUNT; k++)
            {
                memset(au8Plain, 0, sizeof(au8Plain));
                memset(au8Cipher, 0, sizeof(au8Cipher));
                memset(au8Plain[1], 0, sizeof(au8Plain[1]));
                memset(au8Iv,0,sizeof(au8Iv));
                memset(KeyType[i].pu8Key,0,KeyType[i].u8KLen);
                
                AIRRAND_Prand(au8Plain,sizeof(au8Plain));
                AIRRAND_Prand(au8Iv,sizeof(au8Iv));
                AIRRAND_Prand(KeyType[i].pu8Key,KeyType[i].u8KLen);
                
                memset(&callAes, 0, sizeof(callAes));
                //AES enc
                callAes.pu8In = au8Plain[0];
                callAes.u32InLen = sizeof(au8Plain[0]);
                callAes.pu8Out = au8Cipher;
                callAes.u32OutLen = sizeof(au8Cipher);
                callAes.u32Crc = 0;
                callAes.pu8IV = au8Iv;
                callAes.pu8Key = KeyType[i].pu8Key;
                callAes.u16Opt = ModeType[j].u32Val | AIR_AES_OPT_MODE_ENCRYPT | KeyType[i].u32Size;
                callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);
                timer_init_ms(1);
                AIRAES_EncDec(&callAes);                
                get_time(time);
                time_add(time, timeTotal + 0, timeTotal + 0);

                //AES dec             
                callAes.pu8In = au8Cipher;
                callAes.u32InLen = sizeof(au8Cipher);
                callAes.pu8Out = au8Plain[1];
                callAes.u32OutLen = sizeof(au8Plain[1]);
                callAes.u32Crc = 0;
                callAes.pu8IV = au8Iv;
                callAes.pu8Key = KeyType[i].pu8Key;
                callAes.u16Opt = ModeType[j].u32Val | AIR_AES_OPT_MODE_DECRYPT | KeyType[i].u32Size;
                callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);
                timer_init_ms(1);
                AIRAES_EncDec(&callAes);               
                get_time(time + 1);
                time_add(time + 1, timeTotal + 1, timeTotal + 1);

                if(!memcmp(au8Plain[0], au8Plain[1], sizeof(au8Plain[0])))
                {
                    DBG_PRINT("AES %s %s Test Pass...\n", ModeType[j].pu8Name, KeyType[i].pu8Name);
                }
                else
                {
                    DBG_PRINT("AES %s %s Test Fail...\n", ModeType[j].pu8Name, KeyType[i].pu8Name);  
                    while(1);
                }
            }
            DBG_PRINT("\n");
            time_avg(timeTotal + 0, AES_TIME_TEST_COUNT, time + 0);
            time_avg(timeTotal + 1, AES_TIME_TEST_COUNT, time + 1);
            DBG_PRINT("Total %dms %dus avg %dms %dus %s %s enc time_sum\n", 
            timeTotal[0].u32Ms, timeTotal[0].u32Us,  time[0].u32Ms, time[0].u32Us, ModeType[j].pu8Name, KeyType[i].pu8Name);
            DBG_PRINT("Total %dms %dus avg %dms %dus  %s %s dec time_sum\n", 
            timeTotal[1].u32Ms, timeTotal[1].u32Us,  time[1].u32Ms, time[1].u32Us, ModeType[j].pu8Name, KeyType[i].pu8Name);
            DBG_PRINT("\n");
        }
    }
}

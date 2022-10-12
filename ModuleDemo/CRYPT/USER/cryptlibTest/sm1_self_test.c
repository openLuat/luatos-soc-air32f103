#include "air32f10x.h"
#include <string.h>
#include <stdio.h>
#include "air_sm1.h"
#include "debug.h"
#include "air_rand.h"


static const uint8_t cau8Plain[16] = {0x20,0xc4,0xbf,0x20,0xc2,0xbc,0x20,0x0a,0xc7,0xb0,0x20,0xd1,0xd4,0x09,0x36,0x0a};

static const uint8_t cu8Cipher128_8[16] = {0x48,0x03,0xb9,0x22,0x2f,0xb8,0x47,0xb6,0x93,0x11,0x61,0x0f,0xe6,0x5b,0x98,0xd0};
static const uint8_t cu8Skey128_8[16] = {0xa2,0x91,0xb3,0xeb,0xa4,0xed,0x22,0x5f,0x1c,0xea,0xa6,0x07,0x62,0x8f,0xb2,0x36};
static const uint8_t cu8Ekey128_8[16] = {0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef,0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};
static const uint8_t cu8Akey128_8[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


//static const uint8_t cu8Cipher128_14[16] = {0x4a,0xcf,0x76,0x29,0x9b,0x50,0x2a,0x16,0x96,0x3e,0xb9,0x8f,0x65,0x1f,0x47,0xd0};
//static const uint8_t cu8Skey128_14[16] = {0xa2,0x91,0xb3,0xeb,0xa4,0xed,0x22,0x5f,0x1c,0xea,0xa6,0x07,0x62,0x8f,0xb2,0x36};
//static const uint8_t cu8Ekey128_14[16] = {0x40,0xbb,0x12,0xdd,0x6a,0x82,0x73,0x86,0x7f,0x35,0x29,0xd3,0x54,0xb4,0xa0,0x26};
//static const uint8_t cu8Akey128_14[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static const uint8_t cu8Cipher192_10[16] = {0xc4,0x49,0xe8,0x62,0x18,0xe4,0x2a,0xca,0x63,0x30,0xb7,0x81,0x03,0xd7,0xad,0xd0};
static const uint8_t cu8Skey192_10[16] = {0xa2,0x91,0xb3,0xeb,0xa4,0xed,0x22,0x5f,0x1c,0xea,0xa6,0x07,0x62,0x8f,0xb2,0x36};
static const uint8_t cu8Ekey192_10[16] = {0x40,0xBB,0x12,0xDD,0x6A,0x82,0x73,0x86,0x7F,0x35,0x29,0xD3,0x54,0xB4,0xA0,0x26};
static const uint8_t cu8Akey192_10[16] = {0x0C,0x90,0xE1,0x5A,0x4F,0x92,0x36,0xB2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


static const uint8_t cu8Cipher256_12[16] = {0xaa,0xf0,0x79,0x88,0x84,0x1b,0x5c,0x9e,0xaf,0xe6,0x77,0x0f,0x71,0xcd,0x2e,0xee};
static const uint8_t cu8Skey256_12[16] = {0xa2,0x91,0xb3,0xeb,0xa4,0xed,0x22,0x5f,0x1c,0xea,0xa6,0x07,0x62,0x8f,0xb2,0x36};
static const uint8_t cu8Ekey256_12[16] = {0x40,0xBB,0x12,0xDD,0x6A,0x82,0x73,0x86,0x7F,0x35,0x29,0xD3,0x54,0xB4,0xA0,0x26};
static const uint8_t cu8Akey256_12[16] = {0x0C,0x90,0xE1,0x5A,0x4F,0x92,0x36,0xB2,0x12,0x34,0x56,0x78,0x12,0x34,0x56,0x78};


void SM1_128_FuncTest()
{
    uint32_t u32KeyLen;
    uint8_t au8Key[48];//, au8Out[16];
    AIR_SYM_CRYPT_CALL callSm1;
    uint8_t au8Cipher128_8[16] = {0};
    uint8_t au8Plain128_8[16] = {0};
    uint8_t au8Iv[16] = {0};
    
    memset(au8Key, 0, sizeof(au8Key));
    u32KeyLen = 0;
    memcpy(au8Key, cu8Skey128_8, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, cu8Ekey128_8, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, cu8Akey128_8, 16);

    memset(&callSm1, 0, sizeof(callSm1));
    // sm1_128_8 ecb enc
    callSm1.pu8In = (uint8_t *)cau8Plain;
    callSm1.u32InLen = sizeof(cau8Plain);
    callSm1.pu8Out = au8Cipher128_8;
    callSm1.u32OutLen = sizeof(au8Cipher128_8);
    callSm1.pu8IV = au8Iv;
    callSm1.pu8Key = au8Key;
    callSm1.u16Opt = AIR_SM1_OPT_MODE_ENCRYPT | AIR_SM1_OPT_KEY_128 | AIR_SM1_OPT_BLK_CBC | AIR_SM1_OPT_ROUND_8;
    callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRSM1_EncDec(&callSm1);
    ouputRes("SM1 CBC.\n", au8Cipher128_8, 16);
    r_printf(!memcmp(au8Cipher128_8,cu8Cipher128_8,16), "SM1 128_8 ECB Encrypt Test\n");
    
    // sm1_128_8 ecb dec
    callSm1.pu8In = (uint8_t *)au8Cipher128_8;
    callSm1.u32InLen = sizeof(au8Cipher128_8);
    callSm1.pu8Out = au8Plain128_8;
    callSm1.u32OutLen = sizeof(au8Plain128_8);
    callSm1.pu8Key = au8Key;
    callSm1.u16Opt = AIR_SM1_OPT_MODE_DECRYPT | AIR_SM1_OPT_KEY_128 | AIR_SM1_OPT_BLK_ECB | AIR_SM1_OPT_ROUND_8;
    callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRSM1_EncDec(&callSm1);
    r_printf(!memcmp(cau8Plain,au8Plain128_8,16), "SM1 128_8 ECB Decrypt  Test\n");
}

void SM1_192_FuncTest()
{
    uint32_t u32KeyLen;
    uint8_t au8Key[48];//, au8Out[16];
    AIR_SYM_CRYPT_CALL callSm1;
    uint8_t au8Cipher192_10[16] = {0};
    uint8_t au8Plain192_10[16] = {0};
    
    memset(au8Key, 0, sizeof(au8Key));
    u32KeyLen = 0;
    memcpy(au8Key, cu8Skey192_10, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, cu8Ekey192_10, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, cu8Akey192_10, 16);
    
    memset(&callSm1, 0, sizeof(callSm1));
    // sm1_192_10 ecb enc
    callSm1.pu8In = (uint8_t *)cau8Plain;
    callSm1.u32InLen = sizeof(cau8Plain);
    callSm1.pu8Out = au8Cipher192_10;
    callSm1.u32OutLen = sizeof(au8Cipher192_10);
    callSm1.pu8Key = au8Key;
    callSm1.u16Opt = AIR_SM1_OPT_MODE_ENCRYPT | AIR_SM1_OPT_KEY_192 | AIR_SM1_OPT_BLK_ECB | AIR_SM1_OPT_ROUND_10;
    callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRSM1_EncDec(&callSm1);
    r_printf(!memcmp(au8Cipher192_10,cu8Cipher192_10,16), "SM1 192_10 ECB Encrypt Test\n");
    
    // sm1_192_10 ecb dec
    callSm1.pu8In = (uint8_t *)cu8Cipher192_10;
    callSm1.u32InLen = sizeof(cu8Cipher192_10);
    callSm1.pu8Out = au8Plain192_10;
    callSm1.u32OutLen = sizeof(au8Plain192_10);
    callSm1.pu8Key = au8Key;
    callSm1.u16Opt = AIR_SM1_OPT_MODE_DECRYPT | AIR_SM1_OPT_KEY_192 | AIR_SM1_OPT_BLK_ECB | AIR_SM1_OPT_ROUND_10;
    callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRSM1_EncDec(&callSm1);
    r_printf(!memcmp(cau8Plain,au8Plain192_10,16), "SM1 192_10 ECB Decrypt  Test\n");
}

void SM1_256_FuncTest()
{
    uint32_t u32KeyLen;
    uint8_t au8Key[48];//, au8Out[16];
    AIR_SYM_CRYPT_CALL callSm1;
    uint8_t au8Cipher256_12[16] = {0};
    uint8_t au8Plain256_12[16] = {0};
    
    memset(au8Key, 0, sizeof(au8Key));
    u32KeyLen = 0;
    memcpy(au8Key, cu8Skey256_12, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, cu8Ekey256_12, 16);
    u32KeyLen += 16;
    memcpy(au8Key + u32KeyLen, cu8Akey256_12, 16);
    
    memset(&callSm1, 0, sizeof(callSm1));
    // sm1_256_12 ecb enc
    callSm1.pu8In = (uint8_t *)cau8Plain;
    callSm1.u32InLen = sizeof(cau8Plain);
    callSm1.pu8Out = au8Cipher256_12;
    callSm1.u32OutLen = sizeof(au8Cipher256_12);
    callSm1.pu8Key = au8Key;
    callSm1.u16Opt = AIR_SM1_OPT_MODE_ENCRYPT | AIR_SM1_OPT_KEY_256 | AIR_SM1_OPT_BLK_ECB | AIR_SM1_OPT_ROUND_12;
    callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRSM1_EncDec(&callSm1);
    r_printf(!memcmp(cu8Cipher256_12,au8Cipher256_12,16), "SM1 256_12 ECB Encrypt Test\n");
    
    // sm1_192_10 ecb dec
    callSm1.pu8In = (uint8_t *)cu8Cipher256_12;
    callSm1.u32InLen = sizeof(cu8Cipher256_12);
    callSm1.pu8Out = au8Plain256_12;
    callSm1.u32OutLen = sizeof(au8Plain256_12);
    callSm1.pu8Key = au8Key;
    callSm1.u16Opt = AIR_SM1_OPT_MODE_DECRYPT | AIR_SM1_OPT_KEY_256 | AIR_SM1_OPT_BLK_ECB | AIR_SM1_OPT_ROUND_12;
    callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRSM1_EncDec(&callSm1);
    r_printf(!memcmp(cau8Plain,au8Plain256_12,16), "SM1 256_12 ECB Decrypt  Test\n"); 
}

#define SM1_TIME_TEST_DATA              (128)
typedef struct
{
    uint8_t     u8KLen;
    uint8_t *   pu8Key;
    uint32_t    u32Size;
    char *      pu8Name;
}SM1_KEY_TYP;

typedef struct
{
    uint32_t    u32Val;
    char *      pu8Name;
}SM1_MODE_TYP;

typedef struct
{
    uint32_t    u32Round;
    char *      pu8Name;
}SM1_ROUND_NUM;

void SM1_Rand_Test(void)
{
    uint32_t i, j, k, t;
    uint32_t u32MsgLen = 0;
    AIR_SYM_CRYPT_CALL callSm1;  
    SM1_MODE_TYP ModeType[2] = {{AIR_SM1_OPT_BLK_ECB, "ECB"},
                             {AIR_SM1_OPT_BLK_CBC, "CBC"}};
    SM1_KEY_TYP KeyType[3] = {{32, NULL, AIR_SM1_OPT_KEY_128, "KEY_128"},
                           {40, NULL, AIR_SM1_OPT_KEY_192, "KEY_192"},
                           {48, NULL, AIR_SM1_OPT_KEY_256, "KEY_256"}};
    SM1_ROUND_NUM RoundNum[4] = {{AIR_SM1_OPT_ROUND_8,"8_round"},
                                {AIR_SM1_OPT_ROUND_10,"10_round"},
                                {AIR_SM1_OPT_ROUND_12,"12_round"},
                                {AIR_SM1_OPT_ROUND_14,"14_round"}};
    uint8_t au8Plain[2][SM1_TIME_TEST_DATA];
    uint8_t au8Cipher[SM1_TIME_TEST_DATA];
    uint8_t au8Iv[16] = {0};
    uint8_t au8IvTmp[16] = {0};
    uint8_t au8Key[48] = {0}; 
//    uint8_t au8Key128[128/8] = {0}; 
//    uint8_t au8Key192[192/8] = {0};
//    uint8_t au8Key256[256/8] = {0};
    KeyType[0].pu8Key = au8Key;
    KeyType[1].pu8Key = au8Key;
    KeyType[2].pu8Key = au8Key;    
    DBG_PRINT("\n SM1 Test Start\n");
    memset(&callSm1, 0, sizeof(callSm1));
    //key 128 192 256
    for (i = 0; i < 3; i++)
    {
        //pack mode ECB CBC
        for (j = 0; j < 2; j++)
        {
            //round num 8 10 12 14
            for(k = 0; k < 4; k++)
            {
                memset(au8Plain, 0, sizeof(au8Plain));
                memset(au8Cipher, 0, sizeof(au8Cipher));
                memset(au8Plain[1], 0, sizeof(au8Plain[1]));
                memset(au8Iv,0,sizeof(au8Iv));
                memset(KeyType[i].pu8Key,0,sizeof(au8Key));
                AIRRAND_Prand(&u32MsgLen,sizeof(u32MsgLen));
                u32MsgLen = (u32MsgLen & 0x7f) + 1;
                AIRRAND_Prand(au8Plain,u32MsgLen);
                AIRRAND_Prand(au8Iv,sizeof(au8Iv));
                AIRRAND_Prand(KeyType[i].pu8Key,KeyType[i].u8KLen);
                memcpy(au8IvTmp, au8Iv, sizeof(au8Iv));
                
                memset(&callSm1, 0, sizeof(callSm1));
                //SM1 enc
                callSm1.pu8In = au8Plain[0];
                callSm1.u32InLen = u32MsgLen;
                callSm1.pu8Out = au8Cipher;
                callSm1.u32OutLen = ((u32MsgLen + 15)>>4)<<4;                
                callSm1.pu8IV = au8IvTmp;
                callSm1.pu8Key = KeyType[i].pu8Key;
                callSm1.u16Opt = ModeType[j].u32Val | AIR_SM1_OPT_MODE_ENCRYPT | KeyType[i].u32Size | RoundNum[k].u32Round;
                callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);               
                if(AIR_RET_SM1_SUCCESS != AIRSM1_EncDec(&callSm1))
                {
                    r_printf(0, "SM1 enc test\n");
                }

                //SM1 dec   
                memcpy(au8IvTmp, au8Iv, sizeof(au8Iv));           
                callSm1.pu8In = au8Cipher;
                callSm1.u32InLen = ((u32MsgLen + 15)>>4)<<4;
                callSm1.pu8Out = au8Plain[1];
                callSm1.u32OutLen = ((u32MsgLen + 15)>>4)<<4;
                callSm1.pu8IV = au8IvTmp;
                callSm1.pu8Key = KeyType[i].pu8Key;
                callSm1.u16Opt = ModeType[j].u32Val | AIR_SM1_OPT_MODE_DECRYPT | KeyType[i].u32Size | RoundNum[k].u32Round;
                callSm1.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm1, sizeof(AIR_SYM_CRYPT_CALL)-4);
                if(AIR_RET_SM1_SUCCESS != AIRSM1_EncDec(&callSm1))
                {
                    r_printf(0, "SM1 dec test\n");
                }                    
                t = (!memcmp(au8Plain[0], au8Plain[1], u32MsgLen));
                DBG_PRINT(" %s %s %s SM1 Test ", ModeType[j].pu8Name, KeyType[i].pu8Name, RoundNum[k].pu8Name);
                r_printf(t, "\n");
            }
        }
        
    }
}

uint32_t SM1_Check(void)
{
    uint8_t au8SM1_KEY[48] = {0xa2, 0x91, 0xb3, 0xeb, 0xa4, 0xed, 0x22, 0x5f, 0x1c, 0xea, 0xa6, 0x07, 0x62, 0x8f, 0xb2, 0x36,
                                0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t au8SM1_PLAIN[16] = {0x20, 0xc4, 0xbf, 0x20, 0xc2, 0xbc, 0x20, 0x0a, 0xc7, 0xb0, 0x20, 0xd1, 0xd4, 0x09, 0x36, 0x0a};
    uint8_t au8SM1_CIPHER[16] = {0x48, 0x03, 0xB9, 0x22, 0x2F, 0xB8, 0x47, 0xB6, 0x93, 0x11, 0x61, 0x0F, 0xE6, 0x5B, 0x98, 0xD0};
    uint8_t au8Out[16];
    AIR_SYM_CRYPT_CALL call;
    uint32_t u32Ret;
    uint8_t au8Iv[16] = {0};
    memset(&call, 0, sizeof(call));
    call.pu8In = (uint8_t *)au8SM1_PLAIN;
    call.pu8Out = au8Out;
    call.pu8IV = au8Iv;
    call.pu8Key = (uint8_t *)au8SM1_KEY;
    call.u16Opt = 0;
    call.u32InLen = 16;
    call.u32OutLen = 16;
    call.u16Opt = AIR_SM1_OPT_MODE_ENCRYPT | AIR_SM1_OPT_KEY_128 | AIR_SM1_OPT_BLK_CBC | AIR_SM1_OPT_ROUND_8;
    call.u32Crc = AIRCRC_CalcBuff(0xffff, &call, sizeof(call)-4);
    
    AIRSM1_EncDec(&call);
    if(0!=(memcmp(au8Out, au8SM1_CIPHER, sizeof(au8SM1_CIPHER))))
	{
		return -1;
	}
    return 0;
}

void SM1_Test(void)
{
    SM1_Check();
    SM1_128_FuncTest();
    SM1_192_FuncTest();
    SM1_256_FuncTest();
    SM1_Rand_Test();
}













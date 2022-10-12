#include <stdio.h>
#include "air_sm7.h"
#include "debug.h"
#include "air_rand.h"


void SM7_Func_Test(void)
{
    uint8_t au8Cipher1[8] = {0xCE,0x3C,0x08,0xD4, 0x02,0xAE,0x24,0x7C};
    uint8_t  au8Key1[16] = {0x1F,0xD3,0x84,0xD8,0x6B,0x50,0xBE,0x01,0x21,0x43,0xD6,0x16,0x18,0x15,0x19,0x83};
    uint8_t  au8Plain1[8] = {0xE2,0x73,0x2F,0xB8,0x1D,0x7D,0x7E,0x51};
    
    uint8_t au8Cipher2[8] = {0xbe,0x87,0x60,0x5b,0xe2,0xc6,0x1f,0xfa};
    uint8_t  au8Key2[16] = {0x07,0xad,0xe6,0x58,0x45,0x53,0x62,0x6a,0xc8,0x1b,0x1b,0x3d,0x7f,0x4f,0xdf,0x5d};
    uint8_t  au8Plain2[8] = {0x49,0x7b,0x94,0x5f,0x68,0xd8,0xa0,0x7d};
    uint8_t  au8Iv[8] = {0xb0,0xa0,0x47,0x56,0xcb,0x21,0x80,0x71};
    
//    uint8_t au8Cipher1[8] = {0xEB,0x84,0xD6,0x84, 0x9E,0xF6,0xED,0x8B};
//    uint8_t  au8Key1[16] = {0x11,0x22,0x33,0x44, 0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00};
//    uint8_t  au8Plain1[8] = {0x46,0x14,0x76,0xC9, 0x45,0x01,0x20,0x01};
//    
//    uint8_t au8Cipher2[8] = {0x49,0x7b,0x94,0x5f,0x68,0xd8,0xa0,0x7d};
//    uint8_t  au8Key2[16] = {0x07,0xad,0xe6,0x58,0x45,0x53,0x62,0x6a,0xc8,0x1b,0x1b,0x3d,0x7f,0x4f,0xdf,0x5d};
//    uint8_t  au8Plain2[8] = {0x18,0x83,0x19,0xd3,0xeb,0x78,0x72,0xaa};
//    uint8_t  au8Iv[8] = {0xb0,0xa0,0x47,0x56,0xcb,0x21,0x80,0x71};
    
    uint8_t au8mCipher[8];
    uint8_t au8mPlain[8];   
    AIR_SYM_CRYPT_CALL callSm7;
 
    memset(&callSm7, 0, sizeof(callSm7));
    //ECB    
    callSm7.pu8In = au8Plain1;
    callSm7.pu8Key = au8Key1;
    callSm7.pu8Out = au8mCipher;
    callSm7.u32InLen = sizeof(au8Plain1);
    callSm7.u32OutLen = sizeof(au8mCipher);
    callSm7.u16Opt = AIR_SM7_OPT_BLK_ECB | AIR_SM7_OPT_MODE_ENCRYPT;
    callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);      
    AIRSM7_EncDec(&callSm7);
    r_printf(!memcmp(au8mCipher, au8Cipher1, sizeof(au8Plain1)), "SM7 ECB ENC Test\n");
    
    callSm7.pu8In = au8Cipher1;
    callSm7.pu8Key = au8Key1;
    callSm7.pu8Out = au8mPlain;
    callSm7.u32InLen = sizeof(au8Cipher1);
    callSm7.u32OutLen = sizeof(au8mPlain);
    callSm7.u16Opt = AIR_SM7_OPT_BLK_ECB | AIR_SM7_OPT_MODE_DECRYPT;
    callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);    
    AIRSM7_EncDec(&callSm7);     
    r_printf(!memcmp(au8Plain1, au8mPlain, sizeof(au8Plain1)), "SM7 ECB DEC Test\n");   

    //CBC   
    callSm7.pu8In = au8Plain2;
    callSm7.pu8IV = au8Iv;
    callSm7.pu8Key = au8Key2;
    callSm7.pu8Out = au8mCipher;
    callSm7.u32InLen = sizeof(au8Plain2);
    callSm7.u32OutLen = sizeof(au8mCipher);
    callSm7.u16Opt = AIR_SM7_OPT_BLK_CBC | AIR_SM7_OPT_MODE_ENCRYPT;
    callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);       
    AIRSM7_EncDec(&callSm7);
    r_printf(!memcmp(au8mCipher, au8Cipher2, sizeof(au8Plain2)), "SM7 CBC ENC Test\n");
    
    callSm7.pu8In = au8Cipher2;
    callSm7.pu8IV = au8Iv;
    callSm7.pu8Key = au8Key2;
    callSm7.pu8Out = au8mPlain;
    callSm7.u32InLen = sizeof(au8Cipher2);
    callSm7.u32OutLen = sizeof(au8mPlain);
    callSm7.u16Opt = AIR_SM7_OPT_BLK_CBC | AIR_SM7_OPT_MODE_DECRYPT;
    callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);    
    AIRSM7_EncDec(&callSm7);  
    r_printf(!memcmp(au8Plain2, au8mPlain, sizeof(au8Plain2)), "SM7 CBC ENC/DEC Test\n");    
}
 

#define SM7_TIME_TEST_DATA  1024

typedef struct
{
    uint32_t    u32Val;
    char *      pu8Name;
}SM7_MODE_TYP;

void SM7_Rand_Test(void)
{
    uint32_t i, t;
    uint32_t u32MsgLen = 0;
    AIR_SYM_CRYPT_CALL callSm7;  
    SM7_MODE_TYP ModeType[2] = {{AIR_SM7_OPT_BLK_ECB, "ECB"},
                             {AIR_SM7_OPT_BLK_CBC, "CBC"}};
    uint8_t au8Plain[2][SM7_TIME_TEST_DATA];
    uint8_t au8Cipher[SM7_TIME_TEST_DATA]; 
    uint8_t au8Iv[8] = {0};
    uint8_t au8Key[16] = {0};
    DBG_PRINT("\n SM7 Test Start\n");
    memset(&callSm7, 0, sizeof(callSm7));
    for (i = 0; i < 2; i++)
    {
        memset(au8Plain, 0, sizeof(au8Plain));
        memset(au8Cipher, 0, sizeof(au8Cipher));
        memset(au8Iv,0,sizeof(au8Iv));
        memset(au8Key,0,sizeof(au8Key));
        AIRRAND_Prand(&u32MsgLen,sizeof(u32MsgLen));
        u32MsgLen = (u32MsgLen & 0x03ff) + 1;
        AIRRAND_Prand(au8Plain,u32MsgLen);
        AIRRAND_Prand(au8Iv,sizeof(au8Iv));
        AIRRAND_Prand(au8Key,sizeof(au8Key));
        
        memset(&callSm7, 0, sizeof(callSm7));
        //SM7 Enc
        callSm7.pu8In = au8Plain[0];
        callSm7.pu8Out = au8Cipher;
        callSm7.pu8IV = au8Iv;
        callSm7.pu8Key = au8Key;
        callSm7.u32InLen = u32MsgLen;
        callSm7.u32OutLen = ((u32MsgLen + 7)>>3)<<3;
        callSm7.u16Opt = AIR_SM7_OPT_MODE_ENCRYPT | ModeType[i].u32Val;
        callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);
        if(AIR_RET_SM7_SUCCESS != AIRSM7_EncDec(&callSm7))
        {
            r_printf(0, "SM7 enc test\n");
        }
        //SM7 Dec
        callSm7.pu8In = au8Cipher;
        callSm7.u32InLen = ((u32MsgLen + 7)>>3)<<3;
        callSm7.pu8Out = au8Plain[1];
        callSm7.u32OutLen = ((u32MsgLen + 7)>>3)<<3;
        callSm7.pu8IV = au8Iv;
        callSm7.pu8Key = au8Key;
        callSm7.u16Opt = ModeType[i].u32Val | AIR_SM7_OPT_MODE_DECRYPT;
        callSm7.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm7, sizeof(AIR_SYM_CRYPT_CALL)-4);
        if(AIR_RET_SM7_SUCCESS != AIRSM7_EncDec(&callSm7))
        {
            r_printf(0, "SM7 dec test\n");
        }      
        t = (!memcmp(au8Plain[0], au8Plain[1], u32MsgLen));
        DBG_PRINT(" %s SM7 Test ", ModeType[i].pu8Name);
        r_printf(t, "\n");
    }
}

void SM7_Test()
{
    SM7_Func_Test();
    SM7_Rand_Test();
}
















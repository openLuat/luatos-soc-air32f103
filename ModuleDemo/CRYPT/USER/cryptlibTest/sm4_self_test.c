#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_sm4.h"
#include "debug.h"
#include "air_rand.h"


#define SM4_TIME_TEST_DATA  (128)

typedef struct
{
    uint32_t    u32Val;
    char *      pu8Name;
}SM4_MODE_TYP;

void SM4_Rand_Test(void)
{
    uint32_t i, t;
    uint32_t u32MsgLen = 0;
    AIR_SYM_CRYPT_CALL callSm4;  
    SM4_MODE_TYP ModeType[2] = {{AIR_SM4_OPT_BLK_ECB, "ECB"},
                             {AIR_SM4_OPT_BLK_CBC, "CBC"}};
    uint8_t au8Plain[2][SM4_TIME_TEST_DATA];
    uint8_t au8Cipher[SM4_TIME_TEST_DATA]; 
    uint8_t au8Iv[16] = {0};
    uint8_t au8Key[16] = {0};
    uint8_t au8IvTmp[16] = {0};
    DBG_PRINT("\nSM4 Test Start\n");
    memset(&callSm4, 0, sizeof(callSm4));
    for (i = 0; i < 2; i++)
    {
        memset(au8Plain, 0, sizeof(au8Plain));
        memset(au8Cipher, 0, sizeof(au8Cipher));
        memset(au8Iv,0,sizeof(au8Iv));
        memset(au8Key,0,sizeof(au8Key));
        AIRRAND_Prand(&u32MsgLen,sizeof(u32MsgLen));
        u32MsgLen = (u32MsgLen & 0x7f) + 1;
//        u32MsgLen = 32;
        AIRRAND_Prand(au8Plain,u32MsgLen);
        AIRRAND_Prand(au8Iv,sizeof(au8Iv));
        AIRRAND_Prand(au8Key,sizeof(au8Key));
        //DES Enc
		memset((uint8_t *)&callSm4, 0, sizeof(callSm4));
        callSm4.pu8In = au8Plain[0];
        callSm4.pu8Out = au8Cipher;
        callSm4.pu8IV = au8Iv;
        callSm4.pu8Key = au8Key;
        callSm4.u32InLen = u32MsgLen;
        callSm4.u32OutLen = ((u32MsgLen + 15)>>4)<<4;
        callSm4.u16Opt = AIR_SM4_OPT_MODE_ENCRYPT | ModeType[i].u32Val;
        callSm4.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm4, sizeof(AIR_SYM_CRYPT_CALL)-4);
        if(AIR_RET_SM4_SUCCESS != AIRSM4_EncDec(&callSm4))
        {
            r_printf(0, "SM4 enc test\n");
        }
        //DES Dec
        callSm4.pu8In = au8Cipher;
        callSm4.u32InLen = ((u32MsgLen + 15)>>4)<<4;
        callSm4.pu8Out = au8Plain[1];
        callSm4.u32OutLen = ((u32MsgLen + 15)>>4)<<4;
        callSm4.pu8IV = au8Iv;
        callSm4.pu8Key = au8Key;
//        AIRRAND_Prand(&callSm4.u32Rand, sizeof(callSm4.u32Rand));
        callSm4.u16Opt = ModeType[i].u32Val | AIR_SM4_OPT_MODE_DECRYPT;
        callSm4.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm4, sizeof(AIR_SYM_CRYPT_CALL)-4);
        if(AIR_RET_SM4_SUCCESS != AIRSM4_EncDec(&callSm4))
        {
            r_printf(0, "SM4 dec test\n");
        }                    
        t = (!memcmp(au8Plain[0], au8Plain[1], u32MsgLen));
        DBG_PRINT("%s SM4 Test ", ModeType[i].pu8Name);
        r_printf(t, "\n");
    }
}

#define SM4_TEST_LEN    (64)
void SM4_Func_Test()
{
    uint8_t au8Plain[SM4_TEST_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                            0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                            0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40};
    uint8_t au8SM4Cipher_ECB[SM4_TEST_LEN] = {0xA6, 0x38, 0x95, 0x5D, 0x5B, 0xCB, 0x14, 0x94, 0x48, 0x8A, 0x29, 0x49, 0xA1, 0x5F, 0xD1, 0x38,
                                    0x43, 0x29, 0xA6, 0x24, 0x1E, 0x39, 0xAD, 0x7A, 0x9A, 0x40, 0x4A, 0x81, 0x4A, 0x7E, 0xDD, 0x32,
                                    0x7D, 0xCF, 0x36, 0x31, 0xCF, 0x64, 0x07, 0x99, 0x0E, 0x84, 0x4D, 0xE9, 0x85, 0xC3, 0xCA, 0xCA,
                                    0x81, 0x3A, 0xB8, 0xD4, 0xF4, 0xBB, 0x66, 0x72, 0x9F, 0x30, 0x7F, 0x92, 0x30, 0x10, 0x00, 0x43};
    uint8_t au8SM4Cipher_CBC[SM4_TEST_LEN] = {0xDD, 0x60, 0x70, 0x8C, 0xF5, 0xE1, 0xD9, 0x66, 0x40, 0xA5, 0xE1, 0x5D, 0x95, 0x61, 0x3F, 0xAC,
                                    0x52,0xB9, 0x6A, 0xE4, 0xDF, 0xF2, 0x97, 0x57, 0x46, 0x48, 0xDA, 0x86, 0x5B, 0x01, 0xD4, 0xBF,
                                    0x22,0xD5, 0x1F, 0x2E, 0x8E, 0xCC, 0xAE, 0x85, 0xF9, 0x21, 0x42, 0xBE, 0x91, 0xE5, 0x53, 0xB0,
                                    0x9E,0x81, 0x10, 0x13, 0x6F, 0xB6, 0xDC, 0xBF, 0xB2, 0x25, 0x94, 0x93, 0xD5, 0x22, 0x2D, 0x49};
    uint8_t au8Result[SM4_TEST_LEN] = {0};
    uint8_t au8Iv[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};    
    uint8_t au8Key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
    AIR_SYM_CRYPT_CALL callSm4; 

    memset(&callSm4, 0, sizeof(callSm4));
    //ECB ENC
    callSm4.pu8In = au8Plain;
    callSm4.u32InLen = SM4_TEST_LEN;
    callSm4.pu8Out = au8Result;
    callSm4.u32OutLen = SM4_TEST_LEN;                
    callSm4.pu8Key = au8Key;
    callSm4.u16Opt = AIR_SM4_OPT_BLK_ECB | AIR_SM4_OPT_MODE_ENCRYPT;
    callSm4.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm4, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRSM4_EncDec(&callSm4);
    r_printf((0 == memcmp(au8SM4Cipher_ECB, au8Result, SM4_TEST_LEN)), "air_sm4_ecb test\n");
    
    //CBC DEC
    callSm4.pu8In = au8SM4Cipher_CBC;
    callSm4.u32InLen = SM4_TEST_LEN;
    callSm4.pu8Out = au8Result;
    callSm4.u32OutLen = SM4_TEST_LEN;                
    callSm4.pu8IV = au8Iv;
    callSm4.u16Opt = AIR_SM4_OPT_BLK_CBC | AIR_SM4_OPT_MODE_DECRYPT;
    callSm4.pu8Key = au8Key;
    callSm4.u32Crc = AIRCRC_CalcBuff(0xffff, &callSm4, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRSM4_EncDec(&callSm4);
    r_printf((0 == memcmp(au8Plain, au8Result, SM4_TEST_LEN)), "air_sm4_cbc test\n");
}

void SM4_Test()
{
    SM4_Func_Test();
    SM4_Rand_Test();
}

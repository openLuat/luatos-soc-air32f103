#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_aes.h"
#include "debug.h"
#include "air_rand.h"

#define AES_TIME_TEST_DATA              1024
//#define AES_TIME_TEST_COUNT             10

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

void AES_Rand_Test()
{
    uint32_t i, j, t;
    uint32_t u32MsgLen = 0;
    AIR_SYM_CRYPT_CALL callAes = {0};  
    AES_MODE_TYP ModeType[2] = {{AIR_AES_OPT_BLK_ECB, "ECB"},
                             {AIR_AES_OPT_BLK_CBC, "CBC"}};
    AES_KEY_TYP KeyType[3] = {{16, NULL, AIR_AES_OPT_KEY_128, "KEY_128"},
                           {24, NULL, AIR_AES_OPT_KEY_192, "KEY_192"},
                           {32, NULL, AIR_AES_OPT_KEY_256, "KEY_256"}};
    uint8_t au8Plain[2][AES_TIME_TEST_DATA];
    uint8_t au8Cipher[AES_TIME_TEST_DATA];
    uint8_t au8Iv[16] = {0};
    uint8_t au8Key128[128/8] = {0}; 
    uint8_t au8Key192[192/8] = {0};
    uint8_t au8Key256[256/8] = {0};
    KeyType[0].pu8Key = au8Key128;
    KeyType[1].pu8Key = au8Key192;
    KeyType[2].pu8Key = au8Key256;    
    DBG_PRINT("\nAES Test Start\n");
    //key 128 192 256
    for (i = 0; i < 3; i++)
    {
        //pack mode ECB CBC
        for (j = 0; j < 2; j++)
        {
                memset(au8Plain, 0, sizeof(au8Plain));
                memset(au8Cipher, 0, sizeof(au8Cipher));
                memset(au8Plain[1], 0, sizeof(au8Plain[1]));
                memset(au8Iv,0,sizeof(au8Iv));
                memset(KeyType[i].pu8Key,0,KeyType[i].u8KLen);
                AIRRAND_Prand(&u32MsgLen,sizeof(u32MsgLen));
                u32MsgLen = (u32MsgLen & 0x3ff) + 1;
                AIRRAND_Prand(au8Plain,u32MsgLen);
                AIRRAND_Prand(au8Iv,sizeof(au8Iv));
                AIRRAND_Prand(KeyType[i].pu8Key,KeyType[i].u8KLen);
                
				memset((uint8_t*)&callAes, 0, sizeof(callAes));
                //AES enc
                callAes.pu8In = au8Plain[0];
                callAes.u32InLen = u32MsgLen;
                callAes.pu8Out = au8Cipher;
                callAes.u32OutLen = ((u32MsgLen + 15)>>4)<<4;                
                callAes.pu8IV = au8Iv;
                callAes.pu8Key = KeyType[i].pu8Key;
                callAes.u16Opt = ModeType[j].u32Val | AIR_AES_OPT_MODE_ENCRYPT | KeyType[i].u32Size;
                callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);               
                if(AIR_RET_AES_SUCCESS != AIRAES_EncDec(&callAes))
                {
                    r_printf(0, "Aes enc test\n");
                }
				
				memset((uint8_t*)&callAes, 0, sizeof(callAes));

                //AES dec            
                callAes.pu8In = au8Cipher;
                callAes.u32InLen = ((u32MsgLen + 15)>>4)<<4;
                callAes.pu8Out = au8Plain[1];
                callAes.u32OutLen = ((u32MsgLen + 15)>>4)<<4;
                callAes.pu8IV = au8Iv;
                callAes.pu8Key = KeyType[i].pu8Key;
                callAes.u16Opt = ModeType[j].u32Val | AIR_AES_OPT_MODE_DECRYPT | KeyType[i].u32Size;
                callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);
                if(AIR_RET_AES_SUCCESS != AIRAES_EncDec(&callAes))
                {
                    r_printf(0, "Aes dec test\n");
                }                    
                
                t = (!memcmp(au8Plain[0], au8Plain[1], u32MsgLen));
                DBG_PRINT("%s %s AES Test ", ModeType[j].pu8Name, KeyType[i].pu8Name);
                r_printf(t, "\n");
        }
    }
}

#define AES_TEST_LEN    (64)
void AES_Func_Test()
{
    uint8_t au8Plain[AES_TEST_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                            0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                            0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40};
    uint8_t au8AESCipher_ECB[AES_TEST_LEN] = {0x34, 0xC3, 0x3B, 0x7F, 0x14, 0xFD, 0x53, 0xDC, 0xEA, 0x25, 0xE0, 0x1A, 0x02, 0xE1, 0x67, 0x27,
                                    0xD7, 0x21, 0xA0, 0xF1, 0x94, 0x23, 0x18, 0x22, 0xF3, 0x98, 0x70, 0x6D, 0xD1, 0xFF, 0xF2, 0xB7,
                                    0x03, 0x28, 0xD3, 0x25, 0x75, 0xFA, 0x47, 0x6F, 0x38, 0xF6, 0x24, 0xD6, 0x73, 0xEA, 0xA0, 0x9B,
                                    0xA9, 0x05, 0x0C, 0x10, 0xCE, 0x19, 0x04, 0xE7, 0x50, 0x59, 0x77, 0x97, 0xA1, 0x72, 0xB0, 0xF0};
    uint8_t au8AESCipher_CBC[AES_TEST_LEN] = {0xDB, 0xF1, 0x84, 0x11, 0x2E, 0xB9, 0x11, 0x16, 0x59, 0x71, 0x2B, 0xAF, 0xCF, 0xF2, 0xAB, 0x24,
                                    0xF7, 0x2D, 0x2C, 0x50, 0x44, 0x5D, 0x1D, 0x3B, 0xEB, 0xF8, 0xB5, 0x0D, 0x32, 0x32, 0x88, 0xB4,
                                    0xF6, 0x28, 0xD8, 0xA6, 0x53, 0xB2, 0x20, 0xE2, 0x79, 0x37, 0xBB, 0x41, 0x49, 0xCD, 0xD3, 0x64,
                                    0x67, 0x78, 0xAD, 0xAD, 0x93, 0xB9, 0x81, 0x1D, 0x4F, 0xB5, 0x05, 0xB9, 0xF4, 0x24, 0xEF, 0xB9};
    uint8_t au8Result[AES_TEST_LEN] = {0};
	uint8_t au8PN[AES_TEST_LEN] = {0};
    
	uint8_t au8Iv[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};    
    uint8_t au8Key[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
    AIR_SYM_CRYPT_CALL callAes; 
	
	memset((uint8_t*)&callAes, 0, sizeof(callAes));
    //ECB ENC
    callAes.pu8In = au8Plain;
    callAes.u32InLen = AES_TEST_LEN;
    callAes.pu8Out = au8Result;
    callAes.u32OutLen = AES_TEST_LEN;                
    callAes.pu8Key = au8Key;
    callAes.u16Opt = AIR_AES_OPT_BLK_ECB | AIR_AES_OPT_MODE_ENCRYPT | AIR_AES_OPT_KEY_128;
    callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);
    AIRAES_EncDec(&callAes);
    r_printf((0 == memcmp(au8AESCipher_ECB, au8Result, AES_TEST_LEN)), "air_aes_128_ecb ENC test\n");
    
	memset((uint8_t*)&callAes, 0, sizeof(callAes));
	//ECB DEC
    callAes.pu8In = au8Result;
    callAes.u32InLen = AES_TEST_LEN;
    callAes.pu8Out = au8PN;
    callAes.u32OutLen = AES_TEST_LEN;                
    callAes.pu8Key = au8Key;
    callAes.u16Opt = AIR_AES_OPT_BLK_ECB | AIR_AES_OPT_MODE_DECRYPT | AIR_AES_OPT_KEY_128;
    callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRAES_EncDec(&callAes);
    r_printf((0 == memcmp(au8Plain, au8PN, AES_TEST_LEN)), "air_aes_128_ecb DEC test\n");
    
	
	memset((uint8_t*)&callAes, 0, sizeof(callAes));
	//CBC ENC
    callAes.pu8In = au8Plain;
    callAes.u32InLen = AES_TEST_LEN;
    callAes.pu8Out = au8Result;
    callAes.u32OutLen = AES_TEST_LEN;                
    callAes.pu8IV = au8Iv;
    callAes.pu8Key = au8Key;
    callAes.u16Opt = AIR_AES_OPT_BLK_CBC | AIR_AES_OPT_MODE_ENCRYPT | AIR_AES_OPT_KEY_128;
    callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRAES_EncDec(&callAes);
    r_printf((0 == memcmp(au8AESCipher_CBC, au8Result, AES_TEST_LEN)), "air_aes_128_cbc ENC test\n");
	
	memset((uint8_t*)&callAes, 0, sizeof(callAes));
    //CBC DEC
    callAes.pu8In = au8AESCipher_CBC;
    callAes.u32InLen = AES_TEST_LEN;
    callAes.pu8Out = au8Result;
    callAes.u32OutLen = AES_TEST_LEN;                
    callAes.pu8IV = au8Iv;
    callAes.pu8Key = au8Key;
    callAes.u16Opt = AIR_AES_OPT_BLK_CBC | AIR_AES_OPT_MODE_DECRYPT | AIR_AES_OPT_KEY_128;
    callAes.u32Crc = AIRCRC_CalcBuff(0xffff, &callAes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRAES_EncDec(&callAes);
    r_printf((0 == memcmp(au8Plain, au8Result, AES_TEST_LEN)), "air_aes_128_cbc DEC test\n");
}

void AES_Test()
{
	AES_Func_Test();
    AES_Rand_Test();
}

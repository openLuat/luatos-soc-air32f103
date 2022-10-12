#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_des.h"
#include "debug.h"
#include "air_rand.h"

#define DES_TIME_TEST_DATA  (1024)

typedef struct
{
    uint32_t    u32Val;
    char *      pu8Name;
}DES_MODE_TYP;

void DES_Rand_Test(void)
{
    uint32_t i, t;
    uint32_t u32MsgLen = 0;
    AIR_SYM_CRYPT_CALL callDes;  
    DES_MODE_TYP ModeType[3] = {{AIR_DES_OPT_BLK_ECB, "ECB"},
                             {AIR_DES_OPT_BLK_CBC, "CBC"}};
    uint8_t au8Plain[2][DES_TIME_TEST_DATA];
    uint8_t au8Cipher[DES_TIME_TEST_DATA]; 
    uint8_t au8Iv[8] = {0};
    uint8_t au8Key[8] = {0};
    DBG_PRINT("\nDES Test Start\n");
    for (i = 0; i < 2; i++)
    {
        memset(au8Plain, 0, sizeof(au8Plain));
        memset(au8Cipher, 0, sizeof(au8Cipher));
        memset(au8Iv,0,sizeof(au8Iv));
        memset(au8Key,0,sizeof(au8Key));
        AIRRAND_Prand(&u32MsgLen,sizeof(u32MsgLen));
        u32MsgLen = (u32MsgLen & 0x7f) + 1;
        AIRRAND_Prand(au8Plain,u32MsgLen);
        AIRRAND_Prand(au8Iv,sizeof(au8Iv));
        AIRRAND_Prand(au8Key,sizeof(au8Key));
        
        memset(&callDes,0,sizeof(callDes));
        //DES Enc
        callDes.pu8In = au8Plain[0];
        callDes.pu8Out = au8Cipher;
        callDes.pu8IV = au8Iv;
        callDes.pu8Key = au8Key;
        callDes.u32InLen = u32MsgLen;
        callDes.u32OutLen = ((u32MsgLen + 7)>>3)<<3;
        callDes.u16Opt = AIR_DES_OPT_MODE_ENCRYPT | ModeType[i].u32Val;
        callDes.u32Crc = AIRCRC_CalcBuff(0xffff, &callDes, sizeof(AIR_SYM_CRYPT_CALL)-4);
        if(AIR_RET_DES_SUCCESS != AIRDES_EncDec(&callDes))
        {
            r_printf(0, "Des enc test\n");
        }
        //DES Dec
        callDes.pu8In = au8Cipher;
        callDes.u32InLen = ((u32MsgLen + 7)>>3)<<3;
        callDes.pu8Out = au8Plain[1];
        callDes.u32OutLen = ((u32MsgLen + 7)>>3)<<3;
        callDes.pu8IV = au8Iv;
        callDes.pu8Key = au8Key;
        callDes.u16Opt = ModeType[i].u32Val | AIR_DES_OPT_MODE_DECRYPT;
        callDes.u32Crc = AIRCRC_CalcBuff(0xffff, &callDes, sizeof(AIR_SYM_CRYPT_CALL)-4);
        if(AIR_RET_DES_SUCCESS != AIRDES_EncDec(&callDes))
        {
            r_printf(0, "Des dec test\n");
        }  
        t = (!memcmp(au8Plain[0], au8Plain[1], u32MsgLen));
        DBG_PRINT(" %s DES Test ", ModeType[i].pu8Name);
        r_printf(t, "\n");
    }
}

#define DES_TEST_LEN    (64)
void DES_Func_Test()
{
    uint8_t au8Plain[DES_TEST_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                            0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                            0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40};
    uint8_t au8DESCipher_ECB[DES_TEST_LEN] = {0x77, 0xA7, 0xD6, 0xBC, 0xF5, 0x79, 0x62, 0xB9, 0xA3, 0x2D, 0x3D, 0x85, 0x8F, 0xD3, 0xAF, 0xA6,
                                    0x57, 0x51, 0xC8, 0xDE, 0x22, 0x62, 0x85, 0xAA, 0x68, 0x7B, 0xC9, 0xEF, 0xFE, 0x26, 0xAF, 0x37,
                                    0x00, 0x3A, 0xC7, 0x91, 0x4A, 0x56, 0xDA, 0x27, 0x60, 0xFB, 0x12, 0xDF, 0x2E, 0xFE, 0xDA, 0xC2,
                                    0xB8, 0x6D, 0x92, 0xEB, 0xEE, 0xEC, 0x07, 0xF0, 0xC6, 0x3F, 0xDF, 0x39, 0x8A, 0x3E, 0x8B, 0x04};
    uint8_t au8DESCipher_CBC[DES_TEST_LEN] = {0xB0, 0x73, 0xDC, 0x3F, 0xB2, 0x09, 0x53, 0x6D, 0x93, 0x95, 0x9F, 0x07, 0x24, 0x23, 0x8E, 0x18,
                                    0x25, 0xE2, 0xD1, 0xAB, 0xA7, 0x2B, 0xAF, 0x3E, 0xE2, 0x9D, 0xDE, 0xCC, 0x32, 0x97, 0xBD, 0xEB,
                                    0x51, 0x95, 0x56, 0x58, 0x48, 0x5F, 0x87, 0x41, 0xDE, 0xF4, 0xEC, 0x91, 0x0D, 0x65, 0x1C, 0x09,
                                    0xC4, 0xF0, 0xFB, 0x2E, 0xDA, 0x5B, 0xCF, 0x18, 0x90, 0xBB, 0x0B, 0x8C, 0x92, 0x85, 0x01, 0xD5};
    uint8_t au8Result[DES_TEST_LEN] = {0};
    uint8_t au8Iv[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};    
    uint8_t au8Key[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    AIR_SYM_CRYPT_CALL callDes; 
    
    memset(&callDes,0,sizeof(callDes));
    //ECB ENC
    callDes.pu8In = au8Plain;
    callDes.u32InLen = DES_TEST_LEN;
    callDes.pu8Out = au8Result;
    callDes.u32OutLen = DES_TEST_LEN;                
    callDes.u16Opt = AIR_DES_OPT_BLK_ECB | AIR_DES_OPT_MODE_ENCRYPT;
    callDes.pu8Key = au8Key;
    callDes.u32Crc = AIRCRC_CalcBuff(0xffff, &callDes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRDES_EncDec(&callDes);
    r_printf((0 == memcmp(au8DESCipher_ECB, au8Result, DES_TEST_LEN)), "air_des_ecb test\n");
    
    //CBC DEC
    callDes.pu8In = au8DESCipher_CBC;
    callDes.u32InLen = DES_TEST_LEN;
    callDes.pu8Out = au8Result;
    callDes.u32OutLen = DES_TEST_LEN;                
    callDes.pu8IV = au8Iv;
    callDes.u16Opt = AIR_DES_OPT_BLK_CBC | AIR_DES_OPT_MODE_DECRYPT;
    callDes.pu8Key = au8Key;
    callDes.u32Crc = AIRCRC_CalcBuff(0xffff, &callDes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRDES_EncDec(&callDes);
    r_printf((0 == memcmp(au8Plain, au8Result, DES_TEST_LEN)), "air_des_cbc test\n");  
}

void DES_Test()
{
    DES_Func_Test();
    DES_Rand_Test();
}


typedef struct
{
    uint8_t     u8KLen;
    uint8_t *   pu8Key;
    uint32_t    u32Size;
    char *      pu8Name;
}TDES_KEY_TYP;

typedef struct
{
    uint32_t    u32Val;
    char *      pu8Name;
}TDES_MODE_TYP;

void TDES_Rand_Test()
{
    uint32_t i, j, t;
    uint32_t u32MsgLen = 0;
    AIR_SYM_CRYPT_CALL callTdes = {0};  
    TDES_MODE_TYP ModeType[2] = {{AIR_DES_OPT_BLK_ECB, "ECB"},
                             {AIR_DES_OPT_BLK_CBC, "CBC"}};
    TDES_KEY_TYP KeyType[3] = {{16, NULL, AIR_TDES_OPT_KEY_2, "KEY_1_2"},
                           {24, NULL, AIR_TDES_OPT_KEY_3, "KEY_1_2_3"}};
    uint8_t au8Plain[2][DES_TIME_TEST_DATA];
    uint8_t au8Cipher[DES_TIME_TEST_DATA];
    uint8_t au8Iv[8] = {0};
    uint8_t u8Key2[16] = {0}; 
    uint8_t u8Key3[24] = {0};
    KeyType[0].pu8Key = u8Key2;
    KeyType[1].pu8Key = u8Key3;
    DBG_PRINT("\nTDES Test Start\n");

    for (i = 0; i < 2; i++)
    {
        //pack mode ECB CBC
        for (j = 0; j < 2; j++)
        {
                memset(au8Plain, 0, sizeof(au8Plain));
                memset(au8Cipher, 0, sizeof(au8Cipher));
                memset(au8Iv,0,sizeof(au8Iv));
                memset(KeyType[i].pu8Key,0,KeyType[i].u8KLen);
                AIRRAND_Prand(&u32MsgLen,sizeof(u32MsgLen));
                u32MsgLen = (u32MsgLen & 0x03ff) + 1;
                AIRRAND_Prand(au8Plain,u32MsgLen);
                AIRRAND_Prand(au8Iv,sizeof(au8Iv));
                AIRRAND_Prand(KeyType[i].pu8Key,KeyType[i].u8KLen);
            
                memset(&callTdes, 0, sizeof(callTdes));
                //TDES enc
                callTdes.pu8In = au8Plain[0];
                callTdes.u32InLen = u32MsgLen;
                callTdes.pu8Out = au8Cipher;
                callTdes.u32OutLen = ((u32MsgLen + 15)>>4)<<4;                
                callTdes.pu8IV = au8Iv;
                callTdes.pu8Key = KeyType[i].pu8Key;
                callTdes.u16Opt = ModeType[j].u32Val | AIR_DES_OPT_MODE_ENCRYPT | KeyType[i].u32Size;
                callTdes.u32Crc = AIRCRC_CalcBuff(0xffff, &callTdes, sizeof(AIR_SYM_CRYPT_CALL)-4);
                if(AIR_RET_DES_SUCCESS != AIRTDES_EncDec(&callTdes))
                {
                    r_printf(0, "Tdes enc test\n");
                }                    
                
                //TDES dec           
                callTdes.pu8In = au8Cipher;
                callTdes.u32InLen = ((u32MsgLen + 15)>>4)<<4;
                callTdes.pu8Out = au8Plain[1];
                callTdes.u32OutLen = ((u32MsgLen + 15)>>4)<<4;
                callTdes.pu8IV = au8Iv;
                callTdes.pu8Key = KeyType[i].pu8Key;
                callTdes.u16Opt = ModeType[j].u32Val | AIR_DES_OPT_MODE_DECRYPT | KeyType[i].u32Size;
                callTdes.u32Crc = AIRCRC_CalcBuff(0xffff, &callTdes, sizeof(AIR_SYM_CRYPT_CALL)-4);
                if(AIR_RET_DES_SUCCESS != AIRTDES_EncDec(&callTdes))
                {
                    r_printf(0, "Tdes dec test\n");
                }                    
                t = (!memcmp(au8Plain[0], au8Plain[1], u32MsgLen));
                DBG_PRINT(" %s %s TDES Test ", ModeType[j].pu8Name, KeyType[i].pu8Name);
                r_printf(t, "\n");
        }
    }
}

void TDES_Func_Test()
{
    uint8_t au8Plain[DES_TEST_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                            0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                            0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                            0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40};
    uint8_t au8TDESCipher_ECB[DES_TEST_LEN] = {0x7D, 0x78, 0xD9, 0xC8, 0xFA, 0x35, 0xB9, 0x43, 0x6A, 0xF8, 0xCA, 0x58, 0xAB, 0x3A, 0x35, 0x00,
                                    0xAC, 0xBD, 0x15, 0x53, 0xE0, 0xC4, 0x3C, 0x90, 0xF9, 0x5C, 0xE5, 0x97, 0xDE, 0xC4, 0xBF,0x58, 
                                    0x96, 0x9A, 0x18, 0x6D, 0xE8, 0x05, 0x92, 0x80, 0x16, 0x3A, 0xEC, 0x2B, 0x30, 0x24, 0x37,0x4E, 
                                    0x09, 0x2D, 0x46, 0x06, 0x3C, 0x61, 0xE9, 0xAA, 0x21, 0x9C, 0xA8, 0xF6, 0x3F, 0x42, 0xE4, 0xD2};
    uint8_t au8TDESCipher_CBC[DES_TEST_LEN] = {0xAD, 0x17, 0xA7, 0x56, 0x3A, 0x0F, 0xF8, 0xF3, 0x47, 0xB6, 0x3E, 0x78, 0x16, 0x72, 0xCD, 0xD7,
                                    0x6F, 0x52, 0x02, 0x8E, 0xDB, 0x00, 0x49, 0x39, 0x12, 0xD5, 0xB6, 0x8A, 0xD0, 0xCB, 0xDC, 0xC6,
                                    0xFD, 0x98, 0x82, 0x73, 0x18, 0x37, 0xCF, 0x9C, 0x68, 0x10, 0x2E, 0x94, 0x8D, 0x85, 0x2E, 0x09,
                                    0x81, 0x7F, 0x56, 0x70, 0x7E, 0x21, 0xCF, 0x1B, 0x2D, 0x66, 0xD7, 0xE8, 0x96, 0xCC, 0xAC, 0xD4};
    uint8_t au8Result[DES_TEST_LEN] = {0};
    uint8_t au8Key[24] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};    
    uint8_t au8Iv[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    AIR_SYM_CRYPT_CALL callTdes; 
    
    memset(&callTdes, 0, sizeof(callTdes));
    //ECB ENC
    callTdes.pu8In = au8Plain;
    callTdes.u32InLen = DES_TEST_LEN;
    callTdes.pu8Out = au8Result;
    callTdes.u32OutLen = DES_TEST_LEN;                
    callTdes.u16Opt = AIR_DES_OPT_BLK_ECB | AIR_DES_OPT_MODE_ENCRYPT| AIR_TDES_OPT_KEY_3;
    callTdes.pu8Key = au8Key;
    callTdes.u32Crc = AIRCRC_CalcBuff(0xffff, &callTdes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRTDES_EncDec(&callTdes);
    r_printf((0 == memcmp(au8TDESCipher_ECB, au8Result, DES_TEST_LEN)), "air_tdes_ecb test\n");
    
    //CBC DEC
    callTdes.pu8In = au8TDESCipher_CBC;
    callTdes.u32InLen = DES_TEST_LEN;
    callTdes.pu8Out = au8Result;
    callTdes.u32OutLen = DES_TEST_LEN;                
    callTdes.pu8IV = au8Iv;
    callTdes.u16Opt = AIR_DES_OPT_BLK_CBC | AIR_DES_OPT_MODE_DECRYPT| AIR_TDES_OPT_KEY_3;
    callTdes.pu8Key = au8Key;
    callTdes.u32Crc = AIRCRC_CalcBuff(0xffff, &callTdes, sizeof(AIR_SYM_CRYPT_CALL)-4);  
    AIRTDES_EncDec(&callTdes);
    r_printf((0 == memcmp(au8Plain, au8Result, DES_TEST_LEN)), "air_tdes_cbc test\n");
}


void TDES_Test()
{
    TDES_Func_Test();
    TDES_Rand_Test();
}


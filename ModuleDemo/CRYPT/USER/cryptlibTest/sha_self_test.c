#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_sha.h"

void SHA_Update_Test()
{
    uint16_t  i;
    AIRSHA1_Context ctx;
    AIRSHA2_Context ctx2;
    uint8_t buf0[0x40];
    uint8_t buf1[0x40];
    uint8_t t0[2] = "12";
//    uint8_t t1[2] = "12";
//    uint8_t t2[2] = "12";
//    uint8_t t3[2] = "12";
//    uint8_t t4[2] = "12";
//    
    unsigned char TestSHA[100] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
//    uint8_t input[20] = "wangzhanbei";
    uint8_t digest[32] = {0};
    printf("Test the sha function...\r\n");
    AIRSHA1_Starts(&ctx);
//    AIRSHA1_Update(&ctx,t0, 0);
//    AIRSHA1_Update(&ctx,t0, 2);
//    AIRSHA1_Update(&ctx,t0, 2);
//    AIRSHA1_Update(&ctx,t0, 2);
//    AIRSHA1_Update(&ctx,t0, 2);
    AIRSHA1_Update(&ctx,TestSHA, 100);
//    AIRSHA1_Update(&ctx,t0, 2);

//    AIRSHA1_Update(&ctx,input, 11);
//    AIRSHA1_Update(&ctx,TestSHA, 100);
    AIRSHA1_Finish(&ctx,digest);
    printf("The sha_160 result is \n");
    for(i=0;i<20;i++)
    {
        printf("0x%2x\t",digest[i]);
    }
    
    memset(digest,0,32);
    memset(buf0,'a',0x40);
    memset(buf1,'b',0x40);
    memset(digest,0,32);
    AIRSHA256_Starts(&ctx2);
    AIRSHA256_Update(&ctx2,buf0, 0x40);
    AIRSHA256_Update(&ctx2,buf1, 0x40);
    AIRSHA256_Update(&ctx2,buf1, 0);
    AIRSHA256_Finish(&ctx2,digest);
    printf("The sha_256 result is \n");
    for(i=0;i<32;i++)
    {
        printf("0x%2x\t",digest[i]);
    }
}



void SHA_Func_Test()
{
    uint8_t TestSHA[100] = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    uint8_t Cipher_SHA256[] = { 0x9c, 0xfe, 0x7f, 0xaf, 0xf7, 0x05, 0x42, 0x98, 0xca, 0x87, 0x55, 0x7e, 0x15, 0xa1, 0x02, 0x62, 
                                0xde, 0x8d, 0x3e, 0xee, 0x77, 0x82, 0x74, 0x17, 0xfb, 0xdf, 0xea, 0x1c, 0x41, 0xb9, 0xec, 0x23};
    uint8_t Cipher_SHA1[] = {0x29, 0xb0, 0xe7, 0x87, 0x82, 0x71, 0x64, 0x5f, 0xff, 0xb7, 0xee, 0xc7, 0xdb, 0x4a, 0x74, 0x73, 0xa1, 0xc0, 0x0b, 0xc1};
    uint8_t Cipher_SHA224[] = { 0x2C, 0x09, 0xD5, 0x93, 0x30, 0x73, 0x6C, 0x5D, 0x53, 0x19, 0x0B, 0x36, 0x7D, 0x2D, 0x91, 0xCE,
                                0x54, 0xA5, 0x4A, 0xD9, 0x59, 0xA1, 0x6D, 0x40, 0x01, 0x83, 0xF0, 0x7F};
    uint8_t au8Cipher[32] = {0};
    //Sha_256 test
    memset(au8Cipher, 0, 32);
    AIRSHA_Sha(AIRSHA256,TestSHA,sizeof(TestSHA),au8Cipher,32);
    r_printf((0 == memcmp(Cipher_SHA256, au8Cipher, 32)), "air_sha_256 test\n");
    //Sha_160 test
    memset(au8Cipher, 0, 32);
    AIRSHA_Sha(AIRSHA1,TestSHA,sizeof(TestSHA),au8Cipher,20);
    r_printf((0 == memcmp(Cipher_SHA1, au8Cipher, 20)), "air_sha_160 test\n");
    //Sha_160 test
    memset(au8Cipher, 0, 32);
    AIRSHA_Sha(AIRSHA224,TestSHA,sizeof(TestSHA),au8Cipher,28);
    r_printf((0 == memcmp(Cipher_SHA224, au8Cipher, 28)), "air_sha_224 test\n");
}

void SHA_Test()
{
    SHA_Update_Test();
    SHA_Func_Test();
}

void test_Sha2_update(uint8_t *pu8Buff, uint32_t u32Size, uint8_t pu8HashRes[32])
{
    uint32_t t, len, i, j, u32P, u32R;
    uint8_t *pu8In, au8Hash[32];
    char s[40] = {0};
    AIRSHA2_Context ctx;
    uint8_t au8M[] = {1, 2, 3, 5}; //Test update step table.

    for(j = 0; j < sizeof(au8M); j++)
    {
        pu8In = pu8Buff;
        memset(au8Hash, 0, sizeof(au8Hash));

        //Round
        u32P = u32Size / au8M[j];
        //Remaind data, left in tail.
        u32R = u32Size % au8M[j];
        AIRSHA256_Starts(&ctx);
        for(i = 0; i < u32P; i++)
        {
            AIRSHA256_Update(&ctx, pu8In + i * au8M[j], au8M[j]);
        }
        AIRSHA256_Update(&ctx, pu8In + i * au8M[j], u32R);
        
        AIRSHA256_Finish(&ctx, au8Hash);
        sprintf(s, "sha256 update Test len %d step %d\n", u32Size, au8M[j]);
        t = (!memcmp(au8Hash, pu8HashRes, sizeof(au8Hash)));
        r_printf(t, s);
    }
}

void test_Sha1_update(uint8_t *pu8Buff, uint32_t u32Size, uint8_t pu8HashRes[20])
{
    uint32_t t, len, i, j, u32P, u32R;
    uint8_t *pu8In, au8Hash[20];
    char s[40] = {0};
    AIRSHA1_Context ctx;
    uint8_t au8M[] = {1, 2, 3, 5}; //Test update step table.

    for(j = 0; j < sizeof(au8M); j++)
    {
        pu8In = pu8Buff;
        memset(au8Hash, 0, sizeof(au8Hash));

        //Round
        u32P = u32Size / au8M[j];
        //Remaind data, left in tail.
        u32R = u32Size % au8M[j];
        AIRSHA1_Starts(&ctx);
        for(i = 0; i < u32P; i++)
        {
            AIRSHA1_Update(&ctx, pu8In + i * au8M[j], au8M[j]);
        }
        AIRSHA1_Update(&ctx, pu8In + i * au8M[j], u32R);
        
        AIRSHA1_Finish(&ctx, au8Hash);
        sprintf(s, "sha160 update Test len %d step %d\n", u32Size, au8M[j]);
        t = (!memcmp(au8Hash, pu8HashRes, sizeof(au8Hash)));
        r_printf(t, s);
    }
}

void test_SHA_update(void)
{
    uint32_t i;
	uint32_t outLen;
    uint8_t c_512[512] = {0};
    
    uint8_t c1_160[160 /8] = {0};
    uint8_t c1_256[256 /8] = {0};
    
    
    uint8_t const cau8Sha160_55[20] = {0x8A, 0xE2, 0xD4, 0x67, 0x29, 0xCF, 0xE6, 0x8F, 0xF9, 0x27, 0xAF, 0x5E, 0xEC, 0x9C, 0x7D, 0x1B, 0x66, 0xD6, 0x5A, 0xC2};
    uint8_t const cau8Sha160_56[20] = {0x63, 0x6E, 0x2E, 0xC6, 0x98, 0xDA, 0xC9, 0x03, 0x49, 0x8E, 0x64, 0x8B, 0xD2, 0xF3, 0xAF, 0x64, 0x1D, 0x3C, 0x88, 0xCB};
    uint8_t const cau8Sha160_57[20] = {0x7C, 0xB1, 0x33, 0x0F, 0x35, 0x24, 0x4B, 0x57, 0x43, 0x75, 0x39, 0x25, 0x33, 0x04, 0xEA, 0x78, 0xA6, 0xB7, 0xC4, 0x43};
    uint8_t const cau8Sha160_64[20] = {0xC6, 0x13, 0x8D, 0x51, 0x4F, 0xFA, 0x21, 0x35, 0xBF, 0xCE, 0x0E, 0xD0, 0xB8, 0xFA, 0xC6, 0x56, 0x69, 0x91, 0x7E, 0xC7};
    
    uint8_t const cau8Sha160_4B[20] = {0x49, 0x16, 0xD6, 0xBD, 0xB7, 0xF7, 0x8E, 0x68, 0x03, 0x69, 0x8C, 0xAB, 0x32, 0xD1, 0x58, 0x6E, 0xA4, 0x57, 0xDF, 0xC8};
    uint8_t const cau8Sha160_4B_55[20] = {0x6B, 0xF2, 0x34, 0xD7, 0x18, 0xFD, 0x1F, 0x27, 0xBB, 0xFB, 0xFF, 0x81, 0xFB, 0x7F, 0xD6, 0x4A, 0x22, 0xAE, 0x67, 0x45};
    uint8_t const cau8Sha160_4B_56[20] = {0x93, 0x5C, 0xA3, 0xDF, 0xC9, 0xBF, 0xB1, 0xA3, 0xDC, 0xCD, 0x5F, 0x79, 0x4A, 0xB1, 0x7E, 0x38, 0x74, 0x41, 0xA4, 0xD2};
    uint8_t const cau8Sha160_4B_57[20] = {0xBB, 0x6A, 0xC0, 0x03, 0x6E, 0xE0, 0xA5, 0x68, 0x03, 0x04, 0x53, 0x17, 0x74, 0x4D, 0x42, 0x69, 0xEC, 0xFD, 0x0E, 0x62};
    
    uint8_t const cau8Sha256_55[32] = {0x46, 0x3E, 0xB2, 0x8E, 0x72, 0xF8, 0x2E, 0x0A, 0x96, 0xC0, 0xA4, 0xCC, 0x53, 0x69, 0x0C, 0x57, 0x12, 0x81, 0x13, 0x1F, 0x67, 0x2A, 0xA2, 0x29, 0xE0, 0xD4, 0x5A, 0xE5, 0x9B, 0x59, 0x8B, 0x59};
    uint8_t const cau8Sha256_56[32] = {0xDA, 0x2A, 0xE4, 0xD6, 0xB3, 0x67, 0x48, 0xF2, 0xA3, 0x18, 0xF2, 0x3E, 0x7A, 0xB1, 0xDF, 0xDF, 0x45, 0xAC, 0xDC, 0x9D, 0x04, 0x9B, 0xD8, 0x0E, 0x59, 0xDE, 0x82, 0xA6, 0x08, 0x95, 0xF5, 0x62};
    uint8_t const cau8Sha256_57[32] = {0x2F, 0xE7, 0x41, 0xAF, 0x80, 0x1C, 0xC2, 0x38, 0x60, 0x2A, 0xC0, 0xEC, 0x6A, 0x7B, 0x0C, 0x3A, 0x8A, 0x87, 0xC7, 0xFC, 0x7D, 0x7F, 0x02, 0xA3, 0xFE, 0x03, 0xD1, 0xC1, 0x2E, 0xAC, 0x4D, 0x8F};
    uint8_t const cau8Sha256_64[32] = {0xFD, 0xEA, 0xB9, 0xAC, 0xF3, 0x71, 0x03, 0x62, 0xBD, 0x26, 0x58, 0xCD, 0xC9, 0xA2, 0x9E, 0x8F, 0x9C, 0x75, 0x7F, 0xCF, 0x98, 0x11, 0x60, 0x3A, 0x8C, 0x44, 0x7C, 0xD1, 0xD9, 0x15, 0x11, 0x08};
    
    uint8_t const cau8Sha256_4B[32] = {0x40, 0xAF, 0xF2, 0xE9, 0xD2, 0xD8, 0x92, 0x2E, 0x47, 0xAF, 0xD4, 0x64, 0x8E, 0x69, 0x67, 0x49, 0x71, 0x58, 0x78, 0x5F, 0xBD, 0x1D, 0xA8, 0x70, 0xE7, 0x11, 0x02, 0x66, 0xBF, 0x94, 0x48, 0x80};
    uint8_t const cau8Sha256_4B_55[32] = {0x72, 0x72, 0x37, 0x92, 0x75, 0xEF, 0xC5, 0x5B, 0x56, 0x6B, 0x4F, 0x45, 0x4A, 0xF2, 0x9C, 0x6A, 0xEE, 0x1F, 0xE9, 0x94, 0x4C, 0xC4, 0xB1, 0xC3, 0x2C, 0x03, 0x8A, 0x33, 0x6E, 0x1E, 0xEC, 0xB3};
    uint8_t const cau8Sha256_4B_56[32] = {0x4C, 0x43, 0xAC, 0xB4, 0xB8, 0x60, 0x54, 0xA3, 0xFA, 0x2D, 0x0C, 0x52, 0x4E, 0x80, 0xA7, 0x4E, 0x7E, 0x93, 0xEC, 0x54, 0x6B, 0xBF, 0x78, 0x4A, 0x72, 0xCA, 0xED, 0xF9, 0x98, 0x0B, 0xD7, 0x85};
    uint8_t const cau8Sha256_4B_57[32] = {0xEC, 0x20, 0x70, 0x6F, 0xBA, 0xCA, 0x16, 0xCF, 0x3A, 0x84, 0x2F, 0x35, 0xE7, 0xDF, 0x69, 0xE6, 0xA8, 0x0A, 0x93, 0xC1, 0xCD, 0xD3, 0x50, 0x82, 0xF9, 0x49, 0xD7, 0x09, 0x80, 0xBC, 0x43, 0xF2};
    
        
    for(i = 0; i < sizeof(c_512); i++)
    {
        c_512[i] = i;
    }
    
    AIRSHA_Sha(AIRSHA1, c_512, 55, c1_160, outLen);
    ouputArrayHex("sha160_55\n", c1_160, 20);
    AIRSHA_Sha(AIRSHA1, c_512, 56, c1_160, outLen);
    ouputArrayHex("sha160_56\n", c1_160, 20);
    AIRSHA_Sha(AIRSHA1, c_512, 57, c1_160, outLen);
    ouputArrayHex("sha160_57\n", c1_160, 20);
    AIRSHA_Sha(AIRSHA1, c_512, 64, c1_160, outLen);
    ouputArrayHex("sha160_64\n", c1_160, 20);
    
    AIRSHA_Sha(AIRSHA1, c_512, 64 * 4, c1_160, outLen);
    ouputArrayHex("sha160_4B\n", c1_160, 20);
    AIRSHA_Sha(AIRSHA1, c_512, 64 * 4 + 55, c1_160, outLen);
    ouputArrayHex("sha160_4B_55\n", c1_160, 20);
    AIRSHA_Sha(AIRSHA1, c_512, 64 * 4 + 56, c1_160, outLen);
    ouputArrayHex("sha160_4B_56\n", c1_160, 20);
    AIRSHA_Sha(AIRSHA1, c_512, 64 * 4 + 57, c1_160, outLen);
    ouputArrayHex("sha160_4B_57\n", c1_160, 20);
    
    AIRSHA_Sha(AIRSHA256, c_512, 55, c1_256, outLen);
    ouputArrayHex("sha256_55\n", c1_256, 32);
    AIRSHA_Sha(AIRSHA256, c_512, 56, c1_256, outLen);
    ouputArrayHex("sha256_56\n", c1_256, 32);
    AIRSHA_Sha(AIRSHA256,  c_512, 57, c1_256, outLen);
    ouputArrayHex("sha256_57\n", c1_256, 32);
    AIRSHA_Sha(AIRSHA256,  c_512, 64, c1_256, outLen);
    ouputArrayHex("sha256_64\n", c1_256, 32);
    
    AIRSHA_Sha(AIRSHA256, c_512, 64 * 4, c1_256, outLen);
    ouputArrayHex("sha256_4B\n", c1_256, 32);
    AIRSHA_Sha(AIRSHA256, c_512, 64 * 4 + 55, c1_256, outLen);
    ouputArrayHex("sha256_4B_55\n", c1_256, 32);
    AIRSHA_Sha(AIRSHA256, c_512, 64 * 4 + 56, c1_256, outLen);
    ouputArrayHex("sha256_4B_56\n", c1_256, 32);
    AIRSHA_Sha(AIRSHA256, c_512, 64 * 4 + 57, c1_256, outLen);
    ouputArrayHex("sha256_4B_57\n", c1_256, 32);
    
    
    
    test_Sha1_update(c_512, 55, (uint8_t *)cau8Sha160_55);
    test_Sha1_update(c_512, 56, (uint8_t *)cau8Sha160_56);
    test_Sha1_update(c_512, 57, (uint8_t *)cau8Sha160_57);
    test_Sha1_update(c_512, 64, (uint8_t *)cau8Sha160_64);
    
    test_Sha1_update(c_512, 64 *4, (uint8_t *)cau8Sha160_4B);
    test_Sha1_update(c_512, 64 *4 + 55, (uint8_t *)cau8Sha160_4B_55);
    test_Sha1_update(c_512, 64 *4 + 56, (uint8_t *)cau8Sha160_4B_56);
    test_Sha1_update(c_512, 64 *4 + 57, (uint8_t *)cau8Sha160_4B_57);
    
    test_Sha2_update(c_512, 55, (uint8_t *)cau8Sha256_55);
    test_Sha2_update(c_512, 56, (uint8_t *)cau8Sha256_56);
    test_Sha2_update(c_512, 57, (uint8_t *)cau8Sha256_57);
    test_Sha2_update(c_512, 64, (uint8_t *)cau8Sha256_64);
    
    test_Sha2_update(c_512, 64 *4, (uint8_t *)cau8Sha256_4B);
    test_Sha2_update(c_512, 64 *4 + 55, (uint8_t *)cau8Sha256_4B_55);
    test_Sha2_update(c_512, 64 *4 + 56, (uint8_t *)cau8Sha256_4B_56);
    test_Sha2_update(c_512, 64 *4 + 57, (uint8_t *)cau8Sha256_4B_57);
    
}


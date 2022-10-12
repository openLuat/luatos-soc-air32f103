#include <string.h>
#include <stdio.h>
#include "air32f10x.h"
#include "air_sm3.h"
#define c_sm3_len       16
#define c_sm3_msg       "0102030405060708090a0b0c0d0e0f10"
#define c_sm3_hash      "25C9D66ED547714E981BC825B969CC71ACD338F90A4E0F75E789EF71C0921666"



void SM3_Up_Test()
{
    uint16_t  i;
    AIRSM3_Context Ctx;
   
    uint8_t buf0[64];
    uint8_t digest[32] = {0};
    printf("Test the sm3 function...\r\n");
    memset(buf0,'a',sizeof(buf0));
    memset(digest,0,32);
    AIRSM3_Starts(&Ctx);
    AIRSM3_Update(&Ctx,buf0, sizeof(buf0));
    AIRSM3_Update(&Ctx,buf0, sizeof(buf0));
    AIRSM3_Finish(&Ctx,digest);
//    while(1);
}

uint32_t SM3_local_check(void)
{   
    uint32_t ret = 0;
    
    uint32_t t,i;
    uint8_t Message[16] = {0};
    uint8_t Hash[32] = {0};
    uint8_t _Hash[32] = {0}; 

    bn_read_string_to_bytes(Message, sizeof(Message), c_sm3_msg);
    bn_read_string_to_bytes(_Hash, sizeof(_Hash), c_sm3_hash);
                        
    AIRSM3_Cal(Hash, Message, c_sm3_len);
                        
    t = memcmp(Hash, _Hash, sizeof(_Hash));
    
    if(0 != t)
    {
        ret = 1;
        //printf("\nSM3_check check fault!\n");
    }
    
    return ret;
}


void SM3_Test(void)
{
	SM3_Up_Test();
	SM3_local_check();
}


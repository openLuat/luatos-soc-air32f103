#include "air_bignum_tool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Convert an ASCII character to digit value
 */
static int bn_get_digit( uint32_t *d, char c )
{
    *d = 255;

    if( c >= 0x30 && c <= 0x39 ) *d = c - 0x30;
    if( c >= 0x41 && c <= 0x46 ) *d = c - 0x37;
    if( c >= 0x61 && c <= 0x66 ) *d = c - 0x57;

    if( *d >= (uint32_t) 16 )
        return( -1 );

    return( 0 );
}

/*
 * Import from an ASCII string
 */
int bn_read_string( uint32_t *r, uint32_t len, const char *s )
{
    uint32_t i, j, slen;
    uint32_t d;

    slen = strlen( s );

	if ((len * 8) < slen)
		return(-1);

    memset(r, 0, (len * sizeof(uint32_t)));

	for( i = slen, j = 0; i > 0; i--, j++ )
	{
		if( -1 == bn_get_digit( &d, s[i - 1] ) )
			return (-1);
		r[j / 8] |= d << ( (j % 8) << 2 );
	}

    return( 0 );
}


/*
 * Export into an HEX ASCII string
 */
int bn_write_string(char *s, uint32_t *slen, const uint32_t *content, uint32_t len)
{
    int32_t ret = 0;
    uint32_t n;
    char *p;

	int32_t c;
	uint32_t i, j, k;

    n = len	* 8;
    n += 2;		//"\0"

    if( *slen < n )
    {
        *slen = n;
        return( -1 );
    }

    p = s;
 
	for( i = len, k = 0; i > 0; i-- )
	{
		for( j = 4; j > 0; j-- )
		{
			c = ( content[i - 1] >> ( ( j - 1 ) << 3) ) & 0xFF;

			if( c == 0 && k == 0 && ( i + j + 3 ) != 0 )
				continue;

			*(p++) = "0123456789ABCDEF" [c / 16];
			*(p++) = "0123456789ABCDEF" [c % 16];
			k = 1;
		}
	}
   
    *p++ = '\0';
    *slen = p - s;
	
    return( ret );
}

/*
 * Import from an ASCII string to an big endian data array
 */
int bn_read_string_to_bytes( uint8_t *r, uint32_t len, const char *s )
{
    uint32_t i, j, slen;
    uint32_t d;
	
    slen = strlen( s );

	if ((len * 2) < slen)
		return(-1);

	memset(r, 0, len);
	
	len = len - 1;
	
	for( i = slen, j = 0; i > 0; i--, j++)
	{
		if( -1 == bn_get_digit( &d, s[i - 1] ) )
			return (-1);
		r[len - j / 2] |= d << ( (j % 2) << 2 );
	}

    return( 0 );
}

int bn_read_string_from_head( uint8_t *r, uint32_t len, const char *s )
{
    int32_t i, j, slen;
    uint32_t d;
	
    if((slen = strlen( s )) < 0)
    {
        return -1;
    }

	if ((len * 2) < slen)
		return(-1);

    if (slen & 1)
    {
        slen += 1;
    }
    
	memset(r, 0, len);
	
	len = slen >> 1;
	
	for( i = slen, j = 0; i > 0; i--, j++)
	{
		if( -1 == bn_get_digit( &d, s[i - 1] ) )
			return (-1);
		r[len - 1 - j / 2] |= d << ( (j % 2) << 2 );
	}

    return( len );
}



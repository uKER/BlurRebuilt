//-----------------------------------------------------------------------------
// File: EndianSwitch.cpp
//
// Desc: Functions for changing the endianness (byte ordering) of data. This
//       code should generally be run on the development PC at authoring time,
//       but the same code can also run on the console.
//
// Hist: 04.10.04 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <assert.h>
#include <algorithm>
#include "EndianSwitch.h"


//-----------------------------------------------------------------------------
// Name: EndianSwitchWords
// Desc: Byte swap the specified number of two-byte items.
//-----------------------------------------------------------------------------
void* EndianSwitchWords( WORD* pData, int count )
{
    // This could be optimized, but the main optimization is to not do this
    // at load time - it is better to use this function when creating data
    // on your development PC.
    for( int i = 0; i < count; ++i )
    {
        BYTE* pElement = (BYTE*)pData;
        std::swap( pElement[ 0 ], pElement[ 1 ] );
        ++pData;
    }

    return pData;
}


//-----------------------------------------------------------------------------
// Name: EndianSwitchDWords
// Desc: Byte swap the specified number of four-byte items.
//-----------------------------------------------------------------------------
void* EndianSwitchDWords( DWORD* pData, int count )
{
	//return 0;

    // This could be optimized, but the main optimization is to not do this
    // at load time - it is better to use this function when creating data
    // on your development PC.
    for( int i = 0; i < count; ++i )
    {
        BYTE* pElement = (BYTE*)pData;
        std::swap( pElement[ 0 ], pElement[ 3 ] );
        std::swap( pElement[ 1 ], pElement[ 2 ] );
        ++pData;
    }

    return pData;
}


//-----------------------------------------------------------------------------
// Name: EndianSwitchQWords
// Desc: Byte swap the specified number of eight-byte items.
//-----------------------------------------------------------------------------
void* EndianSwitchQWords( unsigned __int64* pData, int count )
{
    // This could be optimized, but the main optimization is to not do this
    // at load time - it is better to use this function when creating data
    // on your development PC.
    for( int i = 0; i < count; ++i )
    {
        BYTE* pElement = (BYTE*)pData;
        std::swap( pElement[ 0 ], pElement[ 7 ] );
        std::swap( pElement[ 1 ], pElement[ 6 ] );
        std::swap( pElement[ 2 ], pElement[ 5 ] );
        std::swap( pElement[ 3 ], pElement[ 4 ] );
        ++pData;
    }

    return pData;
}


//-----------------------------------------------------------------------------
// Name: EndianSwitchWorker
// Desc: This function does byte swapping to change endianness based on a
//       format string. Pass in the beginning of the data and a pointer to
//       right after the data and a format string. You can also pass in
//       repeat count for processing an array of structures.
//       The format string consists of these type identifiers:
//          q, d: quad word or double - eight byte element
//          i, f: int or float - four byte element
//          s: short - two byte element
//          c: char - one byte element (no swapping needed)
//
//       Additionally any type specifier can be preceded by a count, which
//       says there are several of that type. Finally, parentheses can be
//       use for grouping, and a parenthetic group can be repeated.
//       Parentheses can be nested.
//
//       Sample usage would be:
//          struct Data { int i1; short s1, s2, s3; float f1; };
//          Data myData = { 0x12345678, 0x1234, 0x1234, 0x5678, 7.25 };
//          NativeToBigEndian(&myData, "isssf");
//
//       Given an array of four structures, each containing an int and a
//       short, this would swap the endianness of the data:
//
//          EndianSwitchWorker( array, array + 4, "4(is2c)" );
//
//       The "2c" is needed in order to account for padding that the compiler
//       inserts.
//
//       All work is done in-place.
//-----------------------------------------------------------------------------
void* EndianSwitchWorker( void* pData, void* pEnd, const char* format,
                         int blockRepeatCount, const char** updatedFormat )
{
    // Record the position in the format string so we can repeat it
    // multiple times.
    const char* startFormat = format;
    while( blockRepeatCount > 0 )
    {
        // Format strings are terminated by a null character or a closing parenthesis
        while( *format && *format != ')' )
        {
            // First check for a repeat count. Repeat counts apply to the
            // following format specifier or group.
            int count = 1;
            if (isdigit(format[0]))
            {
                count = 0;
                // Convert the number from ascii to an int. atoi or scanf
                // are not used because hand conversion works better for numbers
                // that are not null terminated and have to be skipped over.
                while (isdigit(format[0]))
                {
                    int newCount = count * 10 + format[0] - '0';
                    // Check for overflow - unlikely.
                    assert(newCount >= count);
                    count = newCount;
                    ++format;
                }
            }

            switch( *format )
            {
            // Parentheses are used for grouping blocks. The block can then be repeated a
            // specified number of times. This is useful if you have an array of structs
            // within a structure.
            case '(':
                ++format;
                // Call this function to process the block. Note that it will update the
                // format variable.
                pData = EndianSwitchWorker( pData, pEnd, format, count, &format);
                continue;

            case 'q':   // Quad word - eight bytes
            case 'd':   // Double - eight bytes, swapped the same as 'q'
            {
                unsigned __int64* pQuads = (unsigned __int64*)pData;
                pData = EndianSwitchQWords( pQuads, count );
                break;
            }

            case 'i':   // Integer - four bytes
            case 'f':   // Float - four bytes, swapped the same as 'i'
            {
                DWORD* pDWords = (DWORD*)pData;
                pData = EndianSwitchDWords( pDWords, count );
                break;
            }

            case 's':   // Short - two bytes
            {
                WORD* pWords = (WORD*)pData;
                pData = EndianSwitchWords( pWords, count );
                break;
            }

            case 'c':   // Char - one byte - do nothing
                pData = (CHAR*)pData + count;
                break;

            default:
                assert( 0 );
                break;
            }
            ++format;

            // Make sure there was enough space available. It's too late at this point,
            // but this is still worthwhile error checking.
            if (pData > pEnd)
			{
                //printf("Byte swapping error - format string doesn't match data length.\n");
				//exit(0);
			}
        }

        --blockRepeatCount;
        if (*format)
        {
            assert( *format == ')' );
            // Skip over the parenthesis
            ++format;
            if( blockRepeatCount == 0 )
            {
                // Return without checking the size - only the terminating null
                // character triggers a size check. This means that extra closing
                // parenthesis characters will cause errors that aren't detected.
                // Update the format string in the callee so it resumes processing
                // at the correct place.
                if( updatedFormat )
                    *updatedFormat = format;
                return pData;
            }
        }

        // Repeat the format string again.
        format = startFormat;
    }

    // Make sure the format specifier we were given matches the size of
    // the structure. If this assert triggers it means that the format string is
    // mismatched with the structure, perhaps due to padding.
    // This assert can also trigger if closing parentheses for a grouping are
    // omitted.
    //assert(pData == pEnd);	
    if( updatedFormat )
        *updatedFormat = format;
    return pData;
}

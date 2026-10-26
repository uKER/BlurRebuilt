//-----------------------------------------------------------------------------
// File: EndianSwitch.h
//
// Desc: Functions for changing the endianness (byte ordering) of data. This
//       code should generally be run on the development PC at authoring time,
//       but the same code can also run on the console.
//
// Hist: 04.10.04 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#pragma once
#ifndef ENDIANSWITCH_H
#define ENDIANSWITCH_H

// Disable warnings about using constant conditionals - they are used
// intentionally.
#pragma warning(disable : 4127)


// Use a const bool to specify what the native endianness is. This will be
// evaluated at compile time.
#ifdef _M_PPCBE
// PowerPC/Xbox 2 is big-endian
const bool littleEndianNative = false;
#else
// Other platforms are assumed to be little-endian.
const bool littleEndianNative = true;
#endif


//-----------------------------------------------------------------------------
// Name: EndianSwitchWorker
// Desc: Worker function for byte swapping of complex data structures. It is
//       usually better to call the higher level functions such as
//       LittleEndianToNative.
//       For full documentation of the format string see the EndianSwitchWorker
//       implementation.
//-----------------------------------------------------------------------------
void* EndianSwitchWorker( void* pData, void* pEnd, const char* format,
            int blockRepeatCount = 1, const char** updatedFormat = 0 );


//-----------------------------------------------------------------------------
// Name: EndianSwitchWords
// Desc: Byte swap the specified number of two-byte items. The updated
//       address is returned.
//-----------------------------------------------------------------------------
void* EndianSwitchWords( WORD* pData, int count );


//-----------------------------------------------------------------------------
// Name: EndianSwitchDWords
// Desc: Byte swap the specified number of four-byte items. The updated
//       address is returned.
//-----------------------------------------------------------------------------
void* EndianSwitchDWords( DWORD* pData, int count );



//-----------------------------------------------------------------------------
// Name: EndianSwitchQWords
// Desc: Byte swap the specified number of eight-byte items. The updated
//       address is returned.
//-----------------------------------------------------------------------------
void* EndianSwitchQWords( unsigned __int64* pData, int count );


//--------------------------------------------------------------------------------------
// Name: LittleEndianToNative
// Desc: This helper function uses a template to grab the size of the structure being
//       converted, and also checks to see whether conversion is needed. This makes it
//       easy to write endian-safe code that runs on any platform - converting as
//       needed.
//       Use this function when you have little endian data that you want to use,
//       regardless of what endianness your current platform is.
//--------------------------------------------------------------------------------------
template <typename T>
void LittleEndianToNative( T* pData, const char* format )
{
    if( !littleEndianNative )
        EndianSwitchWorker( pData, pData + 1, format );
}


//--------------------------------------------------------------------------------------
// Name: NativeToLittleEndian
// Desc: Use this function when you want to convert native data to little endian format,
//       regardless of what endianness your current platform is.
//       The same code goes from little endian to native format
//       and vice-versa. The different functions exist so that you
//       can easily show your intent.
//--------------------------------------------------------------------------------------
template <typename T>
void NativeToLittleEndian( T* pData, const char* format )
{
    if( !littleEndianNative )
        EndianSwitchWorker( pData, pData + 1, format );
}


//--------------------------------------------------------------------------------------
// Name: BigEndianToNative
// Desc: This helper function uses a template to grab the size of the structure being
//       converted, and also checks to see whether conversion is needed. This makes it
//       easy to write endian-safe code that runs on any platform - converting as
//       needed.
//       Use this function when you have big endian data that you want to use,
//       regardless of what endianness your current platform is.
//--------------------------------------------------------------------------------------
template <typename T>
void BigEndianToNative( T* pData, const char* format )
{
    if( littleEndianNative )
        EndianSwitchWorker( pData, pData + 1, format );
}


//--------------------------------------------------------------------------------------
// Name: NativeToBigEndian
// Desc: Use this function when you want to convert native data to big endian format,
//       regardless of what endianness your current platform is.
//       The same code goes from big endian to native format
//       and vice-versa. The different functions exist so that you
//       can easily show your intent.
//--------------------------------------------------------------------------------------
template <typename T>
void NativeToBigEndian( T* pData, const char* format )
{
//	return;
    if( littleEndianNative )
        EndianSwitchWorker( pData, pData + 1, format );
}

#endif

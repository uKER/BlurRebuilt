#ifndef MISC_SHARED_UID_FOURCC_H
#define MISC_SHARED_UID_FOURCC_H

#ifdef _MSC_VER
#pragma once
#endif


/**
@define FOURCC_U32
@brief Generates a 32-bit FOURCC code.
Packs the four characters into an unsigned 32-bit integer.
@code
const U32 uniqueId = FOURCC_U32('ROCK');
@endcode
@param fourCC a constant string of four characters.
*/
#define FOURCC_U32(fcc) static_cast<U32>(fcc)


/**
@define FOURCC_HI20
@brief Generates the hi 20 bits.
Mashes the four upper-case alphabetic characters into the hi 20 bits of an
unsigned 32-bit integer.
@code
enum E_UniqueID
{
	E_UniqueID_HiBits = FOURCC_HI20('ROCK'), 

	E_UniqueID_Whatever, 
	// etc.

	E_UniqueID_End, 
	E_UniqueID_Begin = E_UniqueID_HiBits + 1, 
	E_UniqueID_Count = E_UniqueID_End - E_UniqueID_Begin
};
@endcode
@param fourCC a constant string of four upper-case alphabetic characters.
*/
#define FOURCC_HI20_HELPER(fcc, ci) ((static_cast<U32>((fcc >> ((ci) * 8)) - 'A') & 0x0000001f) << (((ci) * 5) + 12))
#define FOURCC_HI20(fcc) (FOURCC_HI20_HELPER(fcc, 3) | FOURCC_HI20_HELPER(fcc, 2) | FOURCC_HI20_HELPER(fcc, 1) | FOURCC_HI20_HELPER(fcc, 0))


#endif // MISC_SHARED_UID_FOURCC_H

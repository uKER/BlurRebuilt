#ifndef __ALERTBITS_H__
#define __ALERTBITS_H__

namespace Scheduler
{

inline void AlertBits(U32 _iCPUBits, U64 &a, U64 &b)
{
	U64 iCPUBits = (U64)_iCPUBits;
	a = ((iCPUBits&1)<<48) + ((iCPUBits&2)<<31) + ((iCPUBits&4)<<14) + ((iCPUBits&8)>>3);
	b = ((iCPUBits&16)<<44) + ((iCPUBits&32)<<27) + ((iCPUBits&64)<<10) + ((iCPUBits&128)>>7);
}

}

#endif

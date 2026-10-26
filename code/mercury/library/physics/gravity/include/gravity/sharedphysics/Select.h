#ifndef FAST_SELECT_H
#define FAST_SELECT_H

namespace SharedPhysics
{

template<class T_Source>
NOALIAS inline T_Source* __restrict Fast_Select(const bool b, const T_Source* const __restrict pIfTrue, const T_Source* const __restrict pIfFalse)
{
	const INTPTR selector = ~(((INTPTR)b)-1);	// [Blur2PC][x64] pointer-wide select
	const INTPTR pTrue((INTPTR)pIfTrue);
	const INTPTR pFalse((INTPTR)pIfFalse);
	return (T_Source*)((pTrue & selector) | (pFalse & (~selector)));
}


};

#endif

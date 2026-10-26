// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_SORT_H
#define BD_SORT_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

#include <bdCore/bdUtilities/bdComparisonFunctors.h>

template <typename T, typename COMPARISON_FUNCTOR>
void BD_CALL bdCombSort(T *const data,
						const bdUInt size,
						COMPARISON_FUNCTOR compare);


template <typename T>
void BD_CALL bdCombSort(T *const data,
						const bdUInt size);


inline void BD_CALL bdNewGap(bdUInt& gap);

#include <bdCore/bdUtilities/bdSort.inl>

#endif // BD_SORT_H

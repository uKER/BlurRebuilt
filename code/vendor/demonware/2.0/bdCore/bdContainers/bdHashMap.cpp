// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdContainers/bdHashMap.h>

#ifdef BD_HASHMAP_USE_PRIME_CAPACITY

const bdUInt bdPrimeTable::m_numPrimes = 21;

const bdUInt bdPrimeTable::m_primes[] = {	11, 
											17,
											37,
											67,
											131,
											257,
											521,
											1031,
											2053,
											4099,
											8209,
											16411,
											32771,
											65537,
											131101,
											262147,
											524309,
											1048583,
											2097169,
											4194319,	
											8388617	};

inline
bdPrimeTable::bdPrimeTable()
{
}

#endif // BD_HASHMAP_USE_PRIME_CAPACITY

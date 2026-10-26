// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdUtilities/bdTrulyRandom.h>
#include <bdPlatform/bdPlatformRandom/bdPlatformTrulyRandom.h>

BD_REGISTER_SINGLETON(bdTrulyRandomImpl)

//public:

bdUInt bdTrulyRandomImpl::getRandomUInt() const
{
	bdUByte8 randomBytes[sizeof(bdUInt)];

	// do we need a sizeof function call here ?
	getRandomUByte8(randomBytes, sizeof(bdUInt));

	bdUInt *p1 = reinterpret_cast<bdUInt*>(&randomBytes[0]);
	const bdUInt result = *p1;
	
	return result;
}

void bdTrulyRandomImpl::getRandomUByte8(bdUByte8 in[], const bdUInt length) const
{
	bdGetRandomUChar8(in, length);
}

//protected:

bdTrulyRandomImpl::bdTrulyRandomImpl()
{
}

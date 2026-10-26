// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PSP-ADHOC
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdSocket/bdAddressMap.h>

#define BD_LOG_LEVEL "bdSocket/addrmap"

bdAddressMap::bdAddressMap()
{
}

bdBool bdAddressMap::getAddrHandle(	const bdCommonAddrRef ca,
									const bdSecurityID &id,
									bdAddrHandleRef &addrHandle)
{
	addrHandle = ca->getAddr();
	return true;
}

bdBool BD_CALL bdAddressMap::addrToCommonAddr(	const bdAddrHandleRef &addrHandle,
										bdCommonAddrRef &ca,
										bdSecurityID &id)
{
	ca = new bdCommonAddr();
	ca->setAddr(addrHandle);
	return true;
}

bdUWord BD_CALL bdAddressMap::addrToString(	const bdAddrHandleRef &addrHandle,
											bdNChar8 *const str,
											const bdUWord size)
{
	addrHandle.toString(str,size);
	return size;
}

bdBool bdAddressMap::unregisterAddr(bdAddrHandleRef &addrHandle)
{
	return true;
}

// deprecated
bdBool bdAddressMap::commonAddrToAddr(const bdCommonAddrRef ca, 
									  const bdSecurityID &id, 
									  bdAddrHandleRef &addrHandle)
{
	BD_WARN(BD_LOG_LEVEL , "This function is deprecated, use getAddrHandle instead. ");
	return getAddrHandle(ca, id , addrHandle);

}

























































// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY		: PUBLIC
// VERSION    : 1.74
//
// DWBS ------------------------------------------------------------------ DWBS

#include "CustomMatchMakingInfo.h"

#include <bdCore/bdSocket/bdCommonAddrInfo.h>

CustomMatchMakingInfo::CustomMatchMakingInfo()
{
	bdUByte8 emptyLicence[] = "";
	setLicenseID(emptyLicence,0);
}

CustomMatchMakingInfo::~CustomMatchMakingInfo()
{
}
		
CustomMatchMakingInfo::CustomMatchMakingInfo(const bdUByte8 *const licenseID,
											 const bdInt licenseIDLength)									 

{
	setLicenseID(licenseID,licenseIDLength);

}

bdInt CustomMatchMakingInfo::getLicenseID(bdUByte8 *const dest, const bdUInt size) const
{
	bdUInt len = BD_MIN(size, BD_MAX_LICENSEID_LENGTH+1);
	bdMemcpy(dest, m_licenseID, len);
	return len;
}

bdInt CustomMatchMakingInfo::setLicenseID(const bdUByte8 *const src, const bdUInt size)
{
	bdUInt len = BD_MIN(size, BD_MAX_LICENSEID_LENGTH+1);
	bdMemcpy(m_licenseID, src, len);
	return len;
}
          
void CustomMatchMakingInfo::serialize(bdByteBuffer& buffer)const
{
	// Call up to the base class to serialize the standard matchmaking info.
	bdMatchMakingInfo::serialize(buffer);

	// Serialize the custom attributes
	buffer.writeBlob(m_licenseID, BD_MAX_LICENSEID_LENGTH);

}

bdBool CustomMatchMakingInfo::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdMatchMakingInfo::deserialize(buffer);


	bdUInt32 licenseIDLen = BD_MAX_LICENSEID_LENGTH;
	ok = ok && buffer->readBlob(m_licenseID, licenseIDLen);
 
 
	return ok;
}

bdUInt CustomMatchMakingInfo::sizeOf()
{
	return sizeof(CustomMatchMakingInfo);
}

void BD_CALL printInfo(const CustomMatchMakingInfo& sessionData,
					   bdNChar8 *buf,
					   const bdUInt size)
{
	if(size > 0)
	{
		bdNChar8 addrStr[BD_COMMON_ADDR_INFO_SIZE];
		bdCommonAddrInfo::getBriefInfo(sessionData.getHostAddrAsCommonAddr(), addrStr, sizeof(addrStr));
		
		bdUInt64 licenseID;
		sessionData.getLicenseID(reinterpret_cast<bdUByte8*>(&licenseID), sizeof(licenseID));

		bdSnprintf(buf, size, "Address: %s", addrStr);
	}
}

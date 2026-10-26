// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_QUOTA_USAGE_H
#define BD_QUOTA_USAGE_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdQuotaUsage : public bdTaskResult
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdQuotaUsage();

	~bdQuotaUsage();

	virtual void serialize(bdByteBuffer &buffer)const;

	virtual bdBool deserialize(bdByteBufferRef buffer);

	virtual bdUInt sizeOf();

	bdUInt32 m_usedStorageSpace;
	bdUInt32 m_usedNumFiles;
	bdUInt32 m_usedDailyUploadBW;
	bdUInt32 m_usedDailyDownloadBW;

	bdUInt32 m_maxStorageSpace;
	bdUInt32 m_maxNumFiles;
	bdUInt32 m_maxDailyUploadBW;
	bdUInt32 m_maxDailyDownloadBW;
};


void BD_CALL printInfo(const bdQuotaUsage& fileID,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // BD_QUOTA_USAGE_H

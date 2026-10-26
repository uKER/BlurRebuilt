// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdContentStreaming/bdQuotaUsage.h>

bdQuotaUsage::bdQuotaUsage()
: m_usedStorageSpace(0),
m_usedNumFiles(0),
m_usedDailyUploadBW(0),
m_usedDailyDownloadBW(0),
m_maxStorageSpace(0),
m_maxNumFiles(0),
m_maxDailyUploadBW(0),
m_maxDailyDownloadBW(0)
{
}

bdQuotaUsage::~bdQuotaUsage()
{
}


void bdQuotaUsage::serialize(bdByteBuffer &buffer)const
{
	buffer.writeUInt32(m_usedStorageSpace);
	buffer.writeUInt32(m_usedNumFiles);
	buffer.writeUInt32(m_usedDailyUploadBW);
	buffer.writeUInt32(m_usedDailyDownloadBW);

	buffer.writeUInt32(m_maxStorageSpace);
	buffer.writeUInt32(m_maxNumFiles);
	buffer.writeUInt32(m_maxDailyUploadBW);
	buffer.writeUInt32(m_maxDailyDownloadBW);

}

bdBool bdQuotaUsage::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_usedStorageSpace);
	ok = ok && buffer->readUInt32(m_usedNumFiles);
	ok = ok && buffer->readUInt32(m_usedDailyUploadBW);
	ok = ok && buffer->readUInt32(m_usedDailyDownloadBW);

	ok = ok && buffer->readUInt32(m_maxStorageSpace);
	ok = ok && buffer->readUInt32(m_maxNumFiles);
	ok = ok && buffer->readUInt32(m_maxDailyUploadBW);
	ok = ok && buffer->readUInt32(m_maxDailyDownloadBW);

	return ok;
}

bdUInt bdQuotaUsage::sizeOf()
{
	return sizeof(bdQuotaUsage);
}

void BD_CALL printInfo(const bdQuotaUsage& quota,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	if(length > 0)
	{
		bdSnprintf(buf, length, "Used: %I32d,%I32d,%I32d,%I32d Max: %I32d,%I32d,%I32d,%I32d", 
			quota.m_usedStorageSpace, 
			quota.m_usedNumFiles,
			quota.m_usedDailyUploadBW,
			quota.m_usedDailyDownloadBW,
			quota.m_maxStorageSpace,
			quota.m_maxNumFiles,
			quota.m_maxDailyUploadBW,
			quota.m_maxDailyDownloadBW);
	}
}

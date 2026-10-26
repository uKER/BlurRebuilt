// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_GET_FILES_RESULT_H
#define BD_GET_FILES_RESULT_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdStorage/bdFileData.h>

class bdGetFilesResult : public bdTaskResult
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdGetFilesResult();
	
	bdGetFilesResult(void* fileData, bdUInt fileSize);

	~bdGetFilesResult();

	void setFileData(void* fileData, bdUInt fileSize);

	virtual bdBool deserialize(bdByteBufferRef buffer);

	virtual bdUInt sizeOf();

	bdEntityID m_ID;
	bdFileData m_data;
};


void BD_CALL printInfo(const bdGetFilesResult& thumnail,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // BD_GET_FILES_RESULT_H

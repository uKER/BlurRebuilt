// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_FILE_ID_H
#define BD_FILE_ID_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdFileID : public bdTaskResult
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdFileID();

	~bdFileID();

	virtual void serialize(bdByteBuffer &buffer)const;

	virtual bdBool deserialize(bdByteBufferRef buffer);

	virtual bdUInt sizeOf();

	bdUInt64 m_fileID;

};


void BD_CALL printInfo(const bdFileID& fileID,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // BD_FILE_ID_H

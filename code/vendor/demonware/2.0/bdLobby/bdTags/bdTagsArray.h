// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_TAGS_ARRAY_H
#define BD_TAGS_ARRAY_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdTags/bdTags.h>
#include <bdLobby/bdTags/bdTag.h>

/// Results class for getTags operation in bdTags API.
class bdTagsArray : public bdTaskResult
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	bdTagsArray();

	~bdTagsArray();

	virtual void serialize(bdByteBuffer &buffer)const;

	virtual bdBool deserialize(bdByteBufferRef buffer);

	virtual bdUInt sizeOf();

	bdEntityID m_entityID;
	bdUInt32 m_numTags;
	bdTag m_tags[BD_MAX_NUM_TAGS];

};


void BD_CALL printInfo(const bdTagsArray& tags,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // BD_TAGS_ARRAY_H

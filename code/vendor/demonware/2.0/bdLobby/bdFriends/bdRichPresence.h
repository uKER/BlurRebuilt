// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef H_BD_RICH_PRESENCE_DATA_H
#define H_BD_RICH_PRESENCE_DATA_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdRichPresence : public bdTaskResult
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
	/// Default constructor
	bdRichPresence();

	/// Constructor
	/// \param richPresence[in]	Pointer to a memory block to store the rich presence data.
	/// \param size[in]		Size of the memory block in bytes.
	bdRichPresence(void *richPresence, const bdUInt size);
		
	~bdRichPresence();

	virtual bdBool deserialize(bdByteBufferRef buffer);
	virtual bdUInt sizeOf();

	/// Size of the memory block
	bdUInt32 m_dataSize;
	
	/// A pointer to the actual rich presence data.
	/// It is the responsibility of the client application to allocate a memory
	/// block large enough for holding the data when retrieving the data from
	/// the friends service.
	void *m_data;
};

#endif

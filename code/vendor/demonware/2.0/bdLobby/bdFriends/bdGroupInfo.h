// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: Container class to describe friend groups.

#ifndef BD_GROUP_INFO_H
#define BD_GROUP_INFO_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

		
class bdGroupInfo : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdGroupInfo();
		~bdGroupInfo();

		/// Serialize the entire group info into a buffer.
		/// \param buffer[in|out]	The buffer into which the member variables 
		///							are serialized.
		virtual void serialize(bdByteBuffer &buffer)const;

		/// Deserialize the group info from a buffer.
		/// \param buffer[in|out]	The buffer from which the member variables 
		///							are deserialized.
		/// \return 
		///		- True, if deserialization completed successfully.
		///		- False, otherwise.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		bdUByte8 m_groupID;
		bdNChar8 m_groupName[BD_MAX_GROUPNAME_LENGTH];
};


#endif // BD_GROUP_INFO_H

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_FILE_INFO_H
#define BD_FILE_INFO_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>


class bdFileInfo : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdVisibility
		{
			BD_VISIBLE_PUBLIC,
			BD_VISIBLE_PRIVATE,
			BD_MAX_VISIBILITY_TYPE
		};

		bdFileInfo();
		
		~bdFileInfo();

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// Unique file ID
		bdUInt64 m_fileID;

		/// Unix timestamp of the file's creation time
		bdUInt32 m_createTime; 

		/// Unix timestamp of the last time the file was modified
		bdUInt32 m_modifedTime; 

		/// File visibility (public/private)
		bdVisibility m_visibility;

		/// The file's owner ID
		bdUInt64 m_ownerID;

		/// The file name
		bdNChar8 m_fileName[BD_MAX_FILE_NAME_LENGTH];

		/// The size of the file in bytes
		bdUInt32 m_fileSize;
};

#endif // BD_FILE_INFO_H

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_FILE_META_DATA_H
#define BD_FILE_META_DATA_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdStorage/bdFileData.h>
#include <bdLobby/bdTags/bdTags.h>

class bdFileMetaData : public bdTaskResult
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		enum bdVisibility
		{
			BD_VISIBLE_PUBLIC,
			BD_VISIBLE_PRIVATE,
			BD_MAX_VISIBILITY_TYPE
		};

		/// Default constructor
		bdFileMetaData();

		~bdFileMetaData();

		void reset();
	
	private:
		void resetArrays();
	
	public:	
		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// Unique file ID
		bdUInt64 m_fileID;

		/// Unix timestamp of the file's creation time
		bdUInt32 m_createTime; 

		/// Unix timestamp of the last time the file was modified
		bdUInt32 m_modifedTime; 

		/// The size of the file in bytes
		bdUInt32 m_fileSize;

		/// The file's owner ID
		bdUInt64 m_ownerID;

		/// the file owner's name
		bdNChar8 m_ownerName[BD_MAX_USERNAME_LENGTH];

		/// The index of the file slot within the users space on the server
		bdUInt16 m_fileSlot;

		/// The file name
		bdNChar8 m_fileName[BD_MAX_FILE_NAME_LENGTH];

		/// The file URL if any
		bdNChar8 m_url[BD_MAX_URL_SIZE];

		// The category, a cosmetic integer
		bdUInt16 m_category;

		// The number of tags last retrieved
		bdUInt m_numTags;

		// The tags of a file. Specified on file upload.
		bdTag m_tags[BD_MAX_NUM_TAGS];

		// The bdStorageService fileID of the thumbnail
		bdEntityID m_thumbNailFileID;

		// The size of the thumbnail data on the server
		bdUInt32 m_thumbNailDataSize;
};

#endif // BD_FILE_META_DATA_H

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_FILE_DATA_H
#define BD_FILE_DATA_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdFileData : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdFileData();

		/// Constructor
		/// \param fileData[in]	Pointer to a memory block large enough to store the file data
		/// \param fileSize[in] Size of the memory block in bytes
		bdFileData(void* fileData, bdUInt fileSize);
		
		~bdFileData();

		virtual bdBool deserialize(bdByteBufferRef buffer);
		
		virtual bdUInt sizeOf();

		/// A pointer to the actual contents of the file.
		/// It is the responsibility of the client application to allocate a memory
		/// block large enough for holding the file data when downloading a file from
		/// the storage service.
		void *m_fileData;

		/// The actual size of the file.
		bdUInt m_fileSize;

};

#endif // BD_FILE_DATA_H

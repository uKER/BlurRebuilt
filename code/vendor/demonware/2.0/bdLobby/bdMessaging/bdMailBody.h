// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_MAIL_BODY_H
#define BD_MAIL_BODY_H

#include <bdCore/bdCore.h>
#include <bdLobby/bdLobbyCommon/bdTaskResult.h>

class bdMailBody : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor
		bdMailBody();

		/// Constructor
		/// \param mailBody[in]	Pointer to a memory block to store the downloaded mail.
		/// \param size[in]		Size of the memory block in bytes.
		bdMailBody(	void *mailBody, 
					const bdUInt size);

		~bdMailBody();

		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// A pointer to the actual contents of the mail.
		/// It is the responsibility of the client application to allocate a memory
		/// block large enough for holding the data when retrieving a mail from
		/// the messaging service.
		void *m_mailBody;

		/// Size of the memory block
		bdUInt m_size;

		/// The mail ID
		bdUInt64 m_mailID;

};
#endif // BD_MAIL_BODY_H

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS3 UNIX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE: Contains additional information, which is obtained via conversion
//          of a PS3 NP ticket into a DemonWare ticket.

#ifndef BD_PS3_AUTH_INFO_H
#define BD_PS3_AUTH_INFO_H

#define BD_PS3_AUTH_INFO_SIZE	17u

#include <bdCore/bdCore.h>

BD_REFERENCE(bdPS3AuthInfo);

class bdPS3AuthInfo : public bdReferencable
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		bdPS3AuthInfo();
		~bdPS3AuthInfo();

		bdBool deserialize(const void *buffer, bdUInt bufferSize);
		bdUInt serialize(void *buffer, bdUInt bufferSize);
	
		bdNChar8 m_region[4];
		bdNChar8 m_domain[4];
		bdBool m_AVChatAvailability;
		bdBool m_ContentRatingEnabled;
		bdUByte8 m_age;
};

#endif //BD_PS3_AUTH_INFO_H

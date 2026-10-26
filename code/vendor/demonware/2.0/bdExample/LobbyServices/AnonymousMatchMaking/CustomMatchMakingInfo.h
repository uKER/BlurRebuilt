// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_CUSTOM_MATCH_MAKING_INFO_H
#define BD_CUSTOM_MATCH_MAKING_INFO_H

#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>
	 
#define BD_MAX_SESSIONNAME_LENGTH (32u + 1u)

/// This class derives from bdMatchMakingInfo to add custom data attributes.
/// It also overrides the base class serialize/deserialize functions so that the
/// custom attributes are transmitted and received correctly
/// See the Advanced MatchMaking tutorial for more info.
class CustomMatchMakingInfo : public bdMatchMakingInfo
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		CustomMatchMakingInfo();	

		virtual ~CustomMatchMakingInfo();

		bdUWord setSessionName(const bdNChar8 *const sessionName);
							  
		bdUWord getSessionName(bdNChar8 *const buf,
							   const bdUInt bufSize) const;		

		void setLevelNum(const bdInt levelNum);

		bdInt getLevelNum() const;		

		virtual void serialize(bdByteBuffer &buffer)const;
		
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();


		bdNChar8 m_sessionName[BD_MAX_SESSIONNAME_LENGTH];
		bdInt m_levelNum;
		bdUByte8 m_securityKey[BD_SECURITY_KEY_LENGTH];
};

void BD_CALL printInfo(const CustomMatchMakingInfo& sessionData,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // BD_CUSTOM_MATCH_MAKING_INFO_H

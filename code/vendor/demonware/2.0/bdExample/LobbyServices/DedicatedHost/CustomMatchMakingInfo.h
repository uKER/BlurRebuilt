
// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
// VERSION    : 1.74
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef CUSTOM_MATCH_MAKING_INFO_H
#define CUSTOM_MATCH_MAKING_INFO_H

#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>
//#include <bdLobby/bdLobbyCommon/bdCustomQueryParams.h>
	 
#define BD_MAX_LICENSEID_LENGTH (8u)

/// This class derives from bdMatchMakingInfo to add custom data attributes.
/// It also overrides the base class serialize/deserialize functions so that the
/// custom attributes are transmitted and received correctly
/// See the Advanced MatchMaking tutorial for more info.
class CustomMatchMakingInfo : public bdMatchMakingInfo
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		CustomMatchMakingInfo();

		~CustomMatchMakingInfo();
		
		CustomMatchMakingInfo(const bdUByte8 *const licenseID,
								const bdInt licenseIDLength);
							  
		bdInt getLicenseID(bdUByte8 *const dest, const bdUInt size) const;
		bdInt setLicenseID(const bdUByte8 *const src, const bdUInt size);
        /// Serialize all matchmaking info into a buffer.
		/// This is used to transmit this information to the service.
		/// \param param[out]	The buffer into which the member variables are 
		///						serialized.
		virtual void serialize(bdByteBuffer &buffer)const;
		
		/// Deserialize matchmaking info from a buffer.
		/// This is used to decode matchmaking results received from the service.
		/// \param param[in]	The buffer from which the matchmaking attributes
		///						should be deserialized.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

	protected:

		bdUByte8 m_licenseID[BD_MAX_LICENSEID_LENGTH+1];
};

void BD_CALL printInfo(const CustomMatchMakingInfo& sessionData,
					   bdNChar8 *buf,
					   const bdUInt length);

#endif // CUSTOM_MATCH_MAKING_INFO_H

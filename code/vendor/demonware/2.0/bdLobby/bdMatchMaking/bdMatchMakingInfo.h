// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// Purpose: bdMatchMakingInfo contains information about online game sessions. 
// It is used to advertise game sessions on the matchmaking service, and
// to connect to them.

#ifndef BD_MATCH_MAKING_INFO_H
#define BD_MATCH_MAKING_INFO_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdTaskResult.h>
#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>

#define BD_MAX_HOST_ADDR_SIZE (255u)

/// Contains information about a single online game session.
/// It is used to create or update game sessions on the Matchmaking service,
/// and to connect to hosted games.
class bdMatchMakingInfo : public bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

 		/// Default Constructor
		bdMatchMakingInfo();

		/// Create an instance with the given info.
		/// \param hostAddr[in]			The address at which the game is hosted. Use this constructor if you
		///								advertise a session using the bdCommonAddr address format. The common address
		///								of the local client can be obtained using bdNet::getLocalCommonAddr().
		/// \param gameType[in]			The game type, eg. capture the flag, free for all etc.
		/// \param maxNumPlayers[in]	The maximum number of players allowed in the session.
		bdMatchMakingInfo(const bdCommonAddrRef hostAddr,
						  const bdInt32  gameType,
						  const bdUInt32 maxNumPlayers);

		/// Create an instance with the given info.
		/// \param hostAddr[in]			The address at which the game is hosted. Use this constructor if you
		///								advertise a session using an address format DIFFERENT from bdCommonAddr.
		/// \param hostAddrSize[in]		The size of the host address in bytes. Must not exceed BD_MAX_HOST_ADDR_SIZE
		/// \param gameType[in]			The game type, eg. capture the flag, free for all etc.
		/// \param maxNumPlayers[in]	The maximum number of players allowed in the session.
		bdMatchMakingInfo(const bdUByte8 *hostAddr,
						  const bdUInt32 hostAddrSize,
						  const bdInt32  gameType,
						  const bdUInt32 maxNumPlayers);

		/// Virtual Destructor
		virtual ~bdMatchMakingInfo();

		/// Set the host address of the advertised game session in the bdCommonAddr address format
		/// \param[in] localCommonAddr	The common address of the game session host. The common address
		///								of the local client can be obtained using bdNet::getLocalCommonAddr().
		void setHostAddr(bdCommonAddrRef localCommonAddr);

		/// Retrieve the host address from an advertised game session in the bdCommonAddr address format.
		/// This call will only succeed if the game session has previously been advertised using the 
		/// bdCommonAddr address format.
		/// \param[in] localCommonAddr	Optional representation of the common address of the game session host. 
		///								The common address of the local client can be obtained using 
		///								bdNet::getLocalCommonAddr().
		///	\return The address of the remote host at which the game session is hosted.
		/// \note	If the localCcommonAddr param is not supplied the returned bdCommonAddrRef
		///			will not be correctly initialized, specifically the m_loopback flag will not
		///			be correctly set.
		bdCommonAddrRef getHostAddrAsCommonAddr(bdCommonAddrRef localCommonAddr = BD_NULL) const ;

		/// Serialize the entire game info into a buffer.
		/// \param buffer[in|out]	The buffer into which the member variables 
		///							are serialized.
		virtual void serialize(bdByteBuffer &buffer)const;

		/// Deserialize the entire game info from a buffer.
		/// \param buffer[in|out]	The buffer from which the member variables 
		///							are deserialized.
		/// \return 
		///		- True, if deserialization completed successfully.
		///		- False, otherwise.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		/// Security credentials, the security ID
		bdSecurityID m_sessionID;

		/// The address at which the game is hosted
		bdUByte8 m_hostAddr[BD_MAX_HOST_ADDR_SIZE];

		/// The actual size of the host address
		bdUInt   m_hostAddrSize;

		/// The game type (eg. free for all, capture the flag, etc)
		bdUInt	m_gameType;

		/// number of public spaces available
		bdUInt m_maxPlayers;

		/// number of players currently in the session
		bdUInt m_numPlayers;


};

#endif // BD_MATCH_MAKING_INFO_H

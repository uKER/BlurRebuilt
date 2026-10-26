// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef LEADERBOARD_INFO_H
#define LEADERBOARD_INFO_H

#include <bdLobby/bdStats/bdStatsInfo.h>
#include <bdLobby/bdStats/bdArbitratedStatsInfo.h>
		 
#define LEADERBOARD_ID (1u)
#define LEADERBOARD2_ID (2u)
#define ARBITRATED_LEADERBOARD_ID (3u)

/// This class derives from bdStatsInfo to add custom data attributes.
/// It also overrides the base class serialize/deserialize functions so that the
/// custom attributes are transmitted and received correctly
/// See the stats tutorial for more info.

class LeaderBoardInfo : public bdStatsInfo
{
	public:
	
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		LeaderBoardInfo();		

		virtual ~LeaderBoardInfo();

		/// Serialize all matchmaking info into a buffer.
		/// This is used to transmit this information to the service.
		/// \param param[out]	The buffer into which the member variables are 
		///						serialized.
		virtual void serialize(bdByteBuffer &buffer) const;

		/// Deserialize matchmaking info from a buffer.
		/// This is used to decode matchmaking results received from the service.
		/// \param param[in]	The buffer from which the matchmaking attributes
		///						should be deserialized.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		bdInt64 m_kills;
		
		bdInt32 m_deaths;
		
		bdInt32 m_started;
		
		bdInt32 m_completed;	
		
};

class LeaderBoardInfo2 : public bdStatsInfo
{
	public:
	
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		LeaderBoardInfo2();		

		/// Serialize all matchmaking info into a buffer.
		/// This is used to transmit this information to the service.
		/// \param param[out]	The buffer into which the member variables are 
		///						serialized.
		virtual void serialize(bdByteBuffer &buffer) const;

		/// Deserialize matchmaking info from a buffer.
		/// This is used to decode matchmaking results received from the service.
		/// \param param[in]	The buffer from which the matchmaking attributes
		///						should be deserialized.
		virtual bdBool deserialize(bdByteBufferRef buffer);

		virtual bdUInt sizeOf();

		bdInt64 m_kills;
		
		bdInt32 m_deaths;
		
		
};

class LeaderBoardInfo3 : public bdArbitratedStatsInfo
{
public:

	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	LeaderBoardInfo3(){};


	virtual bdUInt sizeOf()
	{
		return sizeof(LeaderBoardInfo3);
	}


	/// Serialize all matchmaking info into a buffer.
	/// This is used to transmit this information to the service.
	/// \param param[out]	The buffer into which the member variables are 
	///						serialized.
	virtual void serialize(bdByteBuffer &buffer) const
	{
		// Call up to the base class to serialize the standard statistics info.
		bdArbitratedStatsInfo::serialize(buffer);

		// Serialize the custom attributes.
		buffer.writeInt64(m_kills);
		buffer.writeInt32(m_deaths);
	}

	bdInt64 m_kills;
	bdInt32 m_deaths;


};


void BD_CALL printInfo(const LeaderBoardInfo& sessionData,
					   bdNChar8 *buf,
					   const bdUInt length);


#endif // LEADERBOARD_INFO_H

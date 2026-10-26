// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "LeaderboardEntry.h"

LeaderboardEntry::LeaderboardEntry()
: m_kills(0),    
  m_deaths(0),   
  m_assists(0),   
  m_started(0),   
  m_completed(0),   
  m_killRate(0)
{
}

LeaderboardEntry::~LeaderboardEntry()
{
}

void LeaderboardEntry::setKills(const bdInt64 kills)
{
	m_kills = kills;
}	

bdInt64 LeaderboardEntry::getKills() const
{
	return m_kills;
}

void LeaderboardEntry::setDeaths(const bdInt32 deaths)
{
	m_deaths = deaths;
}        

bdUInt LeaderboardEntry::getDeaths() const
{
	return m_deaths;
}

void LeaderboardEntry::setAssists(const bdInt32 assists)
{
	m_assists = assists;
}
          
bdUInt LeaderboardEntry::getAssists() const
{
	return m_assists;
}

void LeaderboardEntry::setStarted(const bdInt32 started)
{
	m_started = started;
}          
bdUInt LeaderboardEntry::getStarted() const
{
	return m_started;
}

void LeaderboardEntry::setCompleted(const bdInt32 completed)
{
	m_completed = completed;
}
          
bdUInt LeaderboardEntry::getCompleted() const
{
	return m_completed;
}

void LeaderboardEntry::setKillRate(const bdFloat64 killRate)
{
	m_killRate = killRate;
}
          
bdFloat64 LeaderboardEntry::getKillRate() const
{
	return m_killRate;
}
				 
void LeaderboardEntry::serialize(bdByteBuffer &buffer)const
{
	// Call up to the base class to serialize the standard statistics info.
	bdStatsInfo::serialize(buffer);

	// Serialize the custom attributes.
	buffer.writeInt64(m_kills);
	buffer.writeInt32(m_deaths);
	buffer.writeInt32(m_assists);
	buffer.writeInt32(m_started);
	buffer.writeInt32(m_completed);
	buffer.writeFloat64(m_killRate);	
}

bdBool LeaderboardEntry::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdStatsInfo::deserialize(buffer);
	ok = ok && buffer->readInt64(m_kills);	
	ok = ok && buffer->readInt32(m_deaths);	
	ok = ok && buffer->readInt32(m_assists);	
	ok = ok && buffer->readInt32(m_started);	
	ok = ok && buffer->readInt32(m_completed);
	ok = ok && buffer->readFloat64(m_killRate);
	return ok;
}

bdUInt LeaderboardEntry::sizeOf()
{
	return sizeof(LeaderboardEntry);
}

void BD_CALL printInfo(const LeaderboardEntry& sessionData,
					   bdNChar8 *buf,
					   const bdUInt length)
{
/*
	bdNChar8 name[BD_MAX_USERNAME_LENGTH+1];
	sessionData.getEntityName(name, sizeof(name));

	bdUInt bytesLeft = length; 
	const bdInt required = bdSnprintf(buf, bytesLeft, "%s Rank:BD_UINT_64_FORMAT Kills:BD_INT_64_FORMAT Deaths:%u", 
		name, sessionData.getRank(), sessionData.getKills(), sessionData.getDeaths());

	bytesLeft -= BD_CLAMP(static_cast<bdUInt>(required), 0, bytesLeft);

	bdSnprintf(buf, bytesLeft, " Assists:%u Started:%u Completed:%u KillRate:%.2f", 
		sessionData.getAssists(), sessionData.getStarted(), sessionData.getCompleted(), sessionData.getKillRate());
*/
}

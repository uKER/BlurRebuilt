// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef GAME_INFO_H
#define GAME_INFO_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

BD_REFERENCE(GameInfo);

#define GAME_NAME_SIZE 32

class GameInfo : public bdGameInfo
{
	public:

		GameInfo();

		virtual void serialize(bdBitBuffer& bitBuffer) const;

		virtual bdBool deserialize(const bdCommonAddrRef me, 
								   bdBitBuffer& bitBuffer);

		bdNChar8 m_gameName[GAME_NAME_SIZE];
};

#endif // GAME_INFO_H

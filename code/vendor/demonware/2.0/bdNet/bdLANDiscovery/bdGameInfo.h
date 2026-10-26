// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// Purpose: bdGameInfo contains information about hosted games. It is used 
// to advertise game sessions on a LAN and to connect to hosted games.

#ifndef BD_GAME_INFO_H
#define BD_GAME_INFO_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdGameInfo);

class bdGameInfo : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdGameInfo();

		bdGameInfo(bdUInt32 titleID, 
				   const bdSecurityID& securityID, 
				   const bdSecurityKey& securityKey,
				   bdCommonAddrRef hostAddr);

		virtual ~bdGameInfo();

		/// Returns the title id of this game
		/// \return The game's title id
		bdUInt32 getTitleID() const;

		/// Returns the address at which the game is hosted
		/// \return The game's common address
		bdCommonAddrRef getHostAddr() const;

		/// Returns the security ID for this game
		/// \return The game's security ID
		const bdSecurityID& getSecurityID() const;

		/// Returns the security key for this game
		/// \return The game's security key
		const bdSecurityKey& getSecurityKey()const;

		/// Sets the Title ID for this game
		/// \param titleID[in] The game's title ID
		void setTitleID(const bdUInt32 titleID);

		/// Sets the common addr of the game host.
		/// \param hostAddr[in] The common addr of the game's host
		void setHostAddr(const bdCommonAddrRef hostAddr);

		/// Sets the security ID for this game
		/// \param secID[in] The game's security ID
		void setSecurityID(const bdSecurityID& secID);

		/// Sets the security key for this game
		/// \param secKey[in] The game's security key
		void setSecurityKey(const bdSecurityKey& secKey);

		/// Serializes the entire game info into a bitbuffer
		/// \param bitBuffer[in|out] The bitbuffer into which the member variables are serialized.
		virtual void serialize(bdBitBuffer& bitBuffer) const;

		/// Deserializes a game info from the given bitbuffer
		virtual bdBool deserialize(const bdCommonAddrRef me,
								   bdBitBuffer& bitBuffer);

	protected:

		/// The game's title ID
		bdUInt32    m_titleId;

		/// Security credentials
		bdSecurityID m_secID;
		bdSecurityKey m_secKey;

		/// The address of the game host.
		bdCommonAddrRef m_hostAddr;

};

#endif // BD_GAME_INFO_H




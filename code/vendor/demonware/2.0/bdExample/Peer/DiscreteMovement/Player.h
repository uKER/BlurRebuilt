// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef PLAYER_H
#define PLAYER_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

BD_REFERENCE(Player);

class UserCommand;

class Player : public bdPlayer
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		Player();

		Player(const bdNChar8* const name);

		virtual ~Player();

		const bdString& getName() const;
		
		/// bdPlayer interface
		virtual void serialize(bdBitBufferRef buffer);

		/// bdPlayer interface
		virtual bdBool deserialize(bdBitBufferRef buffer);

		void createSphere(const bdFloat32 *const pos,
						  const bdVector4& color);

		void destroySphere();

		void applyCommand(const UserCommand *const command,
						  const bdFloat32 time);

	protected:

		bdString m_name;

		Sphere *m_sphere;
};

#endif // PLAYER_H

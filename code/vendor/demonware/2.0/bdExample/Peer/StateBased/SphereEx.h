// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef SPHERE_EX_H
#define SPHERE_EX_H

#include <bdExample/bdExampleCommonPeer/bdExampleCommonP2P.h>

class SphereEx
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		SphereEx();
			
		~SphereEx();

		Sphere& getSphere();

		void setConnection(const bdConnectionRef connection);

		bdConnectionRef getConnection();

		void writeInitData(const bdBitBufferRef buffer);

		bdBool applyInitData(const bdReceivedMessage& initMsg);

		void createUpdateData(const bdBitBufferRef buffer);

		bdBool applyUpdateData(const bdReceivedMessage& updateMsg);

	protected:

		Sphere m_sphere;
		bdConnectionRef m_connection;
};

#endif // SPHERE_EX_H


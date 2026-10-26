// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef QOS_MANAGER_H
#define QOS_MANAGER_H

#include <bdCore/bdCore.h>

class bdQoSProbe;

class QOSManager
{
	public:

		QOSManager ();

		~QOSManager ();

		void init(	bdQoSProbe *probe,
					const bdSecurityID &secID);

	protected:
		bdQoSProbe *m_qosProber;
		bdUInt32 m_playerCount;
	

};

#endif // QOS_MANAGER_H

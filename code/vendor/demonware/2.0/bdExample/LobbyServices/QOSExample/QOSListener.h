// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef QOS_LISTENER_H
#define QOS_LISTENER_H

#include <bdSocket/bdQoS/bdQoSProbeListener.h>


// A class to perform platform specific initialization and clean up
class QOSListener : public bdQoSProbeListener
{
	public:

		QOSListener();

		virtual ~QOSListener();

		void onQoSProbeSuccess(const bdQoSProbeInfo &info);

		void onQoSProbeFail(bdCommonAddrRef addr);

		void init(bdUInt numProbes);

		bdBool isFinished();

	protected:
		bdUInt m_numProbeRepliesExpected;
		bdUInt m_numProbeRepliesReceived;


};

#endif // QOS_LISTENER_H

#ifndef TESTSECTIONMEDIATORSESSION_H
#define TESTSECTIONMEDIATORSESSION_H

#ifdef _MSC_VER
#pragma once
#endif


#include <TestSections/TestSection.h>
#include <netTransport/netTransport_ConnectedMesh.h>
#include <netTransport/netTransport_SegmentPool.h>
#include <netTransport/netTransport_StreamPacket.h>
#include <netTransport/netTransport_Event.h>
#include <netTransport/netTransport_LANDiscovery.h>
#include <netTransport/netTransport_TrafficAnalyser.h>
#include <netMediator/netMediator_Session.h>

BD_REFERENCE(TestSocketListener);

class C_TestSectionMediatorSession : public Framework::C_TestSection
{
public:

	C_TestSectionMediatorSession(const Util::C_String& name);
	~C_TestSectionMediatorSession();

	void OnRender2DView();
	bool updateInputs();

	enum
	{
		STATE_DISCOVER,
		STATE_START_HOST,
		STATE_START_CLIENT,
		STATE_UPDATE,
		STATE_CLOSE,
		STATE_ERROR,
	};
	bool update();
	bool setState(int state);
	int m_state;
	int m_step;
	netStopwatch m_stateWatch;
	netStopwatch m_botWatch;

	netMediation::SessionRef			m_session;
	netTransport::LANDiscoveryClientRef	m_LANDiscovery;
	netEnumeratedSessionRef				m_game;
};

#endif //TESTSECTIONMEDIATORSESSION_H
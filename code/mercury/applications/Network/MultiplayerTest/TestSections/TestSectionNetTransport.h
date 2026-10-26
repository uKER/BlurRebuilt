#ifndef TESTSECTIONNETTRANSPORT_H
#define TESTSECTIONNETTRANSPORT_H

#ifdef _MSC_VER
#pragma once
#endif


#include <TestSections/TestSection.h>
#include <netTransport/netTransport_ConnectedMesh.h>
#include <netTransport/netTransport_SegmentPool.h>
#include <netTransport/netTransport_StreamPacket.h>
#include <netTransport/netTransport_Event.h>
#include <netTransport/netTransport_LANDiscovery.h>

BD_REFERENCE(TestSocketListener);

class C_TestSectionNetTransport : public Framework::C_TestSection
{
public:

	C_TestSectionNetTransport(const Util::C_String& name);
	~C_TestSectionNetTransport();

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
	netStopwatch m_customMsgWatch;

	netTransport::MessageParserRef		m_messageParser;
	netTransport::SocketRef				m_socket;
	netTransport::ConnectedMeshRef		m_connectedMesh;
	netTransport::LANDiscoveryClientRef	m_LANDiscovery;
	bdGameInfoRef						m_game;
	TestSocketListenerRef				m_socketListener;
};


#endif //TESTSECTIONNETTRANSPORT_H
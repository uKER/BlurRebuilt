#include "stdafx.h"
#include "TestSectionNetTransport.h"
#include "netPlatform/netPlatform.h"
#include "netPlatform/netPlatform_Messages.h"
#include "netPlatform/netPlatform_core.h"
#include <bdNet/bdLANDiscovery/bdGameInfoFactory.h>
#include <bdNet/bdLANDiscovery/bdGameInfo.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <system/debug/I_DebugRender.h>
#include "Framework/Application/UserControls.h"

static bool g_reliable = false;
static bool g_acceptConnections = true;
static bool g_closing = true;
static int  g_closingConnection = 0;
static int g_sendRate= 4;
static const char* s_tempText = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaassssssssssssssssssssssssssssssssssssssstttttttttttttttttttttttttttttttttttwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwuuuuuuuuuuuuuuuuuuuuuuuuuuuuiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiivvvvvvvvvvvvvvvvvvvvvvvvvvvvvmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm";
						
Debug::Logger& g_log = Debug::GetLogger("test_net_transport.log");

/////////////////////////////////////////////////////////////////////////////
// Class       : TestSocketClient
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 06 March 2009
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(TestSocketClient);
class TestSocketClient: public bdReferencable
{
public:
	TestSocketClient(netTransport::SegmentPoolRef segmentPool, netTransport::SocketRef socket, int socketPort, netTransport::ConnGuid guid, bool send);
	~TestSocketClient();

	void poll();
	bool sendText(const char* text);
	bool receive(netTransport::Message* msg);

	netTransport::StreamPacketSendRef m_sentPacket;
	netTransport::StreamPacketReceiveRef m_receivedPacket;

	bool m_send;
	int m_count;
	netVector m_text;
	netStopwatch m_watch;
};

TestSocketClient::TestSocketClient(netTransport::SegmentPoolRef segmentPool, netTransport::SocketRef socket, int socketPort, netTransport::ConnGuid guid, bool send)
: m_text(32)
, m_count(0)
, m_send(send)
{
	m_sentPacket = new netTransport::StreamPacketSend(segmentPool, 0, socket, socketPort, guid);
	m_receivedPacket = new netTransport::StreamPacketReceive(segmentPool, (256 * 1024), 0, socket, socketPort, guid);
}

TestSocketClient::~TestSocketClient()
{}

void TestSocketClient::poll()
{
	// sanity check
	if(!m_receivedPacket->ok())
	{
		m_receivedPacket->reset(m_receivedPacket->sqn());
	}
	
	//----------------------------------------------
	// send packet.
	//----------------------------------------------
	if(m_sentPacket->transmissionFinished() && m_send)
	{
		if(!m_sentPacket->ok())
			m_sentPacket->transmissionStart(m_sentPacket->sqn() + 1, g_reliable);
	
		// packet finished sending. geenerate a new one.
		if(m_sentPacket->transmissionFinished())
		{
			if(!m_watch.started() || m_watch.elapsed(1 * 1000 * 1000))
			{
				m_watch.start();

				char temp[128];
				sprintf_s(temp, sizeof(temp), "       <<<<<< TESTING(%d) >>>>>>\n", m_count);
				m_count++;
				
				if(m_text.getWriteByteCount() > 0)
					m_text.setWriteByteCount(m_text.getWriteByteCount()-1);

				m_text.writeBytes(s_tempText, strlen(s_tempText));
				m_text.writeBytes(temp, strlen(temp)+1);

				if(m_text.getWriteByteCount() > 16000)
					m_text.clear();

				sendText(m_text.buffer());
			}
		}
	}
	m_sentPacket->transmit();
}

bool TestSocketClient::receive(netTransport::Message* msg)
{
	switch(msg->messageId())
	{
	case netTransport::MESSAGE_ID_PACKET_ACK:
		{
			m_sentPacket->receive(msg);
			break;
		}
	default:
		{
			m_receivedPacket->receive(msg);

			//----------------------------------------------
			// receive packet completed.
			//----------------------------------------------
			if(m_receivedPacket->complete() && !m_receivedPacket->eof())
			{
				char string[100];
				int len = (sizeof(string) > m_receivedPacket->getWriteByteCount())? m_receivedPacket->getWriteByteCount() : sizeof(string);

				m_receivedPacket->setReadByteCount(m_receivedPacket->getWriteByteCount() - len);
				m_receivedPacket->readBytes(string, len);

				string[0] = '.';
				string[1] = '.';
				string[2] = '.';
				LOGINFO(g_log, "<TestSocketClient> received %s", string);
			}
			break;
		}
	}	
	return true;
}

bool TestSocketClient::sendText(const char* text)
{
	if(!m_sentPacket->transmissionFinished())
		return false;

	m_sentPacket->clear();
	m_sentPacket->writeBytes(text, strlen(text)+1);
	m_sentPacket->transmissionStart(m_sentPacket->sqn() + 1, g_reliable);
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// Class       : TestSocketListener
//---------------------------------------------------------------------------
// Created By  : Genesis Network Team
// Created On  : 06 March 2009
// Description : [TO DO : add class description here.]
/////////////////////////////////////////////////////////////////////////////
BD_REFERENCE(TestSocketListener);
class TestSocketListener: public netTransport::SocketListener, public bdReferencable
{
public:
	TestSocketListener(netTransport::SocketRef socket);
	~TestSocketListener();
	
	bool recv(netTransport::Message* msg, int socketPort, netTransport::ConnGuid from);
	bool sendText(const char* text, netTransport::ConnGuid client);
	
	bool update();
	bool createClient(netTransport::ConnGuid guid, bool sender);
	bool destroyClient(netTransport::ConnGuid guid);

private:
	netTransport::SegmentPoolRef m_segmentPool;
	netTransport::SocketRef	m_socket;
	bdHashMap<int, TestSocketClientRef> m_clients;
};

TestSocketListener::TestSocketListener(netTransport::SocketRef socket)
: m_socket(socket)
, m_segmentPool(new netTransport::SegmentPool(64))
{
	m_socket->registerListener(this);
}

TestSocketListener::~TestSocketListener()
{
	m_clients.clear();
	m_segmentPool = NULL;
	m_socket->unregisterListener(this);
}

bool TestSocketListener::sendText(const char* text, netTransport::ConnGuid guid)
{
	TestSocketClientRef client;
	
	if(!m_clients.get(guid, client))
		return false;

	return client->sendText(text);
}


bool TestSocketListener::update()
{
	bdHashMap<int, TestSocketClientRef>::Iterator it = m_clients.getIterator();
	while(it)
	{
		TestSocketClientRef client = m_clients.getValue(it);
		m_clients.next(it);
		client->poll();
	}
	m_clients.releaseIterator(it);
	return true;
}

bool TestSocketListener::createClient(netTransport::ConnGuid guid, bool sender)
{
	if(m_clients.containsKey(guid))
	{
		LOGFATAL(g_log, "<TestSocketListener::destroyClient(%d)> client already registered.", guid.m_guid);
		return false;
	}

	m_clients.put(guid, new TestSocketClient(m_segmentPool, m_socket, 0, guid, sender));
	return true;
}

bool TestSocketListener::destroyClient(netTransport::ConnGuid guid)
{
	m_clients.remove(guid);
	return true;
}	


bool TestSocketListener::recv(netTransport::Message* msg, int socketPort, netTransport::ConnGuid from)
{
	switch(msg->messageId())
	{
	default:
		{
			LOGFATAL(g_log, "<TestSocketListener::recv(%d)> unhandled message(%s).", from.m_guid, msg->debugName());
			return false;
		}
	case netTransport::MESSAGE_ID_CONNECT_REQUEST:
		{
			netTransport::MsgConnectRequest* request = (netTransport::MsgConnectRequest*)msg;
				
			bdCommonAddrRef addr = new bdCommonAddr;
			request->m_addr.toAddr(addr);
			char buff[128];
			netPlatform::Core::instance()->getAddrInfo(addr, buff, sizeof(buff));
			const char* nonce = netPlatform::Core::instance()->nonceToString(request->m_connectKey);
			int size =request->m_payloadSize;

			LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : addr(%s), payload(%d), key(%s).", from.m_guid, msg->debugName(), buff, size, nonce);
			return true;
		}
	case netTransport::MESSAGE_ID_CONNECT_ACCEPT:
		{
			netTransport::MsgConnectAccepted* accepted = (netTransport::MsgConnectAccepted*)msg;

			const char* nonce = netPlatform::Core::instance()->nonceToString(accepted->m_connectKey);
			LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : assigned(%d), host(%d), key(%s).", from.m_guid, msg->debugName(), accepted->m_assignedGuid, accepted->m_hostGuid, nonce);
			return true;
		}
	case netTransport::MESSAGE_ID_CONNECT_PING:
		{
			netTransport::MsgConnectPing* ping = (netTransport::MsgConnectPing*)msg;
			
			//LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : timestamp(%llu)", from.m_guid, msg->debugName(), ping->m_timestamp);
			return true;
		}
	case netTransport::MESSAGE_ID_CONNECT_PONG:
		{
			netTransport::MsgConnectPong* pong = (netTransport::MsgConnectPong*)msg;
			
			//LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : timestamp(%llu)", from.m_guid, msg->debugName(), pong->m_timestamp);
			return true;
		}
	case netTransport::MESSAGE_ID_CONNECT_CLOSING:
		{
			netTransport::MsgConnectClosing* closing = (netTransport::MsgConnectClosing*)msg;
			
			LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : reason(%d)", from.m_guid, msg->debugName(), closing->m_closeReason);
			return true;
		}
	case netTransport::MESSAGE_ID_VOICE_FRAGMENTS:
		{
			netTransport::MsgVoiceFragments* fragments = (netTransport::MsgVoiceFragments*)msg;

			LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : fragments(%d)", from.m_guid, msg->debugName(), fragments->m_fragmentsTransmitted);
			return true;
		}
	case netTransport::MESSAGE_ID_PACKET:
		{
			netTransport::MsgPacket* packet = (netTransport::MsgPacket*)msg;

			//LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : size(%d)", from.m_guid, msg->debugName(), packet->m_payloadSize);

			TestSocketClientRef client;	
			if(m_clients.get(from, client))
			{
				client->receive(msg);
			}
			return true;
		}
	case netTransport::MESSAGE_ID_PACKET_FRAGMENT:
		{
			netTransport::MsgPacketFragment* fragment = (netTransport::MsgPacketFragment*)msg;

			//LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : size(%d)", from.m_guid, msg->debugName(), fragment->m_payloadTransmitted);

			TestSocketClientRef client;	
			if(m_clients.get(from, client))
			{
				client->receive(msg);
			}

			return true;
		}
	case netTransport::MESSAGE_ID_PACKET_ACK:
		{
			netTransport::MsgPacketAck* ack = (netTransport::MsgPacketAck*)msg;

			//LOGINFO(g_log, "<TestSocketListener::recv(%d)> [%s] : ptr(%d)", from.m_guid, msg->debugName(), ack->m_packetAck);

			TestSocketClientRef client;	
			if(m_clients.get(from, client))
			{
				client->receive(msg);
			}

			return true;
		}
	}
	return false;
}

C_TestSectionNetTransport::C_TestSectionNetTransport(const Util::C_String& name) : Framework::C_TestSection(name)
{
	LOGERROR(g_log, "<NetTransport> initialising...");
				
	netPlatform::Platform::Startup();

	// register LAN game info class
	BD_REGISTER_GAME_INFO_CLASS(bdGameInfo);

	netPlatform::Core::instance()->requestMode(netPlatform::MODE_SYSTEMLINK);

	setState(STATE_DISCOVER);
}

C_TestSectionNetTransport::~C_TestSectionNetTransport()
{
	LOGERROR(g_log, "<NetTransport> shtudown.");
	
	netPlatform::Core::instance()->requestMode(netPlatform::MODE_OFFLINE);
	netPlatform::Platform::Destroy();
}

void drawText(float x, float y, float s, unsigned int RGBA, int align, const char* format, ...)
{
	char txt[200];
	va_list args;
	va_start(args, format);
	vsprintf_s(txt, sizeof(txt), format, args);
	va_end(args);

	DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dText(Maths::HVec2(x, y), txt, Maths::RGBA(RGBA), s);

	/*
	Display::E_FontRenderAlign alignment = 
		(align < 0)? Display::E_FontRenderAlign_Lt : 
		(align == 0)? Display::E_FontRenderAlign_Ct : 
		Display::E_FontRenderAlign_Rt;

	Display::FontRender()->Draw(txt, x, y, s, s, Maths::RGBA(RGBA), alignment);
	*/
}

void C_TestSectionNetTransport::OnRender2DView()
{
	update();

	//Display::FontRender()->Draw("Hello!", 1024.f/2.f, 768.f/2.f, 50.f, 50.f, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
	float y = 30.0f;
	float x = 30.0f;
	float s = 16.0f;
	
	drawText(x, y, s, 0xffffffff, -1, "(button 'Y') - timeout : %s", netTransport::Debugging::dbg_timeoutEnabled? "true" : "false");
	y += s;

	drawText(x, y, s, 0xffffffff, -1, "(button 'X') - reliable : %s", g_reliable? "true" : "false");
	y += s;

	drawText(x, y, s, 0xffffffff, -1, "(button 'A') - %s connections", g_acceptConnections? "accepting" : "refusing");
	y += s;

	drawText(x, y, s, 0xffffffff, -1, "(up / down) - close connection(%d)", g_closingConnection);
	y += s;

	if(m_connectedMesh.notNull())
	{	
		float latency; 
		int latencyCount;
		m_connectedMesh->getLatency(latency, latencyCount);
		
		netTransport::TrafficSample outboundReport;
		netTransport::TrafficSample inboundReport;
		m_connectedMesh->getOutboundTraffic(outboundReport);
		m_connectedMesh->getInboundTraffic(inboundReport);
		unsigned int colour;
		unsigned int out_latency = (unsigned int)((latency / latencyCount) * 1000);
		unsigned int out_usage = (unsigned int)outboundReport.m_bytes[netTransport::TRAFFIC_TYPE_SOCKET];
		unsigned int out_cap = (unsigned int)outboundReport.limit();
		unsigned int in_usage = (unsigned int)inboundReport.m_bytes[netTransport::TRAFFIC_TYPE_SOCKET];
		unsigned int in_cap = (unsigned int)inboundReport.limit();
		colour = (out_usage > out_cap)? 0xff0000ff : 0xffffffff;

		drawText(x, y, s, 0xffffffff, colour, "out(%.2f/%.2f kB/s, %d ms), in(%.2f/%.2f)kB/s", out_usage / 1024.0f, out_cap / 1024.0f, out_latency, in_usage / 1024.0f, in_cap / 1024.0f);
		y += s;
	}

	if(g_closing)
	{
		drawText(x, y, s, 0xffffffff, -1, "(button 'B') - shutting down");
		y += s;
	}
}

bool C_TestSectionNetTransport::setState(int state)
{
	if(state == m_state)
		return false;

	m_stateWatch.start();
	m_state = state;
	m_step = 0;
	return true;
}
	
bool C_TestSectionNetTransport::updateInputs()
{
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadLeft, 10, 5))
	{
		g_sendRate --;
		if(g_sendRate < 1) g_sendRate = 1;
		LOGINFO(g_log, "<NetTransport::update()> send rate : %f kB/s", (float)g_sendRate);
	}

	if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadRight, 10, 5))
	{
		g_sendRate ++;
		if(g_sendRate > 128) g_sendRate = 128;
		LOGINFO(g_log, "<NetTransport::update()> send rate : %f kB/s", (float)g_sendRate);
	}

	if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_ButtonX, 10, 5))
	{
		g_reliable = !g_reliable;
		LOGINFO(g_log, "<NetTransport::update()> reliable packets %s", g_reliable? "enabled" : "disabled");
	}

	bool timeout = Input::GetGameInput(0)->GetButtonValue(Framework::E_Button_Debug_ButtonY) != 0.0f;

	if(timeout != netTransport::Debugging::dbg_timeoutEnabled)
	{
		netTransport::Debugging::dbg_timeoutEnabled = timeout;
		LOGINFO(g_log, "<NetTransport::update()> timeout %s", timeout? "enabled" : "disabled");
	}
	
	bool accept = Input::GetGameInput(0)->GetButtonValue(Framework::E_Button_Debug_ButtonA) == 0.0f;

	if(accept != g_acceptConnections)
	{
		g_acceptConnections = accept;
		LOGINFO(g_log, "<NetTransport::update()> %s connections", g_acceptConnections? "accepting" : "refusing");
	}

	bool closing = Input::GetGameInput(0)->GetButtonValue(Framework::E_Button_Debug_ButtonB) != 0.0f;

	if(closing != g_closing)
	{
		g_closing = closing;
		if(closing)
			LOGINFO(g_log, "<NetTransport::update()> shutting down.");
	}
	
	if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadUp, 10, 5))
	{
		g_closingConnection ++;
		if(g_closingConnection > netTransport::CONN_GUID_ASSIGNED_LAST)
			g_closingConnection = 0;

		LOGINFO(g_log, "<NetTransport::update()> closing connection(%d)", g_closingConnection);
	}

	if(Input::GetGameInput(0)->GetButtonAutoRepeat(Framework::E_Button_Debug_DPadDown, 10, 5))
	{
		g_closingConnection --;
		if(g_closingConnection < 0)
			g_closingConnection = 0;

		LOGINFO(g_log, "<NetTransport::update()> closing connection(%d)", g_closingConnection);
	}
	return true;
}

bool C_TestSectionNetTransport::update()
{
	SysProfile::GameInvite outGameInvite;
	SysProfile::Notifications notifications;
	SysProfile::C_Manager::Instance().Poll(outGameInvite, notifications);

	netPlatform::Core::instance()->update();

	if((m_state != STATE_ERROR) && netPlatform::Core::instance()->error())
	{
		LOGERROR(g_log, "<NetTransport::update()> platform init error!");
		return setState(STATE_ERROR);
	}

	updateInputs();

	switch(m_state)
	{
	default:
		{
			return false;
		}
	case STATE_DISCOVER:
		{
			if(m_step == 0)
			{
				m_connectedMesh = NULL;
				m_LANDiscovery = NULL;
				m_socketListener = NULL;
				m_game = NULL;
				m_socket = NULL;

				if(!netPlatform::Core::instance()->runningSystemLink() && !netPlatform::Core::instance()->runningOnline())
				{
					return true;
				}

				LOGERROR(g_log, "<NetTransport::update()> platform %s!", netPlatform::Core::instance()->runningOnline()? "ONLINE" : "SYSTEMLINK");
				
				LOGINFO(g_log, "<NetTransport::update()> starting discovery...");

				m_LANDiscovery = new netTransport::LANDiscoveryClient;
				
				m_LANDiscovery->discover('TEST', 5.0f);
				
				m_step++;
			}
			
			m_LANDiscovery->update();

			if(m_LANDiscovery->getStatus() == bdLANDiscoveryClient::BD_PENDING)
				return false;

			if(m_LANDiscovery->getStatus() == bdLANDiscoveryClient::BD_ERROR)
			{
				LOGERROR(g_log, "<NetTransport::update()> discovery error!");
				return setState(STATE_ERROR);
			}

			if(m_LANDiscovery->m_results.isEmpty())
			{
				m_game = netTransport::LANDiscoveryServer::createGame('TEST');

				LOGINFO(g_log, "<NetTransport::update()> no session found.");
				return setState(STATE_START_HOST);
			}
			else
			{
				m_game = m_LANDiscovery->m_results.getHead();
				
				char buff[128];
				netPlatform::Core::instance()->getAddrInfo(m_game->getHostAddr(), buff, sizeof(buff));

				LOGINFO(g_log, "<NetTransport::update()> session found at[%s].", buff);
				
				return setState(STATE_START_CLIENT);
			}
		}
	case STATE_START_HOST:
		{
			LOGINFO(g_log, "<NetTransport::update()> started hosting...");

			netTransport::Debugging::dbg_logPacketOrigin = 1;
			netTransport::Debugging::dbg_logPacketDestination = 1;
			netTransport::Debugging::dbg_logPacketReliable = true;

			m_messageParser = new netTransport::MessageParser();
			m_socket = new netTransport::Socket(m_messageParser);
			m_socketListener = new TestSocketListener(m_socket);
			m_connectedMesh = new netTransport::ConnectedMesh(16, 'TEST', m_socket, 0);
			m_connectedMesh->host(m_game->getSecurityID(), m_game->getSecurityKey());
			m_connectedMesh->advertise(new bdGameInfo);
			return setState(STATE_UPDATE);
		}
	case STATE_START_CLIENT:
		{
			LOGINFO(g_log, "<NetTransport::update()> started client...");

			netTransport::Debugging::dbg_logPacketOrigin = 0;
			netTransport::Debugging::dbg_logPacketDestination = 0;
			netTransport::Debugging::dbg_logPacketReliable = true;

			m_messageParser = new netTransport::MessageParser();
			m_socket = new netTransport::Socket(m_messageParser);
			m_socketListener = new TestSocketListener(m_socket);
			m_connectedMesh = new netTransport::ConnectedMesh(16, 'TEST', m_socket, 0);
			m_connectedMesh->join(m_game->getSecurityID(), m_game->getSecurityKey(), m_game->getHostAddr());
			m_connectedMesh->advertise(new bdGameInfo);
			return setState(STATE_UPDATE);
		}
	case  STATE_UPDATE:
		{
			m_socket->listen();

			m_socketListener->update();
			
			if(g_closing)
			{
				return setState(STATE_CLOSE);
			}
			if(g_closingConnection > 0)
			{
				m_connectedMesh->close(g_closingConnection, netTransport::CLOSE_REASON_KICKED);
			}			

			if(m_connectedMesh->closed())
			{
				return setState(STATE_CLOSE);
			}

			m_connectedMesh->setRates(g_sendRate * 1024, g_sendRate * 1024);

			netTransport::EventRef ev;
			do
			{
				ev = m_connectedMesh->poll();

				if(ev.isNull())
					break;

				switch(ev->m_eventId)
				{
				default:
					{
						LOGERROR(g_log, "<NetTransport::update()> unhandled event(%d)...", ev->m_eventId);
						break;
					}
				case netTransport::EVENT_ID_CONNECT_PENDING:
					{
						netTransport::Event_ConnPendingRef e = ((netTransport::Event_ConnPendingRef)ev);
						LOGINFO(g_log, "<NetTransport::update()> CONNECT PENDING client(%d)...", e->m_guid.m_guid);

						if(g_acceptConnections)
						{
							LOGINFO(g_log, "<NetTransport::update()> accepting client(%d)...", e->m_guid.m_guid);
							m_connectedMesh->accept(e->m_guid);
						}
						else
						{
							LOGINFO(g_log, "<NetTransport::update()> refusing client(%d)...", e->m_guid.m_guid);
							m_connectedMesh->close(e->m_guid, netTransport::CLOSE_REASON_KICKED);
						}
						break;
					}
				case netTransport::EVENT_ID_CONNECT_ACCEPTED:
					{
						netTransport::Event_ConnAcceptedRef e = ((netTransport::Event_ConnAcceptedRef)ev);
						LOGERROR(g_log, "<NetTransport::update()> CONNECT ACCEPTED self(%d), host(%d).", e->m_assignedGuid, e->m_hostGuid);
						break;
					}
				case netTransport::EVENT_ID_CONNECT_ESTABLISHED:
					{
						netTransport::Event_ConnEstablishedRef e = ((netTransport::Event_ConnEstablishedRef)ev);
						LOGERROR(g_log, "<NetTransport::update()> CONNECT ESTABLISHED %s(%d).", m_connectedMesh->hosting()? "client" : "peer", e->m_guid.m_guid);

						if(m_connectedMesh->hosting() || m_connectedMesh->peering())
						{
							bool sender = m_connectedMesh->hosting();
							m_socketListener->createClient(e->m_guid, sender);
						}
						break;
					}
				case netTransport::EVENT_ID_CONNECT_LATENCY:
					{
						netTransport::Event_ConnLatencyRef e = ((netTransport::Event_ConnLatencyRef)ev);
						//LOGERROR(g_log, "<NetTransport::update()> CONNECT LATENCY %s(%d), %d ms.", m_connectedMesh->hosting()? "client" : "peer", e->m_guid, (e->m_latency / 1000));
						break;
					}
				case netTransport::EVENT_ID_CONNECT_CLOSED:
					{
						netTransport::Event_ConnClosedRef e = ((netTransport::Event_ConnClosedRef)ev);
						LOGERROR(g_log, "<NetTransport::update()> CONNECT CLOSED %s(%d) reason(%d).", m_connectedMesh->hosting()? "client" : "peer", e->m_guid.m_guid, e->m_reason);
						
						m_socketListener->destroyClient(e->m_guid);
						break;
					}
				case netTransport::EVENT_ID_MESH_HOSTING:
					{
						netTransport::Event_MeshHostingRef e = ((netTransport::Event_MeshHostingRef)ev);
						LOGERROR(g_log, "<NetTransport::update()> HOSTING (%d).", e->m_selfGuid.m_guid);
						break;
					}
				case netTransport::EVENT_ID_MESH_PEERING:
					{
						netTransport::Event_MeshPeeringRef e = ((netTransport::Event_MeshPeeringRef)ev);
						LOGERROR(g_log, "<NetTransport::update()> PEERING self(%d), host(%d).", e->m_selfGuid, e->m_hostGuid);
						break;
					}
				case netTransport::EVENT_ID_MESH_CLOSED:
					{
						netTransport::Event_MeshClosedRef e = ((netTransport::Event_MeshClosedRef)ev);
						LOGERROR(g_log, "<NetTransport::update()> CLOSED reason(%d).", e->m_reason);
						break;
					}
				}
			}
			while(ev.notNull());
			return true;
		}
		case STATE_CLOSE:
			{
				if(m_step == 0)
				{
					LOGINFO(g_log, "<NetTransport::update()> closing...");
			
					m_connectedMesh->close();
					m_step++;
				}

				m_connectedMesh->poll();

				if(m_step == 1)
				{
					if(m_connectedMesh->closed())
					{
						LOGINFO(g_log, "<NetTransport::update()> closed (reason = %d)...", m_connectedMesh->closeReason());
						m_step++;
					}
				}

				if (m_step == 2)
				{
					if(m_stateWatch.elapsed(5 * 1000 * 1000))
					{
						return setState(STATE_DISCOVER);
					}
				}
				return true;
			}
		case STATE_ERROR:
			{
				LOGINFO(g_log, "<NetTransport::update()> error!");
			
				if(m_stateWatch.elapsed(5 * 1000 * 1000))
				{
					return setState(STATE_DISCOVER);
				}
				return true;
			}
	}
	return true;
}

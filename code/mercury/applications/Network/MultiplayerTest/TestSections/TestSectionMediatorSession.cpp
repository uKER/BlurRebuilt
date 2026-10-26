
#include "stdafx.h"
#include "TestSectionMediatorSession.h"
#include "Framework/Application/UserControls.h"
#include "netDebug/netDebug.h"
#include "netPlatform/netPlatform.h"
#include "netPlatform/netPlatform_core.h"
#include "netPlatform/netPlatform_Messages.h"
#include "netSession/netEnumeratedSession.h"
#include "netSession/netSessionJoinData.h"
#include "netSession/netSessionCreateData.h"
#include "netState/statemanager.h"
#include "netState/factory/ifactory.h"
#include "netTransport/netTransport_Enums.h"
#include "TestSource/SessionApp/TestSession.h"
#include <system/debug/I_DebugRender.h>
#include <system/debug/BuildStamp.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

static bool g_botCommands = false;
static bool g_reliable = false;
static bool g_closing = false;
static int  g_closingConnection = 0;
static int g_titleId = 'tsms';

static Debug::Logger& g_log = Debug::GetLogger("test_net_mediator.log");

#include "netStream/netRLEEncoder.h"

void debugRLEEncoder()
{
	unsigned char bitfield[256];
	int byteCount = 20;
	memset(bitfield, 0, sizeof(bitfield));
		
	for(int i = 0; i < 100; i ++)
	{
		netArray<256> serialiser;
		char copy[256];
		memcpy(copy, bitfield, byteCount);
				
		{
			BitfieldRLE rle(copy, byteCount * 8);
			rle.encode();
			rle.serialise(&serialiser);
		}
		
		{
			BitfieldRLE rle(copy, byteCount * 8);
			rle.deserialise(&serialiser);
			rle.decode();
		}

		{
			if (memcmp(copy, bitfield, byteCount) != 0)
			{
				assert(!"error in rle compression.");
			}
		}

		{
			unsigned char newBits[256];
			bdTrulyRandom::getInstance()->getRandomUByte8((bdUByte8*)&newBits, sizeof(newBits));
			for(int i = 0; i < sizeof(newBits); i ++)
			{
				if((newBits[0] & 7) == 0)
				{
					bitfield[i] ^= newBits[i];
				}
				else
				{
					bitfield[i] |= newBits[i];
				}
			}

			bdTrulyRandom::getInstance()->getRandomUByte8((bdUByte8*)&byteCount, sizeof(byteCount));
			byteCount = abs(byteCount) % 200 + 1;
		}

	}
}

C_TestSectionMediatorSession::C_TestSectionMediatorSession(const Util::C_String& name) : Framework::C_TestSection(name)
{
	const U8* builder = (const U8*)(Debug::GetBuildUser());
	g_titleId = Btl::HashRaw(builder, (int) strlen((const char*)builder), g_titleId);

	if (!stricmp((const char*)builder, "orenault"))
	{
		//g_botCommands = true;
		netTransport::Debugging::dbg_timeoutEnabled = false;
		netTransport::Debugging::dbg_simulatorEnabled = true;
		netTransport::Debugging::dbg_simulatorLatency = 200;
		netTransport::Debugging::dbg_simulatorPacketLoss = 5;
		netTransport::Debugging::dbg_ping = false;
		netTransport::Debugging::dbg_pingId = 1;
		netDebug::dbg_renderMode = E_DbgRnd_Migration; //E_DbgRnd_GameState;
	}

	LOGERROR(g_log, "<C_TestSectionMediatorSession> initialising...");
				
	netPlatform::Platform::Startup();

	// register LAN game info class
	BD_REGISTER_GAME_INFO_CLASS(netEnumeratedSession);

	netTransport::Debugging::dbg_migrationEnabled = true;

	new C_StateManager(1);

	I_Factory::CreateInstance();

	netPlatform::Core::instance()->requestMode(netPlatform::MODE_SYSTEMLINK);

	m_state = -1;
	m_step = 0;
	setState(STATE_DISCOVER);
}

C_TestSectionMediatorSession::~C_TestSectionMediatorSession()
{
	LOGERROR(g_log, "<C_TestSectionMediatorSession> shtudown.");
	
	netPlatform::Core::instance()->requestMode(netPlatform::MODE_OFFLINE);
	netPlatform::Platform::Destroy();
}

static void drawText(float x, float y, float s, unsigned int RGBA, const char* format, va_list args)
{
	char txt[200];
	vsprintf_s(txt, sizeof(txt), format, args);
	DbUtil::DefaultDebugRenderSys()->Immediate()->Draw2dText(Maths::HVec2(x, y), txt, Maths::RGBA(RGBA), s);
}

static void drawText(float x, float y, float s, unsigned int RGBA, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	drawText(x, y, s, RGBA, format, args);
	va_end(args);
}

const float minx = 20.0f;
const float maxx = 1024.0f - 20.0f;
const float miny = 20.0f;
const float maxy = 768.0f - 20.0f;
const float rowHeight = 24.0f;
const int numColumns = 3;

float itemX(int itemId)
{
	int column = (itemId % numColumns);
	return minx + column * ((maxx - minx) / numColumns);
}
float itemY(int itemId)
{
	int row = (itemId / numColumns);
	return miny + row * rowHeight;
}

int nextLine(int itemId)
{
	return (numColumns - (itemId % numColumns)) % numColumns;
}

int drawItem(int itemId, const char* format, ...)
{
	float x = itemX(itemId);
	float y = itemY(itemId);
	va_list args;
	va_start(args, format);
	drawText(x, y, rowHeight, 0xffffffff, format, args);
	va_end(args);
	return 1;
}	

void C_TestSectionMediatorSession::OnRender2DView()
{
//	debugRLEEncoder();

	update();

	int itemId = 0;
	if(m_session.notNull() && m_session->mesh().notNull())
	{
		netTransport::ConnectedMeshRef mesh = m_session->mesh();

		char addrStr[64];
		netTransport::Addr localAddr;
		localAddr.fromAddr(netPlatform::Core::instance()->myCommonAddr());
		localAddr.toString(addrStr, sizeof(addrStr));

		netMediation::PeerStateRef host = m_session->peer(mesh->hostGuid());
		netMediation::PeerStateRef self = m_session->peer(mesh->selfGuid());
		
		float latency; 
		int latencyCount;
		mesh->getLatency(latency, latencyCount);
		
		netTransport::TrafficSample outboundReport;
		netTransport::TrafficSample inboundReport;
		mesh->getOutboundTraffic(outboundReport);
		mesh->getInboundTraffic(inboundReport);
		
		unsigned int colour;
		unsigned int out_latency = (unsigned int)((latency / latencyCount) * 1000);
		float out_usage = outboundReport.m_bytes[netTransport::TRAFFIC_TYPE_SOCKET];
		float out_cap = outboundReport.limit();
		float in_usage = inboundReport.m_bytes[netTransport::TRAFFIC_TYPE_SOCKET];
		float in_cap = inboundReport.limit();
		colour = (out_usage > out_cap)? 0xff0000ff : 0xffffffff;

		char stats[512];
		sprintf_s(stats, sizeof(stats), "out(%.2f/%.2f kB/s, %d ms), in(%.2f/%.2f)kB/s", out_usage / 1024.0f, out_cap / 1024.0f, out_latency, in_usage / 1024.0f, in_cap / 1024.0f);
		
		if(host.notNull() && self.notNull())
		{
			drawItem(itemId, "[%s] self(%s), host(%s) %s", addrStr, self.notNull()? self->m_session.m_name : "...", host.notNull()? host->m_session.m_name : "...", stats);
			itemId = numColumns;
		}

		if(g_botCommands)
		{
			itemId += drawItem(itemId, "<<<<<<<------ MIGRATION MOLESTER ------>>>>>>>");
			itemId += nextLine(itemId);
		}
		
		if(mesh->hosting())
		{
			itemId += drawItem(itemId, "(button 'A') - promote client.");
		}
		itemId += drawItem(itemId, "(button 'LS') - debug migration.");
		itemId += drawItem(itemId, "(button 'B') - shutdown");
		itemId += drawItem(itemId, "(button 'Y') - timeout : %s", netTransport::Debugging::dbg_timeoutEnabled? "true" : "false");
		itemId += drawItem(itemId, "(button 'X') - reliable : %s", g_reliable? "true" : "false");
		itemId += drawItem(itemId, "(up / down) - close connection(%d)", g_closingConnection);
		itemId += nextLine(itemId);
				
		float x = itemX(itemId);
		float y = itemY(itemId);
		m_session->render(DbUtil::DefaultDebugRenderSys()->Immediate(), minx, y, maxx-x, maxy-y);
	}
	else
	{
		char consoleName[64];
		netPlatform::Core::instance()->getConsoleName(consoleName, sizeof(consoleName));
	
		char addrStr[64];
		netTransport::Addr localAddr;
		localAddr.fromAddr(netPlatform::Core::instance()->myCommonAddr());
		localAddr.toString(addrStr, sizeof(addrStr));

		itemId += drawItem(itemId, "[%s] console(%s)", addrStr, consoleName);
		
		switch(m_state)
		{
		case STATE_DISCOVER:
			{
				itemId += drawItem(itemId, "searching for host(%s)", m_game.isNull()? "none found..." : "game found!");
				break;
			}
		case STATE_START_HOST:
			{
				itemId += drawItem(itemId, "starting host...");
				break;
			}
		case STATE_START_CLIENT:
			{
				itemId += drawItem(itemId, "starting client...");
				break;
			}
		case STATE_UPDATE:
			{
				itemId += drawItem(itemId, "updating...");
				break;
			}
		case STATE_CLOSE:
			{
				itemId += drawItem(itemId, "closing...");
				break;
			}
		case STATE_ERROR:
			{
				itemId += drawItem(itemId, "ERROR!!!!");
				break;
			}
		}
	}
}

bool C_TestSectionMediatorSession::setState(int state)
{
	if(state == m_state)
		return false;

	m_stateWatch.start();
	m_state = state;
	m_step = 0;
	return true;
}

int random(int range)
{
	unsigned int val;
	netPlatform::Core::instance()->generateRandom(&val, sizeof(val));
	return val % (range);
}		

bool C_TestSectionMediatorSession::updateInputs()
{
	bool buttonState[Framework::E_Button_Count];

	for(int i = 0; i < Framework::E_Button_Count; i ++)
	{
		buttonState[i] = Input::GetGameInput(0)->GetButtonAutoRepeat(i, 10, 5);
	}

	if(g_botCommands)
	{
		netTransport::Debugging::dbg_timeoutEnabled = true;

		if(m_botWatch.ticked(1000 * 1000))
		{
			if(random(50) == 0)
			{
				netTransport::Debugging::migrate(NULL);
			}

			if(random(100) == 0)
			{
				g_closing = true;
			}
		}
		else
		{
			g_closing = false;
		}
	}

	if(buttonState[Framework::E_Button_Debug_ButtonStart])
	{
		g_botCommands = !g_botCommands;
		LOGINFO(g_log, "<C_TestSectionMediatorSession> bot : %s", g_botCommands? "on" : "off");
	}

	if(buttonState[Framework::E_Button_Debug_DPadLeft])
	{
		netDebug::dbg_bandwidthCap = netDebug::dbg_bandwidthCap * 0.95f;
		if(netDebug::dbg_bandwidthCap < netDebug::dbg_bandwidthCap.Low()) netDebug::dbg_bandwidthCap = netDebug::dbg_bandwidthCap.Low();
		LOGINFO(g_log, "<C_TestSectionMediatorSession> send rate : %f kB/s", (float) netDebug::dbg_bandwidthCap);
	}

	if(buttonState[Framework::E_Button_Debug_DPadRight])
	{
		netDebug::dbg_bandwidthCap = netDebug::dbg_bandwidthCap * 1.05f;
		if(netDebug::dbg_bandwidthCap > netDebug::dbg_bandwidthCap.High()) netDebug::dbg_bandwidthCap = netDebug::dbg_bandwidthCap.High();
		LOGINFO(g_log, "<C_TestSectionMediatorSession> send rate : %f kB/s", (float) netDebug::dbg_bandwidthCap);
	}

	if(buttonState[Framework::E_Button_Debug_ButtonX])
	{
		g_reliable = !g_reliable;
		LOGINFO(g_log, "<C_TestSectionMediatorSession> reliable packets %s", g_reliable? "enabled" : "disabled");
	}

	if(buttonState[Framework::E_Button_Debug_ButtonY])
	{
		netTransport::Debugging::dbg_timeoutEnabled = !netTransport::Debugging::dbg_timeoutEnabled;
		LOGINFO(g_log, "<C_TestSectionMediatorSession> timeout %s", netTransport::Debugging::dbg_timeoutEnabled? "enabled" : "disabled");
	}
	
	if(buttonState[Framework::E_Button_Debug_ButtonLB])
	{
		LOGINFO(g_log, "<C_TestSectionMediatorSession> debug output migration.");
		netTransport::Debugging::outputMigrate(NULL);
	}

	if(buttonState[Framework::E_Button_Debug_ButtonA])
	{
		LOGINFO(g_log, "<C_TestSectionMediatorSession> migrating...");
		netTransport::Debugging::migrate(NULL);
	}

	if(buttonState[Framework::E_Button_Debug_ButtonB])
	{
		g_closing = true;		
		if(g_closing)
			LOGINFO(g_log, "<C_TestSectionMediatorSession> shutting down.");
	}
	
	if(buttonState[Framework::E_Button_Debug_DPadUp])
	{
		g_closingConnection ++;
		if(g_closingConnection > netTransport::CONN_GUID_ASSIGNED_LAST)
			g_closingConnection = 0;

		LOGINFO(g_log, "<C_TestSectionMediatorSession> closing connection(%d)", g_closingConnection);
	}

	if(buttonState[Framework::E_Button_Debug_DPadDown])
	{
		g_closingConnection --;
		if(g_closingConnection < 0)
			g_closingConnection = 0;

		LOGINFO(g_log, "<C_TestSectionMediatorSession> closing connection(%d)", g_closingConnection);
	}
	return true;
}

bool C_TestSectionMediatorSession::update()
{
	SysProfile::GameInvite outGameInvite;
	SysProfile::Notifications notifications;
	SysProfile::C_Manager::Instance().Poll(outGameInvite, notifications);

	netPlatform::Core::instance()->update();

	if((m_state != STATE_ERROR) && netPlatform::Core::instance()->error())
	{
		LOGERROR(g_log, "<C_TestSectionMediatorSession> platform init error!");
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
				m_session = NULL;
				m_LANDiscovery = NULL;
				m_game = NULL;

				if(!netPlatform::Core::instance()->runningSystemLink() && !netPlatform::Core::instance()->runningOnline())
				{
					return true;
				}

				LOGERROR(g_log, "<C_TestSectionMediatorSession> platform %s!", netPlatform::Core::instance()->runningOnline()? "ONLINE" : "SYSTEMLINK");
				
				LOGINFO(g_log, "<C_TestSectionMediatorSession> starting discovery...");

				m_LANDiscovery = new netTransport::LANDiscoveryClient;
				
				m_LANDiscovery->discover(g_titleId, 5.0f);
				
				m_step++;
			}
			
			m_LANDiscovery->update();

			if(m_LANDiscovery->getStatus() == bdLANDiscoveryClient::BD_PENDING)
				return false;

			if(m_LANDiscovery->getStatus() == bdLANDiscoveryClient::BD_ERROR)
			{
				LOGERROR(g_log, "<C_TestSectionMediatorSession> discovery error!");
				return setState(STATE_ERROR);
			}

			if(m_LANDiscovery->m_results.isEmpty())
			{
				LOGINFO(g_log, "<C_TestSectionMediatorSession> no session found.");
				return setState(STATE_START_HOST);
			}
			else
			{
				m_game = (netEnumeratedSessionRef) m_LANDiscovery->m_results.getHead();
				
				char buff[128];
				netPlatform::Core::instance()->getAddrInfo(m_game->getHostAddr(), buff, sizeof(buff));

				LOGINFO(g_log, "<C_TestSectionMediatorSession> session found at[%s].", buff);
				
				return setState(STATE_START_CLIENT);
			}
		}
	case STATE_START_HOST:
		{
			g_closing = false;

			LOGINFO(g_log, "<C_TestSectionMediatorSession> started hosting...");

			m_session = new Test::Session();

			netSessionCreateDataRef create = new netSessionCreateData();
			create->m_online = false;
			create->m_party = false;
			create->m_titleId = g_titleId;
			create->m_pad = 0;
			m_session->create(create, NULL);
			return setState(STATE_UPDATE);
		}
	case STATE_START_CLIENT:
		{
			g_closing = false;

			LOGINFO(g_log, "<C_TestSectionMediatorSession> started client...");
			
			m_session = new Test::Session();
			
			netSessionJoinDataRef join = new netSessionJoinData();
			join->m_online = false;
			join->m_party = false;
			join->m_enumeratedSession = m_game;

			m_session->join(join, NULL);
			return setState(STATE_UPDATE);
		}
	case  STATE_UPDATE:
		{
			m_session->poll();

			if(g_closing)
			{
				return setState(STATE_CLOSE);
			}
			
			if(g_closingConnection > 0)
			{
				netTransport::ConnectedMeshRef mesh = (m_session.notNull())? m_session->mesh() : NULL;
				
				if(mesh.notNull())
					mesh->close(netTransport::ConnGuid(g_closingConnection), netTransport::CLOSE_REASON_KICKED);
			}			

			if(m_session->disconnected())
			{
				return setState(STATE_CLOSE);
			}

			if(m_session->left())
			{
				return setState(STATE_CLOSE);
			}
			return true;
		}
		case STATE_CLOSE:
			{
				if(m_step == 0)
				{
					LOGINFO(g_log, "<C_TestSectionMediatorSession> closing...");
			
					m_session->leave(false);
					m_step++;
				}

				m_session->poll();

				netTransport::ConnectedMeshRef mesh = (m_session.notNull())? m_session->mesh() : NULL;
				
				if(m_step == 1)
				{
					if(m_session->left())
					{
						LOGINFO(g_log, "<C_TestSectionMediatorSession> closed (reason = %d)...", mesh.notNull()? mesh->closeReason() : netTransport::CLOSE_REASON_NOT_SET);
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
				g_closing = false;

				LOGINFO(g_log, "<C_TestSectionMediatorSession> error!");
			
				if(m_stateWatch.elapsed(5 * 1000 * 1000))
				{
					return setState(STATE_DISCOVER);
				}
				return true;
			}
	}
	return true;
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif


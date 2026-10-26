#include "SystemPch.h"

#include <System/Artery/Artery.h>
#include <misc_shared/Messaging/MessageBase.h>
#include "system/automation/Automation.h"
#include "system/automation/AutomationMessages.h"
#include <system/debug/log/LogManager.h>
#include <system/debug/log/Logger.h>
#include <system/memory/MemoryTracker.h>

#include <system/debug/variables.h>
#include <system/debug/BuildStamp.h>

namespace Automation
{

#ifndef CFG_GOLD

static Debug::Logger &g_Log = Debug::GetLogger("automation");

const int VERSION = 2;

static BroadcastMessageClassCallback	g_pBroadcastMessageClassCallback	= NULL;
static bool								g_AutomationInProgress				= false;
static bool								g_AssertHasFired					= false;

Ut::UtVector<Ut::DynamicVector<Callback > >	g_MessageCallbackList;

struct AutomationPeer
{
	ArteryConnection con;
};

const int			kMaxAutomationPeers = 2;
AutomationPeer*		gAutomationPeers[kMaxAutomationPeers] = {NULL};
MT::Lock			gAutomationPeerCs;

bool IsAutomationInProgress()
{
	return g_AutomationInProgress;
}

bool AddAutomationPeer(AutomationPeer* peer)
{
	LOGINFO(g_Log, "Trying AddAutomationPeer...\n");

	bool result = false;

	gAutomationPeerCs.Enter();

	for(int i = 0; i < kMaxAutomationPeers; i++)
	{
		if (gAutomationPeers[i] == 0)
		{
			gAutomationPeers[i]	= peer;
			result				= true;

			g_AutomationInProgress = true;

			LOGINFO(g_Log, "Trying AddAutomationPeer... OK\n");

			break;
		}
	}

	gAutomationPeerCs.Exit();

	return result;
}

void RemoveAutomationPeer(AutomationPeer* peer)
{
	LOGINFO(g_Log, "RemoveAutomationPeer...\n");

	gAutomationPeerCs.Enter();

	for(int i = 0; i < kMaxAutomationPeers; i++)
	{
		if (gAutomationPeers[i] == peer)
		{
			LOGINFO(g_Log, "RemoveAutomationPeer... Removed");
			gAutomationPeers[i] = 0;
			break;
		}
	}

	gAutomationPeerCs.Exit();
}

void MessageProc(ArteryConnection con, void* callback)
{
	if (g_AssertHasFired)
	{
		LOGWARN(g_Log, "Automation: An Assert fired - we're refusing new connections");
		return;
	}

	LOGINFO(g_Log, "Automation: MessageProc Enter");

	// Say Hello
	{
		ArteryMessageBuilder b;

		b.Write("Automation says Hello");

		ArteryMessage m;

		if (b.Build(m))
			con.Send(m);
	}

	AutomationPeer dp;
	dp.con = con;

	if (AddAutomationPeer(&dp))
	{
		ArteryWaitContext ctx;

		while(con.IsConnected())
			con.Wait(ctx);

		RemoveAutomationPeer(&dp);
	}
	else
	{
		ArteryMessageBuilder b;
		b.Write("AddAutomationPeer failed");

		ArteryMessage m;
		if (b.Build(m))
			con.Send(m);
	}

	LOGINFO(g_Log, "Automation: MessageProc Leave");
}

bool gAutomationNetworkStarted = false;

#endif

int InitialiseNetworkConnection(void)
{
#ifndef CFG_GOLD
	gAutomationNetworkStarted = true;

	ArteryRegister("Automation", MessageProc, NULL);

	RegisterMessageDebugInfo();
#endif

	return 0;
}

void RegisterCallback(Callback messageCallback)
{
#ifndef CFG_GOLD
	Ut::UtVector<Ut::DynamicVector<Callback > >::ConstIterator it = g_MessageCallbackList.Begin();

	for (; it != g_MessageCallbackList.End(); it++)
	{
		if (*it == messageCallback)
			return;
	}

	g_MessageCallbackList.PushBack(messageCallback);
#endif
}

void NetworkUpdate()
{
#ifndef CFG_GOLD
	if (!gAutomationNetworkStarted)
		return;

	gAutomationPeerCs.Enter();

	for(int i = 0; i < kMaxAutomationPeers; i++)
	{
		AutomationPeer* peer = gAutomationPeers[i];

		if (!peer || !peer->con.IsConnected())
			continue;

		for(;;)
		{
			ArteryMessage msg;

			if (!peer->con.TryReceive(msg))
				break;

			ArteryMessageReader msgReader(msg);

			const char* msgString(NULL);

			if (msgReader.Read(msgString))
			{
				if (!strcmp(msgString, "Automation"))
				{
					ArteryU32 commandID;
					const char* command(NULL);

					if (msgReader.Read(commandID) && msgReader.Read(command))
					{
						CommandContext ac(peer->con, commandID, command, msgReader);

						if (!stricmp(command, "AutomationVersion"))
						{
							ac.SingleReply(VERSION);
						}
						else if (!stricmp(command, "Ping"))
						{
							ac.SingleReply("Ping");
						}
						else if (!stricmp(command, "BuildVersion"))
						{
							Util::C_String text = Util::C_String::Format("%s!%s!%s", Debug::GetBuildVersion(), Debug::GetBuildDate(), Debug::GetBuildTime());
							ac.SingleReply(text.Chars());
						}
						else if (!stricmp(command, "CheckMemLeak"))
						{
							Memory::MemoryLeakInfo info;

							U32 exclusions[] =
							{
								MEMORY_TRACKER_CONTEXT_ID("debug.memtrack"),
								MEMORY_TRACKER_CONTEXT_ID("lua.ui"),
								MEMORY_TRACKER_CONTEXT_ID("mercury.memorybuffer"),
							};

							// TODO: Add exclusions for lua
							int result = Memory::Tracker::Instance().CheckForMemoryLeaks(lengthof(exclusions), exclusions, info);

							Util::C_String text = Util::C_String::Format("CheckMemLeak:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d",
								info.frontendNoAllocs ? 1 : 0, info.frontendAllocationCount, info.frontendLeakSystem, info.frontendLeakPhysical,
								info.gameNoAllocs ? 1 : 0, info.gameAllocationCount, info.gameLeakSystem, info.gameLeakPhysical,
								info.physicalMemoryDifference, info.totalMemoryDifference);

							ac.SingleReply(text.Chars());
						}
						else if (!stricmp(command, "MemUsage")) // Query how much memory a certain node is using
						{
							const char* contextNodeName(NULL);

							if (msgReader.Read(contextNodeName))
							{
								::Memory::ContextId id = ::Memory::TrackNodes::Instance().FindId(contextNodeName);

								// Query memory size
								size_t usageSize(0);

								if (id != INVALID_CONTEXT_ID)
								{
									::Memory::ContextNode node;
									::Memory::Tracker::Instance().GetContextNode(id, node);

									usageSize = node.Allocated();
								}

								// Send size back to automation
								Util::C_String text = Util::C_String::Format("MemUsage:%s:%d", contextNodeName, usageSize);
								ac.SingleReply(text.Chars());
							}
						}
						else if (!stricmp(command, "LogOnlyFatal"))
						{
							// Turn off all but Fatal messages in the output log
							Debug::LogManager::LoggerMap &loggerMap = Debug::LogManager::Instance().GetLoggerMap();

							for (Debug::LogManager::LoggerMap::Iterator it=loggerMap.Begin(); it != loggerMap.End(); ++it)
							{
								it->GetValue()->SetLevel(DebugLevel::None);
							}

							Debug::GetRootLogger().SetLevel(DebugLevel::Fatal);

							ac.SingleReply(1);
						}
						else
						{
							Ut::UtVector<Ut::DynamicVector<Callback > >::Iterator itMCB = g_MessageCallbackList.Begin();

							bool foundMatch = false;

							for (; !foundMatch && (itMCB != g_MessageCallbackList.End()); itMCB++)
							{
								// Check to see if this handler can process the message
								if ((*itMCB)(ac))
									foundMatch = true;
							}

							// Let the automation know we got an unknown message
							if (!foundMatch)
								ac.SingleReply("-UNKNOWN-");
						}
					}
				}
				else if (!strcmp(msgString, "DebugValue"))
				{
					ArteryU32 commandID;
					const char* variableName(NULL);
					const char* variableValue(NULL);

					if (msgReader.Read(commandID) && msgReader.Read(variableName) && msgReader.Read(variableValue))
					{
						CommandContext ac(peer->con, commandID, "", msgReader);
						
						Debug::VarBase* vb = Debug::FindByName(variableName);

						if (vb)
						{
							switch (vb->Type())
							{
							case Debug::kVtBool:
							{
								bool value = atoi(variableValue) != 0;
								((Debug::Bool*)vb)->SetValue(value);
								break;
							}

							case Debug::kVtInt:
							{
								int value = atoi(variableValue);
								((Debug::Int*)vb)->SetValue(value);
								break;
							}

							case Debug::kVtFloat:
							{
								float value = float(atof(variableValue));
								((Debug::Float*)vb)->SetValue(value);
								break;
							}

							case Debug::kVtEnum:
							{
								((Debug::EnumBase*)vb)->SetValueByName(variableValue);
								break;
							}

							case Debug::kVtString:
							{
								((Debug::String*)vb)->SetValue(variableValue);
								break;
							}

							case Debug::kVtFunction:
							{
								((Debug::Function*)vb)->Call();
								break;
							}

							default:
								BIZ_ASSERT_FAIL("vb->Type(%d)", vb->Type());
							}

							ac.SingleReply(0);
						}
						else
						{
							// Variable not found
							ac.SingleReply(-1);
						}
					}
				}
			}
		}
	}

	gAutomationPeerCs.Exit();
#endif
}

void SendMessageBack(const ArteryMessage& m)
{
#ifndef CFG_GOLD
	gAutomationPeerCs.Enter();

	for (int i=0; i < kMaxAutomationPeers; i++)
	{
		AutomationPeer* peer = gAutomationPeers[i];

		if (peer && peer->con.IsConnected())
		{
			peer->con.Send(m);
		}
	}

	gAutomationPeerCs.Exit();
#endif
}

Debug::Logger &g_AutomationLog( Debug::GetLogger("Test.Automation") );

void Assert(const char* condition, const char* file, int line, const char *func, const char* msg)
{
#ifndef CFG_GOLD
	if (!gAutomationNetworkStarted)
		return;

	g_AssertHasFired = true;

#if 0 // def PLATFORM_XENON
	ULONG_PTR params[] = {
		(ULONG_PTR)strlen(condition),
		(ULONG_PTR)condition,
		(ULONG_PTR)strlen(file),
		(ULONG_PTR)file,
		(ULONG_PTR)strlen(func),
		(ULONG_PTR)func,
		(ULONG_PTR)strlen(msg),
		(ULONG_PTR)msg,
		(ULONG_PTR)line
		};

	for (int i=0; i < _countof(params); i++)
	{
		LOGERROR(g_AutomationLog, "P %2d) - 0x%x", i, params[i]);
	}
	LOGERROR(g_AutomationLog, "A: Cond(%s) file(%s) func(%s) msg(%s) line(%d)", condition, file, func, msg, line);

	// On the 360 we throw an exception to alert automation of an assert - easier to get a crash dump
	RaiseException(0x0000a557, 0, _countof(params), params);
#else
	ArteryMessageBuilder b;
	b.Write("Assert");
	b.Write(condition);
	b.Write(file);
	b.Write(line);
	b.Write(func);
	b.Write(msg);

	ArteryMessage m;
	
	if (b.Build(m))
		SendMessageBack(m);
#endif


#endif
}

void Notify(const char* messageToSend)
{
#ifndef CFG_GOLD
	if (!gAutomationNetworkStarted)
		return;

	ArteryMessageBuilder b;
	b.Write("Notify");
	b.Write(messageToSend);

	ArteryMessage m;
	
	if (b.Build(m))
		SendMessageBack(m);
#endif
}

void Problem(const char* messageToSend)
{
#ifndef CFG_GOLD
	if (!gAutomationNetworkStarted)
		return;

	ArteryMessageBuilder b;
	b.Write("Problem");
	b.Write(messageToSend);

	ArteryMessage m;
	
	if (b.Build(m))
		SendMessageBack(m);
#endif
}

void StopRunning(const char* message)
{
#ifndef CFG_GOLD
	if (!gAutomationNetworkStarted)
		return;

	gAutomationPeerCs.Enter();

	for(int i = 0; i < kMaxAutomationPeers; i++)
	{
		AutomationPeer* peer = gAutomationPeers[i];

		if (!peer || !peer->con.IsConnected())
			continue;

		ArteryMessageBuilder b;
		b.Write("STOP");
		b.Write(message);

		ArteryMessage m;
		if (b.Build(m))
			peer->con.Send(m);
	}

	gAutomationPeerCs.Exit();
#endif
}

void CommandContext::SingleReply(const char* message)
{
#ifndef CFG_GOLD
	ArteryMessageBuilder b;
	b.Write("Automation");
	b.Write(m_commandID);
	b.Write((ArteryU32)1);
	b.Write(message);

	ArteryMessage msg;
	if (b.Build(msg))
	{
		m_connection.Send(msg);
	}
#endif
}

void CommandContext::SingleReply(bool value)
{
	SingleReply(value ? "1" : "0");
}

void CommandContext::SingleReply(int value)
{
	char buffer[128];

#ifndef PLATFORM_PS3
	_itoa_s(value, buffer, lengthof(buffer), 10);
#else
	sprintf(buffer, "%i", value);
#endif

	SingleReply(buffer);
}

void CommandContext::SingleReply(float value)
{
	char buffer[_CVTBUFSIZE];

#ifndef PLATFORM_PS3
	_gcvt(value, 12, buffer);
#else
	sprintf(buffer, "%.f", value);
#endif

	SingleReply(buffer);
}

void AutomationStatus::Add(Util::C_String6 name, int value)
{
	values.PushBack();

	values.Back().name = name;
	values.Back().value = value;
}

void SendProblemFoundMsg(const ProblemString& n, const Automation::ProblemString& area)
{
#ifndef CFG_GOLD
	if (g_pBroadcastMessageClassCallback)
		g_pBroadcastMessageClassCallback(n, -1, area);
#endif
}

void SendProblemFoundMsg(const ProblemString& n, int i, const ProblemString& area)
{
#ifndef BUILD_CONFIG_MASTER
	if (g_pBroadcastMessageClassCallback)
		g_pBroadcastMessageClassCallback(n, i, area);
#endif
}

void SetBroadcastMessageClass(BroadcastMessageClassCallback func)
{
#ifndef CFG_GOLD
	g_pBroadcastMessageClassCallback = func;
#endif
}

void RegisterMessageDebugInfo()
{
	// Game::E_GameMsg
	{
		const char* pMessageNames [] = 
		{
			"E_AutomationMessage_ProblemFound",
		};

		REGISTER_MESSAGE_DEBUG_INFO(pMessageNames, E_AutomationMessage_Count, E_AutomationMessage_Begin);
	}
}

}

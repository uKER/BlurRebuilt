#ifndef _NET_STATE_ENUMS
#define _NET_STATE_ENUMS

const int g_uMaxEntities = 100;
const int g_uMaxGameWorldEntities = 2000;
const int g_uDefaultEntitySize = 32;
const int g_sCalculateSize = -1;
const int g_sNetIdAutoAssign = -1;
const int g_sNoParent = -1;

class netStateDebug
{
public:
	static Debug::Logger& g_Log;
};

class netStateLog
{
public:
	static Debug::Logger& g_Log;
};

enum E_NetState_Type
{
	E_NetState_None=-1,

	E_NetState_Deleting,

	E_NetState_ServerBroadcast,
	E_NetState_PeerBroadcast,
	E_NetState_PeerToServer,				// TODO:
	E_NetState_ServerToPeer,				// TODO:
		
	E_NetState_Count
};

enum E_State_Type
{
	E_State_None=-1,
	E_State_GameWorld,
	E_State_GameLobby,
	E_State_PartyLobby,
	E_State_Count
};

enum E_EntityState
{
	E_EntityState_UnSet = -1,
	E_EntityState_Creating = 0,
	E_EntityState_Waiting,			// TODO
	E_EntityState_First				// TODO
};

enum E_Entity_Migration_Status
{
	E_Entity_Migration_AssumedDead,
	E_Entity_Migration_Alive,
	E_Entity_Migration_New
};

enum E_NetEnt_Engine
{
	E_NetEnt_BlankPeerToPeer = 0,
	E_NetEnt_BlankServer,
	E_NetEnt_Engine_Count,
};

enum E_HandleType 
{ 
	E_Handle_Actor,
	E_Handle_ActorCmp,
	E_Handle_Network,
	E_Handle_System,
	E_Handle_Invalid
};

enum E_GameState_Mode
{
	E_Mode_None,
	E_Mode_Creation,
	E_Mode_Destruction,
	E_Mode_Serialisation,
	E_Mode_Migration_First,
	E_Mode_Migration_Start,
	E_Mode_Migration_ServerKeepAlive,
	E_Mode_Migration_ClientReport,
	E_Mode_Migration_Creation,
	E_Mode_Migration_Finish,
	E_Mode_Migration_Last
};

#endif
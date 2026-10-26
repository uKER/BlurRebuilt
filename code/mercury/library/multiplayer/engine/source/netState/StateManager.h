#ifndef _STATE_MANAGER
#define _STATE_MANAGER

#include <system/debug/I_DebugRender.h>

class C_StateManager
{
public:
	C_StateManager(const int uMaxGameStates);
	~C_StateManager();
	bool					CreateGameState(int uGameStateId, const int uMyHostId, const int uNumEntities, const int uNumClients);
	bool					FreeGameState(int uGameStateId);
	class C_GameState*		GetGameState(const int i);
	bool					GameStateExists(const int i) { return m_pxGameStates && m_pxGameStates[i]; }
	bool					GameStateSynchronised(int i);
	bool					GameStatesSynchronised();
	int						MyHostId();
	bool					IsServer(int uGameState);
	void					Update(bool bNetworkFrame);
	void					Render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
	static C_StateManager*	GetInstance() { return m_pxInstance; }
private:
	static C_StateManager*	m_pxInstance;
	class C_GameState**		m_pxGameStates;
	const int				m_uMaxGameStates;
};

#endif
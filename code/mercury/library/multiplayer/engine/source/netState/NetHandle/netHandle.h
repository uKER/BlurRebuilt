#ifndef _NET_HANDLE
#define _NET_HANDLE

// C_NetHandle
class C_NetHandle
{
public:
							C_NetHandle(int uNetId, int uGameStateId);
							C_NetHandle() : m_uGameStateId(-1), m_uNetId(-1) {;}
	int						GameStateId() const	{ return m_uGameStateId; }
	int						NetId() const { return m_uNetId; }
	class C_EntityState*	GetEntityState();
private:
	int m_uGameStateId;
	int m_uNetId;
};

#endif
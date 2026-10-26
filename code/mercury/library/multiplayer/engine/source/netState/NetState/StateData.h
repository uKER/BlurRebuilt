#ifndef _ENTITY_STATE
#define _ENTITY_STATE

#include "netstate/StateEnums.h"

// entity state
class C_StateData
{
private:
	C_StateData(const C_StateData& xCopy) : m_uCurrentState(xCopy.CurrentState()), m_uSize(xCopy.Size()){;}
public:
	C_StateData(const int uSize, int uCurrentState=E_EntityState_UnSet);
	C_StateData(const C_StateData& xCopy, int uCurrentState);
	~C_StateData();
	void				CurrentState(const int uStateNum)						{ m_uCurrentState = uStateNum; }
	int					CurrentState() const									{ return m_uCurrentState; }
	int					Size() const											{ return m_uSize; }
	char				Data(int i) const										{ assert(i < m_uSize); return m_pcData[i]; }
	void				Data(int i, char c)										{ assert(i < m_uSize); m_pcData[i] = c; }
	bool				IsStateSame(const C_StateData& rhs) const;		
	bool				operator==(const C_StateData& rhs) const;
	void				operator=(const C_StateData& rhs);
public:
	// data
	const int	m_uSize;
	char*		m_pcData;
	int			m_uCurrentState;
};

#endif
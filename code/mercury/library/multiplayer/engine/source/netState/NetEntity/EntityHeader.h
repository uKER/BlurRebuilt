#ifndef _ENTITY_HEADER
#define _ENTITY_HEADER

enum E_DataFlags
{
	E_DataFlag_Header					= (1 << 0),
	E_DataFlag_DeltaCompressed			= (1 << 1),
	E_DataFlag_FullState				= (1 << 2),
	E_DataFlag_CreationState			= (1 << 3),
	E_DataFlag_DestructionState			= (1 << 4),
	E_DataFlag_MigrationKeepAlive		= (1 << 5),
	E_DataFlag_MigrationClientReport	= (1 << 6),
};

class C_Header
{
public:
			C_Header();
			C_Header(const short int uEntityId, const int uTypeId, const char uAuthorityId, const char eStateType, const short int uSize);
	void	SetFlag(char uFlag)		{ m_uFlags |= uFlag; }
	bool	GetFlag(char uFlag)		{ return (m_uFlags & uFlag) ? true : false; }
	bool	Valid() const			{ return m_uEntityId >= 0 && m_uAuthorityId >= 0 && m_uAuthorityId <= 15 && m_uTypeId >= 0 && m_uNetStateType >= 0 && m_uSize > 0; }
	void	NetStateType(int uType) { m_uNetStateType = uType; }
	int		NetStateType() const;
	bool	Serialise(class netStream* stream) const;
	bool	Deserialise(class netStream* stream);
public:
	// TODO: seperate header into header / entity info structures
	short int	m_uEntityId;
	char		m_uAuthorityId;
	int			m_uTypeId;
	short int	m_uSize;
private:
	char		m_uNetStateType;
	char		m_uFlags;
};

#endif
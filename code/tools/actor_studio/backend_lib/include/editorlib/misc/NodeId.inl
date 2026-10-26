FORCE_INLINE C_NodeId::C_NodeId()
:
m_xtId(kInvalidXtId), m_childId(kNoChildId), m_idStringInvalid(true)
{
}

FORCE_INLINE C_NodeId::C_NodeId(const C_NodeId& rhs)
:
m_xtId(rhs.m_xtId), m_childId(rhs.m_childId), m_idStringInvalid(true)
{
}

FORCE_INLINE C_NodeId::C_NodeId(XtId xtId, U32 childId)
:
m_xtId(xtId), m_childId(childId), m_idStringInvalid(true)
{
}

FORCE_INLINE const C_NodeId& C_NodeId::operator=(const C_NodeId& rhs)
{
	m_xtId = rhs.m_xtId;
	m_childId = rhs.m_childId;
	m_idString = rhs.m_idString;
	m_idStringInvalid = rhs.m_idStringInvalid;
	return *this;
}

FORCE_INLINE bool C_NodeId::operator<(const C_NodeId& rhs) const
{
	if (m_xtId == rhs.m_xtId)
		return m_childId < rhs.m_childId;
	return m_xtId < rhs.m_xtId;
}

FORCE_INLINE bool C_NodeId::operator==(const C_NodeId& rhs) const
{
	if (m_xtId == rhs.m_xtId && m_childId == rhs.m_childId)
		return true;
	return false;
}

FORCE_INLINE bool C_NodeId::operator!=(const C_NodeId& rhs) const
{
	if (m_xtId == rhs.m_xtId && m_childId == rhs.m_childId)
		return false;
	return true;
}

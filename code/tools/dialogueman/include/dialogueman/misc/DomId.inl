FORCE_INLINE C_DomId::C_DomId()
:
m_packId(kInvalidXtId), m_recordId(kInvalidXtId), m_childId(-1)
{
}

FORCE_INLINE C_DomId::C_DomId(const C_DomId& rhs)
:
m_packId(rhs.m_packId), m_recordId(rhs.m_recordId), m_childId(rhs.m_childId)
{
}

FORCE_INLINE C_DomId::C_DomId(XtId packId, XtId recordId, S32 childId)
:
m_packId(packId), m_recordId(recordId), m_childId(childId)
{
}

FORCE_INLINE const C_DomId& C_DomId::operator=(const C_DomId& rhs)
{
	m_packId = rhs.m_packId;
	m_recordId = rhs.m_recordId;
	m_childId = rhs.m_childId;
	return *this;
}

FORCE_INLINE bool C_DomId::operator<(const C_DomId& rhs) const
{
	return (m_packId + m_recordId + m_childId) < (rhs.m_packId + rhs.m_recordId + rhs.m_childId);
}

FORCE_INLINE bool C_DomId::operator==(const C_DomId& rhs) const
{
	return memcmp(this, &rhs, sizeof(this)) == 0;
}

FORCE_INLINE bool C_DomId::operator!=(const C_DomId& rhs) const
{
	return memcmp(this, &rhs, sizeof(this)) != 0;
}

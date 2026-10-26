C_UserShape3DVariants::C_UserShape3DVariants()
{
}

const Util::C_String& C_UserShape3DVariants::GetName() const
{
	return m_name;
}

const C_UserShape3DVariants::Shape3DArray& C_UserShape3DVariants::GetShape3DArray() const
{
	return m_userShape3DData;
}

U32 C_UserShape3DVariants::GetNumVertices() const
{
	return m_numVertices;
}

INLINE F32 C_UserShape3DVariants::GetScale() const
{
	return m_scale.z;
}

U32 C_UserShape3DVariants::GetNumPrimitives() const
{
	return m_numVertices / 3;
}

template<typename S>
void C_UserShape3DVariants::Deserialise(S& s)
{
	s >> m_name;
	s >> m_numVertices;
	s >> m_scale.v[2];
	U32 numRecords;
	s >> numRecords;
	m_userShape3DData.Resize(numRecords);
	for (U32 i = 0; i < (U32)m_userShape3DData.Size(); i++)
	{
		m_userShape3DData[i].SetNumVertices(m_numVertices);
		s >> m_userShape3DData[i];
		m_userShape3DData[i].Normalise();
	}
}

void C_UserShape3DVariants::SetName(const Util::C_String& name)
{
	m_name = name;
}

#ifdef PLATFORM_PC
void C_UserShape3DVariants::SetScale(F32 scale)
{
	m_scale = MathsStorage::Float4A16(scale, scale, scale, scale);
}
#endif

void C_UserShape3DVariants::SetNumVertices(U32 numVertices)
{
	m_numVertices = numVertices;
}

C_UserShape3DVariants::Shape3DArray& C_UserShape3DVariants::GetShape3DArray()
{
	return m_userShape3DData;
}

template<typename S>
void C_UserShape3DVariants::Serialise(S& s)
{
	s << m_name;
	s << m_numVertices;
	s << m_scale.v[2];
	s << (U32)m_userShape3DData.Size();
	for (U32 i = 0; i < (U32)m_userShape3DData.Size(); i++)
		s << m_userShape3DData[i];
}




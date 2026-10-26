template<typename S>
void C_UserShape3DData::C_Vertex::Serialise(S& s)
{
	s << position.v[0];	s << position.v[1];	s << position.v[2];
	s << normal.v[0];	s << normal.v[1];	s << normal.v[2];
	s << uv0.v[0];		s << uv0.v[1];
	s << tan0.v[0];		s << tan0.v[1];		s << tan0.v[2];
	s << bin0.v[0];		s << bin0.v[1];		s << bin0.v[2];
}

void C_UserShape3DData::SetNumVertices(U32 numVertices)
{
	m_vertices.Resize(numVertices);
}

void C_UserShape3DData::Normalise()
{
	for(S32 i = 0; i < m_vertices.Size(); ++i)
	{
		Maths::HVec3 vNormal(Maths::Load<Maths::HVec3>(&m_vertices[i].normal));
		vNormal = Maths::Normalise(vNormal);
		Maths::Store(vNormal, &m_vertices[i].normal);
	}
}

C_UserShape3DData::VertexArray& C_UserShape3DData::GetVertexArray()
{
	return m_vertices;
}

const C_UserShape3DData::VertexArray& C_UserShape3DData::GetVertexArray() const
{
	return m_vertices;
}

template<typename S>
void C_UserShape3DData::Serialise(S& s)
{
	for (U32 i = 0; i < (U32)m_vertices.Size(); i++)
		s << m_vertices[i];
}

template<typename S>
void C_UserShape3DData::Deserialise(S& s)
{
	s.Read(&m_vertices[0], m_vertices.Size() * sizeof(C_Vertex));
}


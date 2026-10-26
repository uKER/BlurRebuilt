
inline C_RenderContext::C_RenderContext()
: m_pGeometryCluster(0)
, m_Batch(0)
, m_MaterialIndex(-1)
{
	memset(&m_Flags, 0, sizeof(m_Flags));
}

inline C_RenderContext::C_RenderContext(const C_GeometryCluster* pCluster, U32 batch)
: m_pGeometryCluster(pCluster)
, m_Batch(batch)
, m_MaterialIndex(-1)
{
	memset(&m_Flags, 0, sizeof(m_Flags));
}

inline C_RenderContext::C_RenderContext(const C_RenderContext& rc)
{
	operator=(rc);
}

inline C_RenderContext::~C_RenderContext()
{
}

inline C_RenderContext& C_RenderContext::operator=(const C_RenderContext& rc)
{
	m_VertexShader = rc.m_VertexShader;
	m_PixelShader = rc.m_PixelShader;
	m_Constants = rc.m_Constants;
	m_pGeometryCluster = rc.m_pGeometryCluster;
	m_Batch = rc.m_Batch;
	m_MaterialIndex = rc.m_MaterialIndex;
	m_Flags = rc.m_Flags;

	return *this;
}

inline void C_RenderContext::Clear()
{
	m_Constants.Clear();
}

inline const C_GeometryCluster* C_RenderContext::GetGeometryCluster() const
{
	return m_pGeometryCluster;
}

inline U32 C_RenderContext::GetBatch() const
{
	return m_Batch;
}

inline S16 C_RenderContext::GetRemappedMaterialIndex() const
{
	return m_MaterialIndex;
}

inline void C_RenderContext::SetRemappedMaterialIndex(S16 mat)
{
	m_MaterialIndex = mat;
}

inline U32 C_RenderContext::GetNumShaderConstants() const
{
	return m_Constants.Size();
}

inline void C_RenderContext::AddShaderConstant(const C_ShaderConstant& c)
{
	//replace existing constant if present
	for (S32 i=0; i<m_Constants.Size(); ++i)
	{
		if (m_Constants[i].m_shaderType == c.m_shaderType &&
			m_Constants[i].m_RegisterStart == c.m_RegisterStart)
		{
			m_Constants[i] = c;
			return;
		}
	}
	m_Constants.PushBack(c);
}

inline C_ShaderConstant& C_RenderContext::GetShaderConstant(U32 index)
{
	return m_Constants[index];
}

inline const C_ShaderConstant& C_RenderContext::GetShaderConstant(U32 index) const
{
	return m_Constants[index];
}

inline C_ResourcePtr<Display::C_VertexShader> C_RenderContext::GetVertexShader() const
{
	return m_VertexShader;
}
inline void C_RenderContext::SetVertexShader(C_ResourcePtr<Display::C_VertexShader>& vs)
{
	m_VertexShader = vs;
}

inline C_ResourcePtr<Display::C_PixelShader> C_RenderContext::GetPixelShader() const
{
	return m_PixelShader;
}

inline void C_RenderContext::SetPixelShader(C_ResourcePtr<Display::C_PixelShader>& ps)
{
	m_PixelShader = ps;
}

inline C_RenderContext::Flags& C_RenderContext::GetFlags()
{
	return m_Flags;
}

inline const C_RenderContext::Flags& C_RenderContext::GetFlags() const
{
	return m_Flags;
}

inline void C_RenderContext::Apply() const
{
	Display::Device()->SetVertexShader(m_VertexShader);
	Display::Device()->SetPixelShader(m_PixelShader);

	for (S32 i=0; i<m_Constants.Size(); ++i)
		m_Constants[i].Apply();
}
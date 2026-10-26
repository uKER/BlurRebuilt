
inline C_ShaderConstant::C_ShaderConstant()
: m_shaderType(C_ShaderConstant::E_ShaderType_Unknown), m_RegisterStart(0), m_IntVal(0), m_RegisterCount(0), m_constantType(C_ShaderConstant::E_Type_Unknown)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, bool val)
: m_shaderType(type), m_RegisterStart(regStart), m_BoolVal(val), m_RegisterCount(1), m_constantType(C_ShaderConstant::E_Type_Bool)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, const bool* ptr, U32 regCount)
: m_shaderType(type), m_RegisterStart(regStart), m_BoolPtr(ptr), m_RegisterCount(regCount), m_constantType(C_ShaderConstant::E_Type_BoolPtr)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, S32 val)
: m_shaderType(type), m_RegisterStart(regStart), m_IntVal(val), m_RegisterCount(1), m_constantType(C_ShaderConstant::E_Type_Int)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, const S32* ptr, U32 regCount)
: m_shaderType(type), m_RegisterStart(regStart), m_IntPtr(ptr), m_RegisterCount(regCount), m_constantType(C_ShaderConstant::E_Type_IntPtr)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, F32 val)
: m_shaderType(type), m_RegisterStart(regStart), m_FloatVal(val), m_RegisterCount(1), m_constantType(C_ShaderConstant::E_Type_Float)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, const F32* ptr, U32 regCount)
: m_shaderType(type), m_RegisterStart(regStart), m_FloatPtr(ptr), m_RegisterCount(regCount), m_constantType(C_ShaderConstant::E_Type_FloatPtr)
{
}

inline C_ShaderConstant::C_ShaderConstant(E_ShaderType type, U32 regStart, const Maths::HMat44 *ptr)
: m_shaderType(type), m_RegisterStart(regStart), m_MatrixPtr(ptr), m_RegisterCount(4), m_constantType(C_ShaderConstant::E_Type_MatrixPtr)
{
}

inline C_ShaderConstant::E_ShaderType C_ShaderConstant::GetShaderType()
{
	return m_shaderType;
}

inline C_ShaderConstant::E_ConstantType C_ShaderConstant::GetConstantType()
{
	return m_constantType;
}

inline void C_ShaderConstant::Apply() const
{
	switch (m_constantType)
	{
	case E_Type_Bool:
		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantB(m_RegisterStart, (const BOOL*)&m_BoolVal, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantB(m_RegisterStart, (const BOOL*)&m_BoolVal, m_RegisterCount);
		break;
	case E_Type_BoolPtr:
		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantB(m_RegisterStart, (const BOOL*)m_BoolPtr, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantB(m_RegisterStart, (const BOOL*)m_BoolPtr, m_RegisterCount);
		break;
	case E_Type_Int:
		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantI(m_RegisterStart, &m_IntVal, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantI(m_RegisterStart, &m_IntVal, m_RegisterCount);
		break;
	case E_Type_IntPtr:
		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantI(m_RegisterStart, m_IntPtr, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantI(m_RegisterStart, m_IntPtr, m_RegisterCount);
		break;
	case E_Type_Float:
		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantF(m_RegisterStart, &m_FloatVal, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantF(m_RegisterStart, &m_FloatVal, m_RegisterCount);
		break;
	case E_Type_FloatPtr:
		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantF(m_RegisterStart, m_FloatPtr, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantF(m_RegisterStart, m_FloatPtr, m_RegisterCount);
		break;
	case E_Type_MatrixPtr:
	{
		Display::C_Matrix colMatrix;
		Maths::Store(*m_MatrixPtr, &colMatrix);

		(m_shaderType == E_ShaderType_Vertex)
			? Display::Device()->SetVertexShaderConstantF(m_RegisterStart, (F32*)&colMatrix, m_RegisterCount)
			: Display::Device()->SetPixelShaderConstantF(m_RegisterStart, (F32*)&colMatrix, m_RegisterCount);
		break;
	}
	default:
		break;
	};
}
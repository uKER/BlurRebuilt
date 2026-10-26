#ifndef AFTK_RENDER_SHADER_VARIABLE_H
#define AFTK_RENDER_SHADER_VARIABLE_H

#ifdef _MSC_VER
#pragma once
#endif


namespace AftkRender
{
	class C_ShaderConstant
	{
	public:
		enum E_ShaderType
		{
			E_ShaderType_Vertex,
			E_ShaderType_Pixel,
			E_ShaderType_Unknown
		};

		enum E_ConstantType
		{
			E_Type_Bool,
			E_Type_BoolPtr,
			E_Type_Int,
			E_Type_IntPtr,
			E_Type_Float,
			E_Type_FloatPtr,
			E_Type_MatrixPtr,
			E_Type_Unknown
		};

	C_ShaderConstant();
	C_ShaderConstant(E_ShaderType type, U32 regStart, bool val);
	C_ShaderConstant(E_ShaderType type, U32 regStart, const bool* ptr, U32 regCount);

	C_ShaderConstant(E_ShaderType type, U32 regStart, S32 val);
	C_ShaderConstant(E_ShaderType type, U32 regStart, const S32* ptr, U32 regCount);

	C_ShaderConstant(E_ShaderType type, U32 regStart, F32 val);
	C_ShaderConstant(E_ShaderType type, U32 regStart, const F32* ptr, U32 regCount);

	C_ShaderConstant(E_ShaderType type, U32 regStart, const Maths::HMat44* ptr);

	E_ShaderType GetShaderType();
	E_ConstantType GetConstantType();

	void Apply() const;

	public:
		E_ShaderType	m_shaderType;
		E_ConstantType	m_constantType;
		U32				m_RegisterStart;
		U32				m_RegisterCount;

		union
		{
			BOOL	m_BoolVal;
			S32		m_IntVal;
			F32		m_FloatVal;

			const bool*	m_BoolPtr;
			const S32*	m_IntPtr;
			const F32*	m_FloatPtr;
			const Maths::HMat44* m_MatrixPtr;
		};
	};

	#include "ShaderConstant.inl"
}

#endif

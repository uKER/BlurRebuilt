#ifndef AFTK_RENDER_CONTEXT_H
#define AFTK_RENDER_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"

namespace AftkRender
{
	class C_GeometryCluster;
	class C_ShaderConstant;

	class C_RenderContext
	{
	public:
		struct Flags
		{
			unsigned isBillboard	:1;
			unsigned isSkinned		:1;
			unsigned isTextured		:1;
		};

	public:
		C_RenderContext();

		C_RenderContext(const C_GeometryCluster* pCluster, U32 batch);
		~C_RenderContext();

		C_RenderContext(const C_RenderContext& r);

		C_RenderContext& operator=(const C_RenderContext&);

		void Clear();
		const C_GeometryCluster* GetGeometryCluster() const;
		U32 GetBatch() const;

		S16 GetRemappedMaterialIndex() const;
		void SetRemappedMaterialIndex(S16 mat);

		U32 GetNumShaderConstants() const;
		void AddShaderConstant(const C_ShaderConstant& v);
		C_ShaderConstant& GetShaderConstant(U32 index);
		const C_ShaderConstant& GetShaderConstant(U32 index) const;

		C_ResourcePtr<Display::C_VertexShader> GetVertexShader() const;
		void SetVertexShader(C_ResourcePtr<Display::C_VertexShader>& vs);

		C_ResourcePtr<Display::C_PixelShader> GetPixelShader() const;
		void SetPixelShader(C_ResourcePtr<Display::C_PixelShader>& ps);

		Flags& GetFlags();
		const Flags& GetFlags() const;

		void Apply() const;

	private:
		C_ResourcePtr<Display::C_VertexShader>	m_VertexShader;
		C_ResourcePtr<Display::C_PixelShader>	m_PixelShader;
		Ut::UtVector< Ut::StackVector<C_ShaderConstant, 5> > m_Constants;

		const C_GeometryCluster	*m_pGeometryCluster;
		U32					m_Batch;
		S16					m_MaterialIndex;
		Flags				m_Flags;
	};

	#include "RenderContext.inl"

}

#endif
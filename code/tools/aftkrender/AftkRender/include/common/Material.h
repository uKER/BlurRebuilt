#ifndef AFTK_RENDER_MATERIAL_H
#define AFTK_RENDER_MATERIAL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "common/ResourcePtr.h"

namespace AftkRender
{
	class C_TextureState;

	class C_Material
	{
	public:

		enum E_TextureType
		{
			E_TextureType_Unknown,
			E_TextureType_2D,
			E_TextureType_3D,
			E_TextureType_Cube,
		};

		struct TextureInfo
		{
			Util::C_String m_FileName;
			E_TextureType m_Type;

			TextureInfo(const char* pName="", E_TextureType type=E_TextureType_Unknown)
				: m_FileName(pName), m_Type(type)
			{ }

			TextureInfo& operator=(const TextureInfo& r)
			{
				m_FileName	= r.m_FileName;
				m_Type		= r.m_Type;
				return *this;
			}
		};

		C_Material();
		~C_Material();

		TextureInfo& GetTextureInfo(U32 i);
		const TextureInfo& GetTextureInfo(U32 i) const;
		void AddTextureInfo(const TextureInfo& info);
		U32 GetNumTextures() const;

		C_TextureState& GetTextureState();
		const C_TextureState& GetTextureState() const;
		void SetTextureState(const C_TextureState& textureState);

		Display::E_BlendMode GetBlendMode() const;
		void SetBlendMode(Display::E_BlendMode mode);

		Display::E_CullMode GetCullMode() const;
		void SetCullMode(Display::E_CullMode mode);

		S32 GetMaterialTypeId() const;
		void SetMaterialTypeId(S32 id);

		Ut::UtVector< Ut::DynamicVector<TextureInfo> >& GetTextureInfoArray();
		const Ut::UtVector< Ut::DynamicVector<TextureInfo> >& GetTextureInfoArray() const;

		void Enable(U32 pass = 0) const;
		void Disable() const;

		bool LoadTextures(C_TextureManager& manager, U32 clampSize = 0);
		bool TexturesLoaded() const;

		//Hash function based upon texture names so that we can check for duplicates
		U32 GetHash() const;

	private:
		C_TextureState							m_TextureState;

		//TextureInfo array so that textures can be loaded anytime after initialisation
		Ut::UtVector< Ut::DynamicVector<TextureInfo> >	m_TextureInfo;

		bool					m_TexturesLoaded;
		Display::E_BlendMode	m_BlendMode;
		Display::E_CullMode		m_CullMode;

		S32						m_MaterialTypeId;
	};
}

#endif
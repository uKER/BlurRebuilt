#ifndef AFTK_RENDER_MATERIALS_INCLUDE
#define AFTK_RENDER_MATERIALS_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
}

namespace AftkRender
{
	class C_RenderMaterial
	{
	public:
		struct TextureInfo
		{
			enum E_TextureUsage
			{
				E_TextureUsage_Untextured=-1,
				E_TextureUsage_DiffuseMap,
				E_TextureUsage_NormalMap,
				E_TextureUsage_LightMap,

				E_TextureUsage_Count
			};

			TextureInfo(
				const Util::C_String name = "",
				E_TextureUsage usage = E_TextureUsage_LightMap)
			:
				m_FileName(name),
				m_Usage(usage)
			{
			}

			Util::C_String m_FileName;
			E_TextureUsage m_Usage;
		};

		C_RenderMaterial(C_SceneRenderer* pSceneRenderer);
		~C_RenderMaterial();

		void Clear();

		Aftk::C_Status BuildFromAftkMaterial(
			const Aftk::C_MaterialNode* pAftkMaterial);

		void OnEnable() const;
		void OnDisable() const;

		const Ut::UtVector< Ut::DynamicVector<TextureInfo> >& GetTextureInfoList() const;
		const Ut::UtVector< Ut::DynamicVector<TextureInfo> >& GetTextureInfoList();

		void AddTextureInfo(
			TextureInfo& info);

		bool IsTextureLoaded(U32 index) const;

		bool HasAlpha() const;
		bool HasNormalMap() const;

		U32 GetNumTextures() const;

		//Used for checking for duplicate materials
		U32 GetHash() const;

		Aftk::C_Status loadTextures2D(bool mipmap=true);
		
		Display::C_Texture2D& GetTexture2D(U32 index);

		const Display::C_Texture2D& GetTexture2D(U32 index) const;
		
	private:
		C_SceneRenderer* m_pSceneRenderer;

		Ut::UtVector< Ut::DynamicVector<TextureInfo> > m_TextureInfo;
		Ut::UtVector< Ut::DynamicVector<Display::C_Texture2D> > m_Textures2D;
	};
}
#endif
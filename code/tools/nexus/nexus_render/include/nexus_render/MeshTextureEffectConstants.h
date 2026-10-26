#pragma once

namespace NexusRender
{
	class D3DShaderManager;

	class MeshTextureEffectConstants : public EffectConstants
	{
	public:
		MeshTextureEffectConstants();

		MeshTextureEffectConstants(const MeshTextureEffectConstants& rhs);

		virtual ~MeshTextureEffectConstants();

		const MeshTextureEffectConstants& operator=(const MeshTextureEffectConstants& rhs);

		Tu::Status Build(const Aftk::C_MaterialNode* pAftkMaterialNode);

		virtual void GetTextureInfo(Ut::UtVector<Ut::DynamicVector<const EffectConstants::TextureInfo*> >& textureInfo) const;

		virtual void LoadTextures(SceneRenderer& sceneRenderer);

		Tu::Status LoadCropSheetTexture(SceneRenderer& sceneRenderer);

		virtual EffectWrapper::EEffectType GetEffectType() const;

		IDirect3DTexture9* GetColourTexturePtr();

		IDirect3DTexture9* GetCropSheetTexturePtr();

		void SetColourTexturePtr(IDirect3DTexture9* pTexture);

		virtual U32 GetHash() const;

		virtual bool operator==(const EffectConstants& effectConstants) const;
		
	protected:
		void OneTimeInit();

		IDirect3DTexture9*	m_pCropTexture;
		IDirect3DTexture9*	m_pColourTexture;
		S32					m_cropSheetColourTextureIndex;

		EffectConstants::TextureInfo m_colourTextureInfo;

		friend class MeshTextureEffectWrapper;
		friend class MeshLightmapEffectWrapper;
	};
}




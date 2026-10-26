#pragma once

namespace NexusRender
{
	class D3DShaderManager;
	
	class MeshLightmapEffectConstants : public MeshTextureEffectConstants
	{
	public:
		MeshLightmapEffectConstants();

		MeshLightmapEffectConstants(const MeshLightmapEffectConstants& rhs);

		~MeshLightmapEffectConstants();

		const MeshLightmapEffectConstants& operator=(const MeshLightmapEffectConstants& rhs);

		Tu::Status Build(
			const Aftk::C_MaterialNode* pAftkMaterialNode);

		void GetTextureInfo(Ut::UtVector<Ut::DynamicVector<const EffectConstants::TextureInfo*> >& textureInfo) const;

		void LoadTextures(SceneRenderer& sceneRenderer);

		__forceinline EffectWrapper::EEffectType GetEffectType() const;

		IDirect3DTexture9* GetLightmapTexturePtr();

		U32 GetHash() const;

		bool operator==(const EffectConstants& effectConstants) const;
		
	private:
		void OneTimeInit();

		IDirect3DTexture9*				m_pLightmapTexture;
		EffectConstants::TextureInfo	m_lightmapTextureInfo;
		friend class MeshLightmapEffectWrapper;
	};

	EffectWrapper::EEffectType MeshLightmapEffectConstants::GetEffectType() const
	{
		return EffectWrapper::EFFECT_TYPE_MESH_LIGHTMAP;
	}
}




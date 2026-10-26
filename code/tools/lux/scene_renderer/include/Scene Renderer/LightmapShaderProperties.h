#ifndef LIGHTMAP_SHADER_PROPORTIES_H
#define LIGHTMAP_SHADER_PROPORTIES_H

namespace SRD
{
	class LightmapShaderProperties : public ShaderProperties
	{
	public:
		LightmapShaderProperties();
		LightmapShaderProperties(const LightmapShaderProperties& rhs);
		virtual ~LightmapShaderProperties();

		virtual ShaderPropertiesTypeEnum GetType() const;

		IDirect3DTexture9* GetDiffuseTexturePtr();
		IDirect3DTexture9* GetLightmapTexturePtr();

		void SetDiffuseTexturePtr(IDirect3DTexture9* pTexture);
		void SetLightmapTexturePtr(IDirect3DTexture9* pTexture);

	protected:
		IDirect3DTexture9*	m_pDiffuseTexture;
		IDirect3DTexture9*	m_pLightmapTexture;
	};
}

#endif


#ifndef RENDER_POSTPROCESS_SUNRAYS_H
#define RENDER_POSTPROCESS_SUNRAYS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/Map.h>
#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>

namespace PostProcess
{

class C_SunRays : public I_PostProcess
{
public:

	C_SunRays(C_Manager& manager);
	~C_SunRays();

	bool Create();
	void Destroy();
	bool CreateVolatile();
	void DestroyVolatile();
	void Update() {}

	const Display::C_Texture* GetTexture() const;

	void SetTexture(const Display::C_Texture* pTexture) const;

	void SetSunPosition(const Display::C_Float2& vSunPosition) const;
	void SetSunSize(const Display::C_Float2& vSunSize) const;
	void SetSunIntensity(const F32 fSunIntensity) const;

	void SetGammaEnable(const bool bGamma) const;
	
	void Display() const;
	
private:

	void SetShader() const;
	void SetParameters() const;

private:

	RenderTextures							m_SunRayTextures;
	mutable Btl::Map<S32, S32>				m_SunRayMapping;

	Display::C_PixelShader					m_PixelShader;
	Display::C_PixelShader					m_PixelShaderGamma;

	mutable Display::C_PixelShaderParameter	m_Settings1;
	mutable Display::C_PixelShaderParameter	m_Settings2;

	mutable const Display::C_Texture*		m_pTexture;

	mutable Display::C_Float2				m_vSunPosition;
	mutable Display::C_Float2				m_vSunSize;
	mutable F32								m_fSunIntensity;

	mutable bool							m_bGamma;

};

INLINE C_SunRays::C_SunRays(C_Manager& manager) :
	I_PostProcess( manager ),
	m_bGamma(false),
	m_pTexture(0),
	m_vSunPosition(0.5f, 0.5f),
	m_vSunSize(0.5f, 0.5f),
	m_fSunIntensity(1.0f)
{
}

INLINE C_SunRays::~C_SunRays()
{
}

INLINE const Display::C_Texture* C_SunRays::GetTexture() const
{
	const S32 iViewIndex = m_Manager.GetViewIndex();
	const S32 iTextureIndex = m_SunRayMapping[iViewIndex];
	return &m_SunRayTextures[iTextureIndex]->GetTexture();
}

INLINE void C_SunRays::SetTexture(const Display::C_Texture* pTexture) const
{
	m_pTexture = pTexture;
}

INLINE void C_SunRays::SetSunPosition(const Display::C_Float2& vSunPosition) const
{
	m_vSunPosition = vSunPosition;
}

INLINE void C_SunRays::SetSunSize(const Display::C_Float2& vSunSize) const
{
	m_vSunSize = vSunSize;
}

INLINE void C_SunRays::SetSunIntensity(const F32 fSunIntensity) const
{
	m_fSunIntensity = fSunIntensity;
}

INLINE void C_SunRays::SetGammaEnable(const bool bGamma) const
{
	if(bGamma != m_bGamma)
	{
		m_bGamma = bGamma;
		SetShader();
		SetParameters();
	}
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_SUNRAYS_H

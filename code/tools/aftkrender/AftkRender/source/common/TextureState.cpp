#include "AftkRenderPch.h"
#include "common/TextureState.h"

using namespace AftkRender;

C_TextureState::C_TextureState()
{
	memset(m_Textures, 0, sizeof(Display::C_RefTexture2D*)*Display::E_Sampler_Count);
	for (U32 i=0; i<Display::E_Sampler_Count; ++i)
	{
		m_SamplerStates[i].m_AddressU	= Display::E_TextureAddress_Wrap;
		m_SamplerStates[i].m_AddressV	= Display::E_TextureAddress_Wrap;
		m_SamplerStates[i].m_AddressW	= Display::E_TextureAddress_Wrap;
		m_SamplerStates[i].m_FilterMag	= Display::E_TextureFilter_Linear;
		m_SamplerStates[i].m_FilterMin	= Display::E_TextureFilter_Linear;
		m_SamplerStates[i].m_FilterMip	= Display::E_TextureFilter_Point;
		m_SamplerStates[i].m_Gamma		= Display::E_Bool_False;
	}
}

C_TextureState::~C_TextureState()
{
}

void C_TextureState::Enable() const
{
	for (U32 i=0; i<(U32)Display::E_Sampler_Count; ++i)
	{
		if (m_Textures[i])
		{
			Display::Device()->SetTexture(i, m_Textures[i]);
			Display::Device()->SetTextureAddressUVW(i, m_SamplerStates[i].m_AddressU, m_SamplerStates[i].m_AddressV, m_SamplerStates[i].m_AddressW);
			Display::Device()->SetTextureFilterMagMinMip(i, m_SamplerStates[i].m_FilterMag,  m_SamplerStates[i].m_FilterMin,  m_SamplerStates[i].m_FilterMip);
			Display::Device()->SetTextureGamma(i, m_SamplerStates[i].m_Gamma);
		}
	}
}

void C_TextureState::Disable() const
{
}

C_TextureState& C_TextureState::operator =(const C_TextureState& r)
{
	m_Texture2DResources.Clear();
	m_Texture3DResources.Clear();
	m_TextureCubeResources.Clear();
	memset(m_Textures, 0, sizeof(Display::C_RefTexture*)*Display::E_Sampler_Count);
	
	for (U32 i=0; i<Display::E_Sampler_Count; ++i)
	{	
		m_SamplerStates[i] = r.m_SamplerStates[i];
		m_Textures[i] = r.m_Textures[i];
		
		for (S32 j=0; j<r.m_Texture2DResources.Size(); ++j)
			m_Texture2DResources.PushBack(r.m_Texture2DResources[j]);

		for (S32 j=0; j<r.m_Texture3DResources.Size(); ++j)
			m_Texture3DResources.PushBack(r.m_Texture3DResources[j]);

		for (S32 j=0; j<r.m_TextureCubeResources.Size(); ++j)
			m_TextureCubeResources.PushBack(r.m_TextureCubeResources[j]);	
	}

	return *this;
}

bool C_TextureState::operator ==(const C_TextureState & r)
{
	bool equal = true;
	for (U32 i=0; i<Display::E_Sampler_Count; ++i)
		equal = equal && (m_Textures[i] == r.m_Textures[i]);

	return equal;
}

bool C_TextureState::operator !=(const C_TextureState & r)
{
	bool equal = true;
	for (U32 i=0; i<Display::E_Sampler_Count; ++i)
		equal = equal && (m_Textures[i] == r.m_Textures[i]);

	return !equal;
}

void C_TextureState::Set2D(U32 sampler, 
	C_ResourcePtr<Display::C_RefTexture2D> pResource, 
	const Display::C_SamplerBase* state)
{
	assert(sampler<(U32)Display::E_Sampler_Count && "Invalid texture sampler");
	ReleaseResource(m_Textures[sampler]);
	m_Texture2DResources.PushBack(pResource);
	m_Textures[sampler] = pResource;
	if (state)
		m_SamplerStates[sampler] = *state;
}

void C_TextureState::Set3D(U32 sampler, 
	C_ResourcePtr<Display::C_RefTexture3D> pResource, 
	const Display::C_SamplerBase* state)
{
	assert(sampler<(U32)Display::E_Sampler_Count && "Invalid texture sampler");
	ReleaseResource(m_Textures[sampler]);
	m_Texture3DResources.PushBack(pResource);
	m_Textures[sampler] = pResource;
	if (state)
		m_SamplerStates[sampler] = *state;
}

void C_TextureState::SetCube(U32 sampler, 
	C_ResourcePtr<Display::C_RefTextureCube> pResource, 
	const Display::C_SamplerBase* state)
{
	assert(sampler<(U32)Display::E_Sampler_Count && "Invalid texture sampler");
	ReleaseResource(m_Textures[sampler]);
	m_TextureCubeResources.PushBack(pResource);
	m_Textures[sampler] = pResource;
	if (state)
		m_SamplerStates[sampler] = *state;
}

void C_TextureState::ReleaseResource(const Display::C_RefTexture* pTexture)
{
	for (S32 i=0; i<m_Texture2DResources.Size(); ++i)
	{
		if (m_Texture2DResources[i] == pTexture)
		{
			m_Texture2DResources.EraseNonOrdered(i);
			return;
		}
	}

	for (S32 i=0; i<m_Texture3DResources.Size(); ++i)
	{
		if (m_Texture3DResources[i] == pTexture)
		{
			m_Texture3DResources.EraseNonOrdered(i);
			return;
		}
	}

	for (S32 i=0; i<m_TextureCubeResources.Size(); ++i)
	{
		if (m_TextureCubeResources[i] == pTexture)
		{
			m_TextureCubeResources.EraseNonOrdered(i);
			return;
		}
	}
}


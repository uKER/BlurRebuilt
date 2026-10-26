#ifndef AFTK_RENDER_TEXTURE_STATE_H
#define AFTK_RENDER_TEXTURE_STATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "common/resourcePtr.h"

namespace AftkRender
{
	class C_TextureManager;

	class C_TextureState
	{
	public:
		C_TextureState();
		~C_TextureState();

		void Enable() const;
		void Disable() const;

		C_TextureState& operator=(const C_TextureState& r);

		bool operator==(const C_TextureState& r);
		bool operator!=(const C_TextureState& r);

		void Set2D(U32 sampler, 
			C_ResourcePtr<Display::C_RefTexture2D> pResource, 
			const Display::C_SamplerBase* state=0);

		void Set3D(U32 sampler, 
			C_ResourcePtr<Display::C_RefTexture3D> pResource, 
			const Display::C_SamplerBase* state=0);

		void SetCube(U32 sampler, 
			C_ResourcePtr<Display::C_RefTextureCube> pResource, 
			const Display::C_SamplerBase* state=0);

	private:
		void ReleaseResource(const Display::C_RefTexture* pTexture);

		Display::C_RefTexture*		m_Textures[Display::E_Sampler_Count];
		Display::C_SamplerBase		m_SamplerStates[Display::E_Sampler_Count];

		//store texture resource pointers in use
		Ut::UtVector< Ut::DynamicVector< C_ResourcePtr<Display::C_RefTexture2D> > > m_Texture2DResources;
		Ut::UtVector< Ut::DynamicVector< C_ResourcePtr<Display::C_RefTexture3D> > > m_Texture3DResources;
		Ut::UtVector< Ut::DynamicVector< C_ResourcePtr<Display::C_RefTextureCube> > > m_TextureCubeResources;
		
	};

}

#endif
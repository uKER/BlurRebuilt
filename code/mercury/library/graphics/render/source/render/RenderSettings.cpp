#include "stdafx.h"
#include "Render/Render/RenderSettings.h"
#include <system/debuglog.h>

namespace Render
{

bool	C_RenderSettings::m_ColourBufferEnabled				= true;
bool	C_RenderSettings::m_ColourBufferSupported			= true;

bool	C_RenderSettings::m_DepthStencilBufferEnabled		= true;
bool	C_RenderSettings::m_DepthStencilBufferSupported		= true;

bool	C_RenderSettings::m_HDREnabled						= true;
bool	C_RenderSettings::m_HDRSupported					= true;

bool	C_RenderSettings::m_DOFEnabled						= true;
bool	C_RenderSettings::m_DOFSupported					= true;

bool 	C_RenderSettings::m_ShadowMapEnabled				= true;
bool 	C_RenderSettings::m_ShadowMapSupported				= true;

bool	C_RenderSettings::m_OcclusionQueriesEnabled			= true;
bool	C_RenderSettings::m_OcclusionQueriesSupported		= true;

F32		C_RenderSettings::m_ShadowMapCullingDistance		= 40.0f;

C_RenderSettings::C_RenderSettings()
{
}

C_RenderSettings::~C_RenderSettings()
{
}

void C_RenderSettings::ColourBufferEnabled(const bool colourBufferEnabled)
{
	m_ColourBufferEnabled = colourBufferEnabled;
	if(!m_ColourBufferSupported)
	{
		m_ColourBufferEnabled = false;
	}
}

void C_RenderSettings::DepthStencilBufferEnabled(const bool depthStencilBufferEnabled)
{
	m_DepthStencilBufferEnabled = depthStencilBufferEnabled;
	if(!m_DepthStencilBufferSupported)
	{
		m_DepthStencilBufferEnabled = false;
	}
}

void C_RenderSettings::HDREnabled(const bool hdrEnabled)
{
	m_HDREnabled = hdrEnabled;
	if(!m_HDRSupported)
	{
		m_HDREnabled = false;
	}
}

void C_RenderSettings::DOFEnabled(const bool hdrEnabled)
{
	m_DOFEnabled = hdrEnabled;
	if(!m_DOFSupported)
	{
		m_DOFEnabled = false;
	}
}

void C_RenderSettings::ShadowMapEnabled(const bool shadowMapEnabled)
{
	m_ShadowMapEnabled = shadowMapEnabled;
	if(!m_ShadowMapSupported)
	{
		m_ShadowMapEnabled = false;
	}
}

void C_RenderSettings::OcclusionQueriesEnabled(const bool occlusionQueriesEnabled)
{
	m_OcclusionQueriesEnabled = occlusionQueriesEnabled;
	if(!m_OcclusionQueriesSupported)
	{
		m_OcclusionQueriesEnabled = false;
	}
}

void C_RenderSettings::ShadowMapCullingDistance(const F32 shadowMapCullingDistance)
{
	m_ShadowMapCullingDistance = shadowMapCullingDistance;
}

void C_RenderSettings::InitialiseSettings()
{
#if defined(PLATFORM_PC)
	if(Display::Device()->GetShaderProfile() == Display::E_ShaderProfile_2_0)
	{
		m_ColourBufferSupported						= false;
		m_DepthStencilBufferSupported				= false;
		m_HDRSupported								= false;
		m_DOFSupported								= false;
		m_ShadowMapSupported						= false;
	}
	else
	{
		if(Display::Device()->SupportForRenderTarget(Display::E_ResourceType_Texture2D, Display::E_Format_A8R8G8B8))
		{
			m_ColourBufferSupported					= true;

			U32 formatSupport = 0;
			const bool nativeBlit = SUCCEEDED(Display::Device()->GetD3DDevice()->CheckFormatSupport(
				DXGI_FORMAT_B8G8R8A8_UNORM, &formatSupport)) &&
				(formatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) &&
				(formatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE);
			if(nativeBlit)
			{
				m_HDRSupported						= true;
				m_DOFSupported						= true;
			}
			else
			{
				m_HDRSupported						= false;
				m_DOFSupported						= false;
			}
		}
		else
		{
			m_ColourBufferSupported					= false;
			m_HDRSupported							= false;
			m_DOFSupported							= false;
		}
		
		if(Display::Device()->SupportForRenderTarget(Display::E_ResourceType_Texture2D, Display::E_Format_R32F))
		{
			m_DepthStencilBufferSupported			= true;
			m_ShadowMapSupported					= true;
		}
		else
		{
			m_DepthStencilBufferSupported			= false;
			m_ShadowMapSupported					= false;
		}
	}
	
	ID3D11Query* nativeQuery = nullptr;
	const D3D11_QUERY_DESC nativeQueryDesc = { D3D11_QUERY_OCCLUSION, 0 };
	const HRESULT hr = Display::Device()->GetD3DDevice()->CreateQuery(&nativeQueryDesc, &nativeQuery);
	if (nativeQuery) nativeQuery->Release();
	if(SUCCEEDED(hr))
	{
		m_OcclusionQueriesSupported				= true;
	}
	else
	{
		m_OcclusionQueriesSupported				= false;		
	}
#elif defined(PLATFORM_PS3)
	m_ColourBufferSupported							= true;
	m_DepthStencilBufferSupported					= true;
	m_HDRSupported									= true;
	m_DOFSupported									= true;
	m_ShadowMapSupported							= true;
	m_OcclusionQueriesSupported						= true;
#else
	m_ColourBufferSupported							= false;
	m_DepthStencilBufferSupported					= false;
	m_HDRSupported									= false;
	m_DOFSupported									= false;
	m_ShadowMapSupported							= false;
	m_OcclusionQueriesSupported						= false;
#endif


	m_ColourBufferEnabled							= m_ColourBufferSupported;
	m_DepthStencilBufferEnabled						= m_DepthStencilBufferSupported;
	m_HDREnabled									= m_HDRSupported;
	m_DOFEnabled									= m_DOFSupported;
	m_ShadowMapEnabled								= m_ShadowMapSupported;
	m_OcclusionQueriesEnabled						= m_OcclusionQueriesSupported;
}

void C_RenderSettings::DumpSettings()
{
	LOGINFO(Render::g_Log, "m_ColourBufferEnabled %i\n",				m_ColourBufferEnabled);
	LOGINFO(Render::g_Log, "m_ColourBufferSupported %i\n",				m_ColourBufferSupported);
	LOGINFO(Render::g_Log, "m_DepthStencilBufferEnabled %i\n",			m_DepthStencilBufferEnabled);
	LOGINFO(Render::g_Log, "m_DepthStencilBufferSupported %i\n",		m_DepthStencilBufferSupported);
	LOGINFO(Render::g_Log, "m_HDREnabled %i\n",							m_HDREnabled);
	LOGINFO(Render::g_Log, "m_HDRSupported %i\n",						m_HDRSupported);
	LOGINFO(Render::g_Log, "m_DOFEnabled %i\n",							m_DOFEnabled);
	LOGINFO(Render::g_Log, "m_DOFSupported %i\n",						m_DOFSupported);
	LOGINFO(Render::g_Log, "m_ShadowMapEnabled %i\n",					m_ShadowMapEnabled);
	LOGINFO(Render::g_Log, "m_ShadowMapSupported %i\n",					m_ShadowMapSupported);
	LOGINFO(Render::g_Log, "m_OcclusionQueriesEnabled %i\n",			m_OcclusionQueriesEnabled);
	LOGINFO(Render::g_Log, "m_OcclusionQueriesSupported %i\n",			m_OcclusionQueriesSupported);
	LOGINFO(Render::g_Log, "m_ShadowMapCullingDistance %f\n",			m_ShadowMapCullingDistance);
}

} // namespace Render

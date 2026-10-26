#include "stdafx.h"
#include "Display/Device/DeviceDebug.h"
#include "Display/Surface/SurfaceUtils.h"

DEBUG_ENUM(Display::E_Bool)
	DE_OPTION(Display::E_Bool_True,						"True")
	DE_OPTION(Display::E_Bool_False,					"False")
END_DEBUG_ENUM

DEBUG_ENUM(Display::E_CubeTextureFace)
	DE_OPTION(Display::E_CubeTextureFace_PosX,			"PosX")
	DE_OPTION(Display::E_CubeTextureFace_PosY,			"PosY")
	DE_OPTION(Display::E_CubeTextureFace_PosZ,			"PosZ")
	DE_OPTION(Display::E_CubeTextureFace_NegX,			"NegX")
	DE_OPTION(Display::E_CubeTextureFace_NegY,			"NegY")
	DE_OPTION(Display::E_CubeTextureFace_NegZ,			"NegZ")
END_DEBUG_ENUM

DEBUG_ENUM(Display::E_CullMode)
	DE_OPTION(Display::E_CullMode_Default,				"Default")
	DE_OPTION(Display::E_CullMode_None,					"None")
	DE_OPTION(Display::E_CullMode_Cw,					"Cw")
	DE_OPTION(Display::E_CullMode_Ccw,					"Ccw")
END_DEBUG_ENUM

DEBUG_ENUM(Display::E_TextureAddress)
	DE_OPTION(Display::E_TextureAddress_Default,		"Default")
	DE_OPTION(Display::E_TextureAddress_Wrap,			"Wrap")
	DE_OPTION(Display::E_TextureAddress_Clamp,			"Clamp")
	DE_OPTION(Display::E_TextureAddress_Mirror,			"Mirror")
	DE_OPTION(Display::E_TextureAddress_Border,			"Border")
END_DEBUG_ENUM

DEBUG_ENUM(Display::E_TextureFilter)
	DE_OPTION(Display::E_TextureFilter_Default,			"Default")
	DE_OPTION(Display::E_TextureFilter_None,			"None")
	DE_OPTION(Display::E_TextureFilter_Point,			"Point")
	DE_OPTION(Display::E_TextureFilter_Linear,			"Linear")
	DE_OPTION(Display::E_TextureFilter_Anisotropic,		"Anisotropic")
END_DEBUG_ENUM

namespace Display
{

C_ResolutionManager::C_ResolutionManager() :
	m_MaxSize(0)
{
}

C_ResolutionManager::~C_ResolutionManager()
{
	ClearResolutions();
}

void C_ResolutionManager::ClearResolutions()
{
	m_Resolutions.Clear();
	m_Options.Clear();
	#ifdef PLATFORM_PC
	m_Multisamples.Clear();			// [Blur2PC]
	m_MultisampleOptions.Clear();
	#endif
}

void C_ResolutionManager::ResetResolutions()
{
	ClearResolutions();

	m_Resolutions.PushBack();
	m_Resolutions.Back().m_Value		= 0;
	m_Resolutions.Back().m_Width		= 0;
	m_Resolutions.Back().m_Height		= 0;
	#ifdef PLATFORM_PC
	m_Resolutions.Back().m_RefreshRate	= 0;	// [Blur2PC]
	#endif
	m_Resolutions.Back().m_Multisample	= Display::E_Multisample_None;
	m_Resolutions.Back().m_Name			= "Disabled";
}

void C_ResolutionManager::AddResolution(const U32 width, const U32 height, const E_Format format, const E_Multisample multisample, const U32 flags)
{
	U32 size = GetSurfaceSize(width, height, flags, format, multisample);
	if(size <= m_MaxSize || m_MaxSize == 0)
	{
		Util::C_String aa;
		switch(multisample)
		{
		case E_Multisample_None:
			aa = "1";
			break;
		case E_Multisample_2:
			aa = "2";
			break;
		case E_Multisample_4:
			aa = "4";
			break;
		default:
			return;
		}
		m_Resolutions.PushBack();
		m_Resolutions.Back().m_Value		= m_Resolutions.Size()-1;
		m_Resolutions.Back().m_Width		= width;
		m_Resolutions.Back().m_Height		= height;
		#ifdef PLATFORM_PC
		m_Resolutions.Back().m_RefreshRate	= 0;	// [Blur2PC]
		#endif
		m_Resolutions.Back().m_Multisample	= multisample;
		m_Resolutions.Back().m_Name.Printf("%ix%ix%s", width, height, aa.Chars());
	}
}

void C_ResolutionManager::SetResolutions(Debug::EnumBase* pEnum)
{
	if(pEnum)
	{
		const S32 resolutionCount = m_Resolutions.Size();
		if(resolutionCount > 0)
		{
			m_Options.Clear();

			for(S32 resolutionIndex=0; resolutionIndex<resolutionCount; resolutionIndex++)
			{
				C_Resolution& resolution = m_Resolutions[resolutionIndex];

				m_Options.PushBack();
				m_Options.Back().value = resolution.m_Value;
				m_Options.Back().name = resolution.m_Name.Chars();
			}

			m_Options.PushBack();
			m_Options.Back().value = -1;
			m_Options.Back().name = 0;
		}

		const S32 optionCount = m_Options.Size();
		if(optionCount > 0)
		{
			pEnum->SetOptions(&m_Options.Front());
		}
	}
}

void C_ResolutionManager::SetMaxResolution(const U32 width, const U32 height, const E_Format format, const E_Multisample multisample, const U32 flags)
{
	m_MaxSize = GetSurfaceSize(width, height, flags, format, multisample);
}

C_Resolution* C_ResolutionManager::GetResolution(const S32 resolutionIndex)
{
	#ifdef PLATFORM_PC
	// [Blur2PC] retail 0x005a4910: every entry is a real mode (there is no "Disabled" entry on PC)
	if(resolutionIndex >= 0 && resolutionIndex < m_Resolutions.Size())
	#else
	if(resolutionIndex > 0 && resolutionIndex < m_Resolutions.Size())
	#endif
	{
		return &m_Resolutions[resolutionIndex];
	}
	else
	{
		return NULL;
	}
}

#ifdef PLATFORM_PC
// [Blur2PC] retail Blur PC resolution manager, recovered from Blur.exe 1.2 (0x005a4840-0x005a4930 and the
// DRM-relocated 0x01f1c350, 0x01f0b1b0, 0x020c892d). See docs/analysis.md "Graphics options (PC)".

void C_ResolutionManager::AddResolution(const U32 width, const U32 height, const E_Format format, const E_Multisample multisample, const U32 flags, const U32 refreshRate)
{
	U32 size = GetSurfaceSize(width, height, flags, format, multisample);
	if(size <= m_MaxSize || m_MaxSize == 0)
	{
		m_Resolutions.PushBack();
		m_Resolutions.Back().m_Value		= m_Resolutions.Size()-1;
		m_Resolutions.Back().m_Width		= width;
		m_Resolutions.Back().m_Height		= height;
		m_Resolutions.Back().m_RefreshRate	= refreshRate;
		m_Resolutions.Back().m_Multisample	= multisample;
		m_Resolutions.Back().m_Name.Printf("%ix%i", width, height);
	}
}

void C_ResolutionManager::AddMultisample(const E_Multisample multisample)
{
	m_Multisamples.PushBack();
	m_Multisamples.Back().m_Multisample	= multisample;
	m_Multisamples.Back().m_Name.Printf("%ix", (S32)multisample);
}

void C_ResolutionManager::SetResolutions(Debug::EnumBase* pResolutionEnum, Debug::EnumBase* pMultisampleEnum)
{
	SetResolutions(pResolutionEnum);

	if(pMultisampleEnum)
	{
		const S32 multisampleCount = m_Multisamples.Size();
		if(multisampleCount > 0)
		{
			m_MultisampleOptions.Clear();

			for(S32 multisampleIndex=0; multisampleIndex<multisampleCount; multisampleIndex++)
			{
				m_MultisampleOptions.PushBack();
				m_MultisampleOptions.Back().value = multisampleIndex;
				m_MultisampleOptions.Back().name = m_Multisamples[multisampleIndex].m_Name.Chars();
			}

			m_MultisampleOptions.PushBack();
			m_MultisampleOptions.Back().value = -1;
			m_MultisampleOptions.Back().name = 0;

			pMultisampleEnum->SetOptions(&m_MultisampleOptions.Front());
		}
	}
}

const S32 C_ResolutionManager::FindResolution(const U32 width, const U32 height) const
{
	for(S32 i=0; i<m_Resolutions.Size(); i++)
	{
		if(m_Resolutions[i].m_Width == width && m_Resolutions[i].m_Height == height)
			return i;
	}
	return -1;
}

// retail 0x020c892d: the mode whose pixel count is nearest (first one wins a tie)
const S32 C_ResolutionManager::FindClosestResolution(const U32 width, const U32 height) const
{
	const S32 area = (S32)(width * height);
	S32 best = -1;
	S32 bestDifference = 0x7fffffff;
	for(S32 i=0; i<m_Resolutions.Size(); i++)
	{
		S32 difference = (S32)(m_Resolutions[i].m_Width * m_Resolutions[i].m_Height) - area;
		if(difference < 0)
			difference = -difference;
		if(difference < bestDifference)
		{
			bestDifference = difference;
			best = i;
		}
	}
	return best;
}

const C_MultisampleOption* C_ResolutionManager::GetMultisample(const S32 multisampleIndex) const
{
	if(multisampleIndex >= 0 && multisampleIndex < m_Multisamples.Size())
		return &m_Multisamples[multisampleIndex];
	return NULL;
}
#endif

C_ResolutionManager g_Resolutions;

}; // namespace Display

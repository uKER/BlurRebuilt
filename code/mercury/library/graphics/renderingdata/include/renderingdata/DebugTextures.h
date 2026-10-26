#ifndef __RENDERINGDATA_DEBUGTEXTURES_H__
#define __RENDERINGDATA_DEBUGTEXTURES_H__

namespace RenderingData
{
namespace Target
{

	// DebugTextures

	struct DebugTextures
	{
		DebugTextures() : bEnable(false), m_pSelectedTexture(0) {}

		bool bEnable; // Enable overrides.
		const Display::C_Texture2D *m_pTexture0; // Overrides for texture stages.
		const Display::C_Texture2D *m_pTexture1;
		const Display::C_Texture2D *m_pTexture2;
		const Display::C_Texture2D *m_pTexture3;

		const void* m_pSelectedTexture; // Pointer to GPU texture selected in texture viewer.
	};

	extern DebugTextures g_DebugTextures;

}

}

#endif

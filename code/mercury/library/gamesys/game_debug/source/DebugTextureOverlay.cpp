#include "GameDebugPch.h"
#include <game_debug/DebugTextureOverlay.h>

namespace GameDebug
{

extern Debug::Logger& g_Log;

Debug::Bool		g_debugTextureOverlayEnable("GameSys.DebugTextureOverlay.Enable",		false);

Debug::Float	g_debugTextureOverlayAlpha("GameSys.DebugTextureOverlay.Alpha",		1.0f, Debug::Range(0.0f, 1.0f));
Debug::String	g_debugTextureOverlayFilename("GameSys.DebugTextureOverlay.Filename", "dontship/textures/overlay.bmp");

Debug::Float	g_debugTextureOverlayX("GameSys.DebugTextureOverlay.X",		640.0f, Debug::Range(0.0f, 1280.0f), Debug::Persist() );
Debug::Float	g_debugTextureOverlayY("GameSys.DebugTextureOverlay.Y",		360.0f, Debug::Range(0.0f, 720.0f), Debug::Persist());
Debug::Float	g_debugTextureOverlayWidth("GameSys.DebugTextureOverlay.Width",		1280.0f, Debug::Range(0.0f, 1280.0f), Debug::Persist());
Debug::Float	g_debugTextureOverlayHeight("GameSys.DebugTextureOverlay.Height",		720.0f, Debug::Range(0.0f, 720.0f), Debug::Persist());

Debug::Bool		g_debugTextureOverlayFromFile("GameSys.DebugTextureOverlay.FromFile",		false);
Debug::Bool		g_debugTextureOverlayConstrain("GameSys.DebugTextureOverlay.ConstrainHeight",		false);

C_DebugTextureOverlay::C_DebugTextureOverlay()
{

}

C_DebugTextureOverlay::~C_DebugTextureOverlay()
{

}

void C_DebugTextureOverlay::Init()
{
	
}

void C_DebugTextureOverlay::Update()
{
	bool bEnabled = g_debugTextureOverlayEnable;

	if(bEnabled)
	{
		LoadTexture( g_debugTextureOverlayFilename.Value() );
	}
	else
	{
		FreeTexture();
	}


	if( m_Texture2D.IsCreated() )
	{
		if(g_debugTextureOverlayConstrain)
		{
			F32 textureRatio = (F32)m_Texture2D.GetWidth() / (F32)m_Texture2D.GetHeight();
			F32 screenWidth = g_debugTextureOverlayWidth;
			F32 screenHeight = screenWidth / textureRatio;
			g_debugTextureOverlayHeight = screenHeight;
		}

		if(g_debugTextureOverlayFromFile)
		{
			g_debugTextureOverlayWidth = (F32) m_Texture2D.GetWidth();
			g_debugTextureOverlayHeight = (F32) m_Texture2D.GetHeight();
		}
	}

}

void C_DebugTextureOverlay::Render2d()
{
	if( !m_Texture2D.IsCreated() )
		return;

	F32 w = g_debugTextureOverlayWidth;
	F32 h = g_debugTextureOverlayHeight;

	F32 x = g_debugTextureOverlayX - w * 0.5f;
	F32 y = g_debugTextureOverlayY - h * 0.5f;

	x = x * 1024.0f / 1280.0f;
	y = y * 768.0f / 720.0f;
	w = w * 1024.0f / 1280.0f;
	h = h * 768.0f / 720.0f;

	Display::BasicRender()->DrawTextured2DRect(x, y, w, h, Maths::RGBA(1.0f, 1.0f, 1.0f, (F32)g_debugTextureOverlayAlpha), &m_Texture2D);

}

void C_DebugTextureOverlay::Close()
{
	FreeTexture();
}

void C_DebugTextureOverlay::LoadTexture( const Util::C_String& filename )
{
	if( filename == m_TextureFilename)
		return;

	FreeTexture();

	m_TextureFilename = filename;

	if (!Display::CreateTexture2DFromFile(m_TextureFilename, &m_Texture2D))
	{
	}
	else
	{
		LOGINFO(g_Log, "Loaded texture %s", m_TextureFilename.Chars());
	}
	
}

void C_DebugTextureOverlay::FreeTexture()
{
	m_TextureFilename = "";

	if( m_Texture2D.IsCreated() )
	{
		Display::Device()->Destroy(&m_Texture2D);
	}
}

}; // namespace GameDebug

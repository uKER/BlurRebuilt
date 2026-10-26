#include "stdafx.h"
#include "Display/Game/Game.h"
#include "Display/Device/Device.h"

#if 0

DEBUG_ENUM(Display::E_Resolution)
	DE_OPTION(Display::E_Resolution_640x480x1,	"640x480x1")
	DE_OPTION(Display::E_Resolution_640x480x4,	"640x480x4")
	DE_OPTION(Display::E_Resolution_960x640x2,	"960x640x2")
	DE_OPTION(Display::E_Resolution_960x640x4,	"960x640x4")
	DE_OPTION(Display::E_Resolution_1024x576x2,	"1024x576x2")
	DE_OPTION(Display::E_Resolution_1024x576x4,	"1024x576x4")
	DE_OPTION(Display::E_Resolution_1280x720x1,	"1280x720x1")
	DE_OPTION(Display::E_Resolution_1280x720x2,	"1280x720x2")
	DE_OPTION(Display::E_Resolution_1280x720x4,	"1280x720x4")
END_DEBUG_ENUM

Debug::Enum<Display::E_Resolution> g_debugGameResolution("Graphics.Render.Game.Resolution", Display::E_Resolution_1280x720x2);

#endif

namespace Display
{

#if 0


C_GameView::C_GameView(const U32 width, const U32 height, const E_Multisample multisample) :
	m_Width(width),
	m_Height(height),
	m_Multisample(multisample),
	m_ColourFormat(E_Format_A2B10G10R10F),
	m_DepthStencilFormat(E_Format_D24FS8)
{
}

C_GameView::~C_GameView()
{
}

bool C_GameView::Create(U32& curMemSize, U8*& curMemPtr)
{
	// with aa
	{
		U32 tileWidth = m_Width;
		U32 tileHeight = m_Height;

		C_SurfaceParams surfaceParams;

		Device()->CreateColourSurface(tileWidth, tileHeight, 0, m_ColourFormat, m_Multisample, &surfaceParams, &m_WithAA.m_ColourSurface);


		Device()->CreateDepthStencilSurface(tileWidth, tileHeight, 0, m_DepthStencilFormat, m_Multisample, &surfaceParams, &m_WithAA.m_DepthStencilSurface);
	}

	// without aa
	{
		U32 tileWidth = m_Width;
		U32 tileHeight = m_Height;

		C_SurfaceParams surfaceParams;

		Device()->CreateColourSurface(tileWidth, tileHeight, 0, m_ColourFormat, E_Multisample_None, &surfaceParams, &m_WithoutAA.m_ColourSurface);


		Device()->CreateDepthStencilSurface(tileWidth, tileHeight, 0, m_DepthStencilFormat, E_Multisample_None, &surfaceParams, &m_WithoutAA.m_DepthStencilSurface);
	}

	
	return true;
}

void C_GameView::Destroy()
{
	Device()->Destroy(&m_WithAA.m_ColourSurface);
	Device()->Destroy(&m_WithAA.m_DepthStencilSurface);
	Device()->Destroy(&m_WithoutAA.m_ColourSurface);
	Device()->Destroy(&m_WithoutAA.m_DepthStencilSurface);
	Device()->Destroy(&m_ColourRenderTexture);
	Device()->Destroy(&m_DepthStencilRenderTexture);
}

C_Game::C_Game()
{
	// Memory management
	m_MemID						= -1;
	m_MemSize					= 15 * 1024 * 1024;
	m_MemPtr					= NULL;
	m_MemFlags					= 0;
	// Resolution management
	m_ResMode					= -1;
	m_ResWidth					= 0;
	m_ResHeight					= 0;
	m_ResMultisample			= E_Multisample_None;

	assert(g_pGame == NULL);
	g_pGame = this;

	SetRegion(C_CoreSystems::Instance().GetConsoleRegion());
}

C_Game::~C_Game()
{
	ClearViews();

	assert(g_pGame == this);
	g_pGame = NULL;
}

void C_Game::ClearViews()
{
	for(S32 i=0; i<m_ViewList.Size(); i++)
		delete m_ViewList[i];
	m_ViewList.Clear();
}

S32 C_Game::AddView(const U32 width, const U32 height, const E_Multisample multisample)
{
	C_GameView* pView = new C_GameView(width, height, multisample);
	m_ViewList.PushBack(pView);
	return m_ViewList.Size()-1;
}

bool C_Game::CreateViews()
{
	U32 curMemSize = 0;
	U8* curMemPtr = m_MemPtr;

	for(S32 i=0; i<m_ViewList.Size(); i++)
	{
		if(!m_ViewList[i]->Create(curMemSize, curMemPtr))
			return false;
	}

	if(curMemSize > m_MemSize)
	{
		LOGFATAL(g_Log, "Tried to allocate too much render target memory '%i / %i'\n", curMemSize, m_MemSize);
	}
	else
	{
		LOGINFO(g_Log, "Render target memory '%i'\n", curMemSize);
	}

	return true;
}

void C_Game::DestroyViews()
{
	for(S32 i=0; i<m_ViewList.Size(); i++)
	{
		m_ViewList[i]->Destroy();
	}
}

bool C_Game::CreateMemory()
{
	RESOURCE_MEMORY_ID("RenderTargets");


	return true;
}

void C_Game::DestroyMemory()
{
}

bool C_Game::CheckForResolutionChange(const bool disableMultisample)
{
	const S32 resMode = g_debugGameResolution;

	if(m_ResMode != resMode)
	{
		m_ResWidth = 1280;
		m_ResHeight = 720;
		m_ResMultisample = E_Multisample_2;

		switch(resMode)
		{
		case E_Resolution_640x480x1:
		case E_Resolution_640x480x4:
			m_ResWidth = 640;
			m_ResHeight = 480;
			if (disableMultisample)
			{
				m_ResMultisample = E_Multisample_None;
			}
			else
			{
				switch(resMode)
				{
				case E_Resolution_640x480x1:
					m_ResMultisample = E_Multisample_None;
					break;
				case E_Resolution_640x480x4:
					m_ResMultisample = E_Multisample_4;
					break;
				}
			}
			break;

		case E_Resolution_960x640x2:
		case E_Resolution_960x640x4:
			m_ResWidth = 960;
			m_ResHeight = 640;
			if (disableMultisample)
			{
				m_ResMultisample = E_Multisample_None;
			}
			else
			{
				switch(resMode)
				{
				case E_Resolution_960x640x2:
					m_ResMultisample = E_Multisample_2;
					break;
				case E_Resolution_960x640x4:
					m_ResMultisample = E_Multisample_4;
					break;
				}
			}
			break;

		case E_Resolution_1024x576x2:
		case E_Resolution_1024x576x4:
			m_ResWidth = 1024;
			m_ResHeight = 576;
			if (disableMultisample)
			{
				m_ResMultisample = E_Multisample_None;
			}
			else
			{
				switch(resMode)
				{
				case E_Resolution_1024x576x2:
					m_ResMultisample = E_Multisample_2;
					break;
				case E_Resolution_1024x576x4:
					m_ResMultisample = E_Multisample_4;
					break;
				}
			}
			break;

		case E_Resolution_1280x720x2:
		case E_Resolution_1280x720x4:
			m_ResWidth = 1280;
			m_ResHeight = 720;
			if (disableMultisample)
			{
				m_ResMultisample = E_Multisample_None;
			}
			else
			{
				switch(resMode)
				{
				case E_Resolution_1280x720x1:
					m_ResMultisample = E_Multisample_None;
					break;
				case E_Resolution_1280x720x2:
					m_ResMultisample = E_Multisample_2;
					break;
				case E_Resolution_1280x720x4:
					m_ResMultisample = E_Multisample_4;
					break;
				}
			}
			break;

		default:
			LOGFATAL(g_Log, "Unsupported resolution '%i%\n", resMode);
			return false;
		}


		m_ResMode = resMode;
	}

	return true;
}

C_Game* g_pGame = NULL;

void Game(C_Game* pGame)
{
	g_pGame = pGame;
}

#endif

} // namespace Display

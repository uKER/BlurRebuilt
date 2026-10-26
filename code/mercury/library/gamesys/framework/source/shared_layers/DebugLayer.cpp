#include "FrameworkPch.h"
#include <Framework/shared_layers/DebugLayer.h>
#include <Framework/application/UserControls.h>
#include <Framework/application/applicationbase.h>
#include <Input/GameInput.h>
#include <Render/debug/DebugRender.h>
#include <system/debug/log/DebugLog.h>
#include <system/debug/log/LogManager.h>
#include <system/debug/I_DebugRender.h>
#include <DebugSystems/DebugHUDSubscriber.h>
#include <DebugSystems/DebugHUDMenu.h>
#include <DebugSystems/DebugHUDPerf.h>
#include <DebugSystems/DebugHUDMemory.h>
#include <DebugSystems/DebugHUDresource.h>
#include <Render/font/Font.h>
#include <DebugSystems/FileDialog.h>
#include <DebugSystems/DebugHUDTextureViewer.h>
#include <DebugSystems/FileMonitor.h>
#include <system/debug/BuildStamp.h>
//#include <audio.h>

#if defined(PLATFORM_PS3)
#include <netex/libnetctl.h>
#endif

Debug::Bool			g_DebugDisableDebugHud( "GameSys.DisableDebugHud", false );

Debug::Colour	g_DebugFullScreenBackPanel("GameSys.DebugHUD.DebugFullScreenBackPanel", 0,0,0,0);
Debug::Colour	g_Debug3DBackPanel("GameSys.DebugHUD.Debug3DBackPanel", 0,0,0,0);

Debug::Bool			g_DebugCameraHelper("GameSys.DebugHUD.GridCameraHelper.Show", false);
Debug::Float		g_DebugCameraHelperStepSize("GameSys.DebugHUD.GridCameraHelper.StepSize", 10.f, Debug::Range(1.f, 100.f));
Debug::Int			g_DebugCameraHelperStepCount("GameSys.DebugHUD.GridCameraHelper.StepCount", 30, Debug::Range(10, 200));
Debug::Bool			g_DebugCameraHelperShowCoordinateInfo("GameSys.DebugHUD.GridCameraHelper.ShowCoordinateInfo", true);
Debug::Int			g_DebugCameraHelperCoordinateInfoStep("GameSys.DebugHUD.GridCameraHelper.CoordinateInfoStep", 5, Debug::Range(1,20));
Debug::Int			g_DebugCameraHelperFontSize("GameSys.DebugHUD.GridCameraHelper.FontSize", 20, Debug::Range(5,30));
Debug::Colour		g_DebugCameraHelperColour("GameSys.DebugHUD.GridCameraHelper.Colour", 0.7f, 0.7f, 0.7f, 1.f);
Debug::Colour		g_DebugCameraHelperAxisColour("GameSys.DebugHUD.GridCameraHelper.AxisColour", 0.95f, 0.95f, 0.95f, 1.f);
Debug::Bool			g_DebugCameraHelperShowAxis("GameSys.DebugHUD.GridCameraHelper.ShowAxis", false);

Debug::Bool			g_ArtTextureSizeHelper("Graphics.Debug.ArtTextureSizeHelper", false);

#ifndef CFG_GOLD
Debug::String			g_DebugHudStringMsgString(				"GameSys.DebugHUD.StringMsg.String",				"");
Debug::Float			g_DebugHudStringMsgX(					"GameSys.DebugHUD.StringMsg.X",					512.0f, Debug::Range(0.0f, 1024.0f) );
Debug::Float			g_DebugHudStringMsgY(					"GameSys.DebugHUD.StringMsg.Y",					96.0f, Debug::Range(0.0f, 768.0f) );
Debug::Float			g_DebugHudStringMsgSizeX(					"GameSys.DebugHUD.StringMsg.SizeX",					24.0f, Debug::Range(0.0f, 128.0f) );
Debug::Float			g_DebugHudStringMsgSizeY(					"GameSys.DebugHUD.StringMsg.SizeY",					32.0f, Debug::Range(0.0f, 128.0f) );
Debug::Float			g_DebugHudStringMsgOutlineSize(					"GameSys.DebugHUD.StringMsg.OutlineSize",					0.0f, Debug::Range(0.0f, 128.0f) );

//Debug::Colour			g_DebugHudStringMsgColourTop("GameSys.DebugHUD.StringMsg.ColourTop", 0.7f, 0.7f, 0.7f, 1.f);
//Debug::Colour			g_DebugHudStringMsgColourBottom("GameSys.DebugHUD.StringMsg.ColourBottom", 0.7f, 0.7f, 0.7f, 1.f);
#endif

#ifdef PLATFORM_PS3
Debug::Bool		g_ShowIPAddress(  "ShowIpAddress", false );
#endif

Debug::Bool		g_ShowBuildInfo(  "GameSys.DebugHUD.ShowBuildInfo", false );

Debug::Bool		g_DebugDumpMemoryBudgets("GameSys.DumpMemoryBudgets", false);

namespace Framework
{

Debug::Bool g_DebugKeyCombos("GameSys.Debug.KeyCombos",
#ifdef CFG_GOLD
	false,
#else
	true,
#endif
	Debug::Description("True to enable Back button key combos, False to act like a Master build")
);

//////////////////////////////////////////////////////////////////////////
//	DebugMenuController
//////////////////////////////////////////////////////////////////////////
class C_DebugMenuController : public DebugSystems::I_Controller
{
public:
	C_DebugMenuController(Input::C_GameInput* pGameInput) : m_pGameInput(pGameInput) {};
	
	virtual const bool MoveUp( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuUp, 16, 4 ); }
	virtual const bool MoveDown( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuDown, 16, 4 ); }
	virtual const bool MoveLeft( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuLeft, 16, 0 ); }
	virtual const bool MoveRight( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuRight, 16, 0 ); }

	virtual const bool Confirm( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ); }
	virtual const bool Cancel( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuCancel ); }

	virtual const bool Open( )		{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_DebugMenuActivate ); }
	virtual const bool Close( )		{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_DebugMenuActivate ); }

	virtual const bool PositionMove(F32& deltaX, F32& deltaY)
	{
		deltaX = 2.f * m_pGameInput->GetAxisValue(Framework::E_Axis_Debug_RightStick_X) - 1.f;
		deltaY = -2.f * m_pGameInput->GetAxisValue(Framework::E_Axis_Debug_RightStick_Y) + 1.f;

		return (fabs(deltaX) > 0.1f || fabs(deltaY) > 0.1f);
	}

private:
	Input::C_GameInput*		m_pGameInput;
};

//////////////////////////////////////////////////////////////////////////
//	DebugMenuController
//////////////////////////////////////////////////////////////////////////
class C_DebugTextureViewerController : public DebugSystems::I_ControllerTextureViewer
{
public:
	C_DebugTextureViewerController(Input::C_GameInput* pGameInput) : m_pGameInput(pGameInput) {};

	virtual const bool MoveUp( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_Debug_DPadUp, 16, 0 ); }
	virtual const bool MoveDown( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_Debug_DPadDown, 16, 0 ); }
	virtual const bool MoveLeft( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_Debug_DPadLeft, 16, 0 ); }
	virtual const bool MoveRight( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_Debug_DPadRight, 16, 0 ); }

	virtual const bool Confirm( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ); }
	virtual const bool Cancel( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuCancel ); }

	virtual const bool PositionMove(F32& deltaX, F32& deltaY)
	{
		deltaX = 2.f * m_pGameInput->GetAxisValue(Framework::E_Axis_Debug_RightStick_X) - 1.f;
		deltaY = -2.f * m_pGameInput->GetAxisValue(Framework::E_Axis_Debug_RightStick_Y) + 1.f;

		return (fabs(deltaX) > 0.1f || fabs(deltaY) > 0.1f);
	}

private:
	Input::C_GameInput*		m_pGameInput;
};

#ifdef FILEDIALOG_SAMPLE_ON
//////////////////////////////////////////////////////////////////////////
//	FileDialogController
//////////////////////////////////////////////////////////////////////////
class C_FileDialogController : public DebugSystems::I_FileDialogController
{
public:
	C_FileDialogController(Input::C_GameInput* pGameInput) : m_pGameInput(pGameInput) {};
	
	virtual const bool MoveUp( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuUp, 16, 4 ); }
	virtual const bool MoveDown( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuDown, 16, 4 ); }
	
	virtual const bool Confirm( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ); }
	virtual const bool Cancel( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuCancel ); }

private:
	Input::C_GameInput*		m_pGameInput;
};
#endif



//////////////////////////////////////////////////////////////////////////
//C_DebugLayer
//////////////////////////////////////////////////////////////////////////

C_DebugLayer::C_DebugLayer()
	 : Framework::C_Layer("C_DebugLayer"), m_Freeze( false )
{
	m_pDebugHUDMenuSystem = NULL;
	m_pDebugHUDSubscriber = NULL;
	m_pDebugHUDPerfSystem = NULL;
	m_pDebugHUDMemorySystem = NULL;
	m_pDebugHUDResourceSystem = NULL;
	m_pDebugHUDTextureViewer = NULL;
	m_pDebugTextureViewerController = NULL;
	m_pDebugFrame = NULL;
#ifdef FILEDIALOG_SAMPLE_ON
	m_pFileDialog = NULL;
#endif
	m_pFileMonitor = NULL;

	m_pFontRenderer = NULL;

}

C_DebugLayer::C_DebugLayer( C_DebugLayerSkin& debugLayerSkin ) : Framework::C_Layer("C_DebugLayer"), m_DebugLayerSkin(debugLayerSkin)
{
}

C_DebugLayer::~C_DebugLayer()
{
	SAFE_DELETE_PTR(m_pFontRenderer);
	
	m_MemoryBudgets.ClearBudgets();
}

HRESULT C_DebugLayer::ProcessMessage(const E_LayerMessage message)
{
	switch( message )
	{
		case E_LayerMessage_Create:
			return Create();
		break;

		case E_LayerMessage_Destroy:
			return Destroy();
		break;

		case E_LayerMessage_GlobalUpdate:
			return GlobalUpdate();
		break;

		case E_LayerMessage_RenderFullscreenDebug:
			return RenderFullScreenDebug();
		break;

		case E_LayerMessage_RenderSplitscreenWithAA:
			//return Render3dDebug();
		break;

		case Framework::E_LayerMessage_EndRenderSplitscreenWithoutAA:
			//flush debug info

			//smaller device reset
			Display::Device()->Unbind();
			Display::Device()->SetViewShaderConstants3D();

			Display::Device()->InvalidateState();
			Display::Device()->DefaultStates();

			DbUtil::DefaultDebugRenderSys()->RenderDeferred();
			DbUtil::DefaultDebugRenderSys()->RenderPrimitive();
			Render3dDebug();
		break;
		case Framework::E_LayerMessage_ResetDebugRender:
			DbUtil::DefaultDebugRenderSys()->ResetDeferred();
			DbUtil::DefaultDebugRenderSys()->PrimitiveEndFrame();
			break;
	}

	return S_OK;
}

void C_DebugLayer::InitFontRenderer()
{
	if (m_pFontRenderer == NULL)
	{
		//Create the font renderer
		m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
	}
}

HRESULT C_DebugLayer::Create()
{
	RESOURCE_MEMORY_ID("DebugLayer");

	//Create logger
	m_pDebugHUDSubscriber = new DebugSystems::C_DebugHUDSubscriber();
	m_pDebugHUDSubscriber->SetXMLParameter("format", "%P%c");
	//Subscribe logger
	Debug::GetRootLogger().AddSubscriber(m_pDebugHUDSubscriber);
	

	m_pDebugHUDMenuController = new C_DebugMenuController(Input::GetGameInputManager()->GetGameInput(0));
	
	//Config Skin

	//Create debug frame
	m_pDebugFrame = new DebugSystems::C_Frame(m_DebugLayerSkin.m_FrameSkin);
	DebugSystems::C_Frame::SetCommonFrame(m_pDebugFrame);

	//Create Debug Menu
	m_pDebugHUDMenuSystem = new DebugSystems::C_DebugHUDMenu(m_pDebugHUDMenuController, m_DebugLayerSkin.m_DebugHUDMenuSkin);

	//Create Debug Perf
	m_pDebugHUDPerfSystem = new DebugSystems::C_DebugHUDPerf();

	//Create Debug Memory 
	m_pDebugHUDMemorySystem = new DebugSystems::C_DebugHUDMemory();

	//Create Debug Resource
	m_pDebugHUDResourceSystem = new DebugSystems::C_DebugHUDResource();

	//Create Texture Viewer
	m_pDebugTextureViewerController = new C_DebugTextureViewerController(Input::GetGameInputManager()->GetGameInput(0));
	m_pDebugHUDTextureViewer = new DebugSystems::C_DebugHUDTextureViewer(m_pDebugTextureViewerController);

#ifdef FILEDIALOG_SAMPLE_ON
	m_pFileDialogController = new C_FileDialogController(Input::GetGameInputManager()->GetGameInput(0));

	//File dialog sample
	DebugSystems::C_FileDialogSkin fileDialogSkin;
	m_pFileDialog = new DebugSystems::C_FileDialog("Test, select a file", "*",".\\",m_pFileDialogController, fileDialogSkin);
#endif

	m_pFileMonitor = new DebugSystems::C_FileMonitor();
	m_pFileMonitor->Initialise();

	DumpBuildInfo();

	return S_OK;
}

HRESULT C_DebugLayer::Destroy()
{
	//RemoveSubscribe logger
	Debug::GetRootLogger().RemoveSubscriber(m_pDebugHUDSubscriber);

	SAFE_DELETE_PTR ( m_pFileMonitor );

	SAFE_DELETE_PTR ( m_pDebugHUDSubscriber );
	SAFE_DELETE_PTR ( m_pDebugHUDMenuSystem );
	SAFE_DELETE_PTR ( m_pDebugHUDPerfSystem );
	SAFE_DELETE_PTR ( m_pDebugHUDMemorySystem );
	SAFE_DELETE_PTR ( m_pDebugHUDMenuController );
	SAFE_DELETE_PTR ( m_pDebugHUDResourceSystem );
	SAFE_DELETE_PTR ( m_pDebugTextureViewerController );
	SAFE_DELETE_PTR ( m_pDebugHUDTextureViewer );
	SAFE_DELETE_PTR (m_pDebugFrame);
	DebugSystems::C_Frame::SetCommonFrame(NULL);
#ifdef FILEDIALOG_SAMPLE_ON
	SAFE_DELETE_PTR ( m_pFileDialogController );
	SAFE_DELETE_PTR ( m_pFileDialog );
#endif
	return S_OK;
}

HRESULT C_DebugLayer::GlobalUpdate()
{
	PERF_EVENT("GlobalUpdate");

	//Input configuration
	Input::C_GameInput* controlPtr = Input::GetGameInputManager()->GetGameInput(0); 
	if ( controlPtr->GetButtonAutoRepeat( Framework::E_Button_HudSubscriberScrollUp, 12, 1 ) )
		m_pDebugHUDSubscriber->ScrollUp();
	if( controlPtr->GetButtonAutoRepeat( Framework::E_Button_HudSubscriberScrollDown, 12, 1 ) )
		m_pDebugHUDSubscriber->ScrollDown();

	//Update debug systems
	m_pDebugHUDSubscriber->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
	m_pDebugHUDMenuSystem->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
	m_pDebugHUDPerfSystem->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
	m_pDebugHUDMemorySystem->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
	m_pDebugHUDResourceSystem->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
	m_pDebugHUDTextureViewer->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());

	if( m_pDebugHUDMenuSystem->Active() )
	{
		C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_DebugMenu );
	}
	else
	{
		C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_DebugMenu );
	}

	if(g_DebugKeyCombos && controlPtr->GetButtonPressedDebounced( Framework::E_Button_Freeze ) )
	{
		m_Freeze = !m_Freeze;
		if(m_Freeze)
		{
			LOGINFO( g_Log, "Freeze");
			C_ApplicationBase::GetInstance()->Pause( Framework::E_LayerPause_Freeze );
		}
		else
		{
			LOGINFO( g_Log, "Resume");
			C_ApplicationBase::GetInstance()->Resume( Framework::E_LayerPause_Freeze );
		}
	}

#ifdef FILEDIALOG_SAMPLE_ON
	m_pFileDialog->Update(C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
	
	if (!m_pFileDialog->Active())
	{

		Util::C_String selectedFile;
		if (m_pFileDialog->CheckState(selectedFile))
		{
			LOGINFO(g_Log, "File selected %s", selectedFile.Chars());
		}
		else
		{
			LOGINFO(g_Log, "File dialog canceled");
		}
		m_pFileDialog->Active(true);
	}

#endif

	if(g_DebugDumpMemoryBudgets)
	{
		m_MemoryBudgets.Dump();
		g_DebugDumpMemoryBudgets = false;
	}

	return S_OK;
}

HRESULT C_DebugLayer::RenderFullScreenDebug()
{
	PERF_EVENT("Render.Debug.RenderFullScreenDebug");
	
	if(m_pFontRenderer == NULL)
		m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
	
	m_MemoryBudgets.RenderBudgetsOverMaxSize(Render::Debug::Immediate(), m_pFontRenderer);

	if(g_DebugDisableDebugHud)
	{
		m_pDebugHUDMenuSystem->Render();
		return S_OK;
	}

	//Render Debug back panel
	if (g_DebugFullScreenBackPanel->a > 0.01f)
	{
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		Maths::RGBA colour(U8(Maths::Min(g_DebugFullScreenBackPanel->r, 1.f) * 255.f), U8(Maths::Min(g_DebugFullScreenBackPanel->g, 1.f) * 255.f), U8(Maths::Min(g_DebugFullScreenBackPanel->b, 1.f) * 255.f), U8(Maths::Min(g_DebugFullScreenBackPanel->a, 1.f) * 255.f));
		Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(0.f, 0.f), Maths::HVec2( 1024.f, 768.f ), colour);
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	}


	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
	DbUtil::DefaultDebugRenderSys()->RenderPrimitive();

	if (g_ArtTextureSizeHelper)
		DrawArtTextureSizeHelper();

	// Render FPS counter

	//Render debug Systems
	m_pDebugHUDPerfSystem->Render();
	m_pDebugHUDMemorySystem->Render();
	m_pDebugHUDResourceSystem->Render();
	m_pDebugHUDTextureViewer->Render();

	// took this out temporarily as it was causing linker errors.
	// also, paul malin said the framework should not be dependent on audio
	//if (Audio::GetSystem())
	//	Audio::GetSystem()->DebugRender();

	m_pDebugHUDSubscriber->Render();
	m_pDebugHUDMenuSystem->Render();

	m_pFileMonitor->Render();

#ifdef FILEDIALOG_SAMPLE_ON
	m_pFileDialog->Render();
#endif

	//PS3 IP
	#ifdef PLATFORM_PS3
	#ifndef CFG_GOLD
	{
		if (g_ShowIPAddress)
		{
			ShowIpAddress();
		}
	}
	#endif
	#endif

	#ifndef CFG_GOLD
	{
		if (g_ShowBuildInfo)
		{
			ShowBuildInfo();
		}
	}
	#endif


	#ifndef CFG_GOLD
	ShowStringMsg();
	#endif

	return S_OK;
}

void C_DebugLayer::ShowIpAddress()
{
#ifdef PLATFORM_PS3
	static bool gotAddress = false;
	static char ipAddress[ CELL_NET_CTL_IPV4_ADDR_STR_LEN ];
	CellNetCtlInfo m_netCtlInfo;
	if (!gotAddress)
	{
		if (cellNetCtlGetInfo(CELL_NET_CTL_INFO_IP_ADDRESS, &m_netCtlInfo) < 0)
		{
			  LOGWARN(g_Log, "cellNetCtlGetInfo() : Failed to get IP address");
		}
		else
		{
			memcpy (&ipAddress, m_netCtlInfo.ip_address, CELL_NET_CTL_IPV4_ADDR_STR_LEN );
			gotAddress = true;
		}
	}
	else
	{
		Util::C_String text;
		text.Printf("IP %s", ipAddress );

		InitFontRenderer();
		m_pFontRenderer->SetScaleX(24.f);
		m_pFontRenderer->SetScaleY(24.f);
		m_pFontRenderer->Print( Maths::HVec2(400.0f, 70.0f), text );
	}
#endif
}

void C_DebugLayer::ShowStringMsg()
{
#ifndef CFG_GOLD
	Util::C_String debugHudStringMsg = g_DebugHudStringMsgString.Value();
	if(debugHudStringMsg.Length() > 0)
	{
		InitFontRenderer();
		Util::C_String text;
		text.Printf("%s", g_DebugHudStringMsgString.Value());

		m_pFontRenderer->SetScaleX((F32)g_DebugHudStringMsgSizeX);
		m_pFontRenderer->SetScaleY((F32)g_DebugHudStringMsgSizeY);
		m_pFontRenderer->SetOutlineSize((F32)g_DebugHudStringMsgOutlineSize);
		m_pFontRenderer->Print(Maths::HVec2((F32)g_DebugHudStringMsgX, (F32)g_DebugHudStringMsgY), text.Chars(), Render::Font::E_Justification_Centre );
	}
#endif
}

HRESULT C_DebugLayer::Render3dDebug()
{
	PERF_EVENT("Render.Debug.RenderFullScreenDebug");

	if(g_DebugDisableDebugHud)
		return S_OK;

	//Render Debug back panel
	if (g_Debug3DBackPanel->a > 0.01f)
	{
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		Maths::RGBA colour(U8(Maths::Min(g_Debug3DBackPanel->r, 1.f) * 255.f), U8(Maths::Min(g_Debug3DBackPanel->g, 1.f) * 255.f), U8(Maths::Min(g_Debug3DBackPanel->b, 1.f) * 255.f), U8(Maths::Min(g_Debug3DBackPanel->a, 1.f) * 255.f));
		Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(0.f, 0.f), Maths::HVec2( 1024.f, 768.f ), colour);
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	}

	if (g_DebugCameraHelper)
	{
		DrawCameraHelper();
	}

	Render::Debug::DebugRenderSys()->RenderDeferred();
	Render::Debug::DebugRenderSys()->RenderPrimitive();
	Render::Debug::DebugRenderSys()->PrimitiveEndFrame();

	return S_OK;
}

void C_DebugLayer::DrawCameraHelper()
{
	//Get camera position
	//Maths::HVec3 cameraPosition = C_ApplicationBase::GetInstance()->GetFullscreenView().m_View.GetViewSettings3D().GetCameraPosition();
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	Maths::HVec3 cameraPosition = viewSettings3D.GetCameraPosition();

	//Calculate Grid range
	S32 originX,originY;
	originX = U32(cameraPosition.X() / g_DebugCameraHelperStepSize + 0.5f); //10.f meters grid
	originY = U32(cameraPosition.Z() / g_DebugCameraHelperStepSize + 0.5f); //10.f meters grid

	//Render Grid
	Render::Debug::Primitive()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	Render::Debug::Primitive()->SetDepthEnable( false );
	
	Maths::RGBA gridColour(U8(Maths::Min(g_DebugCameraHelperColour->r, 1.f) * 255.f), U8(Maths::Min(g_DebugCameraHelperColour->g, 1.f) * 255.f), U8(Maths::Min(g_DebugCameraHelperColour->b, 1.f) * 255.f), U8(Maths::Min(g_DebugCameraHelperColour->a, 1.f) * 255.f));
	Maths::RGBA axisColour(U8(Maths::Min(g_DebugCameraHelperAxisColour->r, 1.f) * 255.f), U8(Maths::Min(g_DebugCameraHelperAxisColour->g, 1.f) * 255.f), U8(Maths::Min(g_DebugCameraHelperAxisColour->b, 1.f) * 255.f), U8(Maths::Min(g_DebugCameraHelperAxisColour->a, 1.f) * 255.f));
	for (S32 x = (originX - g_DebugCameraHelperStepCount); x <= (originX + g_DebugCameraHelperStepCount); x++)
	{
		Maths::RGBA colour = (x != 0) ? gridColour : axisColour;

		Render::Debug::Primitive()->Draw3dLine(Maths::HVec3((F32)x * g_DebugCameraHelperStepSize, 0.f, g_DebugCameraHelperStepSize * (originY - g_DebugCameraHelperStepCount)), Maths::HVec3( (F32)x * g_DebugCameraHelperStepSize, 0.f, g_DebugCameraHelperStepSize * (originY + g_DebugCameraHelperStepCount)), colour);
	}

	for (S32 z = (originY - g_DebugCameraHelperStepCount); z <= (originY + g_DebugCameraHelperStepCount); z++)
	{
		Maths::RGBA colour = (z != 0) ? gridColour : axisColour;
		
		Render::Debug::Primitive()->Draw3dLine(Maths::HVec3(g_DebugCameraHelperStepSize * (originX - g_DebugCameraHelperStepCount), 0.f, (F32)z * g_DebugCameraHelperStepSize), Maths::HVec3( g_DebugCameraHelperStepSize * (originX + g_DebugCameraHelperStepCount), 0.f, (F32)z * g_DebugCameraHelperStepSize), colour);
	}

	if (g_DebugCameraHelperShowAxis)
	{
		//Draw axis
		Render::Debug::Primitive()->Draw3dArrow(Maths::HVec3(cameraPosition.X(), 0.f, cameraPosition.Z()), Maths::HVec3(1.f, 0.f, 0.f), 10.f, 1.f, Maths::RGBA(255,0,0));
		Render::Debug::Primitive()->Draw3dArrow(Maths::HVec3(cameraPosition.X(), 0.f, cameraPosition.Z()), Maths::HVec3(0.f, 0.f, 1.f), 10.f, 1.f, Maths::RGBA(0,255,0));
	}

	if (g_DebugCameraHelperShowCoordinateInfo)
	{

		if (m_pFontRenderer == NULL)
		{
			//Create the font renderer
			m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		}
		
		m_pFontRenderer->SetScaleX(F32(g_DebugCameraHelperFontSize));
		m_pFontRenderer->SetScaleY(F32(g_DebugCameraHelperFontSize));
		
		char buffer[256];

		for (S32 z = (originY - g_DebugCameraHelperStepCount); z <= (originY + g_DebugCameraHelperStepCount); z++)
		{
			if ((z % g_DebugCameraHelperCoordinateInfoStep) == 0)
			{
				sprintf_s(buffer, 256, "(%2.2f, %2.2f)", (F32)originX * g_DebugCameraHelperStepSize, (F32)z * g_DebugCameraHelperStepSize);
				m_pFontRenderer->Print3D(Maths::HVec3((F32)originX * g_DebugCameraHelperStepSize, 0.f,(F32)z * g_DebugCameraHelperStepSize),buffer, Render::Font::E_Justification_Centre); 
			}
		}

		for (S32 x = (originX - g_DebugCameraHelperStepCount); x <= (originX + g_DebugCameraHelperStepCount); x++)
		{
			if ((x % g_DebugCameraHelperCoordinateInfoStep) == 0)
			{
				sprintf_s(buffer, 256, "(%2.2f, %2.2f)", (F32)x * g_DebugCameraHelperStepSize, (F32)originY * g_DebugCameraHelperStepSize);
				m_pFontRenderer->Print3D(Maths::HVec3((F32)x * g_DebugCameraHelperStepSize, 0.f,(F32)originY * g_DebugCameraHelperStepSize),buffer, Render::Font::E_Justification_Centre); 
			}
		}
	}

	Render::Debug::Primitive()->SetDepthEnable( true );
}

const char* GetBuildConfig()
{
	#ifdef BUILD_CONFIG_DEBUG
	return "Debug";
	#endif

	#ifdef BUILD_CONFIG_RELEASE
	return "Release";
	#endif

	#ifdef BUILD_CONFIG_SPEED
	return "Speed";
	#endif

	#ifdef BUILD_CONFIG_PROFILE
	return "Profile";
	#endif

	#ifdef BUILD_CONFIG_MASTER
	return "Master";
	#endif
}

const char* GetPlatform()
{

	#ifdef PLATFORM_PC
	return "PC";
	#endif

	#ifdef PLATFORM_PS3
	return "PlayStation 3";
	#endif
}

void C_DebugLayer::DumpBuildInfo()
{
	LOGINFO( g_Log, "Build Version: %s (%s)", Debug::GetBuildVersion(), GetBuildConfig());
	LOGINFO( g_Log, "Revision     : %s", Debug::GetBuildRevision());
	LOGINFO( g_Log, "Build PC     : %s (%s)", Debug::GetBuildPc(), Debug::GetBuildUser());
	LOGINFO( g_Log, "Build Date   : %s (%s)", Debug::GetBuildDate(), Debug::GetBuildTime());
	LOGDEBUG( g_Log, "Platform     : %s", GetPlatform());
}

void C_DebugLayer::ShowBuildInfo()
{
	Util::C_String buildInfo;

	buildInfo << "Build Version: " << Debug::GetBuildVersion() << "(" << GetBuildConfig() << ")\n";
	buildInfo << "Build PC     : " << Debug::GetBuildPc() << "(" << Debug::GetBuildUser() << ")\n";
	buildInfo << "Build Date   : " << Debug::GetBuildDate() << "(" << Debug::GetBuildTime() << ")\n";
	buildInfo << "Platform     : " << GetPlatform() << "\n";
	buildInfo << "Revision     : " << Debug::GetBuildRevision() << "\n";

	InitFontRenderer();
	m_pFontRenderer->SetScaleX(16.f);
	m_pFontRenderer->SetScaleY(16.f);
	m_pFontRenderer->SetShadowEnable( true );
	
	m_pFontRenderer->Print( Maths::HVec2(1024.0f - 64.0f, 768.0f - 64.0f) - m_pFontRenderer->GetDimensions(0, buildInfo), buildInfo, Render::Font::E_Justification_Left );
}

void C_DebugLayer::DrawArtTextureSizeHelper()
{
	F32 width = (F32)Display::Device()->GetBackBuffer().GetWidth();
	F32 height = (F32)Display::Device()->GetBackBuffer().GetHeight();

	for (S32 i=64; i <= 1024; i<<=1)
	{
		F32 left		= 512.f - i/2 * 1024.f / width;
		F32 right		=512.f + i/2 * 1024.f / width;
		F32 top		= 384.f - i/2 * 768.f / height;
		F32 bottom	= 384.f + i/2 * 768.f / height;

		//Draw Rectangle
		Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(left, top),Maths::HVec2(right,	top), Maths::RGBA(1.f,1.f,1.f,1.f));
		Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(left, bottom),Maths::HVec2(right,	bottom), Maths::RGBA(1.f,1.f,1.f,1.f));
		Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(left, top),Maths::HVec2(left,	bottom), Maths::RGBA(1.f,1.f,1.f,1.f));
		Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(right, top),Maths::HVec2(right,	bottom), Maths::RGBA(1.f,1.f,1.f,1.f));

		{
			char buffer2[10];
			sprintf_s(buffer2, lengthof(buffer2), "%d", i);
			if (m_pFontRenderer == NULL)
			{
				//Create the font renderer
				m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
			}
			m_pFontRenderer->SetScaleX(20.f);
			m_pFontRenderer->SetScaleY(20.f);
			m_pFontRenderer->SetShadowEnable(true);

			m_pFontRenderer->Print(Maths::HVec2( right, bottom), buffer2, Render::Font::E_Justification_Right);

			m_pFontRenderer->SetShadowEnable(false);
		}

		// Draw a double thickness border
		{
			float left		= 512.f - (i/2-1) * 1024.f / width;
			float right		=512.f + (i/2+1) * 1024.f / width;
			float top		= 384.f - (i/2-1) * 768.f / height;
			float bottom	= 384.f + (i/2+1) * 768.f / height;

			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(left, top),Maths::HVec2(right,	top), Maths::RGBA(1.f,1.f,1.f,1.f));
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(left, bottom),Maths::HVec2(right,	bottom), Maths::RGBA(1.f,1.f,1.f,1.f));
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(left, top),Maths::HVec2(left,	bottom), Maths::RGBA(1.f,1.f,1.f,1.f));
			Render::Debug::Immediate()->Draw2dLine(Maths::HVec2(right, top),Maths::HVec2(right,	bottom), Maths::RGBA(1.f,1.f,1.f,1.f));
		}
	}
}
}; // namespace Framework

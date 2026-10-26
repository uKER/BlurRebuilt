#include "stdafx.h"

#include "PreviewWindow.h"

// mercury
#include "Render/PostProcess/Manager.h"
#include "Render/PostProcess/HDR/HDR.h"
#include "Render/PostProcess/Colour/Greyscale.h"

// generic rendering
#include "Render/Render/GridRender.h"

// particles
#include "Render/Dynamic/Renderer.h"
#include "Render/Dynamic/Particle/Force.h"
#include "Render/Dynamic/Particle/Emitter.h"
#include "Render/Dynamic/Particle/Event.h"

#include "Render/Dynamic/Particle/Tools_Shared/UserShape3DData.h"
#include "Render/Dynamic/Particle/Tools_Shared/UserShape3DVariants.h"
#include "Render/Dynamic/Particle/Tools_Shared/UserParticleContainer.h"
#include "Render/Dynamic/Particle/UserParticleLibrary.h"
#include "Render/Dynamic/Particle/Generic/Sorcery.h"

// debug rendering
#include <System/Debug/I_DebugRender.h>

IMPLEMENT_CLASS(C_PreviewWindow, wxDisplayWindow)

BEGIN_EVENT_TABLE(C_PreviewWindow, wxDisplayWindow)
  EVT_MOUSE_EVENTS(wxDisplayWindow::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(wxDisplayWindow::OnMouseCaptureLostEvent)
  EVT_KEY_DOWN(wxDisplayWindow::OnKeyDownEvent)
  EVT_KEY_UP(wxDisplayWindow::OnKeyUpEvent)
  EVT_KILL_FOCUS(wxDisplayWindow::OnKillFocusEvent)
  EVT_TIMER(wxID_ANY, wxDisplayWindow::OnTimer)
  EVT_SIZE(wxDisplayWindow::OnSize)
END_EVENT_TABLE()

XT_APPLICATION
	XT_DEPENDS_ON(RenderParticles)
XT_END_APPLICATION

C_PreviewWindow::C_PreviewWindow( wxWindow *parent, wxWindowID id,
                    const wxPoint &pos, const wxSize &size )
        : wxDisplayWindow( parent, id, pos, size)
{
	m_pPostProcessing = new PostProcess::C_Manager;

	m_pHighDynamicRange = new PostProcess::C_HighDynamicRange;
	m_pHighDynamicRange->SetType(PostProcess::E_Type_Final);

	m_pDynamicEffectRenderer = new DynamicEffect::C_Renderer;
	m_pDynamicEffectRenderer->SetParticleColour(MathsStorage::Float4A16(2.0f, 2.0f, 2.0f, 2.0f));
	m_pDynamicEffectRenderer->SetEmissiveColour(MathsStorage::Float4A16(3.f, 3.f, 3.f, 3.f));
	m_pDynamicEffectRenderer->SetLightDirection(MathsStorage::Float3(0.707f, 0.707f, 0.0f));
	m_pDynamicEffectRenderer->ReserveEmitters(2048);
	m_pDynamicEffectRenderer->ReserveEventInstances(256);
	m_pDynamicEffectRenderer->SetSRGB(true);

	m_ParticleId = 0;
	m_ParticleTransform = Maths::C_QuatTrans();
	m_bParticleRepeating = false;

	DynamicEffect::SorceryInterface::Reset();
	DynamicEffect::SorceryInterface::Initialise(m_pDynamicEffectRenderer);

	m_pXtDb = NULL;
	m_pXtCallbackContext = new XtCallbackContext('P');
}

C_PreviewWindow::~C_PreviewWindow()
{
	delete m_pPostProcessing;
	delete m_pDynamicEffectRenderer;
	
	delete m_pXtCallbackContext;
}

bool C_PreviewWindow::OnCreate()
{
	return true;
}

void C_PreviewWindow::OpenAssets(const Util::C_String& assetPath)
{
	m_pDynamicEffectRenderer->SetAssetPath(assetPath + "\\Textures\\");

	Util::C_String userParticles = assetPath + "\\Particles\\particles_global.pcp";

	DynamicEffect::C_UserParticleLibrary::Instance().Reset();
	DynamicEffect::C_UserParticleLibrary::Instance().Open(userParticles);
}

void C_PreviewWindow::OpenFlasks(const Ut::Vector<Util::C_String>& flaskPaths)
{
	if(m_pXtDb)
		CloseFlask();

	XtInitialise();
	
	m_pXtDb	= XtDbCreate(XtGetApplicationSchema(), 10000, m_pXtCallbackContext);

	for(S32 i = 0; i < flaskPaths.Size(); ++i)
	{
		Sys::I_File* file;
		if(file = Sys::C_FileSystem::Instance().Open(flaskPaths[i]))
		{
			wxLogMessage(_T("Loading Xt flask '%s'..."), flaskPaths[i].Chars());
			
			U32 length = (U32)file->Length();
			U8* data = new U8[length];

			if(data)
			{
				if(file->Read(data, length))
					m_XtPacks.PushBack(XtPackOpen(m_pXtDb, data, length));
			}

			delete data;
			delete file;
		}
		else
			wxLogMessage(_T("Failed to load Xt flask '%s'..."), flaskPaths[i].Chars());
	}

	XtNetworkStart(m_pXtDb);
	DynamicEffect::SorceryInterface::Update();
}

void C_PreviewWindow::OnUpdate()
{
	XtNetworkUpdate();
	DynamicEffect::SorceryInterface::Update();

	m_pPostProcessing->SetFrameTime(m_Timer.GetFrameTime());

	m_pDynamicEffectRenderer->Update(m_Timer.GetTime(), m_Timer.GetFrameTime(), m_Timer.IsPaused());
	m_pDynamicEffectRenderer->UpdateEmitters();

	if(m_bParticleRepeating && !m_ParticleHandle.Object())
	{
		SpawnEvent(m_ParticleId, m_ParticleTransform);
	}
}

void C_PreviewWindow::SpawnEvent(const U32 eventId, const Maths::C_QuatTrans& transform)
{
	RemoveEvent();

	m_ParticleId = eventId;
	m_ParticleTransform = transform;
	m_ParticleHandle = DynamicEffect::SorceryInterface::SpawnEvent(eventId, transform);

	if(DynamicEffect::C_EventInstance* pEventInstance = m_ParticleHandle.Object())
	{
		m_bParticleRepeating = pEventInstance->GetEvent()->GetDefinition().bRepeating;
		pEventInstance->SetCollisionPlane(MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 1.0f), MathsStorage::Float4A16(0.0f, 1.0f, 0.0f, 0.0f));
		pEventInstance->SetLighting(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
	}
	else
		m_bParticleRepeating = false;
}

void C_PreviewWindow::RemoveEvent()
{
	if(DynamicEffect::C_EventInstance* pEventInstance = m_ParticleHandle.Object())
		pEventInstance->Free();

	m_bParticleRepeating = false;
}

void C_PreviewWindow::CloseFlask()
{
	if(m_pXtDb)
	{
		for(S32 i = 0; i < m_XtPacks.Size(); ++i)
			XtPackClose(m_pXtDb, m_XtPacks[i]);
		
		XtDbDestroy(m_pXtDb);

		m_XtPacks.Clear();
		m_pXtDb = NULL;
	}

	m_pDynamicEffectRenderer->RemoveTextures();
}

void C_PreviewWindow::OnResize()
{
	// destroy post process and re-add the views
	m_pPostProcessing->DestroyVolatile();
	m_pPostProcessing->ClearViews();

	for(S32 i = 0; i < Display::Device()->GetViewCount(); ++i)
	{
		const Display::C_View& view = *Display::Device()->GetView(i);
		
		if(view.m_UserIndex != -1)
		{
			if(m_pHighDynamicRange->GetType() == PostProcess::E_HighDynamicRangeType_ShaderU32)
				m_pPostProcessing->AddView(view.GetViewport().GetW(), view.GetViewport().GetH(), view.GetIndex(), PostProcess::E_RenderTargetFormat_RGBA_U8, Display::E_Multisample_None, 5);
			else
				m_pPostProcessing->AddView(view.GetViewport().GetW(), view.GetViewport().GetH(), view.GetIndex(), PostProcess::E_RenderTargetFormat_RGBA_F10, Display::E_Multisample_None, 5);
		}
	}
	
	m_pPostProcessing->CreateVolatile();
}

void C_PreviewWindow::OnRender() const
{
	m_pDynamicEffectRenderer->BeginFrame();

	// begin post effects
	const Display::C_View* pView = Display::Device()->GetView();
	m_pPostProcessing->Begin(pView->GetIndex(), pView->GetViewport());
	
	// clear
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(96, 96, 96, 255));

	// render opaque stuff
	m_pDynamicEffectRenderer->BeginView();
	m_pDynamicEffectRenderer->Build(Display::Device()->GetView()->m_Index);
	
	// opaque
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_Opaque);

	// grid
	Render::DrawGrid();

	// transparent
	m_pDynamicEffectRenderer->Display(DynamicEffect::E_RenderPass_TwoDimensional | DynamicEffect::E_RenderPass_Transparent | DynamicEffect::E_RenderPass_Debug);

	// end
	m_pHighDynamicRange->SetBloomGamma(true, true);
	m_pHighDynamicRange->Display();
	m_pPostProcessing->End();

	// debug
	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}


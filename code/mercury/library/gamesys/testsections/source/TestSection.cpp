#include "stdafx.h"
#include "TestSections/TestSection.h"
#include <Framework/Application/ApplicationBase.h>
#include <Framework/Application/UserControls.h>
#include <camera2.h>
#include <camera2/debug/DebugTools.h>

namespace Framework
{

C_TestSection::C_TestSection(const Util::C_String& name) : Display::I_Renderer(name)
{
	// Debug tools (network comms etc...)
	m_DebugTools = new Cam2::C_DebugTools;
	assert(m_DebugTools);
	m_DebugTools->Init();
}

C_TestSection::~C_TestSection()
{
	SAFE_DELETE_PTR(m_DebugTools)
}

void C_TestSection::OnUpdate()
{
	F32 period = Application().GetTimer().GetFrameTime();

	Cam2::C_DebugFeed& debugFeed = Application().GetDebugFeed(0);

	m_DebugTools->SetLocalCamera(debugFeed.Live());

	m_DebugTools->Update(period);

	switch (m_DebugTools->Notify())
	{
		case Cam2::E_NetNotify_SetPos:
			// Set camera position from Maya:
			debugFeed.SetDebugType(Cam2::E_FlyCamera);
			debugFeed.CopyFrom(m_DebugTools->RemoteCamera());
		break;
	}
}

void C_TestSection::OnPreRender()
{
}

void C_TestSection::OnPostRender()
{
}

void C_TestSection::OnRender3DViewWithAA(const S32 viewIndex)
{
}

void C_TestSection::OnRender3DViewWithoutAA(const S32 viewIndex)
{
}

void C_TestSection::OnRender2DView()
{
}

void C_TestSection::OnBeginRenderSplitscreenWithoutAA()
{
}

void C_TestSection::OnEndRenderSplitscreenWithoutAA()
{
}

void C_TestSection::OnResize()
{
}

void C_TestSection::OnSetupUserControls(const S32 viewIndex)
{
	// Set the debug feed to fly camera:
	Cam2::C_DebugFeed& debugFeed = Application().GetDebugFeed(viewIndex);
	debugFeed.SetDebugType(Cam2::E_FlyCamera);

	// Position the fly camera:
	Cam2::C_Camera camera;
	camera.SetPosition(Maths::HVec3(-30.f, 20.f, -30.f));
	camera.LookAt(Maths::HVec3());
	debugFeed.CopyFrom(camera);
}

void C_TestSection::OnUpdateUserControls(const S32 viewIndex)
{
	Application().GetDebugFeed(viewIndex).SetInput( GetFlyCameraControls(viewIndex) );
}

// Helper function to get at Application singleton:
C_ApplicationBase& C_TestSection::Application()
{
	C_ApplicationBase* app = C_ApplicationBase::GetInstance();
	assert(app);
	
	return *app;
}

}; // namespace Framework

#include "GameViewPch.h"
#include "GameView/OffscreenView.h"

namespace GameView
{

C_OffscreenView::C_OffscreenView() :
	m_pRenderTexture(NULL),
	m_Width(256),
	m_Height(256),
	m_Multisample(Display::E_Multisample_None)

{
	m_View.m_UserIndex = 0;
	m_CameraView = Maths::HVec3(0.f, 0.f, 1.f);
	m_CameraUp = Maths::HVec3(0.f, 1.f, 0.f);

	SetupView(1.f, Maths::Pi/3.f, 500.f);
}

C_OffscreenView::~C_OffscreenView()
{
	Destroy();
}

void C_OffscreenView::SetFormat(const U32 width, const U32 height, const Display::E_Multisample multiSample)
{
	m_Width = width;
	m_Height = height;
	m_Multisample = multiSample;
#if PLATFORM_PC
	// [Blur2PC] Retail PC (Blur.exe 1.2 0x00b1fd20) also hands the new format to an existing render target;
	// it takes effect when the device next recreates its volatile resources.
	if(m_pRenderTexture)
		m_pRenderTexture->SetFormat(width, height, multiSample);
#endif
}

void C_OffscreenView::Create()
{
	if(!m_pRenderTexture)
	{
		RESOURCE_MEMORY_ID("OffscreenRender");

		m_pRenderTexture = new Render::C_TextureRender(m_Width, m_Height, m_Multisample);
		
		if(!m_pRenderTexture->Create() || !m_pRenderTexture->CreateVolatile())
		{
			SAFE_DELETE_PTR(m_pRenderTexture);
			//LOGWARN(Game::g_Log, "Failed to create the cube texture render\n");
		}
	}
}

void C_OffscreenView::Destroy()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->DestroyVolatile();
		m_pRenderTexture->Destroy();
		delete m_pRenderTexture;
		m_pRenderTexture = NULL;
	}

}

void C_OffscreenView::Bind()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->Bind();
		Display::Device()->SetView(&m_View);
	}
}

void C_OffscreenView::Unbind()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->Unbind();
	}	
}


void C_OffscreenView::DebugRender()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->DebugRender();
	}
}

void C_OffscreenView::SetCamera( const Maths::HVec3 position, const Maths::HVec3 view, const Maths::HVec3 up)
{
	m_Position = position;
	m_CameraView = view;
	m_CameraUp = up;
}

void C_OffscreenView::SetupView(const F32 aspectRatio, const F32 fov, const F32 farPlane)
{
	m_View.m_Type = Display::E_ViewType_3D;
	m_ViewSetup.m_ViewSetup3D.m_PerspDist = 1024.0f;
	F32 cheight = tanf(fov * 0.5f) * m_ViewSetup.m_ViewSetup3D.m_PerspDist;
	F32 cwidth = cheight * aspectRatio;
	m_ViewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds(-cwidth, -cheight, cwidth, cheight);
	m_ViewSetup.m_ViewSetup3D.m_FarClip = farPlane;
}

void C_OffscreenView::SetCameraToWorld(const Maths::HVec3& pos, const Maths::HVec3& dir, const Maths::HVec3& up)
{
	Maths::HVec3 az;
	az = dir;
	az.Normalise();

	Maths::HVec3 ax;
	ax = up ^ az;
	ax.Normalise();

	Maths::HVec3 ay;
	ay = az ^ ax;
	ay.Normalise();

	Maths::HMat33 rot;
	rot.AX(ax);
	rot.AY(ay);
	rot.AZ(az);

	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43(rot);
	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld.AW(pos);
}

const Render::C_TextureRender* C_OffscreenView::GetTextureRender() const
{
	return m_pRenderTexture;
}

void C_OffscreenView::SetupView()
{
	SetCameraToWorld(m_Position, m_CameraView, m_CameraUp);
	m_View.Setup(m_ViewSetup, m_Width, m_Height);
}
}

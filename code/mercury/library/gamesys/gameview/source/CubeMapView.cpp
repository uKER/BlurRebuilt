#include "GameViewPch.h"
#include "GameView/CubeMapView.h"

namespace GameView
{

C_CubeMapView::C_CubeMapView() :
	m_pCurrentTexture(NULL),
	m_pRenderTexture(NULL),
	m_pSaverTexture(NULL),
	m_Width(256),
	m_Multisample(Display::E_Multisample_None),
	m_SaveTexture(false)
{
	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		m_View[i].m_UserIndex = 0;
		m_View[i].m_Type = Display::E_ViewType_3D;
	}
}

C_CubeMapView::~C_CubeMapView()
{
	Destroy();
}

void C_CubeMapView::SetFormat(const U32 size, const Display::E_Multisample multiSample)
{
	m_Width = size;
	m_Multisample = multiSample;
}

void C_CubeMapView::SetFarClip(const F32 farPlane)
{
	m_ViewSetup.m_ViewSetup3D.m_FarClip = farPlane;
}

const Display::C_TextureCube* C_CubeMapView::GetRenderTexture() const
{
	return m_pRenderTexture ? m_pRenderTexture->GetTexture() : NULL;
}

void C_CubeMapView::Create()
{
	F32 height = tanf(Maths::DegToRad(90.0f) * 0.5f) * 1024.f;
	F32 width = height * 1.0f;
	m_ViewSetup.m_ViewSetup3D.m_PerspDist = 1024.0f;
	m_ViewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds(-width, -height, width, height);

	if(!m_pRenderTexture)
	{
		RESOURCE_MEMORY_ID("CubeMapRender");

		m_pRenderTexture = new Render::C_CubeTextureRender(m_Width, m_Multisample);
		if(!m_pRenderTexture->Create() || !m_pRenderTexture->CreateVolatile())
		{
			SAFE_DELETE_PTR(m_pRenderTexture);
			//LOGWARN(Game::g_Log, "Failed to create the cube texture render\n");
		}
	}

	if(!m_pSaverTexture && m_SaveTexture)
	{
		RESOURCE_MEMORY_ID("CubeMapSaver");

		m_pSaverTexture = new Render::C_CubeTextureSaver(m_Width);
		if(!m_pSaverTexture->Create() || !m_pSaverTexture->CreateVolatile())
		{
			SAFE_DELETE_PTR(m_pSaverTexture);
			//LOGWARN(Game::g_Log, "Failed to create the cube texture saver\n");
		}
	}
}

void C_CubeMapView::Destroy()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->DestroyVolatile();
		m_pRenderTexture->Destroy();
		delete m_pRenderTexture;
		m_pRenderTexture = NULL;
	}
	
	if(m_pSaverTexture)
	{
		m_pSaverTexture->DestroyVolatile();
		m_pSaverTexture->Destroy();
		delete m_pSaverTexture;
		m_pSaverTexture = NULL;
	}
}

void C_CubeMapView::Bind()
{
	if(!m_SaveTexture)
	{
		m_pCurrentTexture = m_pRenderTexture;
	}
	else
	{
		m_pCurrentTexture = m_pSaverTexture;
	}
}

void C_CubeMapView::Unbind()
{
	//Save if needed
	if (m_SaveTexture && m_pSaverTexture)
	{
		m_pSaverTexture->Save(m_SaveTextureFileName);
		m_SaveTexture = false;
	}
}

void C_CubeMapView::BindFace(const Display::E_CubeTextureFace face)
{
	assert(face < Display::E_CubeTextureFace_Count);
	
	if(m_pCurrentTexture)
	{
		m_pCurrentTexture->Bind(face);

		Display::Device()->SetView(&m_View[face]);
	}
}

void C_CubeMapView::UnbindFace(const Display::E_CubeTextureFace face)
{
	assert(face < Display::E_CubeTextureFace_Count);
	
	if(m_pCurrentTexture)
	{
		m_pCurrentTexture->Unbind(face);
	}
}

void C_CubeMapView::DebugRender()
{
	if(m_pCurrentTexture)
	{
		m_pCurrentTexture->DebugRender();
	}
}

void C_CubeMapView::Save(const Util::C_String& saveTextureFileName)
{
	m_SaveTexture = true;
	m_SaveTextureFileName = saveTextureFileName;
}

void C_CubeMapView::SetPosition( const Maths::HVec3 position )
{
	m_Position = position;
}

void C_CubeMapView::SetCameraToWorld(const Maths::HVec3& pos, const Maths::HVec3& dir, const Maths::HVec3& up)
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

void C_CubeMapView::SetupView(const Display::E_CubeTextureFace face)
{
	switch (face)
	{
	case Display::E_CubeTextureFace_PosX:
		SetCameraToWorld(m_Position, Maths::HVec3( 1.0f,  0.0f,  0.0f), Maths::HVec3( 0.0f,  1.0f,  0.0f));
		break;
	case Display::E_CubeTextureFace_PosY:
		SetCameraToWorld(m_Position, Maths::HVec3( 0.0f,  1.0f,  0.0f), Maths::HVec3( 0.0f,  0.0f, -1.0f));
		break;
	case Display::E_CubeTextureFace_PosZ:
		SetCameraToWorld(m_Position, Maths::HVec3( 0.0f,  0.0f,  1.0f), Maths::HVec3( 0.0f,  1.0f,  0.0f));
		break;
	case Display::E_CubeTextureFace_NegX:
		SetCameraToWorld(m_Position, Maths::HVec3(-1.0f,  0.0f,  0.0f), Maths::HVec3( 0.0f,  1.0f,  0.0f));
		break;
	case Display::E_CubeTextureFace_NegY:
		SetCameraToWorld(m_Position, Maths::HVec3( 0.0f, -1.0f,  0.0f), Maths::HVec3( 0.0f,  0.0f,  1.0f));
		break;
	case Display::E_CubeTextureFace_NegZ:
		SetCameraToWorld(m_Position, Maths::HVec3( 0.0f,  0.0f, -1.0f), Maths::HVec3( 0.0f,  1.0f,  0.0f));
		break;
	}

	m_View[face].Setup(m_ViewSetup, m_Width, m_Width);
}

void C_CubeMapView::Gather()
{
	PERF_EVENT("Render.GatherCubeMap");

	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		PERF_EVENT("Render.GatherCubeMapFace");

		SetupView(Display::E_CubeTextureFace(i));
		GatherFace(i);
	}
}

void C_CubeMapView::Schedule()
{
	PERF_EVENT("Render.ScheduleCubeMap");

	// render the faces of the cube map
	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		PERF_EVENT("Render.ScheduleCubeMapFace");

		ScheduleFace(i);
	}
}

void C_CubeMapView::Render(bool save, const Util::C_String& fileName)
{
	PERF_EVENT("Render.RenderCubeMap");

	if (save)
	{
		Save(fileName);
		Create();
	}

	// bind the current view
	Bind();

	// render the faces of the cube map
	for(U32 i=0; i<Display::E_CubeTextureFace_Count; i++)
	{
		PERF_EVENT("Render.RenderCubeMapFace");

		const Display::E_CubeTextureFace face = (Display::E_CubeTextureFace)i;

		BindFace(face);	// sets the rendertarget
		RenderFace(i);
		UnbindFace(face);	// this does the resolve
	}

	// unbind the current view
	Unbind();
}

}

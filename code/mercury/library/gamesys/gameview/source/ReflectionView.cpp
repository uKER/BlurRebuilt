#include "GameViewPch.h"
#include "GameView/ReflectionView.h"
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>
#include "FX/FX.h"

namespace GameView
{

C_ReflectionView::C_ReflectionView() :
	m_pRenderTexture(NULL),
	m_Width(256),
	m_Height(256),
	m_Multisample(Display::E_Multisample_None),
	m_NumMips(1)
{
	m_View.m_UserIndex = 0;
	m_View.m_Type = Display::E_ViewType_3D;
}

C_ReflectionView::~C_ReflectionView()
{
	Destroy();
}

void C_ReflectionView::SetFormat(const U32 width, const U32 height, const Display::E_Multisample multiSample, const U32 numMips)
{
	m_Width = width;
	m_Height = height;
	m_Multisample = multiSample;
	m_NumMips = numMips;
#if PLATFORM_PC
	// [Blur2PC] Retail PC (Blur.exe 1.2 0x00b20450) also hands the new format to an existing render target
	// (with one mip whatever numMips says); it takes effect when the device next recreates its volatile resources.
	if(m_pRenderTexture)
		m_pRenderTexture->SetFormat(width, height, multiSample);
#endif
}

void C_ReflectionView::Create()
{
	if(!m_pRenderTexture)
	{
		RESOURCE_MEMORY_ID("OffscreenRender");

		m_pRenderTexture = new Render::C_TextureRender(m_Width, m_Height, m_Multisample, m_NumMips);
		
		if(!m_pRenderTexture->Create() || !m_pRenderTexture->CreateVolatile())
		{
			SAFE_DELETE_PTR(m_pRenderTexture);
			//LOGWARN(Game::g_Log, "Failed to create the cube texture render\n");
		}
	}
}

void C_ReflectionView::Destroy()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->DestroyVolatile();
		m_pRenderTexture->Destroy();
		delete m_pRenderTexture;
		m_pRenderTexture = NULL;
	}

}

void C_ReflectionView::Bind()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->Bind();
		Display::Device()->SetView(&m_View);
	}
}

void C_ReflectionView::Unbind()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->Unbind();
	}	
}


void C_ReflectionView::DebugRender()
{
	if(m_pRenderTexture)
	{
		m_pRenderTexture->DebugRender();
	}
}

void C_ReflectionView::SetupView( const Display::C_ViewSetup& viewSetup, const Maths::C_Plane reflectionPlane, const F32 fFovExpand )
{
	m_ViewSetup = viewSetup;
	
	//Reflect camera	
	Maths::HMat44 matrixReflect(reflectionPlane);

	//Inverse Z, triangles get flipped after reflection
	Maths::HMat43 cameraMatrixZInversed = m_ViewSetup.m_ViewSetup3D.m_CameraToWorld;
	cameraMatrixZInversed.AY( -cameraMatrixZInversed.AY());
	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43(cameraMatrixZInversed * matrixReflect);

	//Change FOV
	m_ViewSetup.m_ViewSetup3D.m_PerspDist /= (1.f + fFovExpand);

	m_View.Setup(m_ViewSetup, m_Width, m_Height);

	m_ReflectionPlane = reflectionPlane;
}

Render::C_TextureRender* C_ReflectionView::GetTextureRender()
{
	return m_pRenderTexture;
}

const Display::C_Matrix& C_ReflectionView::GetReflectedViewMatrix()
{
	return m_View.GetViewSettings3D().GetViewMatrix();
}

}

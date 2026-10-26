#include "stdafx.h"

#include <EditorApp/Editor/HUDElements.h>
#include <EditorApp/Editor/RenderViewport.h>

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>

#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Shaders/Constants.h>

#include <shaders/vsConstants.h>
#include <shaders/psConstants.h>

namespace Editor
{
namespace HUDElements
{

void RenderOrientationAxes(const C_RenderViewport *pViewport, const wxPoint &pos, U32 size)
{
	const Display::Vertex_XYZ_C vertices[6] = 
	{
		{0.f, 0.f, 0.f, 0xffff0000},
		{1.f, 0.f, 0.f, 0xffff0000},
		{0.f, 0.f, 0.f, 0xff00ff00},
		{0.f, 1.f, 0.f, 0xff00ff00},
		{0.f, 0.f, 0.f, 0xff0000ff},
		{0.f, 0.f, 1.f, 0xff0000ff}
	};

	const AftkRender::C_ViewContext::View view = pViewport->GetViewContext().GetView();
	const AftkRender::C_Camera& camera = pViewport->GetCamera();
	Maths::HVec3 position;
	{
		Maths::HVec3 toZ= camera.GetFocusPosition()-camera.GetPosition();
		toZ.Normalise();
		position = camera.GetPosition() + 0.5f*(camera.GetNearClipDistance()+camera.GetFarClipDistance()) * toZ;
	}
	F32 scale;
	{
		if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
			scale = tanf(Maths::DegToRad(0.5f*camera.GetFieldOfView())) * 0.5f*(camera.GetNearClipDistance()+camera.GetFarClipDistance());
		else
			scale = 0.5f*camera.GetHeightExtent();
	}

	ApplyView(pViewport, Maths::C_Rect(Maths::HVec2(pos.x, pos.y), Maths::HVec2(pos.x+size, pos.y+size)));


	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	worldMatrix._11 *= scale;
	worldMatrix._22 *= scale;
	worldMatrix._33 *= scale;
	worldMatrix._41 = position[0];
	worldMatrix._42 = position[1];
	worldMatrix._43 = position[2];


	Display::BasicRender()->BindColoured3D();
	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);


	Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*) Display::Device()->BeginVertices(
		Display::E_PrimitiveType_LineList, 6, sizeof(Display::Vertex_XYZ_C), 
		Display::Vertex_XYZ_C::m_pDeclaration, true);

	if (!pVertex)
		return;

	memcpy(pVertex, vertices, sizeof(Display::Vertex_XYZ_C)*6);
	pVertex += 6;
	Display::Device()->EndVertices(pVertex);

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
	//calculate where to render the axis labels
	Maths::HVec3 pX = viewSettings3D.TransformWorldToWindow(position + Maths::HVec3(scale, 0.f, 0.f));
	Maths::HVec3 pY = viewSettings3D.TransformWorldToWindow(position + Maths::HVec3(0.f, scale, 0.f));
	Maths::HVec3 pZ = viewSettings3D.TransformWorldToWindow(position + Maths::HVec3(0.f, 0.f, scale));

	//revert viewport changes
	RevertView(pViewport);


	//Render text
	AftkRender::C_FontRenderer *pFontRenderer = &C_RenderCore::Instance().GetRenderPipeline().GetFontRenderer();
	{
		pFontRenderer->Begin();
		RECT rX = pFontRenderer->GetRect("Arial", 10, "x", 0, 0);
		pFontRenderer->Draw("Arial", 10, "x", pX[0]-0.5f*rX.right, pX[1]-rX.bottom, Maths::RGBA(255,0,0,255));
	
		RECT rY = pFontRenderer->GetRect("Arial", 10, "y", 0, 0);
		pFontRenderer->Draw("Arial", 10, "y", pY[0]-0.5f*rY.right, pY[1]-rY.bottom, Maths::RGBA(0,255,0,255));
	
		RECT rZ = pFontRenderer->GetRect("Arial", 10, "z", 0, 0);
		pFontRenderer->Draw("Arial", 10, "z", pZ[0]-0.5f*rZ.right, pZ[1]-rZ.bottom, Maths::RGBA(0,0,255,255));
		pFontRenderer->End();
	}
}


void ApplyView(const C_RenderViewport *pViewport, const Maths::C_Rect region)
{
	wxSize viewportSize = pViewport->GetSize();

	const AftkRender::C_ViewContext::View& view = pViewport->GetViewContext().GetView();
	const AftkRender::C_Camera& camera = view.camera;

	//tweak the current view settings so that the rendering is performed in the specified viewport region
	Display::C_View* pDisplayView = Display::Device()->GetView(view.viewIndex);
	if (!pDisplayView)
		return;

	Display::C_ViewSetup setup;
	{
		//Initialise a viewSetup object
		setup.m_WindowCoords = region;

		setup.m_ViewSetup2D.m_ViewExtents = region;

		const Maths::HVec3& position = camera.GetPosition();
		const Maths::HVec3& direction = camera.GetFocusPosition() - camera.GetPosition();

		Maths::HVec3 toX, toY, toZ;
		Maths::HMat33 lookMatrix;
		toZ = direction;
		toZ.Normalise();

		toX = camera.GetUpVector()^toZ;
		toX.Normalise();

		toY = toZ^toX;
		toY.Normalise();

		lookMatrix.AX(toX);
		lookMatrix.AY(toY);
		lookMatrix.AZ(toZ);

		setup.m_ViewSetup3D.m_AspectRatio = region.Size()[0]/region.Size()[1];
		setup.m_ViewSetup3D.m_CameraToWorld = Maths::HMat43(lookMatrix);
		setup.m_ViewSetup3D.m_CameraToWorld.AW(position);
		setup.m_ViewSetup3D.m_FieldOfView = camera.GetFieldOfView();
		setup.m_ViewSetup3D.m_NearClip = camera.GetNearClipDistance();
		setup.m_ViewSetup3D.m_FarClip = camera.GetFarClipDistance();
		setup.m_ViewSetup3D.m_PerspDist = camera.GetFarClipDistance();

		F32 h;
		if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Orthographic)
		{
			setup.m_ViewSetup3D.m_PerspDist = 0.f;
			h = 0.5f*camera.GetHeightExtent();
			if (h<Maths::Constants<F32>::Epsilon())
				h = Maths::Tan(Maths::DegToRad(camera.GetFieldOfView())*0.5f);
		}
		else
			h = camera.GetFarClipDistance()*Maths::Tan(Maths::DegToRad(camera.GetFieldOfView())*0.5f);

		F32 w = h*setup.m_ViewSetup3D.m_AspectRatio;
		setup.m_ViewSetup3D.m_ViewExtents.SetBounds(-w, -h, w, h);
	}

	//setup the view...
	pDisplayView->Setup(setup, viewportSize.x, viewportSize.y);

	//now need to reset the viewport to avoid non-uniform scaling due to fixed device coords (C_ViewSetup::GetDeviceCoords())
	{
		Maths::C_Transform2d deviceTransformInv(Maths::C_Rect(Maths::HVec2(0.0f, 0.0f), Maths::HVec2(viewportSize.x, viewportSize.y)));
		deviceTransformInv.Invert();

		Maths::C_Transform2d windowTransform(setup.m_WindowCoords);
		Maths::C_Transform2d viewportTransform(deviceTransformInv * windowTransform);

		pDisplayView->m_Viewport.m_X		= U32(0.5f + F32(viewportSize.x) * viewportTransform.Offset().X());
		pDisplayView->m_Viewport.m_W		= U32(0.5f + F32(viewportSize.x) * viewportTransform.Scale().X());
		pDisplayView->m_Viewport.m_Y		= U32(0.5f + F32(viewportSize.y) * viewportTransform.Offset().Y());
		pDisplayView->m_Viewport.m_H		= U32(0.5f + F32(viewportSize.y) * viewportTransform.Scale().Y());
		pDisplayView->m_Viewport.m_MinZ	= 0.0f;
		pDisplayView->m_Viewport.m_MaxZ	= 1.0f;
	}

	//reapply the view...
	Display::Device()->SetView(pDisplayView, true);
}


void RevertView(const C_RenderViewport *pViewport)
{
	pViewport->GetViewContext().BeginRender();
}



}
}
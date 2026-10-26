#include "stdafx.h"
#include "Display/View/View.h"
#include "Display/Device/Device.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

C_View::C_View() :
	m_Index(-1),
	m_Type(E_ViewType_2D | E_ViewType_3D),
	m_UserIndex(-1)
{
}

C_View::C_View(const S32 index, const U32 type, const S32 userIndex) :
	m_Index(index),
	m_Type(type),
	m_UserIndex(userIndex)
{
}

C_View::~C_View()
{
}

void C_View::Setup(const C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight)
{
	SetViewport(viewSetup, deviceWidth, deviceHeight);

	if(m_Type & E_ViewType_2D)
	{
		SetViewSettings2D(viewSetup, deviceWidth, deviceHeight);
	}

	if(m_Type & E_ViewType_3D)
	{
		SetViewSettings3D(viewSetup);
	}
}

void C_View::SetViewport(const C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight)
{
	Maths::C_Transform2d deviceTransformInv(viewSetup.GetDeviceCoords());
	deviceTransformInv.Invert();

	Maths::C_Transform2d windowTransform(viewSetup.m_WindowCoords);
	
	Maths::C_Transform2d viewportTransform(deviceTransformInv * windowTransform);

	m_Viewport.m_X		= U32(0.5f + F32(deviceWidth) * viewportTransform.Offset().X());
    m_Viewport.m_W		= U32(0.5f + F32(deviceWidth) * viewportTransform.Scale().X());
	m_Viewport.m_Y		= U32(0.5f + F32(deviceHeight) * viewportTransform.Offset().Y());
	m_Viewport.m_H		= U32(0.5f + F32(deviceHeight) * viewportTransform.Scale().Y());
	m_Viewport.m_MinZ	= 0.0f;
	m_Viewport.m_MaxZ	= 1.0f;
}

void C_View::SetViewSettings2D(const C_ViewSetup& viewSetup, const U32 deviceWidth, const U32 deviceHeight)
{
	Maths::C_Rect deviceBounds(Maths::HVec2(0, 0), Maths::HVec2((F32)deviceWidth, (F32)deviceHeight));
    Maths::C_Transform2d deviceTransform = Maths::C_Transform2d(deviceBounds);

	Maths::C_Rect shaderBounds(Maths::HVec2(-1.0f, 1.0f), Maths::HVec2(1.0f, -1.0f));
    Maths::C_Transform2d shaderTransform = Maths::C_Transform2d(shaderBounds);
    
    Maths::C_Transform2d deviceWindowTransformInv = Maths::C_Transform2d(viewSetup.GetDeviceCoords()); // old position?
    deviceWindowTransformInv.Invert();

    Maths::C_Transform2d windowTransform = Maths::C_Transform2d(viewSetup.m_WindowCoords); // old position coordinate range?

    Maths::C_Transform2d viewportTransformInv = Maths::C_Transform2d(viewSetup.m_ViewSetup2D.m_ViewExtents);
    viewportTransformInv.Invert();

	m_ViewSettings2D.m_ViewExtents		= viewSetup.m_ViewSetup2D.m_ViewExtents;
    m_ViewSettings2D.m_DeviceTransform	= deviceTransform * deviceWindowTransformInv * windowTransform * viewportTransformInv;
	m_ViewSettings2D.m_ShaderTransform	= shaderTransform * viewportTransformInv;
	m_ViewSettings2D.m_ClipTransform.x	= m_ViewSettings2D.m_ShaderTransform.Scale().X();
	m_ViewSettings2D.m_ClipTransform.y	= m_ViewSettings2D.m_ShaderTransform.Scale().Y();
	m_ViewSettings2D.m_ClipTransform.z	= m_ViewSettings2D.m_ShaderTransform.Offset().X();
	m_ViewSettings2D.m_ClipTransform.w	= m_ViewSettings2D.m_ShaderTransform.Offset().Y();
}

void C_View::SetViewSettings3D(const C_ViewSetup& viewSetup)
{
	const C_ViewSetup3D& vs3d = viewSetup.m_ViewSetup3D;

	// set previous data

	m_ViewSettings3D.m_ViewPositionPrev = m_ViewSettings3D.m_ViewPosition;
	m_ViewSettings3D.m_ViewMatrixPrev = m_ViewSettings3D.m_ViewMatrix;
	m_ViewSettings3D.m_ProjMatrixPrev = m_ViewSettings3D.m_ProjMatrix;
	m_ViewSettings3D.m_ViewProjMatrixPrev = m_ViewSettings3D.m_ViewProjMatrix;
	m_ViewSettings3D.m_CameraToWorldPrev = m_ViewSettings3D.m_CameraToWorld;
	m_ViewSettings3D.m_CameraToDevicePrev = m_ViewSettings3D.m_CameraToDevice;
	m_ViewSettings3D.m_WorldToCameraPrev = m_ViewSettings3D.m_WorldToCamera;
	m_ViewSettings3D.m_WorldToDevicePrev = m_ViewSettings3D.m_WorldToDevice;

	// set the camera to world matrix

	m_ViewSettings3D.m_CameraToWorld = vs3d.m_CameraToWorld;
	m_ViewSettings3D.m_ViewPosition = MathsStorage::Float4(m_ViewSettings3D.m_CameraToWorld.AW().X(), m_ViewSettings3D.m_CameraToWorld.AW().Y(), m_ViewSettings3D.m_CameraToWorld.AW().Z(), 0.f);

	// calculate the world to camera matrix

	m_ViewSettings3D.m_WorldToCamera = m_ViewSettings3D.m_CameraToWorld.InverseRotTrans();
	Maths::Store(Maths::HMat44(m_ViewSettings3D.m_WorldToCamera), &m_ViewSettings3D.m_ViewMatrix);
	
	// calculate the perspective projection matrix
	Maths::HMat44 projection;
	CalculateProjectionMatrix(vs3d, projection);

	// calculate the view scale

	Maths::HMat44 viewScale;
	{
		F32 viewXMin = vs3d.m_ViewExtents.Min().X();
	    F32 viewXMax = vs3d.m_ViewExtents.Max().X();

	    F32 viewYMin = vs3d.m_ViewExtents.Min().Y();
	    F32 viewYMax = vs3d.m_ViewExtents.Max().Y();

		F32 xs = 2.0f / (viewXMax - viewXMin);
	    F32 ys = 2.0f / (viewYMax - viewYMin);

	    F32 xo = -viewXMin * xs - 1.0f;
	    F32 yo = -viewYMin * ys - 1.0f;

	    viewScale.AX(Maths::HVec4( xs, 0.f, 0.f, 0.f));
	    viewScale.AY(Maths::HVec4(0.f,  ys, 0.f, 0.f));
	    viewScale.AZ(Maths::HVec4(0.f, 0.f, 1.f, 0.f));
		viewScale.AW(Maths::HVec4( xo,  yo, 0.f, 1.f));
	}

	// calculate the camera to device matrix

	m_ViewSettings3D.m_CameraToDevice = projection * viewScale;
	Maths::Store(m_ViewSettings3D.m_CameraToDevice, &m_ViewSettings3D.m_ProjMatrix);

	// calculate the world to device matrix

	m_ViewSettings3D.m_WorldToDevice = Maths::HMat44(m_ViewSettings3D.m_WorldToCamera) * m_ViewSettings3D.m_CameraToDevice;
	Maths::Store(m_ViewSettings3D.m_WorldToDevice, &m_ViewSettings3D.m_ViewProjMatrix);

	// calculate world to window matrix
	CalculateWorldToWindow(viewSetup, projection);

	// set the view settings

	m_ViewSettings3D.m_ViewExtents = vs3d.m_ViewExtents;
	m_ViewSettings3D.m_AspectRatio = vs3d.m_WindowProportion * vs3d.m_AspectRatio;
	m_ViewSettings3D.m_FieldOfView = vs3d.m_FieldOfView;
	m_ViewSettings3D.m_PerspDist = vs3d.m_PerspDist;
	m_ViewSettings3D.m_NearClip = vs3d.m_NearClip;
	m_ViewSettings3D.m_FarClip = vs3d.m_FarClip;

	// setup frustum

	Maths::HF32 windowWidth = viewSetup.m_WindowCoords.Size().X() / viewSetup.GetDeviceCoords().Size().X();

	m_ViewFrustum.Setup(m_ViewSettings3D.m_CameraToDevice, m_ViewSettings3D.m_WorldToDevice, windowWidth);
}

void C_View::CalculateProjectionMatrix(const C_ViewSetup3D& vs3d, Maths::HMat44& projection)
{
	// calculate the perspective projection matrix
	if(vs3d.m_PerspDist > 0.0f)
	{
		F32 z_near, z_far;
		// perspective projection
		if(Display::Device()->GetDepthFlip())
		{
			z_far = vs3d.m_NearClip;
			z_near = vs3d.m_FarClip;
		}
		else
		{
			z_near = vs3d.m_NearClip;
			z_far = vs3d.m_FarClip;
		}
		
		F32 Q = z_far / (z_far - z_near);
        F32 ppd = vs3d.m_PerspDist;

		projection.AX(Maths::HVec4(ppd, 0.f, 0.f, 0.f ));
		projection.AY(Maths::HVec4(0.f, ppd, 0.f, 0.f ));
		projection.AZ(Maths::HVec4(0.f, 0.f, Q, 1.f));
		projection.AW(Maths::HVec4(0.f, 0.f, -Q * z_near, 0.f));
	}
	else
	{
		// orthographic projection
		F32 z_near, z_far;
		if (Display::Device()->GetDepthFlip())
		{
			z_near = vs3d.m_FarClip;
			z_far = vs3d.m_NearClip;
		}
		else
		{
			z_near = vs3d.m_NearClip;
			z_far = vs3d.m_FarClip;
		}

		F32 Q = z_far / (z_far - z_near);
		F32 ppd = vs3d.m_PerspDist;

		projection.AX(Maths::HVec4(1.0f, 0.f, 0.f, 0.f ));
		projection.AY(Maths::HVec4(0.f, 1.0f, 0.f, 0.f ));
		projection.AZ(Maths::HVec4(0.f, 0.f, 1.0f/(z_far - z_near), 0.0f));
		projection.AW(Maths::HVec4(0.f, 0.f, z_near/(z_near - z_far), 1.0f));
	}
}

void C_View::CalculateWorldToWindow(const C_ViewSetup& viewSetup, const Maths::HMat44& projection)
{
	// calculate the unit scale matrix

	Maths::HMat44 unitScale;
    {
		F32 viewXMin = viewSetup.m_ViewSetup3D.m_ViewExtents.Min().X();
        F32 viewXMax = viewSetup.m_ViewSetup3D.m_ViewExtents.Max().X();

        F32 viewYMin = viewSetup.m_ViewSetup3D.m_ViewExtents.Min().Y();
        F32 viewYMax = viewSetup.m_ViewSetup3D.m_ViewExtents.Max().Y();

        F32 xs = 1.0f / (viewXMax - viewXMin);
        F32 ys = 1.0f / (viewYMax - viewYMin);

        F32 xo = -viewXMin * xs;
        F32 yo = -viewYMin * ys;

		unitScale.AX(Maths::HVec4( xs, 0.f, 0.f, 0.f));
        unitScale.AY(Maths::HVec4(0.f,  ys, 0.f, 0.f));
        unitScale.AZ(Maths::HVec4(0.f, 0.f, 1.f, 0.f));
        unitScale.AW(Maths::HVec4( xo,  yo, 0.f, 1.f));
    }

	// calculate the unit to window matrix
    
    Maths::HMat44 unitToWindow;
    {
        F32 windowXMin = viewSetup.m_ViewSetup2D.m_ViewExtents.Min().X();
        F32 windowXMax = viewSetup.m_ViewSetup2D.m_ViewExtents.Max().X();

        F32 windowYMin = viewSetup.m_ViewSetup2D.m_ViewExtents.Max().Y();
        F32 windowYMax = viewSetup.m_ViewSetup2D.m_ViewExtents.Min().Y();

        F32 xs = (windowXMax - windowXMin);
        F32 ys = (windowYMax - windowYMin);

        F32 xo = windowXMin;
        F32 yo = windowYMin;

        unitToWindow.AX(Maths::HVec4( xs, 0.f, 0.f, 0.f));
        unitToWindow.AY(Maths::HVec4(0.f,  ys, 0.f, 0.f));
        unitToWindow.AZ(Maths::HVec4(0.f, 0.f, 1.f, 0.f));
        unitToWindow.AW(Maths::HVec4( xo,  yo, 0.f, 1.f));
    }

	// calculate the world to window matrix

	m_ViewSettings3D.m_WorldToWindow = Maths::HMat44(m_ViewSettings3D.m_WorldToCamera) * ((projection * unitScale) * unitToWindow);
}

void C_View::SetPreviousFromCurrent()
{
	// set all the previous settings
	m_ViewSettings3D.m_ViewMatrixPrev = m_ViewSettings3D.m_ViewMatrix;
	m_ViewSettings3D.m_ProjMatrixPrev = m_ViewSettings3D.m_ProjMatrix;
	m_ViewSettings3D.m_ViewProjMatrixPrev = m_ViewSettings3D.m_ViewProjMatrix;

	m_ViewSettings3D.m_CameraToWorldPrev = m_ViewSettings3D.m_CameraToWorld;
	m_ViewSettings3D.m_CameraToDevicePrev = m_ViewSettings3D.m_CameraToDevice;
	m_ViewSettings3D.m_WorldToCameraPrev = m_ViewSettings3D.m_WorldToCamera;
	m_ViewSettings3D.m_WorldToDevicePrev = m_ViewSettings3D.m_WorldToDevice;
}

}; // namespace Display

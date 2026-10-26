
#include "stdafx.h"
#include <camera2/Feed.h>
#include <camera2/Camera.h>
#include <camera2/CameraType.h>
#include <camera2/stock/TvFeed.h>
#if !CFG_CONVERTER
#include <Display.h>
#endif

namespace Cam2
{

static F32 g_ShutterEnd = 1.f;
// [Blur2PC] render interpolation: shifts the view between the camera's previous and current
// update poses (0 = current pose; -1 = previous). Set around each render by the PC loop.
static F32 g_Blur2PCRenderOffset = 0.f;
void I_Feed::SetRenderOffset(F32 offset) { g_Blur2PCRenderOffset = offset; }
static U32 g_Blur2PCTick = 1;
void I_Feed::Blur2PCAdvanceTick() { ++g_Blur2PCTick; }
U32 I_Feed::Blur2PCTick() { return g_Blur2PCTick; }
static Debug::Float g_MatrixZoom("Camera2.PhotoMode.MatrixZoom", 1.f, Debug::Range(0.1f, 5.f));
static Debug::Float g_MatrixDeflectX("Camera2.PhotoMode.MatrixDeflectX", 0.f, Debug::Range(-5.f, 5.f));
static Debug::Float g_MatrixDeflectY("Camera2.PhotoMode.MatrixDeflectY", 0.f, Debug::Range(-5.f, 5.f));
static F32 g_HeightScale = 1.f;
static F32 g_StrafeMoveX = 0.f, g_StrafeMoveY = 0.f;
static F32 g_StrafeAngleX = 0.f, g_StrafeAngleY = 0.f;

I_Feed::I_Feed()
{
	m_type = 0;
	m_ScreenOffsetX = m_ScreenOffsetY = 0.0f;
}

Maths::HVec3 I_Feed::Position() const
{
	return Live().Position();
}

Maths::C_Quaternion I_Feed::Orient() const
{
	return Live().Orient();
}

Maths::C_Quaternion I_Feed::Shaken() const
{
	return Live().Shaken();
}

Maths::HMat43 I_Feed::Mat43() const
{
	return Maths::HMat43(Shaken(), Position());
}

F32 I_Feed::Fov() const
{
	return Live().Fov();
}

// -----------------------------------------------

const C_Camera& C_Feed::Live() const
{
	if (m_feed)
		return m_feed->Live();
	return Cam2::NullCamera;
}


#if !CFG_CONVERTER
static F32 CalculateNearClip(F32 fov)
{
	// Adjust near clip outwards if the camera is very zoomed in:
	F32 nearClip = 0.1f;
	if (fov > 1e-5f)
		nearClip += 0.01745f / fov;
	return nearClip;
}

#endif

// Copy the camera matrix to a C_ViewSetup structure for the Display library
void I_Feed::ToViewSetup(Display::C_ViewSetup3D& vs, bool blurEnd) const
{
#if !CFG_CONVERTER
	assert(this);

	const C_Camera& camera = Live();
	
	// Get matrix at start / end of motion blur:

	F32 timeVal = g_ShutterEnd;
	if (!blurEnd)
		timeVal -= camera.ShutterAngle();

	vs.m_CameraToWorld = camera.InterMat43(timeVal);
	F32 fov = camera.InterFov(timeVal);

	// [Blur2PC] render interpolation: for a camera updated on the latest tick, move the view from the
	// latest pose to the pose at the render fraction between the previous and the latest update. The
	// shift is applied on top of the motion-blur pose above, whose 'previous' values are blur inputs.
	if (camera.m_Blur2PCTick == g_Blur2PCTick && g_Blur2PCRenderOffset != 0.f)
	{
		const F32 t = 1.f + g_Blur2PCRenderOffset;
		const Maths::HVec3 renderPos = camera.m_Blur2PCPrevPosition + (camera.Position() - camera.m_Blur2PCPrevPosition) * t;
		const Maths::C_Quaternion renderShaken = camera.m_Blur2PCPrevShaken.ShortestSlerp(camera.Shaken(), t);
		const Maths::HMat43 latest(camera.Shaken(), camera.Position());
		const Maths::HMat43 render(renderShaken, renderPos);
		vs.m_CameraToWorld = vs.m_CameraToWorld * (latest.InverseRotTrans() * render);
		fov += (camera.m_Blur2PCPrevFov + (camera.Fov() - camera.m_Blur2PCPrevFov) * t) - camera.Fov();
	}

	// Strafe matrix to create depth of field in Photo Mode:
	HVec3 strafe((F32)g_StrafeMoveX, (F32)g_StrafeMoveY, 0.f);
	strafe = strafe * vs.m_CameraToWorld.Rotation();
	vs.m_CameraToWorld.AW( vs.m_CameraToWorld.AW() + strafe );

	vs.m_PerspDist = 1024.f;

	F32 fovScale = 1.f;

	F32 width = 0.f, height = 0.f;

	// Update the global aspect ratio:
	vs.m_AspectRatio = Display::Device()->GetAspectRatio();

	if (fov >= 0.f)
	{
		vs.m_FieldOfView = fov;
		height = tanf(fov * 0.5f) * vs.m_PerspDist;
		width = height * vs.m_WindowProportion * vs.m_AspectRatio;
	}
	else
	{
		// Orthographic:
		vs.m_PerspDist = 0.f;
		height = -fov;
		width = height * vs.m_WindowProportion * vs.m_AspectRatio;
	}

	height *= g_HeightScale;

	if (g_MatrixZoom > 1e-5f)
	{
		width  /= g_MatrixZoom;
		height /= g_MatrixZoom;
	}

	F32 ox = (m_ScreenOffsetX + g_MatrixDeflectX) * width;
	F32 oy = (m_ScreenOffsetY + g_MatrixDeflectY) * height;
	
	ox += g_StrafeAngleX * vs.m_PerspDist;
	oy += g_StrafeAngleY * vs.m_PerspDist;
	
	vs.m_ViewExtents.SetBounds(ox - width , oy - height, ox + width, oy + height);

	if (camera.NearClip() == 0.f)
		vs.m_NearClip = CalculateNearClip(fov); // if not set, calculate based on field of view
	else
		vs.m_NearClip = camera.NearClip();
	
	vs.m_FarClip = camera.FarClip();
#endif
}


void I_Feed::DeflectMatrix(F32 zoom, F32 dx, F32 dy, F32 heightScale)
{
	g_MatrixZoom = zoom;
	g_MatrixDeflectX = dx;
	g_MatrixDeflectY = dy;
	g_HeightScale = heightScale;
}

void I_Feed::SetShutterEnd(F32 end)
{
	g_ShutterEnd = end;
}

void I_Feed::StrafeMatrix(F32 movex, F32 movey, F32 anglex, F32 angley)
{
	g_StrafeMoveX = movex;
	g_StrafeMoveY = movey;
	g_StrafeAngleX = anglex;
	g_StrafeAngleY = angley;
}

} // End of namespace Cam2

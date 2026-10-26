 
#include "stdafx.h"
#include "camera2/CameraType.h"
#include "camera2/stock/AnimCamera.h"
#include "camera2/tv/AnimData.h"
#include <system/debug/I_DebugRender.h>

namespace Cam2
{

static Debug::Bool g_AnimRepeat("Camera2.Anim.Repeat", true);
static Debug::Bool g_AnimRestart("Camera2.Anim.Restart", false);

C_AnimCamera::C_AnimCamera()
{
	SetType(E_AnimCamera);
	SetName("Anim");
	m_animData = 0;
	m_length = m_counter = 0.f;
}

C_AnimCamera::~C_AnimCamera()
{
}

void C_AnimCamera::OnUpdate()
{
	if (m_animData == 0)
		return;

	if (g_AnimRestart)
	{
		m_counter = 0.0f;
		g_AnimRestart = false;
	}

	m_evalCamera = Evaulate(m_counter);

	SetPosition(m_evalCamera.Position());
	StandardOrientFov();

	// Increase animation counter:
	m_counter += Period();
	if (m_counter >= m_length)
	{
		if (g_AnimRepeat)
			m_counter -= m_length;
		else
			m_counter = m_length;
	}
}

C_Camera C_AnimCamera::Evaulate(F32 ctr) const
{
	C_Camera camera;
	if (m_animData == 0)
		return camera;

	S32 max = m_animData->m_frame.Size();
	if (max <= 0)
		return camera;

	Maths::HVec3 position;
	Maths::C_Quaternion orient;

	m_animData->EvaluatePosition(ctr, position, orient);
	camera.SetPosition( position );
	camera.SetOrient  ( orient );

	F32 fov = m_animData->EvaluateAttribute(m_counter, E_Attr_Fov);
	camera.SetFov     ( fov );
	
	return camera;
}

void C_AnimCamera::StandardOrientFov()
{
	F32 objectLook = m_animData->EvaluateAttribute(m_counter, E_Attr_ObjectLook);
	
	if (objectLook > 0.f)
	{
		Maths::HVec3 relative = Object().position - m_evalCamera.Position();
		Maths::C_Quaternion lookOrient = GetLookQuat(relative);

		Maths::C_Quaternion orient;
		if (objectLook >= 1.f)
			orient = lookOrient;
		else
			orient = m_evalCamera.Orient().ShortestLerp(lookOrient, objectLook);

		orient.Normalise();
		
		SetOrient(orient);
	}
	else
	{
		SetOrient(m_evalCamera.Orient());
	}

	SetFov(m_evalCamera.Fov());
}

void C_AnimCamera::SetAnimData(const C_AnimData* anim)
{
	m_animData = anim;
	m_length = m_animData->CalcLength();
	m_counter = 0.f;
	Update(0.f);
}

const Util::C_String& C_AnimCamera::Name() const
{
	static Util::C_String nullString;
	
	if (m_animData)
		return m_animData->m_name;
	else
		return nullString;
}

void C_AnimCamera::DebugRender(Maths::RGBA colour, bool renderFrustum) const
{
	if (m_animData == 0)
		return;

	C_Camera::DebugRender(colour, renderFrustum);
	DrawPath(colour);

	DbUtil::I_DebugRender* debugRender = GetDebugRender();
	
	debugRender->Draw3dText(Position(), 1.f, Name());
}

// Draw the path of the camera animation
void C_AnimCamera::DrawPath(Maths::RGBA colour) const
{
	if (m_animData == 0)
		return;

	DbUtil::I_DebugRender* debugRender = GetDebugRender();

	colour.A(colour.A() >> 2);

	Maths::HVec3 lastPos;
	F32 ctr = 0.f;
	for (ctr = 0.f; ctr < m_length; ctr += 0.5f)
	{
		Maths::HVec3 pos = Evaulate(ctr).Position();
		
		if (ctr > 0.f)
			debugRender->Draw3dLine(lastPos, pos, colour);

		lastPos = pos;
	}
}

} // End of namespace Cam2

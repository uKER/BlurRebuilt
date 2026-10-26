
#include "StdAfx.h"
#include <Framework/Application/UserControls.h>

#pragma warning (push, 4)

#include "BoxCar.h"

// A debug BoxCar, so I can attach a chase camera to it

namespace Framework
{

C_BoxCar::C_BoxCar()
{
	m_right = m_forw = m_steering = m_rise = 0.f;
	m_activeControls = 1;
}

static void DrawLine(const Maths::HVec3& start, const Maths::HVec3& end, Maths::RGBA col)
{
	Display::BasicRender()->DrawColoured3DLine(start, end, col);
}

static void DrawBox(const Maths::HVec3& centre, const Maths::HVec3& size, Maths::RGBA colour, const Maths::C_Quaternion& orient)
{
	Maths::HVec3 p[8] = {Maths::HVec3()};
	
	Maths::HMat33 mat = (Maths::HMat33)orient;
	
	int i = 0;
	for (i = 0; i < 8; i++)
	{
		p[i] = centre;
		p[i] += mat.AX() * size.X() * (i & 1 ? 1.f : -1.f);
		p[i] += mat.AY() * size.Y() * (i & 2 ? 1.f : -1.f);
		p[i] += mat.AZ() * size.Z() * (i & 4 ? 1.f : -1.f);
	}

	for (i=0; i<4; i++ ) DrawLine(p[i], p[i+4], colour);
	for (i=0; i<8; i+=2) DrawLine(p[i], p[i+1], colour);
	DrawLine(p[0], p[2], colour);
	DrawLine(p[1], p[3], colour);
	DrawLine(p[4], p[6], colour);
	DrawLine(p[5], p[7], colour);
}

void C_BoxCar::Update(F32 period)
{
	F32 rota = m_steering * period;
	F32 rotb = m_rise * 0.5f * period;
 
	m_orient = m_orient * Maths::C_Quaternion(Maths::HVec3(sin(rotb), 0.f, 0.f), Maths::HVecW(cos(rotb)));
	m_orient = Maths::C_Quaternion(Maths::HVec3(0.f, sin(-rota), 0.f), Maths::HVecW(cos(-rota))) * m_orient;
	m_orient.Normalise();

	Maths::HMat33 mat = m_orient;
	m_position += mat.AX() * m_right * period;
	m_position += mat.AZ() * m_forw * period;
}

void C_BoxCar::ReadControls()
{
	m_right = m_forw = m_steering = m_rise = 0.f;

	if (!m_activeControls)
		return;
	
	F32 speed = 1.5f * 44.7f;
	
	Input::C_GameInput* gameInput = Input::GetGameInput(0);
	m_forw  = gameInput->GetAxisSignedValue(E_Axis_FlyMoveZ);
	m_right = gameInput->GetAxisSignedValue(E_Axis_FlyMoveX);
	m_steering = gameInput->GetAxisSignedValue(E_Axis_FlyRotateX);
	m_rise = gameInput->GetAxisSignedValue(E_Axis_FlyRotateY);

	m_forw  *= speed; // 150 mph
	m_right *= speed;
}

void C_BoxCar::Render() const
{
	DrawBox(m_position, Maths::HVec3(1.f, 0.5f, 2.f), Maths::RGBA(255, 255, 255, 255), m_orient);
}

}

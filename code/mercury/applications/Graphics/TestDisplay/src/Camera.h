#ifndef CAMERA_H
#define CAMERA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Input.h"

struct C_Camera
{
	C_Camera();
	Maths::HVec3 GetAt() const;
	void Update(C_Input* pMoveInput, C_Input* pLookInput, const F32 elapsedTime);
	void UpdateDir(Maths::HVec3& dir, const F32 inc, const Maths::HVec3& axis);

	F32					m_AspectRatio;
	F32					m_FieldOfView;
	F32					m_NearClip;
	F32					m_FarClip;
	Maths::HVec3		m_Pos;
	Maths::HVec3		m_Dir;
	Maths::HVec3		m_Up;
};

#endif // CAMERA_H

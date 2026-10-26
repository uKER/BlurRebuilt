
#include "stdafx.h"

#include <camera2/Camera.h>
#include <camera2/CameraType.h>
#if !CFG_CONVERTER
#pragma warning (push, 3)
#include <system/debug/I_DebugRender.h>
#include <Display.h>
#pragma warning (pop)
#endif

// This is elements of the camera library which interact with the Display library
// This can (hopefully) be linked out if the Display library is not present, for example in a tool

namespace Cam2
{
using namespace Maths;

bool g_RenderDeferred = false;


DbUtil::I_DebugRender* GetDebugRender()
{
#if !CFG_CONVERTER
	DbUtil::I_DebugRender* debugRender = 0;
	if (g_RenderDeferred)
		debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
	else
		debugRender = DbUtil::DefaultDebugRenderSys()->Immediate();
	
	return debugRender;
#else
	return 0;
#endif
}

void C_Camera::DebugRender(RGBA colour, bool renderFrustum) const
{
	assert(this);
#if !CFG_CONVERTER && !defined(CFG_GOLD)
	HMat33 mat = (HMat33)Shaken();
	HVec3 lens = Position() + mat.AZ() * 0.2f;

	DbUtil::I_DebugRender* debugRender = GetDebugRender();
	debugRender->SetBlendMode(DbUtil::E_BlendMode_Alpha);
	
	F32 wide = tan(Fov() * 0.5f) * 0.2f;
	wide += 0.001f;
	HVec3 up = mat.AY() * wide;
	HVec3 side = mat.AX() * wide * 16.f / 9.f;

	HVec3 p[16] = {HVec3()};
	// Find the extent of the lens to draw a rectangle
	p[0] = lens - side - up;
	p[1] = lens - side + up;
	p[2] = lens + side + up;
	p[3] = lens + side - up;

	// 4 points at a 'film' point
	HVec3 film = Position() - mat.AZ() * 0.001f;
	S32 i = 0;
	for (i = 0; i < 4; i++)
		p[4 + i] = film + (p[i] - film) * 0.5f;
	
	// 4 points at the 'back' of the camera
	HVec3 back = Position() - mat.AZ() * 0.2f;
	p[ 8] = back - mat.AX() * 0.1f - mat.AY() * 0.1f;
	p[ 9] = back - mat.AX() * 0.1f + mat.AY() * 0.1f;
	p[10] = back + mat.AX() * 0.1f + mat.AY() * 0.1f;
	p[11] = back + mat.AX() * 0.1f - mat.AY() * 0.1f;
	
	for (i = 0; i < 4; i++)
	{
		S32 j = (i + 1) & 3;

		debugRender->Draw3dLine(p[    i], p[    j], colour);
		debugRender->Draw3dLine(p[4 + i], p[4 + j], colour);
		debugRender->Draw3dLine(p[8 + i], p[8 + j], colour);

		debugRender->Draw3dLine(p[    i], p[4 + i], colour);
		debugRender->Draw3dLine(p[4 + i], p[8 + i], colour);
	}

	if (renderFrustum)
	{
		// And then 4 vectors 'outwards'
		HVec3 out[4] = {HVec3()};

		for (i = 0; i < 4; i++)
		{
			out[i] = p[i] + (p[i] - film) * 100.f;
		}

		RGBA faint = colour;
		faint.A(faint.A() >> 2);

		for (i = 0; i < 4; i++)
		{
			S32 j = (i + 1) & 3;
			debugRender->Draw3dLine(p[i], out[i], faint);
			debugRender->Draw3dLine(out[i], out[j], faint);
		}
	}

#endif
}

} // End of namespace

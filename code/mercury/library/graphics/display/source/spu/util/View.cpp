#include "Core/Core.h"
#include "Core/Maths.h"
#include "Display/SPU/Util/View.h"

#ifndef PLATFORM_SPU
#include <cell/gcm.h>
#include <Display/Device/DeviceTypes.h>
#include <Display/View/View.h>
#include <Display/View/Viewport.h>
#endif



namespace Display
{
namespace Spu
{



#ifndef PLATFORM_SPU
C_View::C_View(const C_View &view)
{
	m_ScissorRect[0] = view.m_ScissorRect[0];
	m_ScissorRect[1] = view.m_ScissorRect[1];
	m_ScissorRect[2] = view.m_ScissorRect[2];
	m_ScissorRect[3] = view.m_ScissorRect[3];
	m_MinZ = view.m_MinZ;
	m_MaxZ = view.m_MaxZ;

	m_ViewportScales[0] = view.m_ViewportScales[0];
	m_ViewportScales[1] = view.m_ViewportScales[1];
	m_ViewportScales[2] = view.m_ViewportScales[2];
	m_ViewportScales[3] = view.m_ViewportScales[3];
	m_ViewportOffsets[0] = view.m_ViewportOffsets[0];
	m_ViewportOffsets[1] = view.m_ViewportOffsets[1];
	m_ViewportOffsets[2] = view.m_ViewportOffsets[2];
	m_ViewportOffsets[3] = view.m_ViewportOffsets[3];

	m_ViewProjMatrix = view.m_ViewProjMatrix;

	m_MultisampleType = CELL_GCM_SURFACE_CENTER_1;
}

C_View::C_View(const Display::C_View * const pView)
{
	m_ScissorRect[0] = pView->m_Viewport.m_X;
	m_ScissorRect[1] = pView->m_Viewport.m_Y;
	m_ScissorRect[2] = pView->m_Viewport.m_W;
	m_ScissorRect[3] = pView->m_Viewport.m_H;
	m_MinZ = pView->m_Viewport.m_MinZ;
	m_MaxZ = pView->m_Viewport.m_MaxZ;

	m_ViewportScales[0] = (F32) pView->m_Viewport.m_W * 0.5f;
	m_ViewportScales[1] = (F32) pView->m_Viewport.m_H * -0.5f;
	m_ViewportScales[2] = 1.0f / (pView->m_Viewport.m_MaxZ - pView->m_Viewport.m_MinZ);
	m_ViewportScales[3] = 0.0f;
	m_ViewportOffsets[0] = (F32) pView->m_Viewport.m_X + (F32) pView->m_Viewport.m_W * 0.5f;
	m_ViewportOffsets[1] = (F32) pView->m_Viewport.m_Y + (F32) pView->m_Viewport.m_H * 0.5f;
	m_ViewportOffsets[2] = pView->m_Viewport.m_MinZ / (pView->m_Viewport.m_MaxZ - pView->m_Viewport.m_MinZ);
	m_ViewportOffsets[3] = 0.0f;

	Maths::Store(pView->m_ViewSettings3D.m_WorldToDevice, &m_ViewProjMatrix);
}
#endif



}; // namespace Spu
}; // namespace Display

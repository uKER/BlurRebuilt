#include "stdafx.h"

#include <lux_app/LpvsGui.h>

using namespace LPVS;


LPVS::LpvsGui* LPVS::LpvsGui::m_pSingleton = 0;

LPVS::LpvsGui::LpvsGui()
{
	Reset();
}

LPVS::LpvsGui::~LpvsGui()
{
}

void LPVS::LpvsGui::Init()
{
	m_pSingleton->Reset();
}

LPVS::LpvsGui& LPVS::LpvsGui::GetSingleton()
{
	if (!m_pSingleton) 
	{
		m_pSingleton = new LpvsGui;
		Init();
	}

	return *m_pSingleton;
}

bool LPVS::LpvsGui::Create(CWnd* pRenderWindowCWnd)
{
	m_pRenderWindowCWnd = pRenderWindowCWnd;

	return true;
}

void LPVS::LpvsGui::Destroy()
{
	m_pSingleton->Reset();

	delete m_pSingleton;
}

void LPVS::LpvsGui::Reset()
{
	m_pRenderWindowCWnd = 0;
}


void LPVS::LpvsGui::KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool viewInvalidated = false;

	F32 cameraSpeed = 0.05f;

	switch (nChar)
	{

	/*
		if (DIMAN.GetKeyboardButtonHeld(DIK_LSHIFT))
		{
			cameraSpeed = 0.5f;
			viewInvalidated = true;
		}
	*/
		case 'W':
		case 'w':
		{
			LPVSCORE.GetCameraPtr()->MoveForward(cameraSpeed);
			viewInvalidated = true;
			break;
		}

		case 'S':
		case 's':
		{
			LPVSCORE.GetCameraPtr()->MoveForward(-cameraSpeed);
			viewInvalidated = true;
			break;
		}

		case 'A':
		case 'a':
		{
			LPVSCORE.GetCameraPtr()->MoveRight(-cameraSpeed);
			viewInvalidated = true;
			break;
		}

		case 'D':
		case 'd':
		{
			LPVSCORE.GetCameraPtr()->MoveRight(cameraSpeed);
			viewInvalidated = true;
			break;
		}

		case 'Q':
		case 'q':
		{
			LPVSCORE.GetCameraPtr()->MoveUp(cameraSpeed);
			viewInvalidated = true;
			break;
		}

		case 'Z':
		case 'z':
		{
			LPVSCORE.GetCameraPtr()->MoveUp(-cameraSpeed);
			viewInvalidated = true;
			break;
		}

		case 'C':
		case 'c':
		{
			LPVSCORE.CentreCamera();
			viewInvalidated = true;
		}
	/*
		if (DIMAN.GetMouseButtonHeld(1))
		{
			S32 mouseX;
			S32 mouseY;
			F32 mouseXF = F32(mouseX);
			mouseXF *= 0.01f;
			F32 mouseYF = F32(mouseY);
			mouseYF *= -0.01f;

			m_viewportCamera.RotatePitch(mouseYF);
			m_viewportCamera.RotateYaw(mouseXF);
			viewInvalidated = true;
		}
		*/
	}

	if (viewInvalidated)
	{
		LPVSCORE.RenderScene();
	}
}

void LPVS::LpvsGui::MouseMoveEvent(UINT nFlags, CPoint point)
{
}

void LPVS::LpvsGui::MouseButtonEvent(int buttonId, bool doubleClick, UINT nFlags, CPoint point)
{
}


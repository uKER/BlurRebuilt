#include "stdafx.h"
#include "TestSections/TestSectionLivery.h"
#include <Display/Render/BasicRender.h>
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>
#include "Framework/Application/UserControls.h"

#include <Input/GameInput.h>
#include <Input/Device.h>
#include <Input/DeviceManager.h>
#include <Input/GameInputMapping.h>

#if defined(PLATFORM_PC)
#include <Input/Pad360.h>
#endif

#if defined(PLATFORM_PS3)
#include <Input/PadPS3.h>
#endif

#if defined(PLATFORM_PC)
#include <Input/KeyboardPC.h>
#endif

#include "Vehicle/Livery/LiveryEditor.h"
#include "Vehicle/Livery/LiveryEditorUI.h"

#include "Framework/Application/Application.h"

using namespace Vehicle::Livery;

namespace Framework
{

C_TestSectionLivery::C_TestSectionLivery(const Util::C_String& name) : C_TestSection(name)
{
	m_pSkyDome		= new Render::Sky::C_SkyDome();

	C_Application::GetInstance()->SetInputButtonCount(E_LiveryAppButton_Count);
	C_Application::GetInstance()->SetInputAxisCount(E_LiveryAppAxis_Count);
	C_Application::GetInstance()->SetupUserControls();

	Input::C_GameInput* pGameInput = Input::GetGameInput(0);
	Input::C_Device* pPad = C_Application::GetInstance()->GetPad(0);
	
#if defined(PLATFORM_PC)

	pGameInput->SetButtonInput(E_LiveryAppButton_Toggle,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_Y));
	pGameInput->SetButtonInput(E_LiveryAppButton_MenuDown,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadDown));
	pGameInput->SetButtonInput(E_LiveryAppButton_MenuUp,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadUp));
	pGameInput->SetButtonInput(E_LiveryAppButton_MenuBack,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadLeft));
	pGameInput->SetButtonInput(E_LiveryAppButton_MenuNext,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_DPadRight));
	pGameInput->SetButtonInput(E_LiveryAppButton_Proceed,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_A));	
	pGameInput->SetButtonInput(E_LiveryAppButton_Back,			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_B));	
	pGameInput->SetButtonInput(E_LiveryAppButton_Blue,			new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_X));
	pGameInput->SetButtonInput(E_LiveryAppButton_SpeedSlow,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftTrigger));
	pGameInput->SetButtonInput(E_LiveryAppButton_SpeedFast,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_RightTrigger));
	pGameInput->SetButtonInput(E_LiveryAppButton_NextLayer,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_RightShoulder));
	pGameInput->SetButtonInput(E_LiveryAppButton_PrevLayer,		new Input::C_ButtonInputFromDevice(pPad, 	Input::C_Pad360::E_Button_LeftShoulder));
	
	pGameInput->SetAxisInput(E_LiveryAppAxis_ChangeLX,			new Input::C_AxisInputFromDevice(pPad, Input::C_Pad360::E_Axis_LeftStick_X));
	pGameInput->SetAxisInput(E_LiveryAppAxis_ChangeLY,			new Input::C_AxisInputFromDevice(pPad, Input::C_Pad360::E_Axis_LeftStick_Y));
	pGameInput->SetAxisInput(E_LiveryAppAxis_ChangeRX,			new Input::C_AxisInputFromDevice(pPad, Input::C_Pad360::E_Axis_RightStick_X));
	pGameInput->SetAxisInput(E_LiveryAppAxis_ChangeRY,			new Input::C_AxisInputFromDevice(pPad, Input::C_Pad360::E_Axis_RightStick_Y));

#endif

}

C_TestSectionLivery::~C_TestSectionLivery()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

bool C_TestSectionLivery::OnCreateVolatile()
{
	return true;
}

void C_TestSectionLivery::OnDestroyVolatile()
{
}

bool C_TestSectionLivery::OnCreate()
{
	C_LiveryEditor::Instance().Initialise();
	C_LiveryEditorUI::Instance().Initialise();
	m_DebugVehicle.Initialise();

	return true;
}

void C_TestSectionLivery::OnDestroy()
{
	C_LiveryEditor::Instance().Shutdown();
	C_LiveryEditorUI::Instance().Shutdown();
	m_DebugVehicle.Shutdown();
}

void C_TestSectionLivery::OnUpdate()
{
	C_LiveryEditorUI::Instance().UpdateControls();
	C_LiveryEditorUI::Instance().Update();
	m_DebugVehicle.Update();
}

void C_TestSectionLivery::OnPreRender()
{
	C_LiveryEditor::Instance().Render();
}

void C_TestSectionLivery::OnRender3DViewWithAA(const S32 viewIndex)
{
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_pSkyDome->Render();

	Render::DrawGrid();

	m_DebugVehicle.Render();
}

void C_TestSectionLivery::OnRender2DView()
{
	C_LiveryEditorUI::Instance().RenderMenu();
}

}; // namespace Framework

namespace Vehicle
{

namespace Livery
{

/*
===============
NAME: C_DebugVehicle

NOTE: C_DebugVehicle
===============
*/
C_DebugVehicle::C_DebugVehicle()
{

}

/*
===============
NAME: Initialise

NOTE: Initialise
===============
*/
void C_DebugVehicle::Initialise()
{
	m_Width		= 2.0f;
	m_Height	= 1.0f;
	m_Length	= 4.5f;
}

/*
===============
NAME: Shutdown

NOTE: Shutdown
===============
*/
void C_DebugVehicle::Shutdown()
{

}

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_DebugVehicle::Update()
{

}

/*
===============
NAME: Render

NOTE: Render
===============
*/
void C_DebugVehicle::Render()
{
	float w = m_Width * 0.5f;
	float h = m_Height * 0.5f;
	float l = m_Length * 0.5f;

	Maths::C_RGBA_F32 white(1.0f, 1.0f, 1.0f, 0.1f);
//	Maths::C_RGBA_F32 red(1.0f, 0.0f, 0.0f, 1.0f);
	Maths::C_RGBA_F32 red(1.0f, 1.0f, 1.0f, 1.0f);

	//Left
	{
		bool selected = C_LiveryEditor::Instance().m_CurrentCanvas == E_Canvas_Left;

		const Display::C_Texture2D* pTexture = &C_LiveryEditor::Instance().m_Canvases[E_Canvas_Left].m_RenderTexture.GetTexture();

		Maths::HVec3 p0(w, h, -l); p0 += m_Position;
		Maths::HVec3 p1(w, -h, -l); p1 += m_Position;
		Maths::HVec3 p2(w, -h, l); p2 += m_Position;
		Maths::HVec3 p3(w, h, l); p3 += m_Position;
		Display::C_Float2 uvs[3];
		
		uvs[0].x = 0.0f; uvs[0].y = 1.0f;
		uvs[1].x = 0.0f; uvs[1].y = 0.0f;
		uvs[2].x = 1.0f; uvs[2].y = 1.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p1, p0, p2, uvs, selected ? red : white, pTexture);
		
		uvs[0].x = 0.0f; uvs[0].y = 0.0f;
		uvs[1].x = 1.0f; uvs[1].y = 1.0f;
		uvs[2].x = 0.0f; uvs[2].y = 0.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p3, p2, p0, uvs, selected ? red : white, pTexture);
	}

	//Right
	{
		bool selected = C_LiveryEditor::Instance().m_CurrentCanvas == E_Canvas_Right;

		const Display::C_Texture2D* pTexture = &C_LiveryEditor::Instance().m_Canvases[E_Canvas_Right].m_RenderTexture.GetTexture();

		Maths::HVec3 p0(-w, h, l); p0 += m_Position;
		Maths::HVec3 p1(-w, -h, l); p1 += m_Position;
		Maths::HVec3 p2(-w, -h, -l); p2 += m_Position;
		Maths::HVec3 p3(-w, h, -l); p3 += m_Position;
		Display::C_Float2 uvs[3];
		
		uvs[0].x = 0.0f; uvs[0].y = 1.0f;
		uvs[1].x = 0.0f; uvs[1].y = 0.0f;
		uvs[2].x = 1.0f; uvs[2].y = 1.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p1, p0, p2, uvs, selected ? red : white, pTexture);
		
		uvs[0].x = 1.0f; uvs[0].y = 0.0f;
		uvs[1].x = 1.0f; uvs[1].y = 1.0f;
		uvs[2].x = 0.0f; uvs[2].y = 0.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p3, p2, p0, uvs, selected ? red : white, pTexture);
	}

	//Top
	{
		bool selected = C_LiveryEditor::Instance().m_CurrentCanvas == E_Canvas_Top;

		const Display::C_Texture2D* pTexture = &C_LiveryEditor::Instance().m_Canvases[E_Canvas_Top].m_RenderTexture.GetTexture();

		Maths::HVec3 p0(-w, h, -l); p0 += m_Position;
		Maths::HVec3 p1(w, h, -l); p1 += m_Position;
		Maths::HVec3 p2(w, h, l); p2 += m_Position;
		Maths::HVec3 p3(-w, h, l); p3 += m_Position;
		Display::C_Float2 uvs[3];
		
		uvs[0].x = 0.0f; uvs[0].y = 1.0f;
		uvs[1].x = 0.0f; uvs[1].y = 0.0f;
		uvs[2].x = 1.0f; uvs[2].y = 1.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p1, p0, p2, uvs, selected ? red : white, pTexture);
		
		uvs[0].x = 0.0f; uvs[0].y = 0.0f;
		uvs[1].x = 1.0f; uvs[1].y = 0.0f;
		uvs[2].x = 1.0f; uvs[2].y = 1.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p0, p3, p2, uvs, selected ? red : white, pTexture);
	}

	//Front
	{
		bool selected = C_LiveryEditor::Instance().m_CurrentCanvas == E_Canvas_Front;

		const Display::C_Texture2D* pTexture = &C_LiveryEditor::Instance().m_Canvases[E_Canvas_Front].m_RenderTexture.GetTexture();

		Maths::HVec3 p0(-w, h, -l); p0 += m_Position;
		Maths::HVec3 p1(-w, -h, -l); p1 += m_Position;
		Maths::HVec3 p2(w, -h, -l); p2 += m_Position;
		Maths::HVec3 p3(w, h, -l); p3 += m_Position;
		Display::C_Float2 uvs[3];
		
		uvs[0].x = 0.0f; uvs[0].y = 1.0f;
		uvs[1].x = 0.0f; uvs[1].y = 0.0f;
		uvs[2].x = 1.0f; uvs[2].y = 1.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p1, p0, p2, uvs, selected ? red : white, pTexture);
		
		uvs[0].x = 1.0f; uvs[0].y = 0.0f;
		uvs[1].x = 1.0f; uvs[1].y = 1.0f;
		uvs[2].x = 0.0f; uvs[2].y = 0.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p3, p2, p0, uvs, selected ? red : white, pTexture);
	}

	//Rear
	{
		bool selected = C_LiveryEditor::Instance().m_CurrentCanvas == E_Canvas_Rear;

		const Display::C_Texture2D* pTexture = &C_LiveryEditor::Instance().m_Canvases[E_Canvas_Rear].m_RenderTexture.GetTexture();

		Maths::HVec3 p0(w, h, l); p0 += m_Position;
		Maths::HVec3 p1(w, -h, l); p1 += m_Position;
		Maths::HVec3 p2(-w, -h, l); p2 += m_Position;
		Maths::HVec3 p3(-w, h, l); p3 += m_Position;
		Display::C_Float2 uvs[3];
		
		uvs[0].x = 0.0f; uvs[0].y = 1.0f;
		uvs[1].x = 0.0f; uvs[1].y = 0.0f;
		uvs[2].x = 1.0f; uvs[2].y = 1.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p1, p0, p2, uvs, selected ? red : white, &m_Texture_Top);
		
		uvs[0].x = 1.0f; uvs[0].y = 0.0f;
		uvs[1].x = 1.0f; uvs[1].y = 1.0f;
		uvs[2].x = 0.0f; uvs[2].y = 0.0f;
		Display::BasicRender()->DrawTextured3DTriangle(p3, p2, p0, uvs, selected ? red : white, &m_Texture_Top);
	}
}

	//float						m_Width;
	//float						m_Length;
	//float						m_Height;

	//Maths::HVec3				m_Position;

	//Display::C_Texture2D		m_Texture_Left;
	//Display::C_Texture2D		m_Texture_Right;
	//Display::C_Texture2D		m_Texture_Front;
	//Display::C_Texture2D		m_Texture_Rear;
	//Display::C_Texture2D		m_Texture_Top;

}; // namespace Livery

}; // namespace Vehicle


#include "stdafx.h"
#include <camera2/debug/CameraSwitch.h>
#include <system/debug/I_DebugRender.h>

namespace Cam2
{

C_CamOption::C_CamOption(S32 type, const Util::C_String& name)
{
	m_Type = type;
	m_Name = name;
}

C_CameraSwitch::C_CameraSwitch()
{
	Reset();
}

void C_CameraSwitch::Reset()
{
	m_CamGroup = m_CamSel = m_Choice = m_PrevChoice = 0;
	m_ShowList = false;
	m_CamList.Clear();
	memset(m_Key, 0, sizeof(m_Key));
	m_Fader = 0.f;
	m_ScrollY = 0.f;
}

void C_CameraSwitch::AddGroup()
{
	m_CamList.PushBack();
}

void C_CameraSwitch::AddOption(S32 type, const Util::C_String& name)
{
	m_CamList.Back().PushBack(C_CamOption(type, name));
}

void C_CameraSwitch::Update(F32 period)
{
	S32 x = 0, y = 0;
	if (m_ShowList)
	{
		if (m_Key[E_SwitchKey_Left ]) x = -1;
		if (m_Key[E_SwitchKey_Right]) x =  1;
		if (m_Key[E_SwitchKey_Up   ]) y = -1;
		if (m_Key[E_SwitchKey_Down ]) y =  1;
	}
	
	S32 oldGroup = m_CamGroup;
	
	S32 max = m_CamList.Size() - 1;
	if (max >= 0)
	{
		m_CamGroup += y;
		if (m_CamGroup < 0)   m_CamGroup = 0;
		if (m_CamGroup > max) m_CamGroup = max;
	}
	
	Ut::Vector<C_CamOption>* subList = &m_CamList[m_CamGroup];
	max = subList->Size() - 1;
	if (m_CamSel > max && m_CamGroup != oldGroup)
	{
		// Don't step up/down if no option in new group:
		m_CamGroup = oldGroup;
		subList = &m_CamList[m_CamGroup];
		max = subList->Size() - 1;
	}		
	
	if (max >= 0)
	{
		m_CamSel += x;
		if (m_CamSel < 0)   m_CamSel = 0;
		if (m_CamSel > max) m_CamSel = max;
	}
	
	if (!m_ShowList)
	{
		if (m_Key[E_SwitchKey_ShowMenu])
		{
			// Show camera menu:
			SyncCameraChoice();
			m_ShowList = true;
		}
	}
	else
	{
		if (m_Key[E_SwitchKey_Confirm])
		{
			// Confirm selection
			m_Choice = (*subList)[m_CamSel].m_Type;
			m_ShowList = false;
		}

		if (m_Key[E_SwitchKey_ShowMenu]) // Cancel selection
		{
			m_ShowList = false;
		}
	}

	// Show changing using Debug Menu or shortcut keys:
	if (m_PrevChoice > 0 && m_Choice != m_PrevChoice)
	{
		SyncCameraChoice();
		m_ShowList = false;
	}
	m_PrevChoice = m_Choice;

	m_Fader -= period;
	if (m_Fader < 0.f)
		m_Fader = 0.f;
	
	m_ScrollY += ((F32)m_CamGroup - m_ScrollY) * 0.1f;
}


void C_CameraSwitch::RenderFullscreen()
{
	using namespace Maths;
	using namespace Util;
	
	DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Immediate();

	F32 listFade = m_ShowList ? 1.f : m_Fader;

	if (listFade <= 0.f)
		return;
		
	// Draw camera menu:
	S32 y = 0;
	for (y = 0; y <  m_CamList.Size(); y++)
	{
		const Ut::Vector<C_CamOption>& subList = m_CamList[y];
		
		S32 x = 0;
		for (x = 0; x < subList.Size(); x++)
		{
			const C_CamOption& opt = subList[x];
			
			F32 px = x * 120.f;
			F32 py = (F32)y - m_ScrollY;
			
			// Fade out from the row we are on:
			F32 rowFade = 2.f - (py > 0.f ? py : -py);
			if (rowFade <= 0.f)
				continue;
			if (rowFade > 1.f)
				rowFade = 1.f;

			F32 fade = listFade * rowFade;

			// Place at bottom of screen:
			px += 64.f;
			py = 604.f + py * 32.f;
			
			RGBA col(0xa0a0a0);
			F32 alpha = 1.f;
			if (m_Choice == opt.m_Type)
				col = RGBA(0xc0c080);

			if (y == m_CamGroup && x == m_CamSel)
				col = RGBA(0x40ff40);
			
			col.A((U8)(alpha * fade * 255.f));
			
			debugRender->Draw2dText(HVec2(px, py), opt.m_Name, col, 32.f);
		}
	}
}

void C_CameraSwitch::SyncCameraChoice()
{
	S32 group = 0, sub = 0;
	for (group = 0; group < m_CamList.Size(); group++)
	{
		Ut::Vector<C_CamOption>& subList = m_CamList[group];
		for (sub = 0; sub < subList.Size(); sub++)
		{
			if (m_Choice == subList[sub].m_Type)
			{
				m_CamGroup = group;
				m_CamSel = sub;
			}
		}
	}
}

S32 C_CameraSwitch::Choice()
{
	return m_Choice;
}

}

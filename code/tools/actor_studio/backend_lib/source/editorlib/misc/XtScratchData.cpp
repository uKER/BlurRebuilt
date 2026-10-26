#include "stdafx.h"

#include <EditorLib/Misc/XtScratchData.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtHelpers.h>

namespace EditorLib
{

C_XtScratchData::C_XtScratchData()
{
	// Create new component

	m_ScratchData = new Xt::DomComponent();
	GenerateIdForComponent(m_ScratchData, false);

	Xt::Monitor(&m_DomMonitor, m_ScratchData);
	m_DomMonitor.SetCallback(&C_XtScratchData::StaticOnXtDomMonitorEvent, (C_XtScratchData*)this);

}

C_XtScratchData::~C_XtScratchData()
{
	// Remove component
	delete m_ScratchData;

	Xt::Monitor(&m_DomMonitor, 0);
}

void C_XtScratchData::SetCallback(Callback callback, void* userData)
{
	m_Callback = callback;
	m_UserData = userData;
}

Xt::DomComponent* C_XtScratchData::GetXtData()
{
	return m_ScratchData;
}

void C_XtScratchData::StaticOnXtDomMonitorEvent(Xt::DomMonitor* monitor, Xt::DomEvent* event, C_XtScratchData* data)
{
	((C_XtScratchData*)data)->OnXtDomMonitorEvent(monitor, event);
}

void C_XtScratchData::OnXtDomMonitorEvent(Xt::DomMonitor*, Xt::DomEvent*)
{
	
}

}
#include "stdafx.h"	

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtHelpers.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_XtObject::C_Unbinder::C_Unbinder(C_XtObject* pObj)
{
	m_pViewDisabler = new C_ViewDisabler();
	BIZ_ASSERT(pObj);
	m_pObj = pObj;
	m_pObj->XtUnbind();
}

C_XtObject::C_Unbinder::~C_Unbinder()
{
	m_pObj->XtBind();
	delete m_pViewDisabler;
}

C_XtObject::C_XtObject()
{
	m_pLarvaType = 0;
	//Start unbound.
	m_unbinder = 1;
}

C_XtObject::~C_XtObject()
{
	Xt::Monitor(&m_rootMonitor, NULL);
}

C_Status C_XtObject::XtInitialise(
	C_XtLevel* pLevel, 
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	m_pLarvaType = pLarvaType;
	if (pData)
	{
		SetXtData(pData);
		//Initialise as dirty to trigger update.
		m_xtDirty.SetDirty(pData);
	}
	//Tag this node for updating.
	return E_Status_Success;
}

void C_XtObject::XtBind() const
{
	BIZ_ASSERT(m_unbinder > 0);
	m_unbinder--;
}

void C_XtObject::XtUnbind() const
{
	m_unbinder++;
}

bool C_XtObject::IsBound() const
{
	return m_unbinder == 0;
}

Xt::DomElement* C_XtObject::GetXtData()
{
	return m_rootMonitor.element;
}

const Xt::DomElement* C_XtObject::GetXtData() const
{
	return m_rootMonitor.element;
}

void C_XtObject::SetXtData(Xt::DomElement* pData)
{
	Xt::DomComponent* pComponent = Xt::AsComponent(pData);
	if (pComponent)
		m_componentType = pComponent->type;
	else
		m_componentType = "";
	Xt::Monitor(&m_rootMonitor, pData);
	m_rootMonitor.SetCallback(
		&C_XtObject::StaticOnXtDomInternalMonitorEvent, (C_XtObject*)this);
}

LarvaType* C_XtObject::GetXtType()
{
	return m_pLarvaType;
}

const LarvaType* C_XtObject::GetXtType() const
{
	return m_pLarvaType;
}

C_XtDirty& C_XtObject::GetXtDirty()
{
	return m_xtDirty;
}

const C_XtDirty& C_XtObject::GetXtDirty() const
{
	return m_xtDirty;
}

void C_XtObject::StaticOnXtDomInternalMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_XtObject* pThis)
{
	//If this isn't a monitor we're interested in, or if we're not bound...
	if (pMonitor != &pThis->m_rootMonitor)
		return;

	bool isXtDirty = pThis->GetXtDirty().IsDirty();
	bool typeChanged = false;
	//Look for type changing.
	if (pEvent->code == Xt::kDomComponentChanged)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(pThis->m_rootMonitor.element);
		if (pComponent && pThis->m_componentType != pComponent->type)
			typeChanged = true;
	}
	else if (pEvent->code == Xt::kDomRemoved)
		//Bail if we're being removed.
		return;

	if (pThis->IsBound())
		//Continue handling this event.
		pThis->OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
}

void C_XtObject::StaticOnXtDomExternalMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_XtObject* pThis)
{
	if (!pThis->IsBound())
		return;
	
	pThis->OnXtDomMonitorEvent(pMonitor, pEvent, false);
}

const char* C_XtObject::GetXtTypeName() const
{
	Xt::DomComponent* pComp = Xt::AsComponent(const_cast<Xt::DomElement*>(GetXtData()));
	if (pComp)
	{
		return m_componentType.Text();
	}
	else if (GetXtType())
	{
		return GetXtType()->name.Text();
	}

	return "";
}

}

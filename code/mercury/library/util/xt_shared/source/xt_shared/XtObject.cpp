#include "stdafx.h"

#include <Xt_Shared/XtObject.h>
#include <Xt_Shared/XtObjectImpl.h>

namespace XtShared
{

C_XtObject::C_Unbinder::C_Unbinder(C_XtObject* pObj)
{
	assert(pObj);
	m_pObj = pObj;
	m_pObj->XtUnbind();
}

C_XtObject::C_Unbinder::~C_Unbinder()
{
	m_pObj->XtBind();
}



C_XtObject::C_XtObject()
{
	m_pXtObjectImpl = 0;
}

C_XtObject::~C_XtObject()
{
	if (m_pXtObjectImpl)
		delete m_pXtObjectImpl;
}

bool C_XtObject::XtInitialise(
	C_XtType xtType,
	bool isPolymorphic,
	Xt::DomElement* pData)
{
	assert(m_pXtObjectImpl);
	m_pXtObjectImpl->XtInitialise(xtType, isPolymorphic, pData);
	return true;
}

void C_XtObject::XtBind() const
{
	assert(m_pXtObjectImpl);
	m_pXtObjectImpl->XtBind();
}

void C_XtObject::XtUnbind() const
{
	assert(m_pXtObjectImpl);
	m_pXtObjectImpl->XtUnbind();
}

bool C_XtObject::IsBound() const
{
	if (m_pXtObjectImpl)
		return m_pXtObjectImpl->IsBound();
	return false;
}

Xt::DomElement* C_XtObject::GetXtData()
{
	assert(m_pXtObjectImpl);
	return m_pXtObjectImpl->GetXtData();
}

C_XtType C_XtObject::GetXtType() const
{
	assert(m_pXtObjectImpl);
	return m_pXtObjectImpl->GetXtType();
}

void C_XtObject::XtDeserialise(Xt::DomElement* pFrom)
{
	assert(m_pXtObjectImpl);
}

void C_XtObject::XtDeserialiseDeferred()
{
}

void C_XtObject::StaticOnXtDomExternalMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_XtObject* pThis)
{
	if (!pThis->IsBound())
		return;
	
	pThis->OnXtDomMonitorEvent(pMonitor, pEvent);
}

void C_XtObject::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent)
{
}

void C_XtObject::DeleteXtData()
{
	assert(m_pXtObjectImpl);
	m_pXtObjectImpl->DeleteXtData();
}

void C_XtObject::AttachXtObjectImpl(C_XtObjectImpl* pImpl)
{
	if (m_pXtObjectImpl && m_pXtObjectImpl != pImpl)
	{
		assert(!"Needs DetachXtObjectImpl calling first!");
	}
	m_pXtObjectImpl = pImpl;
	if (m_pXtObjectImpl)
		m_pXtObjectImpl->Attach(this);
}

void C_XtObject::DetachXtObjectImpl()
{
	assert(m_pXtObjectImpl);
	if (m_pXtObjectImpl)
	{
		m_pXtObjectImpl->Detach();
		m_pXtObjectImpl = 0;
	}
}

Xt::DomMonitor* C_XtObject::GetXtMonitor(E_Monitor monitor)
{
	assert(m_pXtObjectImpl);
	return m_pXtObjectImpl->GetXtMonitor(monitor);
}

C_XtObjectImpl* C_XtObject::GetXtObjectImpl()
{
	return m_pXtObjectImpl;
}

}

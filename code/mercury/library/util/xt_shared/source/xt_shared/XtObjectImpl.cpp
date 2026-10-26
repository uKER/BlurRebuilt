#include "stdafx.h"
#include <Xt_Shared/XtObject.h>
#include <Xt_Shared/XtObjectImpl.h>

namespace XtShared
{

C_XtObjectImpl::C_XtObjectImpl()
{
	m_pXtObject = 0;

	//Start unbound.
	m_unbinder = 1;
	m_typeMonitorDirty = false;
}

C_XtObjectImpl::~C_XtObjectImpl()
{
	if (IsBound())
	{
		//If we're deleting an xtbound object then take the xt data with us.
		DeleteXtData();
	}

	Xt::Monitor(&m_monitors[C_XtObject::E_Monitor_Root], NULL);
	Xt::Monitor(&m_monitors[C_XtObject::E_Monitor_Type], NULL);

	if (m_typeMonitorDirty)
		//If we're being deleted, remove from deferred update list if necessary.
		RemoveXtObjectFromDeferredUpdate(this);
}

void C_XtObjectImpl::XtInitialise(
	C_XtType xtType,
	bool isPolymorphic,
	Xt::DomElement* pData)
{
	m_initialType = xtType;
	m_xtType = xtType;
	Xt::Monitor(&m_monitors[C_XtObject::E_Monitor_Root], pData);
	m_monitors[C_XtObject::E_Monitor_Root].SetCallback(
		&C_XtObjectImpl::StaticOnXtDomInternalMonitorEvent, (C_XtObjectImpl*)this);

	//Look for type changes.
	Xt::DomComponent* pTypeData = Xt::AsComponent(GetXtData());
	if (pTypeData)
	{
		Xt::Monitor(&m_monitors[C_XtObject::E_Monitor_Type], pTypeData);
		m_monitors[C_XtObject::E_Monitor_Type].SetCallback(
			&C_XtObjectImpl::StaticOnXtDomInternalMonitorEvent, (C_XtObjectImpl*)this);

		Xt::DomDb* pDb = Xt::DbOf(pData);
		assert(pDb);
		
		LarvaType* pItemTypeFromField = FindTypeByName(pDb->schema, pTypeData->type);
		if (pItemTypeFromField)
		{
			if (xtType.AsType())
				//Type sanity check.
				assert(xtType.AsType() == pItemTypeFromField);
			else
				//Use type specified in data.
				m_xtType = C_XtType(pItemTypeFromField);
		}
	}
}

void C_XtObjectImpl::XtDeserialiseDeferred()
{
	if (m_typeMonitorDirty)
	{
		Xt::DomComponent* pTypeData = Xt::AsComponent(GetXtData());
		if (!pTypeData)
		{
			assert(!"Missing component!");
			return;
		}

		//Re-attach type monitor.
		Xt::Monitor(&m_monitors[C_XtObject::E_Monitor_Type], pTypeData);
		m_monitors[C_XtObject::E_Monitor_Type].SetCallback(
			&C_XtObjectImpl::StaticOnXtDomInternalMonitorEvent, (C_XtObjectImpl*)this);

		Xt::DomDb* pDb = Xt::DbOf(pTypeData);
		if (pDb)
		{
			//Bear in mind it's valid for this to be null!
			m_xtType = C_XtType(FindTypeByName(pDb->schema, pTypeData->type));
		}

		OnChangeXtObjectType(this);
		m_typeMonitorDirty = false;
	}

	if (m_pXtObject)
		m_pXtObject->XtDeserialiseDeferred();
}

void C_XtObjectImpl::XtBind() const
{
	assert(m_unbinder > 0);
	m_unbinder--;
}

void C_XtObjectImpl::XtUnbind() const
{
	m_unbinder++;
}

bool C_XtObjectImpl::IsBound() const
{
	return m_unbinder == 0;
}

Xt::DomElement* C_XtObjectImpl::GetXtData()
{
	return m_monitors[C_XtObject::E_Monitor_Root].element;
}

C_XtType C_XtObjectImpl::GetXtType() const
{
	return m_isPolymorphic ? m_xtType : m_initialType;
}

Xt::DomMonitor* C_XtObjectImpl::GetXtMonitor(C_XtObject::E_Monitor monitor)
{
	return &m_monitors[monitor];
}

void C_XtObjectImpl::StaticOnXtDomInternalMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent, 
	C_XtObjectImpl* pThis)
{
	if (!pThis->IsBound())
		return;

	if (pMonitor == &pThis->m_monitors[(U32)C_XtObject::E_Monitor_Type])
	{
		pThis->m_typeMonitorDirty = true;
		AddXtObjectForDeferredUpdate(pThis);
		return;
	}

	if (pEvent->code == Xt::kDomRemoved && 
		pMonitor == &pThis->m_monitors[(U32)C_XtObject::E_Monitor_Root])
		//Bail if we're being removed.
		return;

	pThis->m_pXtObject->OnXtDomMonitorEvent(pMonitor, pEvent);
}

void C_XtObjectImpl::DeleteXtData()
{
}

void C_XtObjectImpl::Attach(C_XtObject* pXtObject)
{
	m_pXtObject = pXtObject;
}

void C_XtObjectImpl::Detach()
{
	m_pXtObject = 0;
	Xt::Monitor(&m_monitors[C_XtObject::E_Monitor_Type], NULL);
}

bool C_XtObjectImpl::TypeDirty() const
{
	return m_typeMonitorDirty;
}

}

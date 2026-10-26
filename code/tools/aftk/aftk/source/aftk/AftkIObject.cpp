#include <stdafx.h>

using namespace Aftk;

I_Object::~I_Object()
{
	for (U32 i = 0; i < (U32)m_connectedPlugs.Size(); i++)
	{
		//Disconnect ourselves from this plug.
		m_connectedPlugs[i]->DisconnectFromObject();
	}
}

bool I_Object::AddConnectedPlugPtr(I_Plug* pPlug)
{
	for (U32 i = 0; i < (U32)m_connectedPlugs.Size(); i++)
	{
		if (m_connectedPlugs[i] == pPlug)
			return false;
	}
	m_connectedPlugs.PushBack(pPlug);
	return true;
}

void I_Object::DeleteConnectedPlugPtr(I_Plug* pPlug)
{
	for (U32 i = 0; i < (U32)m_connectedPlugs.Size(); i++)
	{
		if (m_connectedPlugs[i] == pPlug)
		{
			m_connectedPlugs.EraseByIndex(i);
			return;
		}
	}
}

bool I_Object::HasType(E_ObjType type) const
{
	if (type == Aftk::E_ObjType_IObject)
		return true;

	return false;
}

U32 I_Object::GetNumConnectedPlugs() const
{
	return m_connectedPlugs.Size();
}

const Ut::UtVector<Ut::DynamicVector<I_Plug*> >& I_Object::GetConnectedPlugs() const
{
	return m_connectedPlugs;
}

#include "stdafx.h"
#include <EditorLib/Misc/Selection.h>

namespace EditorLib
{

C_Selection::C_Selection()
{
	m_mode = E_Mode_Unknown;
}

C_Selection::~C_Selection()
{
}

C_Selection::C_Selection(const C_Selection& selection)
{
	*this = selection;
}

const C_Selection& C_Selection::operator=(const C_Selection& selection)
{
	m_mode = selection.m_mode;
	m_itemArray = selection.m_itemArray;
	m_itemMap = selection.m_itemMap;
 
	return *this;
}

bool C_Selection::operator==(const C_Selection& rhs) const
{
	if (m_mode != rhs.m_mode || m_itemArray.Size() != rhs.m_itemArray.Size())
		return false;

	for (S32 i = 0; i < m_itemArray.Size(); i++)
	{
		const C_Item& lhsItem = m_itemArray[i];
		const C_Item& rhsItem = rhs.m_itemArray[i];
		if (lhsItem.id != rhsItem.id ||
			lhsItem.components.Size() != rhsItem.components.Size() ||
			((lhsItem.components.Size() > 0) ? memcmp(
				&lhsItem.components[0], 
				&rhsItem.components[0], 
				sizeof(C_Item)*lhsItem.components.Size()) != 0 : false))
			return false;
	}

	return true;
}

bool C_Selection::operator!=(const C_Selection& rhs) const
{
	return !(*this==rhs);
}

void C_Selection::Add(E_Mode mode, C_NodeId id)
{
	Ut::Vector<U32> components;
	Add(mode, id, components);
}

void C_Selection::Add(E_Mode mode, C_NodeId id, const Ut::Vector<U32>& components)
{
	//Clear selection during mode switch.
	if (m_mode != mode)
		Clear();
	//Set mode.
	m_mode = mode;

	C_Item* pItem = _Find(id);

	if (!pItem)
	{
		m_itemArray.PushBack();
		pItem = &m_itemArray.Back();
		pItem->id = id;
		m_itemMap.Insert(C_Selection::ItemMap::DataType(id, m_itemArray.Size()-1));
	}

	//Merge components. n^2 for the moment, need to use a sorted container here instead.
	for (S32 i = 0; i < components.Size(); i++)
	{
		bool found = false;
		for (S32 j = 0; j < pItem->components.Size(); j++)
		{
			if (pItem->components[j] == components[i])
			{
				found = true;
				break;
			}
		}
		if (!found)
			pItem->components.PushBack(components[i]);
	}
}

void C_Selection::Toggle(E_Mode mode, C_NodeId id)
{
	Ut::Vector<U32> components;
	Toggle(mode, id, components);
}

void C_Selection::Toggle(E_Mode mode, C_NodeId id, const Ut::Vector<U32>& components)
{
	//Clear selection during mode switch.
	if (m_mode != mode)
		Clear();
	//Set mode.
	m_mode = mode;

	C_Item* pSelectionItem = _Find(id);
	//Component level toggle.
	if (mode == E_Mode_Component)
	{
		if (pSelectionItem)
		{
			const Ut::Vector<U32>& currentComponents = pSelectionItem->components;
			
			if (components.Size())
			{
				Ut::Vector<U32> newComponents = currentComponents;

				//toggle components
				for (S32 i=0; i<components.Size(); ++i)
				{
					S32 found = -1;
					for (S32 j=0; j<newComponents.Size(); ++j)
					{
						if (newComponents[j] == components[i])
						{
							found = j;
							break;
						}
					}
					if (found==-1)
						newComponents.PushBack(components[i]);
					else
						newComponents.EraseNonOrdered(found);
				}

				//replace selection
				pSelectionItem->components = newComponents;
			}
		}
		else
		{
			//add node
			Add(mode, id, components);
		}
	}
	//Object level toggle
	else
	{
		if (pSelectionItem)
		{
			Remove(mode, id);
		}
		else
		{
			Add(mode, id);
		}
	}
}

void C_Selection::Remove(E_Mode mode, C_NodeId id)
{
	Ut::Vector<U32> components;
	Remove(mode, id, components);
}

void C_Selection::Remove(E_Mode mode, C_NodeId id, const Ut::Vector<U32>& components)
{
	C_Selection::ItemMap::Iterator it = m_itemMap.Find(id);
	if (it == m_itemMap.End())
		return;

	U32 index = it->GetValue();

	//Deselect node.
	if (components.Size() == 0)
	{
		m_itemArray.EraseByIndex(index);
		m_itemMap.Erase(m_itemMap.Find(id));

		//update indices
		it = m_itemMap.Begin();
		while (it != m_itemMap.End())
		{
			if (it->GetValue()>index)
				it->GetValue()--;
			++it;
		}

		return;
	}
	
	C_Item& info = m_itemArray[index];
	//Erase components.
	for (S32 i = 0; i < components.Size(); i++)
	{
		bool found = false;
		for (S32 j = 0; j < info.components.Size(); j++)
		{
			info.components.EraseByIndex(j);
			break;
		}
	}
}

void C_Selection::Clear()
{
	m_itemArray.Clear();
	m_itemMap.Clear();
	m_mode = E_Mode_Unknown;
}

const C_Selection::C_Item* C_Selection::Find(C_NodeId id) const
{
	C_Selection::ItemMap::ConstIterator it = m_itemMap.Find(id);
	if (it != m_itemMap.End())
		return &m_itemArray[it->GetValue()];
		
	return 0;
}

C_Selection::C_Item* C_Selection::_Find(C_NodeId id)
{
	return const_cast<C_Selection::C_Item*>(Find(id));
}

const C_Selection::C_Item* C_Selection::Find(C_NodeId id, U32 component) const
{
	const C_Selection::C_Item *pItem = Find(id);
	if (!pItem)
		return pItem;

	const Ut::Vector<U32>& components = pItem->components;
	for (S32 i=0; i<components.Size(); i++)
	{
		if (components[i] == component)
			return pItem;
	}
	return 0;
}

const C_Selection::ItemArray& C_Selection::GetItems() const
{
	return m_itemArray;
}

C_Selection::E_Mode C_Selection::GetMode() const
{
	return m_mode;
}

}

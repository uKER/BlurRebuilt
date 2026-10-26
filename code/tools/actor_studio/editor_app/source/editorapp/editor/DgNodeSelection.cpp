#include "stdafx.h"

#include <EditorApp/Editor/DgNodeSelection.h>

namespace Editor
{

C_DgNodeSelection::C_DgNodeSelection()
{
}

C_DgNodeSelection::~C_DgNodeSelection()
{
}

C_DgNodeSelection::C_DgNodeSelection(const C_DgNodeSelection& selection)
{
	*this = selection;
}

const C_DgNodeSelection& C_DgNodeSelection::operator=(const C_DgNodeSelection& selection)
{
	m_itemArray = selection.m_itemArray;
	m_itemMap = selection.m_itemMap;
 
	return *this;
}

void C_DgNodeSelection::Add(EditorLib::I_DgNode* pNode)
{
	Ut::Vector<U32> components;
	Add(pNode, components);
}

void C_DgNodeSelection::Add(EditorLib::I_DgNode* pNode, const Ut::Vector<U32>& components)
{
	C_Item* pItem = _Find(pNode);

	if (!pItem)
	{
		m_itemArray.PushBack();
		pItem = &m_itemArray.Back();
		pItem->pDgNode = pNode;
		m_itemMap.Insert(C_DgNodeSelection::ItemMap::DataType(pNode, m_itemArray.Size()-1));
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


void C_DgNodeSelection::Remove(const EditorLib::I_DgNode* pNode)
{
	Ut::Vector<U32> components;
	Remove(pNode, components);
}

void C_DgNodeSelection::Remove(const EditorLib::I_DgNode* pNode, const Ut::Vector<U32>& components)
{
	C_DgNodeSelection::ItemMap::Iterator it = m_itemMap.Find(pNode);
	if (it == m_itemMap.End())
		return;

	U32 index = it->GetValue();

	//Deselect node.
	if (components.Size() == 0)
	{
		m_itemArray.EraseByIndex(index);
		m_itemMap.Erase(m_itemMap.Find(pNode));

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

void C_DgNodeSelection::Clear()
{
	m_itemArray.Clear();
	m_itemMap.Clear();
}

const C_DgNodeSelection::C_Item* C_DgNodeSelection::Find(const EditorLib::I_DgNode* pNode) const
{
	C_DgNodeSelection::ItemMap::ConstIterator it = m_itemMap.Find(pNode);
	if (it != m_itemMap.End())
		return &m_itemArray[it->GetValue()];
		
	return 0;
}

C_DgNodeSelection::C_Item* C_DgNodeSelection::_Find(const EditorLib::I_DgNode* pNode)
{
	return const_cast<C_DgNodeSelection::C_Item*>(Find(pNode));
}

const C_DgNodeSelection::C_Item* C_DgNodeSelection::Find(const EditorLib::I_DgNode* pNode, U32 component) const
{
	const C_DgNodeSelection::C_Item *pItem = Find(pNode);
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

const C_DgNodeSelection::ItemArray& C_DgNodeSelection::GetItems() const
{
	return m_itemArray;
}

}
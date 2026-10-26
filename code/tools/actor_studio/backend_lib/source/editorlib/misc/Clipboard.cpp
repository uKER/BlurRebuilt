#include "stdafx.h"
#include <core/Containers/radix.h>
#include <EditorLib/Misc/Clipboard.h>

namespace EditorLib
{

C_Clipboard::C_Clipboard()
{
}

C_Clipboard::~C_Clipboard()
{
}

C_Clipboard::C_Clipboard(const C_Clipboard& clipboard)
{
	*this = clipboard;
}

const C_Clipboard& C_Clipboard::operator=(const C_Clipboard& clipboard)
{
	m_itemArray = clipboard.m_itemArray;

	return *this;
}

bool C_Clipboard::operator==(const C_Clipboard& rhs) const
{
	if (m_itemArray.Size() != rhs.m_itemArray.Size())
		return false;

	for (S32 i = 0; i < m_itemArray.Size(); i++)
	{
		const C_Item& lhsItem = m_itemArray[i];
		const C_Item& rhsItem = rhs.m_itemArray[i];
		if (lhsItem.id != rhsItem.id)
			return false;
	}

	return true;
}

bool C_Clipboard::operator!=(const C_Clipboard& rhs) const
{
	return !(*this==rhs);
}

void C_Clipboard::Add(C_NodeId id)
{
	C_Item *pItem = _Find(id);
	if (!pItem)
	{
		C_Item item;
		item.id = id;
		m_itemArray.PushBack(item);
	};
}

void C_Clipboard::Remove(C_NodeId id)
{
	for (S32 i=0; i<m_itemArray.Size(); ++i)
	{
		if (m_itemArray[i].id == id)
		{
			m_itemArray.EraseByIndex(i);
			return;
		}
	}
}

void C_Clipboard::Clear()
{
	m_itemArray.Clear();
}

const C_Clipboard::C_Item* C_Clipboard::Find(C_NodeId id) const
{
	for (S32 i=0; i<m_itemArray.Size(); ++i)
		if (m_itemArray[i].id == id)
			return &m_itemArray[i];

	return 0;
}

const C_Clipboard::ItemArray& C_Clipboard::GetItems() const
{
	return m_itemArray;
}

C_Clipboard::C_Item* C_Clipboard::_Find(C_NodeId id)
{
	return const_cast<C_Clipboard::C_Item*>(Find(id));
}

}

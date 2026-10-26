#include "stdafx.h"

#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Misc/StringUtil.h>

namespace DMan
{

C_ColumnSet::C_ColumnSet()
{
}

C_ColumnSet::~C_ColumnSet()
{
	for (U32 i = 0; i < m_keys.size(); ++i)
		delete m_keys[i];
}

void C_ColumnSet::AddColumn(C_Column* pColumn)
{
	for (U32 i = 0; i < m_keys.size(); ++i)
		if (m_keys[i]->GetKey() == pColumn->GetKey())
		{
			//Key clash.
			delete pColumn;
			return;
		}

	m_keys.push_back(pColumn);
}

C_Column& C_ColumnSet::InsertColumn(const wxString& colKey, LarvaField* pLarvaField, S32 index)
{
	C_Column* col = GetColumn(colKey);

	if(col == NULL)
	{
		col = new C_Column(colKey, pLarvaField);
		if(index != -1)
		{
			m_keys.insert(m_keys.begin() + index, col);
		}
		else
		{
			m_keys.push_back(col);
		}
	}
	else
	{
		KeysIt colPos;
		for(KeysIt it = m_keys.begin(); it != m_keys.end(); ++it)
		{
			if((*it)->GetKey() == colKey)
			{
				colPos = it;
				break;
			}
		}

		m_keys.erase(colPos);
		if(index != -1)
		{
			m_keys.insert(m_keys.begin() + index, col);
		}
		else
		{
			m_keys.push_back(col);
		}
	}
	
	return *col;
}

C_Column* C_ColumnSet::GetColumn(const wxString& colKey) 
{
	for(KeysConstIt it = m_keys.begin(); it != m_keys.end(); ++it)
	{
		if((*it)->GetKey() == colKey)
		{
			return *it;
		}
	}
	
	return NULL;
}

C_Column* C_ColumnSet::GetColumn(size_t colSetKeysIndex) 
{
	if(colSetKeysIndex < m_keys.size())
	{
		return m_keys[colSetKeysIndex];
	}
	return NULL;
}

U32 C_ColumnSet::GetNumColumns() const
{
	return m_keys.size();
}

C_Column::C_Column(const wxString& key, LarvaField* pLarvaField) : 
	m_key(key),
	m_pLarvaField(pLarvaField),
	m_label(wxT("")), 
	m_hidden(false),
	m_sortable(true), 
	m_importable(true), 
	m_dataType(E_DataType_String),
	m_metaData(""),
	m_width(-1)
{
}


} 
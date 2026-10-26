
#include "stdafx.h"

#include "ExplorerItemData.h"
#include "../Record/Record.h"

C_ExplorerItemData::~C_ExplorerItemData()
{
}

void C_ExplorerItemData::SetName(const wxString& name)
{
	m_Name = name;
}

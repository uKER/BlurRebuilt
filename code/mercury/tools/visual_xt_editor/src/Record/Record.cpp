
#include "stdafx.h"

#include "Record.h"

void C_Record::SetName(const wxString& name)
{
	m_Name = name.AfterLast('.');
	m_Path = name;
}

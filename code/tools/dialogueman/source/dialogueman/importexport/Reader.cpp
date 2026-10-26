#include "stdafx.h"
#include <DialogueMan/ImportExport/Reader.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Rowset.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <wx/regex.h>

namespace DMan
{
C_Reader::C_Reader() :
	m_pItemType(NULL), 
	m_pLocTable(NULL)
{}

C_Reader::~C_Reader()
{
}
 
C_Reader& C_Reader::SetItemType(LarvaType* pItemType)
{
	m_pItemType = pItemType;
	return *this;
}

C_Reader& C_Reader::SetLocTable(Xt::DomRecord* pLocTable)
{
	m_pLocTable = pLocTable;
	return *this;
}

C_Reader& C_Reader::SetFilename(const wxString& filename) 
{ 
	m_fileName.Assign(filename); 
	return *this;
}


} //namespace DMan
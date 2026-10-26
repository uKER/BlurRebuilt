#include "stdafx.h"

#include <Dialogueman/Misc/StringUtil.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>

#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>

#include <wx/regex.h>

namespace DMan
{

wxString BuildColumnKey(const wxString& typeName, const wxString& xtFieldName)
{
	return typeName + wxT(".") + xtFieldName;
}

wxString XtFieldNameFromColumnKey(const wxString& columnKey)
{
	size_t index = columnKey.Find(wxT("."));
	if (index == wxString::npos)
		return wxEmptyString;

	size_t start = index+1;
	return columnKey.substr(start, columnKey.length() - start);
}

wxString XtTypeNameFromColumnKey(const wxString& columnKey)
{
	size_t index = columnKey.Find(wxT("."));
	if (index == wxString::npos)
		return wxEmptyString;

	return columnKey.substr(0, index);
}

wxString ColNumToStr(U32 colNum)
{
	assert(colNum > 0);
	
	colNum--;  //calculations easier when indexing from 0
	U32 lettersRange = (U32)wxT('Z') - (U32)wxT('A') + 1;
	U32 timesRange = colNum / lettersRange;
	U32 remainder = colNum % lettersRange; 

	assert(timesRange <= 10);

	wxString colStr(wxT(""));
	if(timesRange > 0)
	{
		wxChar major = (wxChar )((U32)wxT('A') + timesRange - 1);
		wxChar minor = (wxChar )((U32)wxT('A') + remainder);
		colStr << major << minor;
	}
	else
	{
		colStr << (wxChar)((U32)wxT('A') + remainder);
	}
		
	return colStr;
}

U32 ColStrToNum(const wxString& colStr) 
{
	U32 colNum = 0;
	U32 lettersRange = (U32)wxT('Z') - (U32)wxT('A') + 1;
	U32 len = colStr.Length();

	if(len > 2 || len < 1)
	{
		return -1;
	}

	if(len == 2)
	{
		U32 colNumTmp1 = colStr.GetChar(0) - wxT('A') + 1;
		U32 colNumTmp2 = colStr.GetChar(1) - wxT('A') + 1;
		if((colNumTmp1 > lettersRange) || (colNumTmp2 > lettersRange))
			return -1;

		colNum += colNumTmp1 * lettersRange;
		colNum += colNumTmp2;
	} 
	else if(len == 1)
	{
		U32 colNumTmp = colStr.GetChar(0) - wxT('A') + 1;
		if(colNumTmp > lettersRange)
			return -1;

		colNum += colNumTmp;
	}
	return colNum;
}

Util::C_String RemoveUnexpandedMacros(const Util::C_String& str)
{
	wxString tmp = str.Chars();
	wxRegEx macro = wxT("%.*%");	
	if ( macro.Matches(tmp) )
	{
		macro.Replace(&tmp, wxT(""));
	}
	return tmp.c_str();
}

Util::C_String RemoveFoldersFromRecordName(const Util::C_String& str)
{
	wxString retVal;
	wxString tmp = str.Chars();
	int lastDot = tmp.Find('.', true);
	if(lastDot != wxNOT_FOUND)
	{
		retVal = wxString(tmp, lastDot + 1, tmp.Len() - lastDot - 1);
	}
	else
	{
		retVal = tmp;
	}
	return retVal.c_str();
}

Util::C_String CurrentDialogueRecordName()
{
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	C_RowSet* pRowSet = selection.GetCurrentRowSet();
	if(pRowSet)
	{
		const C_Selection::ItemArray& arr = selection.GetItems();
		const C_Selection::C_Item& item = arr.Back();
		C_Record* pDialogueRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());
		Util::C_String dialogueRecord =	
			RemoveFoldersFromRecordName((const char*)pDialogueRecord->GetName());
		return dialogueRecord;
	}
	return "";
}

}

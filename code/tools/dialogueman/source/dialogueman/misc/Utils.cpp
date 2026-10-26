#include "stdafx.h"

#include <dialogueman/Misc/Utils.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/StringUtil.h>

#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Row.h>

#include <wx/progdlg.h>
#include <wx/regex.h>
#include <wx/file.h>

namespace DMan
{

Xt::DomBlock* FindLocEntry(Xt::DomBlock* pDialogueEntry)
{
	Xt::DomBlock* pLocBlock = Xt::AsBlock(Xt::At(pDialogueEntry, "TextId"));

	if(pLocBlock)
	{
		
		XtId recordID = Xt::ParseId(Xt::AsText(Xt::At(pLocBlock, "record")));

		//Lookup into LocTable based on the entryID and recordID
		Xt::DomRecord* pLocRecord = Xt::FindRecordById(wxGetApp().GetDb(), recordID);
		if (!pLocRecord || pLocRecord->root.type == kXtNullString)
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot find Loc Record with ID: %X.\n", recordID);
			return NULL;
		}

		Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pLocRecord->root.data, "strings"));
		XtId entryID = Xt::ParseId(Xt::AsText(Xt::At(pLocBlock, "id")));
		Xt::DomBlock* pLocEntry = Xt::AsBlock(XtUt::At(&pStringArray->elements, entryID));

		if (!pLocEntry)
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot find Loc entry with ID: %X.\n", entryID);
			return NULL;
		}

		return pLocEntry;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Cannot find TextID block at: %X.\n", pDialogueEntry->parentId);
	}

	return NULL;
}

bool IsPathOk(const wxString& path)
{
	wxString _path = path;
	//get rid of the macro for validation
	wxRegEx macro = wxT("%.*%");
	if ( macro.Matches(_path) )
	{
		macro.Replace(&_path, wxT("DUMMY"));
	}
	
	wxFileName output(path, wxEmptyString);
	
	bool hasFordbidden = PathHasForbiddenChars(output.GetPath(0)) || PathHasForbiddenChars(output.GetVolume());

	if(output.IsOk() && output.IsDir() && output.HasVolume() && output.IsAbsolute() && !hasFordbidden)
	{
		return true;
	}
	return false;
}

Xt::DomRecord* FindRecordByName(Xt::DomPack* pPack, const wxString& recordName)
{
	for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&(pPack->records)); recordIt; ++recordIt)
	{
		Xt::DomRecord* pRec = recordIt;
		if (recordName == (const char *)pRec->name)
			return pRec;
	}

	return 0;
}

} //namespace DMan

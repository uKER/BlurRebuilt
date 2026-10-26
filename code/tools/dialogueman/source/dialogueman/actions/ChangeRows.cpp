#include "stdafx.h"

#include <DialogueMan/Actions/ChangeRows.h>

#include <Tools_Shared/Utility/MacroExpander.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/ViewDisabler.h>
#include <DialogueMan/Misc/StringUtil.h>

#include <DialogueMan/App.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <System/Misc/StringUtil.h>

namespace DMan
{

bool ChangeRows::Change(LarvaType* pRowType, Xt::DomArray* pDialogueArray, Btl::Map<wxString, C_MutantValue>& row, size_t index)
{
	if(!pRowType || !pDialogueArray)
		return false;

	if(index >= Xt::Count(pDialogueArray)) 
	{
		LOGERROR(wxGetApp().GetLogger(), "Invalid index.\n");
		return false;
	}

	bool dataChanged = false;

	Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pDialogueArray, index));
	Xt::DomBlock* pBlock = &pComponent->data;

	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(pRowType);

	Btl::Map<wxString, C_MutantValue>::Iterator itCol;
	XtString name;

	for(itCol = row.Begin(); itCol != row.End(); ++itCol)
	{
		const C_Column* pCol = pColSet->GetColumn(itCol->GetKey());
		if(!pCol) 
			continue;

		C_MutantValue value = itCol->GetValue();

		//For the LocID field
		if (pCol->GetLarvaField()->kind == kLfLocString)
		{
			Xt::DomBlock* pLocBlock = Xt::AsBlock(Xt::At(pBlock, "TextId"));
			XtId id = Xt::ParseId(Xt::AsText(Xt::At(pLocBlock, "id")));
			XtString record = Xt::AsText(Xt::At(pLocBlock, "record"));

			//Lookup into LocTable based on the id and record
			Xt::DomRecord* pRecord = Xt::FindRecordById(wxGetApp().GetDb(), Xt::ParseId(record));
			if (!pRecord || pRecord->root.type == kXtNullString)
			{
				LOGERROR(wxGetApp().GetLogger(), "Cannot find LocTable record.\n");
				return false;
			}

			Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pRecord->root.data, "strings"));
			Xt::DomBlock* pStringBlock = NULL;

			if (id)
				pStringBlock = Xt::AsBlock(XtUt::At(&pStringArray->elements, id));

			if (!pStringBlock)
			{
				LOGERROR(wxGetApp().GetLogger(), "Cannot find entry on LocTable record.\n");
				return false;
			}
			name = Xt::AsText(Xt::At(pStringBlock, "name"));

			Xt::Set(pStringBlock, "english", wxString(value).c_str());
			dataChanged = true;
		}
		else
		{
			Xt::Set(pBlock, pCol->GetLarvaField()->name, wxString(value).c_str());
			dataChanged = true;
		}
	}

	if(dataChanged)
	{
		LOGINFO( wxGetApp().GetLogger(), "Changed row: %s\n", name.Text() );
	}
	else
	{
		LOGERROR( wxGetApp().GetLogger(), "Failed to change row: %s\n", name.Text() );
	}

	return dataChanged;
}

}

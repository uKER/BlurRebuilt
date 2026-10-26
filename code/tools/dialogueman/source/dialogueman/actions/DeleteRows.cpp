#include "stdafx.h"

#include <DialogueMan/Actions/DeleteRows.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/ViewDisabler.h>
#include <DialogueMan/Misc/Utils.h>
#include <DialogueMan/App.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/DbManager.h>
#include <System/Misc/StringUtil.h>

namespace DMan
{

bool DeleteRows::Do()
{
	wxBusyCursor wait; //Display an HourGlass cursor
	
	C_ViewDisabler viewDisabler;
	
	C_Selection selection;
	wxGetApp().GetSelection(selection);

	if (selection.GetMode() != C_Selection::E_Mode_Row)
	{
 		wxMessageDialog msg(NULL, 
			wxT("Selection is invalid.\n") 
			wxT("Please select Rows and try again\n"), 
			wxT("Delete Rows"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return false;
	}

	C_RowSet* pRowSet = selection.GetCurrentRowSet();
	if (!pRowSet)
		return false;

	if (selection.GetItems().Size() == 0)
	{
		assert(!"Must have valid selection!");
		return false;
	}

	C_Selection::C_Item item = selection.GetItems().Back();
	C_Record* pDialogueRecord = wxGetApp().GetDbManager().GetRecord(item.id.GetRecordId());

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	U32 count = Xt::Count(pArray);
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to delete.\n");
		return false;	
	} 
	
	U32 lastIndex = 0;
	Btl::Vector<U32> components = item.components;
	BubbleSort(components);

	if(components.Size() > 0)
	{
		wxMessageDialog confirmation(NULL, 
			wxT("Are you sure wou want to delete the selected row(s) ?"), 
			wxT("Delete Rows Confirmation"),
			wxYES_NO | wxICON_QUESTION);
		if(confirmation.ShowModal() == wxID_YES)
		{
			int totalDeleted = 0;
			for(int i = 0; i < components.Size(); i++)
			{
				lastIndex = components[i];
				if(Delete(Xt::AsArray(pRowSet->GetXtData()), lastIndex))
					totalDeleted++;
				DecreaseCompIndices(components); //so the next iteration works on a valid index
			}
			wxString deleteMsg = wxString::Format("Deleted %i row(s).\n", totalDeleted);
			LOGINFO(wxGetApp().GetLogger(), deleteMsg);
		}
		else
		{
			LOGINFO(wxGetApp().GetLogger(), "Nothing deleted.\n");
			return true;
		}
	}
	
	//try selecting the row that follows the last one selected...
	selection.Clear();
	count = Xt::Count(pArray);

	if(count > 0)
	{
		components.Clear();
		if(lastIndex < count)
		{
			components.PushBack(lastIndex);
		}
		else //... or the previous one if we deleted at the end...
		{
			components.PushBack(count - 1);
		}
		selection.Add(C_Selection::E_Mode_Row, item.id, components);
	}
	//...or we can have a blank selection if we deleted everything
	wxGetApp().SetSelection(selection);

	if(count == 0) //Reset UID
	{
		Xt::DomRecord* pRec = FindRecordById(wxGetApp().GetDb(), pDialogueRecord->GetRecordId());
		Xt::DomField* pNextUidField = Xt::AcquireField(&pRec->root.data, "nextUid");
		Xt::Set(pNextUidField, Format("%i", 0));
	}

	return true;
}

void DeleteRows::DecreaseCompIndices(Btl::Vector<U32>& components)
{
	for(int i = 0; i < components.Size(); i++)
	{
		components[i] = components[i] - 1;
	}
}

bool DeleteRows::Delete(Xt::DomArray* pDialogueArray, size_t rowIndex)
{
	bool rowEntryRemoved = false;
	bool locEntryRemoved = false;

	U32 count = Xt::Count(pDialogueArray);
	if(rowIndex >= count)
	{
		LOGERROR(wxGetApp().GetLogger(), "Invalid index.\n");
		return false;
	}

	Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pDialogueArray, rowIndex));
	Xt::DomBlock* pBlock = &pComponent->data;
	XtId dialogueID = pComponent->id;

	//Find the Loc id and record to lookup in the LocTable
	Xt::DomBlock* pLocBlock = Xt::AsBlock(Xt::At(pBlock, "TextId"));
	XtId locID = Xt::ParseId(Xt::AsText(Xt::At(pLocBlock, "id")));
	XtString record = Xt::AsText(Xt::At(pLocBlock, "record"));

	//Lookup into LocTable based on the id and record
	Xt::DomRecord* pRecord = Xt::FindRecordById(wxGetApp().GetDb(), Xt::ParseId(record));
	if (pRecord && pRecord->root.type != kXtNullString) //attempt to delete Loc entry only if we can find it!
	{
		Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pRecord->root.data, "strings"));
		Xt::DomBlock* pStringBlock = NULL;

		int locIndex = 0;
		for (Xt::DomArray::IndexIterator it(pStringArray); it; ++it)
		{
			if (it.Key() == locID)
			{
				pStringBlock = Xt::AsBlock(it);
				locIndex = it.index;
				break;
			}
		}

		if (pStringBlock)
		{
			XtString name = Xt::AsText(Xt::At(pStringBlock, "name"));
			locEntryRemoved = Xt::Remove(pStringArray, locIndex);
		}
	}

	rowEntryRemoved = Xt::Remove(pDialogueArray, rowIndex);

	if(rowEntryRemoved)
	{
		LOGINFO(wxGetApp().GetLogger(), "Deleted Row: %X\n", dialogueID);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to delete row: %X\n", dialogueID);
		return false;
	}

	return false;
}

}

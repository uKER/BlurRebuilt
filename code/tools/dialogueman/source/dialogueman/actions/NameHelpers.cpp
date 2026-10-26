#include "stdafx.h"

#include <DialogueMan/Actions/NameHelpers.h>

#include <DialogueMan/Misc/DomId.h>
#include <Dialogueman/Misc/StringUtil.h>
#include <Dialogueman/Misc/Utils.h>
#include <DialogueMan/Misc/Selection.h>
#include <DialogueMan/Misc/MacroExpander.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <DialogueMan/App.h>

#include <System/Misc/StringUtil.h>
#include <wx/progdlg.h>


namespace DMan
{

namespace NameHelpers
{

Xt::DomField* FindGroupField(LarvaType* pLarvaType, Xt::DomBlock* pData)
{
	assert(pLarvaType->kind == kLtStruct);
	LtStruct* pStructType = (LtStruct*)pLarvaType;

	LarvaBaseType* pBase = ((LtStruct*)pLarvaType)->base;
	
	if(pBase)
	{
		LtStruct* pBaseType = pBase->type;
		Xt::DomField* pField = FindGroupField(pBaseType, pData);
		if (pField)
			return pField;
	}

	for(XtUt::List<LarvaField>::Iterator it(&pStructType->fields); it; ++it)
	{
		LarvaField* pField = it;
		if (!pField->attrs.FindParam(kXtSemanticAttr, "finalDraftConversation"))
			continue;

		Xt::DomField* pDomField = Xt::AsField(Xt::At(pData, pField->name)); 
		if (!pDomField) 
			continue;

		return pDomField;
	}

	return 0;
}

Btl::OpenHashSet<Character> GetCharactersInRecord(C_Record* pRecord)
{
	Btl::OpenHashSet<Character> characters;

	C_RowSet* pRowSet = pRecord->GetRowSet(0);
	if (!pRowSet)
		return characters;

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	LfArray* arrayField = (LfArray*)pRowSet->GetXtType().AsField();

	for(Xt::DomArray::IndexIterator it(pArray); it; ++it)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		Xt::DomBlock* pBlock = &pComponent->data;

		XtString ch = Xt::AsText(Xt::At(pBlock, "Character"));
		if(ch != "")
			characters.Insert(Util::C_String(ch.Text()).Upper());
	}

	return characters;
}

void BuildConversationGroups(
	ConversationGroups& groups, 
	ConversationGroupMapping& groupMapping,
	C_RowSet* pRowSet)
{
	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);
	LfArray* arrayField = (LfArray*)pRowSet->GetXtType().AsField();

	Btl::Map<Btl::String, U32> contextRefCount;
	Btl::String context;
	U32 refCount = 0;
	for(S32 i = 0; i < pRowSet->GetNumRows(); i++)
	{
		C_Row* pRow = pRowSet->GetRow(i);

		Xt::DomComponent* pComponent = Xt::AsComponent(pRow->GetXtData());
		Xt::DomBlock* pBlock = &pComponent->data;

		LarvaType* pRowType = Xt::TypeOf(wxGetApp().GetDb()->schema, arrayField->field, pComponent);
		if(!pRowType) //cannot determine the type of this row, skip
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot determine the type of row: %X, position: %d.\n", pComponent->id, i + 1);
			continue;
		}

		assert(pRowType->kind == kLtStruct);
		LtStruct* pStructType = (LtStruct*)pRowType;

		Xt::DomField* pDomField = FindGroupField(pStructType, pBlock);
		if (!pDomField || pDomField->value == "")
			continue;

		Xt::DomField* pContext = Xt::AsField(Xt::At(pBlock, "Context")); 
		if (!pContext)
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot determine context for converstation at row: %X, position: %d.\n", pComponent->id, i + 1);
			continue;
		}

		Xt::DomField* pUid = Xt::AsField(Xt::At(pBlock, "uid")); 
		if (!pUid)
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot determine uid for converstation at row: %X, position: %d.\n", pComponent->id, i + 1);
			continue;
		}

		//if need to switch context
		if (context != pContext->value.Text())
		{
			context = pContext->value.Text();
			Btl::Map<Btl::String, U32>::Iterator it = contextRefCount.Find(context);
			if (it == contextRefCount.End())
			{
				//Initialise with ref count of 0.
				refCount = 0;
				contextRefCount.Insert(Btl::Map<Btl::String, U32>::DataType(context, refCount));
			}
			else
			{
				//Increment ref count.
				++(it->GetValue());
				refCount = it->GetValue();
			}
		}

		Btl::String conversationId;
		conversationId.Printf("GroupId_%s:%s:Instance_%i", pDomField->value.Text(), pContext->value.Text(), refCount);
		ConversationGroups::Iterator groupIt = groups.Find(conversationId);
		if (groupIt == groups.End())
		{
			C_Conversation conversation;
			conversation.firstRowUid = ToU32(pUid->value.Text());
			conversation.context = pContext->value.Text();
			ConversationGroups::InsertionReturnType r = 
				groups.Insert(ConversationGroups::DataType(conversationId, conversation));
			groupIt = r.iterator;
		}
		groupIt->GetValue().rowIndices.PushBack(i);
		groupMapping.Insert(ConversationGroupMapping::DataType(i, conversationId));
	}
}

void AddFieldValue(C_MacroExpander& expander, C_ColumnSet* pColumnSet, C_Row* pRow)
{
	for(U32 i = 0; i < pColumnSet->GetNumColumns(); i++)
	{
		C_Column* pColumn = pColumnSet->GetColumn(i);
		if(pColumn)
		{
			const C_Row::ColumnMap& columnMap = pRow->GetColumns();
			C_Row::ColumnMap::ConstIterator it = columnMap.Find(pColumn->GetKey());
			if (it != columnMap.End())
				expander.Insert((const char*)pColumn->GetLabel(), wxString(it->GetValue()).c_str());
		}
	}
}

Util::C_String GetFullWavFileName(C_MacroExpander& expander)
{
	Util::C_String wavFileName = expander.GetValue("Wav Filename");

	if(wavFileName == "")
	{
		wavFileName = expander.GetValue("Name");
		wavFileName += ".wav";
	}

	Util::C_String fullWavFileName = wxGetApp().GetConfig().tts.path.Chars();
	fullWavFileName = Util::AddTrailingSlash(fullWavFileName, '\\');
	fullWavFileName += wavFileName;

	expander.Expand(fullWavFileName);
	fullWavFileName = RemoveUnexpandedMacros(fullWavFileName);
	fullWavFileName = Util::FormatWindowsFileString(fullWavFileName);
	return fullWavFileName;
}

bool GenerateNames(
	C_Record* pDialogueRecord,
	C_RowSet* pRowSet, 
	const ConversationGroups& groups, 
	const ConversationGroupMapping& groupMapping, 
	const wxString& format)
{
 	U32 count = pRowSet->GetNumRows();
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	C_MacroExpander expander;

	int itemsProcessed = 0;

	wxProgressDialog progress(_T("Generating Row Names..."),
							_T("Generating Row Names"),
							count, // range
							NULL,  // parent
							wxPD_APP_MODAL |
							wxPD_AUTO_HIDE |
							wxPD_ELAPSED_TIME |
							wxPD_ESTIMATED_TIME |
							wxPD_REMAINING_TIME
							);

	for(U32 i = 0; i < count; i++)
	{
		progress.Update(itemsProcessed, wxString::Format("Row %d of %d", itemsProcessed, count));

		Util::C_String newName = format.c_str();
		C_Row* pRow = pRowSet->GetRow(i);
		Xt::DomComponent* pComponent = Xt::AsComponent(pRow->GetXtData());
		Xt::DomBlock* pBlock = &pComponent->data;
		
		expander.Clear();
		expander.Insert("Dialogue Record", RemoveFoldersFromRecordName((const char*)pDialogueRecord->GetName()));

		AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);

		Xt::DomBlock* pStringBlock = FindLocEntry(pBlock);
		Xt::DomField* pNameField = Xt::AsField(Xt::At(pStringBlock, "name"));
		XtString oldName = Xt::AsText(pNameField);
		XtString line = Xt::AsText(Xt::At(pStringBlock, "english"));
		
		expander.Expand(newName);
		newName = RemoveUnexpandedMacros(newName);
		newName = newName.Replace(" ", "_");
			
		//now set the new name
		Xt::Set(pNameField, newName);
		pRow->SetDirty();

		itemsProcessed++;
		LOGINFO(wxGetApp().GetLogger(), "Changed Row Name: %s -> %s\n", oldName.Text(), newName.Chars());
	
	} //for(int i = 0; i < pRowSet->GetNumRows(); i++)

	LOGINFO(wxGetApp().GetLogger(), "Generated %i Row Name(s).\n", itemsProcessed);
	return true;
}

bool GenerateWavNames(
	C_Record* pDialogueRecord,
	C_RowSet* pRowSet, 
	const ConversationGroups& groups, 
	const ConversationGroupMapping& groupMapping, 
	const wxString& format)
{
	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);
	LfArray* arrayField = (LfArray*)pRowSet->GetXtType().AsField();

	int rowNumber = 1;
	C_MacroExpander expander;

	int itemsToProcess = Xt::Count(pArray);
	int itemsProcessed = 0;

	if (!itemsToProcess) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	wxProgressDialog progress(_T("Generating Wav Names..."),
							_T("Generating Wav Names"),
							itemsToProcess, // range
							NULL,  // parent
							wxPD_APP_MODAL |
							wxPD_AUTO_HIDE |
							wxPD_ELAPSED_TIME |
							wxPD_ESTIMATED_TIME |
							wxPD_REMAINING_TIME
							);

	for(Xt::DomArray::IndexIterator it(pArray); it; ++it, ++rowNumber)
	{
		progress.Update(itemsProcessed, wxString::Format("Row %d of %d", itemsProcessed, itemsToProcess));
		C_Row* pRow = pRowSet->GetRow(it.Index());
		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		Xt::DomBlock* pBlock = &pComponent->data;

		expander.Clear();
		Util::C_String wavFileName = format;

		AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);

		//now for the TextId
		Xt::DomBlock* pLocBlock = Xt::AsBlock(Xt::At(pBlock, "TextId"));
		XtId id = Xt::ParseId(Xt::AsText(Xt::At(pLocBlock, "id")));
		XtString record = Xt::AsText(Xt::At(pLocBlock, "record"));

		//Lookup into LocTable based on the id and record
		Xt::DomRecord* pRecord = Xt::FindRecordById(wxGetApp().GetDb(), Xt::ParseId(record));
		if (!pRecord || pRecord->root.type == kXtNullString)
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot find Loc record at row: %X, position: %d.\n", pComponent->id, rowNumber);
			continue;
		}

		Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pRecord->root.data, "strings"));
		Xt::DomBlock* pStringBlock = NULL;
		
		if (id)
			pStringBlock = Xt::AsBlock(XtUt::At(&pStringArray->elements, id));

		if (!pStringBlock)
		{
			LOGERROR(wxGetApp().GetLogger(), "Cannot find Loc string at row: %X, position: %d.\n", pComponent->id, rowNumber);
			continue;
		}

		XtString line = Xt::AsText(Xt::At(pStringBlock, "english"));
		XtString name = Xt::AsText(Xt::At(pStringBlock, "name"));

		expander.Insert("Dialogue Record", RemoveFoldersFromRecordName((const char*)pDialogueRecord->GetName()));
		expander.Insert("Loc Record", (const char*)pRecord->name);
		expander.Insert("Name", (const char*)name);
		expander.Insert("English", (const char*)line);
		
		//Obtain the full WAV path
		//bool isConversation = true;
		//ConversationGroupMapping::ConstIterator groupIt = groupMapping.Find(it.Index());
		//If this row is part of a conversation...
		//if (groupIt != groupMapping.End())
			//expander.Insert("CHARACTER", "Conversation");

		expander.Expand(wavFileName);
		wavFileName = RemoveUnexpandedMacros(wavFileName);
		wavFileName = wavFileName.Replace("\\\\", "\\");
		wavFileName = Util::FormatWindowsFileString(wavFileName);

		//now set the new name
		Xt::DomField* pWavField = Xt::AcquireField(pBlock, "Wav");
		Xt::Set(pWavField, wavFileName);
		pRow->SetDirty();

		++itemsProcessed;
		LOGINFO(wxGetApp().GetLogger(), "Generated Wav Name: %s\n", wavFileName.Chars());
	}

	LOGINFO(wxGetApp().GetLogger(), "Generated %d Wav Names.\n", itemsProcessed);
	return true;
}

} //namespace NameHelpers

} //namespace DMan

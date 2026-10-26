#include "stdafx.h"

#include <DialogueMan/Actions/GenerateHashes.h>
#include <DialogueMan/Actions/NameHelpers.h>

#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <DialogueMan/Misc/Utils.h>

#include <DialogueMan/App.h>

#include <tools_shared/utility/MacroExpander.h>

#include <wx/progdlg.h>

namespace DMan
{

namespace GenerateHashes
{

void WriteHashesArray(Xt::DomBlock* pBlock, const Btl::Vector<U32>& hashes)
{
	Util::C_String loadedProject = "%BIZARRE_PROJECT%";
	Tools::C_MacroExpander::Instance().Expand(loadedProject);
	assert(loadedProject == "Amax");
	
	Xt::DomArray* pHashArray = Xt::AcquireArray(pBlock, "CharacterHashesInConversation");
	if(pHashArray)
	{
		Xt::Clear(pHashArray);
		for(int i = 0; i < hashes.Size(); i++)
		{
			Xt::DomField* pValue = new Xt::DomField();
			pValue->value = Format("%u", hashes[i]);
			Xt::Append(pHashArray, pValue);
		}
	}
}

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO GENERATE HASHES FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	DialogueManConfig& config = wxGetApp().GetConfig(); 
	Util::C_String layout = config.locId;

	C_RowSet* pRowSet = options.GetRecord() ->GetRowSet(0);
	if (!pRowSet)
		return false;
	
	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);
	LfArray* arrayField = (LfArray*)pRowSet->GetXtType().AsField();

	U32 count = Xt::Count(pArray);
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	wxProgressDialog progress(_T("Generating Hashes..."),
								_T("Please Wait..."),
								100,    // range
								NULL,   // parent
								wxPD_APP_MODAL |
								wxPD_AUTO_HIDE);

	U32 generated = 0;
	for (S32 iCol = 0; iCol < (S32)pRowSet->GetAllColumnSet()->GetNumColumns(); ++iCol)
	{
		C_Column* pColumn = pRowSet->GetAllColumnSet()->GetColumn(iCol);
		if(pColumn->GetLarvaField())
		{
			XtString hashOf = pColumn->GetLarvaField()->attrs.GetString(kXtSemanticAttr, "hashOf", "");
			if(hashOf != "")
			{
				for(Xt::DomArray::IndexIterator it(pArray); it; ++it)
				{
					progress.Pulse();

					Xt::DomComponent* pComponent = Xt::AsComponent(it);
					Xt::DomBlock* pBlock = &pComponent->data;
					Btl::String valueToHash = Xt::AsText(Xt::At(pBlock, hashOf));
					if(valueToHash != "")
					{
						U32 hash = Btl::Hash(valueToHash.Upper().Chars());
						Xt::Set( pBlock, pColumn->GetLarvaField()->name, Format("%u", hash));
						generated++;
					}
				}
			}
		}
	}

	Util::C_String loadedProject = "%BIZARRE_PROJECT%";
	Tools::C_MacroExpander::Instance().Expand(loadedProject);

	if(loadedProject == "Amax")
	{
		NameHelpers::ConversationGroups groups;
		NameHelpers::ConversationGroupMapping groupMapping;		
		NameHelpers::BuildConversationGroups(groups, groupMapping, pRowSet);

		Btl::String prevConversation = "";
		Btl::String currConversation = "";

		Xt::DomBlock* pFirstConversationBlock = NULL;

		Btl::Vector<U32> characterHashesInConversation;
		int rowIndex = 0;
		for(Xt::DomArray::IndexIterator it(pArray); it; ++it, ++rowIndex)
		{
			progress.Pulse();

			Xt::DomComponent* pComponent = Xt::AsComponent(it);
			Xt::DomBlock* pBlock = &pComponent->data;

			NameHelpers::ConversationGroupMapping::ConstIterator groupIt = groupMapping.Find(rowIndex);
			//If this row is part of a conversation...
			if (groupIt != groupMapping.End())
			{
				if(prevConversation == "")
					prevConversation = groupIt->GetValue();

				if(!pFirstConversationBlock)
					pFirstConversationBlock = pBlock;

				currConversation = groupIt->GetValue();
				if(currConversation != prevConversation) 
				{
					WriteHashesArray(pFirstConversationBlock, characterHashesInConversation);
					characterHashesInConversation.Clear();
					pFirstConversationBlock = pBlock;
				}

				Btl::String character = Xt::AsText(Xt::At(pBlock, "Character"));
				if(character != "")
				{
					U32 hash = Btl::Hash(character.Upper().Chars());
					characterHashesInConversation.PushBack(hash);
				}

				prevConversation = currConversation; 						
			}
		}

		/* HACK to remove duplicates. Do not use
		for(Xt::DomArray::IndexIterator it(pArray); it; ++it)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(it);
			Xt::DomBlock* pBlock = &pComponent->data;

			Xt::DomBlock* pLocBlock = FindLocEntry(pBlock);
			if(pLocBlock)
			{
				XtString name = Xt::AsText(Xt::At(pLocBlock, "name"));
			
				Xt::DomRecord* pLocRecord = Xt::FindRecordById(wxGetApp().GetDb(), Xt::ParseId("41F94941"));
				Xt::DomArray* pStringArray = Xt::AsArray(Xt::At(&pLocRecord->root.data, "strings"));

				for(Xt::DomArray::IndexIterator locIt(pStringArray); locIt; ++locIt)
				{
					Xt::DomBlock* pOtherLocBlock = Xt::AsBlock(locIt);
					XtString otherName = Xt::AsText(Xt::At(pOtherLocBlock, "name"));
					if(otherName == name && pLocBlock != pOtherLocBlock)
					{
						Xt::RemoveById(pStringArray, pOtherLocBlock->parentId);
					}
				}

			}

		}*/

		//for the last conversation block
		WriteHashesArray(pFirstConversationBlock, characterHashesInConversation);
		characterHashesInConversation.Clear();
	}

	LOGINFO(wxGetApp().GetLogger(), "Generated %u hash values.\n", generated);
	LOGINFO(wxGetApp().GetLogger(), "<<< DONE GENERATING HASHES FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	return true;
}

} //namespace GenerateHashes

} //namespace DMan
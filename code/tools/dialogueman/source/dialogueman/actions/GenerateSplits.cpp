#include "stdafx.h"

#include <DialogueMan/Actions/GenerateSplits.h>
#include <DialogueMan/Actions/NameHelpers.h>

#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>

#include <DialogueMan/App.h>

#include <tools_shared/utility/MacroExpander.h>

#include <wx/progdlg.h>

namespace DMan
{

namespace GenerateSplits
{

bool Do(const C_Options& options)
{
	LOGINFO(wxGetApp().GetLogger(), ">>> STARTING TO GENERATE SPLIT GROUPS FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	C_RowSet* pRowSet = options.GetRecord()->GetRowSet(0);
	if (!pRowSet)
		return false;

	Util::C_String loadedProject = "%BIZARRE_PROJECT%";
	Tools::C_MacroExpander::Instance().Expand(loadedProject);

	bool ok = true;
	if(loadedProject == "Amax")
	{
		Xt::DomElement* pElement = pRowSet->GetXtData();
		Xt::DomArray* pArray = Xt::AsArray(pElement);

		//Generate Conversation Hashes
		for(Xt::DomArray::IndexIterator it(pArray); it; ++it)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(it);
			Xt::DomBlock* pBlock = &pComponent->data;

			Btl::String conversation = Xt::AsText(Xt::At(pBlock, "Conversation"));
				  
			U32 hash = Btl::Hash(conversation.Upper().Chars());
			Xt::Set( pBlock, "ConversationHash", Format("%u", hash));
		}

		//Obtain Split Groups
		int splitGroup = 0;
		Xt::DomArray::IndexIterator begin(pArray);
		Xt::DomArray::IndexIterator end(pArray);

		bool ok = true;

		while(end)
		{
			FindEnd(begin, end);

			int splitIndex = 0;
			for(Xt::DomArray::IndexIterator it = begin; it.index < end.index; ++it)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(it);
				Xt::DomBlock* pBlock = &pComponent->data;
				Xt::Set( pBlock, "SplitGroup", Format("%u", splitGroup)); //Set this for visualization/debug only
				Xt::Set( pBlock, "SplitIndex", Format("%u", splitIndex)); //Set this for visualization/debug only
				splitIndex++;
			}

			splitGroup++;
			begin = end;
		}
	}
	else if(loadedProject == "Nightwing")
	{
		LOGERROR(wxGetApp().GetLogger(), "This tool is only available in Amax at the moment");
		return false;
	}

	LOGINFO(wxGetApp().GetLogger(), "<<< DONE GENERATING SPLIT GROUPS FOR RECORD \"%s\"", 
		options.GetRecord()->GetName().Text());

	return true;
}

void FindEnd(const Xt::DomArray::IndexIterator begin, Xt::DomArray::IndexIterator& end)
{
	wxString prevConv = "";
	wxString currConv = "";
	wxString nextConv = "";

	int count = 0;
	int countAtNextConv = 0;

	Xt::DomArray::IndexIterator it = begin;
	for(; it; ++it, ++count)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		Xt::DomBlock* pBlock = &pComponent->data;

		currConv = Xt::AsText(Xt::At(pBlock, "ConversationHash"));
		wxString uid = Xt::AsText(Xt::At(pBlock, "uid"));

		if(currConv != prevConv)
		{
			countAtNextConv = count;
			Xt::DomArray::IndexIterator splitIt = it;
			Xt::DomArray::IndexIterator tmpIt = it;
			bool passedLimit = false;
			bool currConvMayBeBroken = false;

			while(true)
			{
				++splitIt;
				++countAtNextConv;

				if(countAtNextConv % SPLIT_SIZE == 0)
					passedLimit = true;

				if(!splitIt)
					break;

				Xt::DomComponent* pSplitComponent = Xt::AsComponent(splitIt);
				Xt::DomBlock* pSplitBlock = &pSplitComponent->data;
				nextConv = Xt::AsText(Xt::At(pSplitBlock, "ConversationHash"));

				if(nextConv == currConv) //limit is in the middle of conversation 
				{
					if(passedLimit)
					{
						currConvMayBeBroken = true;
						break;
					}
				}
				else
				{
					currConvMayBeBroken = false;
					break;
				}
			}

			if(currConvMayBeBroken)
			{
				end = tmpIt;
				return;
			}
			else
			{
				if(passedLimit)
				{
					end = splitIt;
					return;	
				}
			}
		}

		prevConv = currConv;
	}
	
	end = it;
	return;
}

} //namespace GenerateSplits

} //namespace DMan
#include "stdafx.h"
#include <DialogueMan/ImportExport/Reader.h>
#include <DialogueMan/ImportExport/FinalDraftReader.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Rowset.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <wx/regex.h>
#include <wx/textfile.h>

namespace DMan
{
C_FinalDraftReader::C_FinalDraftReader()
{}

C_FinalDraftReader::~C_FinalDraftReader()
{
	CloseFile();
}

bool C_FinalDraftReader::OpenFile()
{
	if(IsFileOpen())
	{
		CloseFile();
	}
		
	return m_file.Open(m_fileName.GetFullPath(), wxFile::read);
}

bool C_FinalDraftReader::CloseFile()
{
	if(IsFileOpen())
	{
		m_file.Close();
		return true;
	}
	return false;
}


bool C_FinalDraftReader::Read()
{
	if(IsFileOpen())
	{	
		ObtainColumnMapping();
		wxFileOffset size = m_file.Length();
		if(size == wxInvalidOffset)
			return false;

		/*
		The Final Draft binary file is composed of chunks of different elements.
		Each element has the following pattern:

		"FTXT" x00 x00 x00 ID x00 x00 x00 SIGNATURE VALUE x0D

		ID uniquely identifies a value, for instance Bond character could 
		be identified by xB7 and M by 0xA1 (ignored by now).
		
		SIGNATURE identifies an element type, for instance x03 means this 
		chunk is a Character.
		
		VALUE is the actual value of the element, for instance "Bond" for a Character type.
		*/

		//Get Element chunks
		{

			U8* pData = new U8[size];
			if(m_file.Read(pData, (size_t)size) != size)
			{
				delete pData;
				pData = NULL;
				return false;
			}

			//extract elements that start with FTXT
			wxFileOffset currOff = 0;
			wxFileOffset prevOff = 0;
			for(currOff = 0; currOff < size - 3; currOff++)
			{
				if( pData[currOff    ] == 'F' && 
					pData[currOff + 1] == 'T' && 
					pData[currOff + 2] == 'X' &&
					pData[currOff + 3] == 'T')
				{
					wxMemoryBuffer elementBuffer;
					for(wxFileOffset i = prevOff; i < currOff - 1; i++) //ignore CARRIAGE_RETURN
					{
						 elementBuffer.AppendByte(pData[i]);
					}
					m_rawElements.PushBack(elementBuffer);
					prevOff = currOff;
				}
			}
			//now push the last element because the previous loop didn't consider it
			wxMemoryBuffer elementBuffer;
			for(wxFileOffset i = prevOff; i < size - 1; i++) //ignore CARRIAGE_RETURN
			{
				 elementBuffer.AppendByte(pData[i]);
			}
			m_rawElements.PushBack(elementBuffer);
			  
			delete pData;
			pData = NULL;
		}

		const char* kTriggerColonStr = "TRIGGER:";
		const char* kLocationColonStr = "LOCATION:";
		const char* kEventColonStr = "EVENT:";
		const char* kConversationColonStr = "CONVERSATION:";

		//Process each raw element
		{
			Btl::Map<wxString, C_MutantValue> readRow;
			int sequenceNum = 1;

			for(int i = 0; i < m_rawElements.Size(); i++)	
			{
				const wxMemoryBuffer& rawElement = m_rawElements[i];
				wxString rawElementStr = wxString::From8BitData(rawElement, rawElement.GetDataLen());
				int ftxtPos = rawElementStr.Find("FTXT");
				if(ftxtPos == wxNOT_FOUND)
					continue;

				//Get the SIGNATURE
				U8 elementSignature = rawElement[ftxtPos + 11];

				//Get the VALUE
				wxString rawValue = rawElementStr.Mid(ftxtPos + 12).Trim(true).Trim(false);
				if(rawValue.Length() == 0)
					continue;
				
				//Check for Trigger
				int triggerStrStart = rawValue.Upper().Find(kTriggerColonStr);
				if (triggerStrStart != wxNOT_FOUND)
					elementSignature = SIGNATURE_TRIGGER;

				//Check for Location
				int locationStrStart = rawValue.Upper().Find(kLocationColonStr);
				if (locationStrStart != wxNOT_FOUND)
					elementSignature = SIGNATURE_LOCATION;

				//Check for Event
				int eventStrStart = rawValue.Upper().Find(kEventColonStr);
				if (eventStrStart != wxNOT_FOUND)
					elementSignature = SIGNATURE_EVENT;

				//Check for Conversation
				int conversationStrStart = rawValue.Upper().Find(kConversationColonStr);
				if (conversationStrStart != wxNOT_FOUND)
					elementSignature = SIGNATURE_CONVERSATION;
				
				Btl::Map<U8, const C_Column*>::Iterator it = m_ColumnMapping.Find(elementSignature);
				const C_Column* pCol = NULL;
				if(it != m_ColumnMapping.End())
				{
					pCol = it->GetValue();
				}

				switch(elementSignature)
				{
				case SIGNATURE_GENERAL:
					{
						if(pCol)
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);

						break;
					}

				case SIGNATURE_SCENEHEADING:
					{
						wxString sceneHeadingIntro, sceneHeadingLocation, sceneHeadingTime;

						ProcessSceneHeadingElement(rawValue, 
							sceneHeadingIntro, sceneHeadingLocation, sceneHeadingTime);

						if(pCol)
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, sceneHeadingLocation);

						break;
					}

				case SIGNATURE_ACTION:
					{
						if(pCol)
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);

						break;
					}

				case SIGNATURE_CHARACTER:
					{
						wxString characterName, characterExtension;

						ProcessCharacterElement(rawValue, 
							characterName, characterExtension);

						if(pCol)
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, characterName);

						break;
					}

				case SIGNATURE_PARENTHETICAL:
					{
						wxString parenthetical;

						ProcessParentheticalElement(rawValue, 
							parenthetical);

						if(pCol)
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, parenthetical);

						break;
					}

				case SIGNATURE_DIALOGUE:
					{
						if(pCol) 
						{
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);

							//Store sequence
							it = m_ColumnMapping.Find(SIGNATURE_SEQUENCE);
							if(it != m_ColumnMapping.End())
							{
								pCol = it->GetValue();
								C_MutantValue blank;
								readRow[pCol->GetKey()] = C_MutantValue(E_DataType_Int, sequenceNum);
							}
							sequenceNum++;

							//Push the row filled so far in the array because a Dialogue value is the finest
							//level of granularity we expect on a row. 
							m_readRows.PushBack(readRow);

							//Clear Parenthetical because it should be unique to each Dialogue
							it = m_ColumnMapping.Find(SIGNATURE_PARENTHETICAL);
							if(it != m_ColumnMapping.End())
							{
								pCol = it->GetValue();
								C_MutantValue blank;
								readRow[pCol->GetKey()] = blank;
							}
						}

						break;
					}

				case SIGNATURE_TRANSITION:
					{
						if(pCol)
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);

						break;
					}
	
				case SIGNATURE_TRIGGER:
					{
						if(pCol)
						{
							rawValue = rawValue.SubString(triggerStrStart + strlen(kTriggerColonStr), rawValue.Length()).Trim(true).Trim(false);
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);
						}
						break;
					}
				case SIGNATURE_LOCATION:
					{
						if(pCol)
						{
							rawValue = rawValue.SubString(locationStrStart + strlen(kLocationColonStr), rawValue.Length()).Trim(true).Trim(false);
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);
						}
						break;
					}
				case SIGNATURE_CONVERSATION:
					{
						if(pCol)
						{
							rawValue = rawValue.SubString(conversationStrStart + strlen(kConversationColonStr), rawValue.Length()).Trim(true).Trim(false);
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);
						}
						break;
					}
				case SIGNATURE_EVENT:
					{
						if(pCol)
						{
							rawValue = rawValue.SubString(eventStrStart + strlen(kEventColonStr), rawValue.Length()).Trim(true).Trim(false);
							readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, rawValue);
						}
						break;
					}
				}
			}
		}

		return true;	
	} //if(IsFileOpen())
	
	return false;	
}

void C_FinalDraftReader::ObtainColumnMapping()
{	
	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(GetItemType());
							
	if(pColSet)
	{
		m_ColumnMapping.Clear();
		U32 numCols = pColSet->GetNumColumns();
		for(U32 i = 0; i < numCols; ++i)
		{
			C_Column* pCol = pColSet->GetColumn(i);
			if(pCol && pCol->IsImportable() && pCol->GetLarvaField())
			{
				//Retrieve index from annotation if available
				int finalDraftSignature = pCol->GetLarvaField()->attrs.GetInt(kXtSemanticAttr, "finalDraftSignature", -1);
				if(finalDraftSignature != -1)
				{
					if (pCol->GetLarvaField()->attrs.FindParam(kXtSemanticAttr, "finalDraftConversation"))
						m_ColumnMapping[SIGNATURE_CONVERSATION] = pCol;
					else if (pCol->GetLarvaField()->attrs.FindParam(kXtSemanticAttr, "finalDraftTrigger"))
						m_ColumnMapping[SIGNATURE_TRIGGER] = pCol;
					else if (pCol->GetLarvaField()->attrs.FindParam(kXtSemanticAttr, "finalDraftLocation"))
						m_ColumnMapping[SIGNATURE_LOCATION] = pCol;
					else if (pCol->GetLarvaField()->attrs.FindParam(kXtSemanticAttr, "finalDraftEvent"))
						m_ColumnMapping[SIGNATURE_EVENT] = pCol;
					else
						m_ColumnMapping[finalDraftSignature] = pCol;
				}
			}
		}
	}
}

const C_Column* C_FinalDraftReader::GetColumnFromKey(const wxString& colKey)
{
	Btl::Map<U8, const C_Column*>::Iterator itCol;
			
	for(itCol = m_ColumnMapping.Begin(); itCol != m_ColumnMapping.End(); ++itCol)
	{
		const C_Column* pCol = itCol->GetValue();
		if(pCol == NULL) 
			continue;

		if(pCol->GetKey() == colKey)
			return pCol;
	}

	return NULL;
}

void C_FinalDraftReader::ProcessSceneHeadingElement(const wxString& rawSceneElement, 
		wxString& sceneIntro, wxString& sceneLocation, wxString& sceneTime)
{
	wxRegEx regEx;
	
	bool ok = regEx.Compile("^(.*\\.)(.*)(\\-.*)$");
	assert(ok);

	if(regEx.Matches(rawSceneElement))
	{
		sceneIntro = regEx.GetMatch(rawSceneElement, 1).RemoveLast().Trim(true).Trim(false);
		sceneLocation = regEx.GetMatch(rawSceneElement, 2).Trim(true).Trim(false);
		sceneTime = regEx.GetMatch(rawSceneElement, 3).Mid(1).Trim(true).Trim(false);
	}
	else
	{
		sceneLocation = rawSceneElement;
	}
}

void C_FinalDraftReader::ProcessCharacterElement(const wxString& rawCharacterElement, 
	wxString& characterName, wxString& characterExtension)
{
	wxRegEx regEx;
	
	bool ok = regEx.Compile("^(.*)(\\(.*\\))$");
	assert(ok);

	if(regEx.Matches(rawCharacterElement))
	{
		characterName = regEx.GetMatch(rawCharacterElement, 1).Trim(true).Trim(false);
		characterExtension = regEx.GetMatch(rawCharacterElement, 2).Mid(1).RemoveLast().Trim(true).Trim(false);
	}
	else
	{
		characterName = rawCharacterElement;
	}
}

void C_FinalDraftReader::ProcessParentheticalElement(const wxString& rawParentheticalElement, 
		wxString& parenthetical)
{
	wxRegEx regEx;
	
	bool ok = regEx.Compile("^\\((.*)\\)$");
	assert(ok);

	if(regEx.Matches(rawParentheticalElement))
	{
		parenthetical = regEx.GetMatch(rawParentheticalElement, 1).Trim(true).Trim(false);
	}
	else
	{
		parenthetical = rawParentheticalElement;
	}

}

} //namespace DMan
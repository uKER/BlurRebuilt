#include "stdafx.h"

#include <DialogueMan/Actions/Nightwing/NightwingAudioTemplates.h>
#include <Dialogueman/Actions/NameHelpers.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <Dialogueman/Misc/StringUtil.h>
#include <Dialogueman/Misc/Utils.h>
#include <Dialogueman/Misc/MacroExpander.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/DbManager.h>

#include <DialogueMan/App.h>

#include <System/Misc/StringUtil.h>

#include <wx/progdlg.h>
#include <wx/regex.h>
#include <wx/file.h>

namespace DMan
{
namespace AudioXML
{
namespace Nightwing
{

AudioXML::AudioXML(const wxString& fileName) : m_fileName(fileName)
{
}
		
bool AudioXML::Write(C_Record* pRecord)
{
	C_RowSet* pRowSet = pRecord->GetRowSet(0);
	if (!pRowSet)
		return false;

	U32 count = pRowSet->GetNumRows();
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	Btl::OpenHashSet<Character> charactersInRecord = NameHelpers::GetCharactersInRecord(pRecord);
	wxArrayString characterNames;
	Btl::OpenHashSet<Character>::Iterator charactersIt;

	wxString fileContents = WriteHeader(
		RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Chars());
	
	//Get all the character names
	C_MacroExpander expander;
	for(charactersIt = charactersInRecord.Begin(); charactersIt != charactersInRecord.End(); ++charactersIt)
	{
		Util::C_String theCharacter = charactersIt->GetKey();
		characterNames.Add(theCharacter.Chars());
	}

	//Write Character Stop Block
	fileContents += WriteCharacterStopBlock(characterNames);

	//Write Character Play Block
	for(size_t i = 0; i < characterNames.GetCount(); i++)
	{
		wxString characterName = characterNames[i];

		wxArrayString locRecordNames;
		for(int i = 0; i < pRowSet->GetNumRows(); i++)
		{
			C_Row* pRow = pRowSet->GetRow(i);
			Xt::DomComponent* pComponent = Xt::AsComponent(pRow->GetXtData());
			Xt::DomBlock* pBlock = &pComponent->data;
			Btl::String rowCharacter = Xt::AsText(Xt::At(pBlock, "Character"));

			if(rowCharacter.Upper() != characterName.Upper())
				continue;

			expander.Clear();
			NameHelpers::AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);				
			locRecordNames.Add((const char*)expander.GetValue("Name"));		
		}
		fileContents += WriteCharacterPlayBlock(
			RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Chars(), 
			characterName, locRecordNames);

	}
	fileContents += WriteFooter(characterNames);

	Util::C_String outputFile = m_fileName.c_str();
	expander.Clear();
	expander.Insert("Dialogue Record", RemoveFoldersFromRecordName((const char*)pRecord->GetName()));
	expander.Expand(outputFile);
	outputFile = RemoveUnexpandedMacros(outputFile);
	outputFile = outputFile.Replace("\\\\", "\\");
	outputFile = Util::FormatWindowsFileString(outputFile);

	wxFileName fileName(outputFile.Chars());
	wxString directory = fileName.GetPath();
	wxString fullName = fileName.GetFullPath();
	bool ok = wxFileName::Mkdir(directory, 0777, wxPATH_MKDIR_FULL);
	
	wxFile file;
	if(ok)
		ok = file.Create(fullName, true);
	if(ok)
		ok = file.Open(fullName, wxFile::write);
	if(ok)
		ok = file.Write(fileContents);
	
	if(ok)
	{
		LOGINFO(wxGetApp().GetLogger(), "Audio XML file written at: %s\n", fullName);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to write Audio XML file written at: %s\n", fullName);
		return false;
	}

}

wxString AudioXML::WriteHeader(wxString dialogueRecordName)
{
	wxString retVal;
	dialogueRecordName.Replace(" ", "_");

	retVal +=  wxString::Format("<Object Name = \"VO_%s\">\n\n", dialogueRecordName);
	
	return retVal;
}

wxString AudioXML::WriteCharacterStopBlock(wxArrayString characterNames)
{
	wxString retVal;

	retVal += 
"<ActionIn \n\
Name = \"AllCharacterStop_ActionIn\" \n\
ActionScr = \"AllCharacterStop_Script\" \n\
/>\n\n";

	retVal +=
"<Script \n\
Name = \"AllCharacterStop_Script\">\n";

	for(size_t i = 0; i < characterNames.GetCount(); i++)
	{
		wxString characterName = characterNames[i];
		characterName.Replace(" ", "_");
		retVal += wxString::Format("SEND \"STOP\" \"%s_Sound\"\n", characterName);
	}

	retVal += "EXIT\n";

	retVal += 
"</Script>\n\n";
	
	return retVal;
}

wxString AudioXML::WriteCharacterPlayBlock(wxString dialogueRecordName, 
							 wxString characterName, 
							 wxArrayString locRecordNames)
{
	wxString retVal;
	dialogueRecordName.Replace(" ", "_");
	characterName.Replace(" ", "_");

	retVal += wxString::Format("<!-- Character: %s -->\n\n", characterName);
	
	retVal += wxString::Format("<ActionIn Name = \"%s_ActionIn\"\nActionScr = \"%s_Script\"\n/>\n\n", 
		characterName, characterName);
	
	retVal += wxString::Format(
		"<Script \nName = \"%s_Script\"> \nEXEC \"%s_PlayScript\" \nEXIT\n</Script>\n\n", 
		characterName, characterName);
	
	retVal += wxString::Format(
		"<Script \nName = \"%s_PlayScript\"> \nSEND \"PLAY\" \"%s_Sound\" \nEXIT\n</Script>\n\n", 
		characterName, characterName);
	
	retVal += wxString::Format(
		"<ParameterIn \nName = \"%s_Index\" \nDefaultVal = \"0.0\"\n/>\n\n", characterName);
	
	retVal += wxString::Format(
		"<MonoOneShotSoundTable \nName = \"%s_Sound\" \nBank = \"VO_%s_Bank\" \nIndexCom = \"%s_Index\">\n", 
		characterName, dialogueRecordName, characterName);

	for(size_t i = 0; i < locRecordNames.GetCount(); i++)
	{
		retVal += wxString::Format("<MonoOneShotSoundTableEntry \nWave = \"%s\"/>\n", locRecordNames[i]);
	}
	
	//HACK... We have to repeat the first entry to prevent issues with the external audio component 
	retVal += wxString::Format("<MonoOneShotSoundTableEntry \nWave = \"%s\"/>\n", locRecordNames[0]);
	//...HACK

	retVal += "</MonoOneShotSoundTable>\n\n";

	retVal += wxString::Format(
		"<MonoToMonoMixer \nName = \"%s_Mixer\" \nGainVal = \"0.0\"> \n<MonoToMonoMixerInput \nInputCom = \"%s_Sound\"/> \n</MonoToMonoMixer>\n\n", 
		characterName, characterName);

	retVal += wxString::Format(
		"<MonoAudioOutput \nName = \"VO_%s_Output\" \nInputCom = \"%s_Mixer\" \n/>\n\n", characterName, characterName);
	
	return retVal;
}

wxString AudioXML::WriteFooter(wxArrayString characterNames)
{
	wxString retVal;

	retVal += "<!-- DialogueFinished --> \n\n";
	retVal += "<Sum	\nName = \"DialogueActive\">\n";

	//HACK... We have put this entry to prevent issues with the external audio component 
	retVal += "<SumInput InputVal = \"0\"/>\n";
	//...HACK

	for(size_t i = 0; i < characterNames.GetCount(); i++)
	{
		wxString characterName = characterNames[i];
		characterName.Replace(" ", "_");
		retVal += wxString::Format("<SumInput InputCom = \"%s_Sound(Active)\"/>\n", characterName);
	}

	retVal += "</Sum>\n\n";

	retVal += 
"\t<GateSwitch \n\
\t\tName = \"DialogueFinishedGate\" \n \
\t\tInputCom = \"DialogueActive\" \n \
\t\tThresholdVal = \"0.1\" \n \
\t\tFallingScr = \"DialogueFinishedScript\"/> \n\n \
\t<Script Name = \"DialogueFinishedScript\"> \n \
\t\tEXEC \"AudioStopped\" \n \
\t\tEXIT \n \
\t</Script> \n\n \
\t<ActionOut \n \
\t\tName = \"AudioStopped\"/>\n\n";

	retVal += "</Object>";
	
	return retVal;
}

} //namespace Nightwing
} //namespace AudioXML
} //namespace DMan

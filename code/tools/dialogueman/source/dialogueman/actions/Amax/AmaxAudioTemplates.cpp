#include "stdafx.h"

#include <DialogueMan/Actions/Amax/AmaxAudioTemplates.h>
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
namespace Amax
{

/////////////////////////// AudioXML \\\\\\\\\\\\\\\\\\\\\\\\\\\

AudioXML::AudioXML(const wxString& fileName) : m_fileName(fileName)
{
}
		
bool AudioXML::Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end)
{
	C_RowSet* pRowSet = pRecord->GetRowSet(0);
	if (!pRowSet)
		return false;

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	U32 count = end.index - begin.index;
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	Util::C_String dialogueRecord =	
		RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Replace(" ", "_");

	Util::C_String splitGroupName = Format("%s_%i", dialogueRecord.Chars(), splitGroup);

	wxProgressDialog progress(_T("Generating AudioXML..."),
								_T("Generating AudioXML"),
								count,    // range
								NULL,   // parent
								wxPD_APP_MODAL |
								wxPD_AUTO_HIDE |
								wxPD_ELAPSED_TIME |
								wxPD_ESTIMATED_TIME |
								wxPD_REMAINING_TIME
								); 

	wxString fileContents = WriteHeader(splitGroupName.Chars());

	int rowNumber = 1;
	wxArrayString wavFileNames;
	for(Xt::DomArray::IndexIterator it = begin; it.index < end.index; ++it, ++rowNumber)
	{
		progress.Update(rowNumber, wxString::Format("Row %d of %d", rowNumber, count));
		wxMilliSleep(10);

		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		Xt::DomBlock* pBlock = &pComponent->data;
		Util::C_String wavFileName = Xt::AsText(Xt::At(pBlock, "Wav"));
		wavFileName = Util::ExtractFilename(wavFileName);
		wavFileName = Util::RemoveFileExtension(wavFileName);
		wavFileNames.Add(wavFileName.Chars());		
	}
	fileContents += WriteBody(splitGroupName.Chars(), wavFileNames);
	fileContents += WriteFooter(splitGroupName.Chars());

	Util::C_String outputFile = m_fileName.c_str();

	C_MacroExpander expander;
	expander.Insert("DIALOGUE RECORD", dialogueRecord);
	Util::C_String splitGroupStr = Format("%i", splitGroup);
	expander.Insert("SPLIT GROUP", splitGroupStr);
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
		LOGINFO(wxGetApp().GetLogger(), "Generated Audio XML file: %s\n", fullName);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to generate Audio XML file: %s\n", fullName);
		return false;
	}

}

wxString AudioXML::WriteHeader(const wxString& groupName)
{
	wxString retVal;

	retVal +=  wxString::Format("<Object Name = \"%s\" FrameSkip = \"24\">\n\n", groupName);

	retVal += 
"<!--Inputs--> \n\n\
\t<ActionIn \n\
\t\tName = \"VOTriggerActionIn\" \n\
\t\tActionScr = \"VOPlayScript\"/> \n\n\
\t<ParameterIn \n\
\t\tName = \"SampleIndex\"/>\n\n";
	
	return retVal;
}

wxString AudioXML::WriteBody(const wxString& groupName, 
							 const wxArrayString& wavFileNames)
{
	wxString retVal;

	retVal += wxString::Format("<!--Sounds & Mixers-->\n\n");
	
	retVal += wxString::Format(
"\t<MonoOneShotSoundTable \n\
\t\tName = \"%sSound\" \n\
\t\tBank = \"%s\" \n\
\t\tIndexCom = \"SampleIndex\">\n", groupName, groupName);
	
	for(size_t i = 0; i < wavFileNames.GetCount(); i++)
	{
		retVal += wxString::Format(
"\t\t<MonoOneShotSoundTableEntry \n\
\t\t\tWave = \"%s\"/>\n", wavFileNames[i]);
	}
	
	retVal += "\t</MonoOneShotSoundTable>\n\n";

	retVal += "<!--Parameters--> \n\n";

	retVal += wxString::Format(
"\t<GateSwitch \n\
\t\tName = \"VOActiveGate\" \n\
\t\tInputCom = \"%sSound\" \n\
\t\tInputOut = \"1\" \n\
\t\tThresholdVal = \"0.01\" \n\
\t\tFallingScr = \"VOFinishedScript\"/> \n\n", groupName);

	retVal += "<!--Scripts--> \n\n";

	retVal += wxString::Format(
"\t<Script \n\
\t\tName = \"VOPlayScript\"> \n\
\t\tSEND \"PLAY\" \"%sSound\" \n\
\t\tEXIT \n\
\t</Script> \n\n", groupName);

	retVal +=
"\t<Script \n\
\t\tName = \"VOFinishedScript\"> \n\
\t\tEXEC \"VOTriggerActionOutScript\" \n\
\t\tEXIT \n\
\t</Script> \n\n";

		retVal +=
"\t<Script \n\
\t\tName = \"VOTriggerActionOutScript\"> \n\
\t\tEXEC \"VOFinished\" \n\
\t\tEXIT \n\
\t</Script> \n\n";

	return retVal;
}

wxString AudioXML::WriteFooter(const wxString& groupName)
{
	wxString retVal;
	retVal += 
"<!-- Outputs-->\n\n\
\t<MonoAudioOutput \n\
\t\tName = \"VOMonoOutput\" \n";

	retVal += wxString::Format(
"\t\tInputCom = \"%sSound\"/> \n\n", groupName);

	retVal +=
"\t<ActionOut \n\
\t\tName = \"VOFinished\"/> \n\n\
</Object>";
	
	return retVal;
}

/////////////////////////// BankPC \\\\\\\\\\\\\\\\\\\\\\\\\\\

BankPC::BankPC(const wxString& fileName) : m_fileName(fileName)
{
}
		
bool BankPC::Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end)
{
	C_RowSet* pRowSet = pRecord->GetRowSet(0);
	if (!pRowSet)
		return false;

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	U32 count = end.index - begin.index;
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	Util::C_String dialogueRecord =	
		RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Replace(" ", "_");

	Util::C_String splitGroupName = Format("%s_%i", dialogueRecord.Chars(), splitGroup);
															
	wxProgressDialog progress(_T("Generating Bank PC..."),
								_T("Generating Bank PC"),
								count,    // range
								NULL,   // parent
								wxPD_APP_MODAL |
								wxPD_AUTO_HIDE |
								wxPD_ELAPSED_TIME |
								wxPD_ESTIMATED_TIME |
								wxPD_REMAINING_TIME
								);

	wxString fileContents = WriteHeader(splitGroupName.Chars());

	C_MacroExpander expander;
	int rowNumber = 1;
	wxArrayString wavFileNames;
	wxArrayString fullWavFileNames;
	for(Xt::DomArray::IndexIterator it = begin; it.index < end.index; ++it, ++rowNumber)
	{
		progress.Update(rowNumber, wxString::Format("Row %d of %d", rowNumber, count));
		wxMilliSleep(10);

		expander.Clear();
		C_Row* pRow = pRowSet->GetRow(it.index);
		NameHelpers::AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);		

		Util::C_String wavFileName = expander.GetValue("Wav Filename");
		wavFileName = Util::ExtractFilename(wavFileName);
		wavFileName = Util::RemoveFileExtension(wavFileName);
		wavFileNames.Add(wavFileName.Chars());	

		Util::C_String fullWavFileName = NameHelpers::GetFullWavFileName(expander);
		fullWavFileNames.Add(fullWavFileName.Chars());
	}
	fileContents += WriteBody(fullWavFileNames, wavFileNames);
	fileContents += WriteFooter();

	Util::C_String outputFile = m_fileName.c_str();

	expander.Clear();
	expander.Insert("DIALOGUE RECORD", dialogueRecord);
	Util::C_String splitGroupStr = Format("%i", splitGroup);
	expander.Insert("SPLIT GROUP", splitGroupStr);
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
		LOGINFO(wxGetApp().GetLogger(), "Generated Bank PC file: %s\n", fullName);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to generate Bank PC file: %s\n", fullName);
		return false;
	}

}

wxString BankPC::WriteHeader(const wxString& groupName)
{
	wxString retVal;

	retVal +=  wxString::Format(
"<wavebank name=\"%s\" endian=\"big\" streaming=\"yes\" bitDepth=\"16\">\n", 
		groupName);
	
	return retVal;
}

wxString BankPC::WriteBody(const wxArrayString& fullWavFileNames, const wxArrayString& wavFileNames)
{
	wxString retVal;

	assert(fullWavFileNames.GetCount() == wavFileNames.GetCount());
	
	for(size_t i = 0; i < fullWavFileNames.GetCount(); i++)
	{
		retVal += wxString::Format(
"\t<wave \n\
\t\tfilename = \"%s\" \n\
\t\tinternalName = \"%s\" \n\
\t\tformat = \"pcm_wav_s16\" \n\
\t\tsamplerate = \"48000\" \n\
\t\tloop = \"no\"/>\n", fullWavFileNames[i], wavFileNames[i]);
	}

	return retVal;
}

wxString BankPC::WriteFooter()
{
	wxString retVal;
	retVal += "</wavebank>"; 	
	return retVal;
}

/////////////////////////// Bank360 \\\\\\\\\\\\\\\\\\\\\\\\\\\

Bank360::Bank360(const wxString& fileName) : m_fileName(fileName)
{
}
		
bool Bank360::Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end)
{
	C_RowSet* pRowSet = pRecord->GetRowSet(0);
	if (!pRowSet)
		return false;

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	U32 count = end.index - begin.index;
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	Util::C_String dialogueRecord =	
		RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Replace(" ", "_");

	Util::C_String splitGroupName = Format("%s_%i", dialogueRecord.Chars(), splitGroup);
	
	wxProgressDialog progress(_T("Generating Bank 360..."),
								_T("Generating Bank 360"),
								count,    // range
								NULL,   // parent
								wxPD_APP_MODAL |
								wxPD_AUTO_HIDE |
								wxPD_ELAPSED_TIME |
								wxPD_ESTIMATED_TIME |
								wxPD_REMAINING_TIME
								);

	wxString fileContents = WriteHeader(splitGroupName.Chars());

	C_MacroExpander expander;
	int rowNumber = 1;
	wxArrayString wavFileNames;
	wxArrayString fullWavFileNames;
	for(Xt::DomArray::IndexIterator it = begin; it.index < end.index; ++it, ++rowNumber)
	{
		progress.Update(rowNumber, wxString::Format("Row %d of %d", rowNumber, count));
		wxMilliSleep(10);

		expander.Clear();
		C_Row* pRow = pRowSet->GetRow(it.index);
		NameHelpers::AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);		

		Util::C_String wavFileName = expander.GetValue("Wav Filename");
		wavFileName = Util::ExtractFilename(wavFileName);
		wavFileName = Util::RemoveFileExtension(wavFileName);
		wavFileNames.Add(wavFileName.Chars());	

		Util::C_String fullWavFileName = NameHelpers::GetFullWavFileName(expander);
		fullWavFileNames.Add(fullWavFileName.Chars());
	}
	fileContents += WriteBody(fullWavFileNames, wavFileNames);
	fileContents += WriteFooter();

	Util::C_String outputFile = m_fileName.c_str();

	expander.Clear();
	expander.Insert("DIALOGUE RECORD", dialogueRecord);
	Util::C_String splitGroupStr = Format("%i", splitGroup);
	expander.Insert("SPLIT GROUP", splitGroupStr);
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
		LOGINFO(wxGetApp().GetLogger(), "Generated Bank 360 file: %s\n", fullName);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to generate Bank 360 file: %s\n", fullName);
		return false;
	}

}

wxString Bank360::WriteHeader(const wxString& groupName)
{
	wxString retVal;

	retVal +=  wxString::Format(
"<wavebank name=\"%s\" endian=\"big\" streaming=\"yes\" bitDepth=\"16\">\n", 
		groupName);
	
	return retVal;
}

wxString Bank360::WriteBody(const wxArrayString& fullWavFileNames, const wxArrayString& wavFileNames)
{
	wxString retVal;

	assert(fullWavFileNames.GetCount() == wavFileNames.GetCount());
	
	for(size_t i = 0; i < fullWavFileNames.GetCount(); i++)
	{
		retVal += wxString::Format(
"\t<wave \n\
\t\tfilename = \"%s\" \n\
\t\tinternalName = \"%s\" \n\
\t\tformat = \"pcm_wav_s16\" \n\
\t\tsamplerate = \"48000\" \n\
\t\tloop = \"no\"/>\n", fullWavFileNames[i], wavFileNames[i]);
	}

	return retVal;
}

wxString Bank360::WriteFooter()
{
	wxString retVal;
	retVal += "</wavebank>"; 	
	return retVal;
}

/////////////////////////// BankPS3 \\\\\\\\\\\\\\\\\\\\\\\\\\\

BankPS3::BankPS3(const wxString& fileName) : m_fileName(fileName)
{
}
		
bool BankPS3::Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end)
{
	C_RowSet* pRowSet = pRecord->GetRowSet(0);
	if (!pRowSet)
		return false;

	Xt::DomElement* pElement = pRowSet->GetXtData();
	Xt::DomArray* pArray = Xt::AsArray(pElement);

	U32 count = end.index - begin.index;
	if (!count) 
	{
		LOGINFO(wxGetApp().GetLogger(), "Empty rowset, nothing to generate.\n");
		return false;	
	}

	Util::C_String dialogueRecord =	
		RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Replace(" ", "_");

	Util::C_String splitGroupName = Format("%s_%i", dialogueRecord.Chars(), splitGroup);

	wxProgressDialog progress(_T("Generating Bank PS3..."),
								_T("Generating Bank PS3"),
								count,    // range
								NULL,   // parent
								wxPD_APP_MODAL |
								wxPD_AUTO_HIDE |
								wxPD_ELAPSED_TIME |
								wxPD_ESTIMATED_TIME |
								wxPD_REMAINING_TIME
								);

	wxString fileContents = WriteHeader(splitGroupName.Chars());

	C_MacroExpander expander;
	int rowNumber = 1;
	wxArrayString wavFileNames;
	wxArrayString fullWavFileNames;
	for(Xt::DomArray::IndexIterator it = begin; it.index < end.index; ++it, ++rowNumber)
	{
		progress.Update(rowNumber, wxString::Format("Row %d of %d", rowNumber, count));
		wxMilliSleep(10);

		expander.Clear();
		C_Row* pRow = pRowSet->GetRow(it.index);
		NameHelpers::AddFieldValue(expander, pRowSet->GetAllColumnSet(), pRow);		

		Util::C_String wavFileName = expander.GetValue("Wav Filename");
		wavFileName = Util::ExtractFilename(wavFileName);
		wavFileName = Util::RemoveFileExtension(wavFileName);
		wavFileNames.Add(wavFileName.Chars());	

		Util::C_String fullWavFileName = NameHelpers::GetFullWavFileName(expander);
		fullWavFileNames.Add(fullWavFileName.Chars());
	}
	fileContents += WriteBody(fullWavFileNames, wavFileNames);
	fileContents += WriteFooter();

	Util::C_String outputFile = m_fileName.c_str();

	expander.Clear();
	expander.Insert("DIALOGUE RECORD", dialogueRecord);
	Util::C_String splitGroupStr = Format("%i", splitGroup);
	expander.Insert("SPLIT GROUP", splitGroupStr);
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
		LOGINFO(wxGetApp().GetLogger(), "Generated Bank PS3 file: %s\n", fullName);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to generate Bank PS3 file: %s\n", fullName);
		return false;
	}

}

wxString BankPS3::WriteHeader(const wxString& groupName)
{
	wxString retVal;

	retVal +=  wxString::Format(
"<wavebank name=\"%s\" endian=\"big\" streaming=\"yes\" bitDepth=\"16\">\n", 
		groupName);
	
	return retVal;
}

wxString BankPS3::WriteBody(const wxArrayString& fullWavFileNames, const wxArrayString& wavFileNames)
{
	wxString retVal;

	assert(fullWavFileNames.GetCount() == wavFileNames.GetCount());
	
	for(size_t i = 0; i < fullWavFileNames.GetCount(); i++)
	{
		retVal += wxString::Format(
"\t<wave \n\
\t\tfilename = \"%s\" \n\
\t\tinternalName = \"%s\" \n\
\t\tformat = \"pcm_wav_s16_to_adpcm\" \n\
\t\tsamplerate = \"48000\" \n\
\t\tloop = \"no\"/>\n", fullWavFileNames[i], wavFileNames[i]);
	}

	return retVal;
}

wxString BankPS3::WriteFooter()
{
	wxString retVal;
	retVal += "</wavebank>"; 	
	return retVal;
}

/////////////////////////// SetupAudioRecords \\\\\\\\\\\\\\\\\\\\\\\\\\\

bool SetupAudioRecords::Do(C_Record* pRecord, int splitGroup)
{
	//Create Audio Object
	Xt::DomRecord* pAudioObjectRec = NULL;
	{
		wxString packName = "Audio";
		wxString typeName = "AudioObject";
		wxString recordName = wxString::Format("AudioObjects.VO_%s_%i", pRecord->GetName().Text(), splitGroup);

		pAudioObjectRec = CreateRecord(packName, recordName, typeName);

		if(pAudioObjectRec)
		{
			Xt::DomField* pField = Xt::AcquireField(&pAudioObjectRec->root.data, "filename");
			wxString fieldValue = wxString::Format("%s_%i.dat", pRecord->GetName().Text(), splitGroup);
			Xt::Set(pField, fieldValue.c_str());
		}
	}

	//Create Audio Entity
	Xt::DomRecord* pAudioEntityRec = NULL;
	{
		wxString packName = "AudioEntities";
		wxString typeName = "AudioEntityDef";
		wxString recordName = wxString::Format("VO_%s_%i", pRecord->GetName().Text(), splitGroup);

		pAudioEntityRec = CreateRecord(packName, recordName, typeName);

		if(pAudioEntityRec)
		{
			//General tab
			Xt::DomField* pField = Xt::AcquireField(&pAudioEntityRec->root.data, "name");
			wxString value = wxString::Format("%s_%i", pRecord->GetName().Text(), splitGroup);
			Xt::Set(pField, value.c_str());

			pField = Xt::AcquireField(&pAudioEntityRec->root.data, "group");
			value = "2BEF3959";
			Xt::Set(pField, value.c_str());

			Xt::DomBlock* pBlock = Xt::AcquireBlock(&pAudioEntityRec->root.data, "objCreate");
			Xt::DomArray* pArray = Xt::AcquireArray(pBlock, "objCreates");

			Xt::DomComponent* pObjCreatesComponent = new Xt::DomComponent;
			GenerateIdForComponent(pObjCreatesComponent, false);

			pField = Xt::AcquireField(&pObjCreatesComponent->data, "object");
			value = wxString::Format("%X", pAudioObjectRec->id);
			Xt::Set(pField, value.c_str());

			Xt::Append(pArray, pObjCreatesComponent);

			//Entity Create Tab
			pBlock = Xt::AcquireBlock(&pAudioEntityRec->root.data, "instFunction");

			pArray = Xt::AcquireArray(pBlock, "instances");

			Xt::DomComponent* pInstancesComponent = new Xt::DomComponent;
			GenerateIdForComponent(pInstancesComponent, false);
			pField = Xt::AcquireField(&pInstancesComponent->data, "name");
			Xt::Set(pField, "VO");
			Xt::DomRef* pRef = Xt::AcquireRef(&pInstancesComponent->data, "object");
			Xt::Set(pRef, pObjCreatesComponent->id);
			Xt::Append(pArray, pInstancesComponent);

			pArray = Xt::AcquireArray(pBlock, "connectorsIn");
			
			Xt::DomComponent* pVOTriggerActionInComponent = new Xt::DomComponent;
			GenerateIdForComponent(pVOTriggerActionInComponent, false);
			pField = Xt::AcquireField(&pVOTriggerActionInComponent->data, "component");
			Xt::Set(pField, "VOTriggerActionIn");
			pRef = Xt::AcquireRef(&pVOTriggerActionInComponent->data, "instance");
			Xt::Set(pRef, pInstancesComponent->id);
			Xt::Append(pArray, pVOTriggerActionInComponent);

			Xt::DomComponent* pSampleIndexComponent = new Xt::DomComponent;
			GenerateIdForComponent(pSampleIndexComponent, false);
			pField = Xt::AcquireField(&pSampleIndexComponent->data, "component");
			Xt::Set(pField, "SampleIndex");
			pRef = Xt::AcquireRef(&pSampleIndexComponent->data, "instance");
			Xt::Set(pRef, pInstancesComponent->id);
			Xt::Append(pArray, pSampleIndexComponent);

			pArray = Xt::AcquireArray(pBlock, "connectorsOut");
			
			Xt::DomComponent* pVOMonoOutputComponent = new Xt::DomComponent;
			GenerateIdForComponent(pVOMonoOutputComponent, false);
			pField = Xt::AcquireField(&pVOMonoOutputComponent->data, "component");
			Xt::Set(pField, "VOMonoOutput");
			pRef = Xt::AcquireRef(&pVOMonoOutputComponent->data, "instance");
			Xt::Set(pRef, pInstancesComponent->id);
			Xt::Append(pArray, pVOMonoOutputComponent);

			Xt::DomComponent* pVOFinishedComponent = new Xt::DomComponent;
			GenerateIdForComponent(pVOFinishedComponent, false);
			pField = Xt::AcquireField(&pVOFinishedComponent->data, "component");
			Xt::Set(pField, "VOFinished");
			pRef = Xt::AcquireRef(&pVOFinishedComponent->data, "instance");
			Xt::Set(pRef, pInstancesComponent->id);
			Xt::Append(pArray, pVOFinishedComponent);

			//Externals Tab
			pArray = Xt::AcquireArray(&pAudioEntityRec->root.data, "externs");
			
			Xt::DomComponent* pComponent = new Xt::DomComponent;
			GenerateIdForComponent(pComponent, false);
			pRef = Xt::AcquireRef(&pComponent->data, "connector");
			Xt::Set(pRef, pVOFinishedComponent->id);
			pField = Xt::AcquireField(&pComponent->data, "referenceString");
			Xt::Set(pField, "VOFinished");
			Xt::Append(pArray, pComponent);

			pComponent = new Xt::DomComponent;
			GenerateIdForComponent(pComponent, false);
			pRef = Xt::AcquireRef(&pComponent->data, "connector");
			Xt::Set(pRef, pVOTriggerActionInComponent->id);
			pField = Xt::AcquireField(&pComponent->data, "referenceString");
			Xt::Set(pField, "VOTriggerActionIn");
			Xt::Append(pArray, pComponent);

			pComponent = new Xt::DomComponent;
			GenerateIdForComponent(pComponent, false);
			pRef = Xt::AcquireRef(&pComponent->data, "connector");
			Xt::Set(pRef, pSampleIndexComponent->id);
			pField = Xt::AcquireField(&pComponent->data, "referenceString");
			Xt::Set(pField, "SampleIndex");
			Xt::Append(pArray, pComponent);

			//Connect Entity
			pArray = Xt::AcquireArray(&pAudioEntityRec->root.data, "connectFunctions");
			pComponent = new Xt::DomComponent;
			GenerateIdForComponent(pComponent, false);
			pField = Xt::AcquireField(&pComponent->data, "connectEnv");
			Xt::Set(pField, "2A5C281F");
			Xt::Append(pArray, pComponent);

			Xt::DomArray* pOutgoingArray = Xt::AcquireArray(&pComponent->data, "outgoing");
			Xt::DomComponent* pOutgoingComponent = new Xt::DomComponent;
			GenerateIdForComponent(pOutgoingComponent, false);
			Xt::Append(pOutgoingArray, pOutgoingComponent);

			pRef = Xt::AcquireRef(&pOutgoingComponent->data, "from");
			Xt::Set(pRef, pVOMonoOutputComponent->id);

			Xt::DomBlock* pToBlock = Xt::AcquireBlock(&pOutgoingComponent->data, "to");
			pField = Xt::AcquireField(pToBlock, "componentFrom");
			Xt::Set(pField, "VOMonoInput");
			pField = Xt::AcquireField(pToBlock, "instanceFrom");
			Xt::Set(pField, "VO_Upmixer");

		}
	}

	
	//Create Audio Entity Proxy
	Xt::DomRecord* pAudioEntityProxyRec = NULL;
	{
		wxString packName = "Audio";
		wxString typeName = "AudioEntityProxy";
		wxString recordName = wxString::Format("EntityProxies.VO_%s_%i", pRecord->GetName().Text(), splitGroup);

		pAudioEntityProxyRec = CreateRecord(packName, recordName, typeName);

		if(pAudioEntityProxyRec)
		{
			Xt::DomField* pField = Xt::AcquireField(&pAudioEntityProxyRec->root.data, "EntityDef");
			wxString fieldValue = wxString::Format("%X", pAudioEntityRec->id);
			Xt::Set(pField, fieldValue.c_str());
		}
	}
		
	return true;
}

Xt::DomRecord* SetupAudioRecords::CreateRecord(const wxString& packName, 
			const wxString& recordName,
			const wxString& typeName)
{
	Xt::DomPack* pPack = NULL;
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(wxGetApp().GetDb()->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pThePack = packIt;
		if (!packName.CmpNoCase((const char *)pThePack->name))
		{
			pPack = pThePack;
			break;
		}
	}

	if(!pPack)
	{
		LOGERROR(wxGetApp().GetLogger(), "Cannot find pack with name \"%s\"", packName);
		return NULL;
	}

	LarvaType* pRecordType = FindTypeByName(wxGetApp().GetDb()->schema, typeName.c_str());

	if(!pRecordType)
	{
		LOGERROR(wxGetApp().GetLogger(), "Cannot find type \"%s\"", typeName);
		return NULL;
	}

	Xt::DomRecord* pRecord = FindRecordByName(pPack, recordName);
	if(pRecord)
	{
		LOGINFO(wxGetApp().GetLogger(), "Removing previously found record: \"%s\"", recordName);
		Xt::RemoveRecord(pPack, pRecord->id);
	}
	   
	pRecord = Xt::NewRecord(pPack, recordName.c_str(), pRecordType->name);
	return pRecord;
}

/////////////////////////// Generate360Bat \\\\\\\\\\\\\\\\\\\\\\\\\\\

Generate360Bat::Generate360Bat(const wxString& build360Path, const wxString& bank360Path, const wxString& audioXMLPath) : 
	m_build360Path(build360Path),
	m_bank360Path(bank360Path),
	m_audioXMLPath(audioXMLPath)
{
}

bool Generate360Bat::Do(C_Record* pRecord, int splitGroupCount)
{
	wxString fileContents;

	Util::C_String dialogueRecord =	
			RemoveFoldersFromRecordName((const char*)pRecord->GetName()).Replace(" ", "_");

	fileContents +=
"rem BUILD 360 \n\n\
set WAVEBUNDLER=z:\\tools\\common_bin\\wavebundler.exe /p\n\
set XMLTODAT=z:\\tools\\common_bin\\xmltodat.exe /noprompt\n\n\
rem BUILD BANKS\n\n";

	for(int splitGroup = 0; splitGroup < splitGroupCount; splitGroup++)
	{
		Util::C_String bank360Path = m_bank360Path.c_str();

		C_MacroExpander expander;
		expander.Insert("DIALOGUE RECORD", dialogueRecord);
		Util::C_String splitGroupStr = Format("%i", splitGroup);
		expander.Insert("SPLIT GROUP", splitGroupStr);
		expander.Expand(bank360Path);

		Util::C_String bank360Filename = Util::ExtractFilename(bank360Path);
		
		fileContents += wxString::Format("%%WAVEBUNDLER%% %s\n", bank360Filename);
	}

	fileContents +=
"\nrem BUILD DAT FILES\n\n";

	for(int splitGroup = 0; splitGroup < splitGroupCount; splitGroup++)
	{
		Util::C_String audioXMLPath = m_audioXMLPath.c_str();

		C_MacroExpander expander;
		expander.Insert("DIALOGUE RECORD", dialogueRecord);
		Util::C_String splitGroupStr = Format("%i", splitGroup);
		expander.Insert("SPLIT GROUP", splitGroupStr);
		expander.Expand(audioXMLPath);

		Util::C_String audioXMLFilename = Util::ExtractFilename(audioXMLPath);
		
		fileContents += wxString::Format("%%XMLTODAT%% %s\n", audioXMLFilename);
	}

	fileContents +=
"\nrem COPY TO WORKING\n\n\
del z:\\amax\\working\\360\\audio\\*.baf\n\
del z:\\amax\\working\\360\\audio\\*.dat\n\
Copy *.dat Z:\\Amax\\Working\\360\\Audio\n\
Copy *.baf Z:\\Amax\\Working\\360\\Audio\n\n\
rem COPY TO KIT\n\n\
\"%XEDK%\\bin\\win32\\xbcp\" /r /y /d /t \"Z:\\amax\\working\\360\\Audio\\*.dat\" \"devkit:\\amax\\audio\"\n\
\"%XEDK%\\bin\\win32\\xbcp\" /r /y /d /t \"Z:\\amax\\working\\360\\Audio\\*.baf\" \"devkit:\\amax\\audio\"\n\n\
pause";

	Util::C_String outputFile = m_build360Path.c_str();
	C_MacroExpander expander;
	expander.Insert("DIALOGUE RECORD", dialogueRecord);
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
		LOGINFO(wxGetApp().GetLogger(), "Generated Build 360 file: %s\n", fullName);
		return true;
	}
	else
	{
		LOGERROR(wxGetApp().GetLogger(), "Failed to generate Build 360 file: %s\n", fullName);
		return false;
	}

	return true;
}

} //namespace Amax
} //namespace AudioXML
} //namespace DMan

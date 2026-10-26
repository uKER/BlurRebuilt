#ifndef DMAN_NIGHTWING_AUDIO_TEMPLATES_H
#define DMAN_NIGHTWING_AUDIO_TEMPLATES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace AudioXML
{
namespace Nightwing
{

	class AudioXML
	{
	public:
		AudioXML(const wxString& fileName);	
		bool Write(C_Record* pRecord);

	private:
		wxString WriteHeader(wxString dialogueRecordName);
		
		wxString WriteCharacterStopBlock(wxArrayString characterNames);

		wxString WriteCharacterPlayBlock(wxString dialogueRecordName, 
									 wxString characterName, 
									 wxArrayString locRecordNames);
		
		wxString WriteFooter(wxArrayString characterNames);

		wxString m_fileName;
	};

} //namespace Nightwing
} //namespace AudioXML
} //namespace DMan

#endif //DMAN_NIGHTWING_AUDIO_TEMPLATES_H

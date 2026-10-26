#ifndef DMAN_AMAX_AUDIO_TEMPLATES_H
#define DMAN_AMAX_AUDIO_TEMPLATES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
namespace AudioXML
{
namespace Amax
{
	/////////////////////////// AudioXML \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class AudioXML
	{
	public:
		AudioXML(const wxString& fileName);	
		bool Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end);

	private:
		wxString WriteHeader(const wxString& groupName);
		
		wxString WriteBody(const wxString& groupName, 
			const wxArrayString& wavFileNames);
		
		wxString WriteFooter(const wxString& groupName);

		wxString m_fileName;
	};

	/////////////////////////// BankPC \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class BankPC
	{
	public:
		BankPC(const wxString& fileName);	
		bool Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end);

	private:
		wxString WriteHeader(const wxString& groupName);
		
		wxString WriteBody(const wxArrayString& fullWavFileNames, const wxArrayString& wavFileNames);
		
		wxString WriteFooter();

		wxString m_fileName;
	};

	/////////////////////////// Bank360 \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class Bank360
	{
	public:
		Bank360(const wxString& fileName);	
		bool Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end);

	private:
		wxString WriteHeader(const wxString& groupName);
		
		wxString WriteBody(const wxArrayString& fullWavFileNames, const wxArrayString& wavFileNames);
		
		wxString WriteFooter();

		wxString m_fileName;
	};

	/////////////////////////// BankPS3 \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class BankPS3
	{
	public:
		BankPS3(const wxString& fileName);	
		bool Write(C_Record* pRecord, 
			int splitGroup, 
			const Xt::DomArray::IndexIterator& begin, 
			const Xt::DomArray::IndexIterator& end);

	private:
		wxString WriteHeader(const wxString& groupName);
		
		wxString WriteBody(const wxArrayString& fullWavFileNames, const wxArrayString& wavFileNames);
		
		wxString WriteFooter();

		wxString m_fileName;
	};

	/////////////////////////// SetupAudioRecords \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class SetupAudioRecords
	{
	public:
		bool Do(C_Record* pRecord, int splitGroup);

	private:
		Xt::DomRecord* CreateRecord(const wxString& packName, 
			const wxString& recordName,
			const wxString& typeName);
	};

	/////////////////////////// Generate360Bat \\\\\\\\\\\\\\\\\\\\\\\\\\\

	class Generate360Bat
	{
	public:
		Generate360Bat(const wxString& build360Path, const wxString& bank360Path, const wxString& audioXMLPath);
		bool Do(C_Record* pRecord, int splitGroupCount);
	private:
		wxString m_build360Path;
		wxString m_bank360Path;
		wxString m_audioXMLPath;
	};

} //namespace Amax
} //namespace AudioXML
} //namespace DMan

#endif //DMAN_AMAX_AUDIO_TEMPLATES_H

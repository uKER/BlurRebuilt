#ifndef DMAN_FINALDRAFTREADER_H
#define DMAN_FINALDRAFTREADER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_FinalDraftReader : public C_Reader
{
public:
	C_FinalDraftReader();
	virtual ~C_FinalDraftReader();

	virtual E_ReaderType		GetReaderType() {  return C_Reader::E_ReaderType_FinalDraft; };

	virtual bool				HasValidColumnMapping() { return true; };
	virtual const C_Column*		GetColumnFromKey(const wxString& colKey);
	
	virtual bool				OpenFile();
	virtual bool				CloseFile();
	virtual bool				Read();
	virtual bool				IsFileOpen() const { return m_file.IsOpened(); }

private:
	
	static const U8 CARRIAGE_RETURN			= 0X0D;
		
	static const U8 SIGNATURE_GENERAL		= 0X00;
	static const U8 SIGNATURE_SCENEHEADING	= 0X01;
	static const U8 SIGNATURE_ACTION		= 0X02;
	static const U8 SIGNATURE_CHARACTER		= 0X03;
	static const U8 SIGNATURE_PARENTHETICAL	= 0X04;
	static const U8 SIGNATURE_DIALOGUE		= 0X05;
	static const U8 SIGNATURE_TRANSITION	= 0X06;
	static const U8 SIGNATURE_SEQUENCE		= 0X07;

	wxFile		m_file;

	//Mapping from Signature to Column object
	Btl::Map<U8, const C_Column*> m_ColumnMapping;
	void ObtainColumnMapping();

	Btl::Vector<wxMemoryBuffer> m_rawElements;

	void ProcessSceneHeadingElement(const wxString& rawSceneElement, 
		wxString& sceneIntro, wxString& sceneLocation, wxString& sceneTime);

	void ProcessCharacterElement(const wxString& rawCharacterElement, 
		wxString& characterName, wxString& characterExtension);

	void ProcessParentheticalElement(const wxString& rawParentheticalElement, 
		wxString& parenthetical);
};

} //namespace DMan

#endif //DMAN_FINALDRAFTREADER_H
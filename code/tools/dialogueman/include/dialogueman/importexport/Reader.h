#ifndef DMAN_READER_H
#define DMAN_READER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_Reader 
{
public:
	C_Reader();
	virtual ~C_Reader();
	
	LarvaType*					GetItemType() const { return m_pItemType; }
	Xt::DomRecord*				GetLocTable() const { return m_pLocTable; }
	wxFileName					GetFileName() const { return m_fileName; }

	C_Reader&					SetItemType(LarvaType* pItemType);
	C_Reader&					SetLocTable(Xt::DomRecord* pLocTable);
	C_Reader&					SetFilename(const wxString& fileName);

	Btl::Vector< Btl::Map<wxString, C_MutantValue> >& GetReadRows() { return m_readRows; }

	enum E_ReaderType
	{
		E_ReaderType_Excel,
		E_ReaderType_FinalDraft
	};

	//Abstract methods
	virtual E_ReaderType		GetReaderType() = 0;

	virtual bool				HasValidColumnMapping() = 0;
	virtual const C_Column*		GetColumnFromKey(const wxString& colKey) = 0;

	virtual bool				OpenFile() = 0;
	virtual bool				CloseFile() = 0;
	virtual bool				Read() = 0;
	virtual bool				IsFileOpen() const = 0;
	

protected:
	LarvaType*					m_pItemType;
	Xt::DomRecord*				m_pLocTable;

	wxFileName 					m_fileName;

	Btl::Vector< Btl::Map<wxString, C_MutantValue> > m_readRows;
};

} //namespace DMan

#endif //DMAN_READER_H
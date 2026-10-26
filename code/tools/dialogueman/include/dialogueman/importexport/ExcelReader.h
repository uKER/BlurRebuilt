#ifndef DMAN_EXCELREADER_H
#define DMAN_EXCELREADER_H

#ifdef _MSC_VER
#pragma once
#endif



namespace DMan
{

class C_ExcelReader : public C_Reader
{
public:
	C_ExcelReader();
	virtual ~C_ExcelReader();

	virtual E_ReaderType		GetReaderType() {  return C_Reader::E_ReaderType_Excel; };

	virtual bool				HasValidColumnMapping();
	virtual const C_Column*		GetColumnFromKey(const wxString& colKey);
	
	virtual bool				OpenFile();
	virtual bool				CloseFile();
	virtual bool				Read();
	virtual bool				IsFileOpen()			const { return m_isExcelDocOpen && m_pExcelDoc; }
		
	size_t						GetSheet()				const { return m_sheet; }
	
	wxString					GetRangeStr()			const;
	U32							GetMinRow()				const { return m_minRow; }
	U32							GetMinCol()				const { return m_minCol; }
	U32							GetMaxRow()				const { return m_maxRow; }
	U32							GetMaxCol()				const { return m_maxCol; }
	wxString					GetMinColStr()			const;
	wxString					GetMaxColStr()			const;
	
	YExcel::BasicExcel&	GetExcelDoc()		
	{ 
		assert(m_pExcelDoc); 
		return *m_pExcelDoc; 
	}

	C_ExcelReader&	SetSheet(size_t sheet) 
	{ 
		m_sheet = sheet; 
		return *this;
	}
	
	//Rows and Cols starts at 1
	C_ExcelReader&	SetRange(const wxString& rangeStr); 
	C_ExcelReader&	SetRange(U32 minRow, U32 minCol, U32 maxRow, U32 maxCol); 
		
	Btl::Vector< Btl::Map<wxString, C_MutantValue> >& GetReadRows() { return m_readRows; }

private:
	//Excel document object
	YExcel::BasicExcel*					m_pExcelDoc;
	bool								m_isExcelDocOpen;

	//Selected Sheet
	U32									m_sheet;

	//Range
	U32									m_minRow;
	U32									m_minCol;
	U32									m_maxRow;
	U32									m_maxCol;
	
	//Mapping from Internal Column object to Excel Column number (zero based)
	Btl::Map<const C_Column*, U32>		m_ColumnMapping;
	void ObtainColumnMapping();
};

} //namespace DMan

#endif //DMAN_EXCELREADER_H
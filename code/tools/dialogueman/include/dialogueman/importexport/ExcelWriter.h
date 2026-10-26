#ifndef DMAN_EXCELWRITER_H
#define DMAN_EXCELWRITER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_ExcelWriter 
{
public:
	C_ExcelWriter();
	virtual ~C_ExcelWriter();
	
	LarvaType*			GetItemType()			const { return m_pItemType; }
	wxString			GetFilename()			const { return m_excelFilename; }
	
	//State verification
	bool				IsExcelDocCreated()		const { return m_isExcelDocCreated; }
	bool				IsExcelDocWritten()		const { return m_isExcelDocWritten; }
	bool				IsExcelDocSaved()		const { return m_isExcelDocSaved; }
	bool				IsExcelDocClosed()		const { return m_isExcelDocClosed; }

	//State transitions
	bool				CreateExcelDoc();
	bool				Write();
	bool				SaveExcelDoc();
	bool				CloseExcelDoc();
	
	bool				HasValidColumnMapping();
		
	YExcel::BasicExcel&	GetExcelDoc()		
	{ 
		assert(m_pExcelDoc); 
		return *m_pExcelDoc; 
	}

	C_ExcelWriter& SetItemType(LarvaType* pItemType)
	{
		m_pItemType = pItemType;
		return *this;
	}
	
	C_ExcelWriter&	SetFilename(const wxString& excelFilename) 
	{ 
		m_excelFilename = excelFilename; 
		return *this;
	}

private:
	LarvaType*							m_pItemType;
	
	//Excel file name and document object
	wxString							m_excelFilename;
	YExcel::BasicExcel*					m_pExcelDoc;

	//State vars
	bool								m_isExcelDocCreated;
	bool								m_isExcelDocWritten;
	bool								m_isExcelDocSaved; 
	bool								m_isExcelDocClosed;
	
	//Mapping from Internal Column object to Excel Column number (zero based)
	Btl::Map<const C_Column*, U32>		m_ColumnMapping;
	void ObtainColumnMapping();
};

} //namespace DMan

#endif //DMAN_EXCELWRITER_H
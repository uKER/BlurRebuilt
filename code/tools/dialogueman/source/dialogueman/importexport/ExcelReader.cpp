#include "stdafx.h"
#include <dialogueman/ImportExport/ExcelReader.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Rowset.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <wx/regex.h>

namespace DMan
{
C_ExcelReader::C_ExcelReader() :
	m_pExcelDoc(NULL),
	m_isExcelDocOpen(false),
	m_sheet(0),
	m_minRow(1),
	m_minCol(1),
	m_maxRow(1),
	m_maxCol(1)
{}

C_ExcelReader::~C_ExcelReader()
{
	if(m_pExcelDoc)
	{
		delete m_pExcelDoc;
		m_pExcelDoc = NULL;
	}
}

wxString C_ExcelReader::GetMinColStr() const
{
	return ColNumToStr(m_minCol);

}
wxString C_ExcelReader::GetMaxColStr() const
{
	return ColNumToStr(m_maxCol);
}

bool C_ExcelReader::HasValidColumnMapping()
{ 
	return !m_ColumnMapping.Empty();
}

const C_Column* C_ExcelReader::GetColumnFromKey(const wxString& colKey)
{
	Btl::Map<const C_Column*, U32>::Iterator itCol;
			
	for(itCol = m_ColumnMapping.Begin(); itCol != m_ColumnMapping.End(); ++itCol)
	{
		const C_Column* pCol = itCol->GetKey();
		if(pCol == NULL) 
			continue;

		if(pCol->GetKey() == colKey)
			return pCol;
	}

	return NULL;
}

wxString C_ExcelReader::GetRangeStr() const 
{ 
	wxString minColStr = GetMinColStr();
	wxString maxColStr = GetMaxColStr();

	wxString range;
	range << minColStr << m_minRow << wxT(":") << maxColStr << m_maxRow;  
	return range;
}

C_ExcelReader& C_ExcelReader::SetRange(const wxString& rangeStr) 
{ 
	wxString minColStr;
	wxString maxColStr;
	wxString minRowStr;
	wxString maxRowStr;
	
	wxRegEx reRange = wxT("([a-zA-Z]+)([0-9]+):([a-zA-Z]+)([0-9]+)"); 
	if(reRange.Matches(rangeStr))
	{
		minColStr = reRange.GetMatch(rangeStr, 1).MakeUpper();
		minRowStr = reRange.GetMatch(rangeStr, 2);
		maxColStr = reRange.GetMatch(rangeStr, 3).MakeUpper();
		maxRowStr = reRange.GetMatch(rangeStr, 4);
	}
	else
	{
		wxMessageDialog msg(NULL, 
			wxT("Range is invalid\n") 
			wxT("Put verify the entered range and try again\n"), 
			wxT("Import Error"),
			wxOK | wxICON_WARNING);
		msg.ShowModal();
		return *this;
	}

	unsigned long minRowNum, maxRowNum;
	minRowStr.ToULong(&minRowNum);
	maxRowStr.ToULong(&maxRowNum);

	int minColNum = ColStrToNum(minColStr);
	int maxColNum = ColStrToNum(maxColStr);

	SetRange((U32)minRowNum, (U32)minColNum, (U32)maxRowNum, (U32)maxColNum);

	return *this;
}

//Rows and Cols starts at 1
C_ExcelReader& C_ExcelReader::SetRange(U32 minRow, U32 minCol, U32 maxRow, U32 maxCol) 
{ 
	if(IsFileOpen())
	{
		YExcel::BasicExcelWorksheet* pSheet = m_pExcelDoc->GetWorksheet(GetSheet());

		if(pSheet)
		{
			if(minRow < 1 || minCol < 1 || maxRow < 1 || maxCol < 1 ||
			   minRow > pSheet->GetTotalRows() ||
			   minCol > pSheet->GetTotalCols() ||
			   maxRow > pSheet->GetTotalRows() ||
			   maxCol > pSheet->GetTotalCols() ||
			   minRow > maxRow ||
			   minCol > maxCol )
			{
				wxMessageDialog msg(NULL, 
					wxT("Range is invalid\n") 
					wxT("Put verify the entered range and try again\n"), 
					wxT("Import Error"),
					wxOK | wxICON_WARNING);
				msg.ShowModal();
				return *this;
			}

			m_minRow =  minRow;
			m_minCol =  minCol;
			m_maxRow =  maxRow;
			m_maxCol =  maxCol;

			ObtainColumnMapping();
		}
	}
	return *this;
}

bool C_ExcelReader::OpenFile()
{
	if(IsFileOpen())
	{
		CloseFile();
	}

	m_pExcelDoc = new YExcel::BasicExcel();
	const wxCharBuffer filenameAscii = ToAscii(GetFileName().GetFullPath());
	try
	{
		m_isExcelDocOpen = m_pExcelDoc->Load( (const char*)filenameAscii );
	}
	catch(std::bad_alloc& e)
	{
		wxMessageBox(
			wxString::Format("Not enough memory to open this file. Please close some programs and try again."), 
			"Import Error", wxICON_ERROR | wxOK);
		m_isExcelDocOpen = false;
		delete m_pExcelDoc;
		m_pExcelDoc = NULL;
	}
	return m_isExcelDocOpen;
}

bool C_ExcelReader::CloseFile()
{
	if(IsFileOpen())
	{
		delete m_pExcelDoc;
		m_pExcelDoc = NULL;
		m_isExcelDocOpen = false;
		return true;
	}
	return false;
}

void C_ExcelReader::ObtainColumnMapping()
{
	if(IsFileOpen())
	{
		YExcel::BasicExcelWorksheet* pSheet = m_pExcelDoc->GetWorksheet(GetSheet());

		if(pSheet)
		{
			int row = GetMinRow() - 1; // to index from 0
			if(row < 0)
				row = 0;
			
			C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(GetItemType());
							
			if(pColSet)
			{
				m_ColumnMapping.Clear();
				U32 numCols = pColSet->GetNumColumns();
				for(U32 i = 0; i < numCols; ++i)
				{
					C_Column* pCol = pColSet->GetColumn(i);
					if(pCol && pCol->IsImportable() && 
						(pCol->GetMetaData() == "" || pCol->GetMetaData() == "string"))
					{
						//Retrieve index from annotation if available
						int excelColumnIndex = pCol->GetLarvaField()->attrs.GetInt(kXtSemanticAttr, "excelColumnIndex", -1);
						if(excelColumnIndex != -1)
						{
							m_ColumnMapping[pCol] = excelColumnIndex;
						}
						else
						{
							//Search for a cell with a matching column name
							for(U32 col = GetMinCol() - 1; col < GetMaxCol(); ++col)
							{
								YExcel::BasicExcelCell* pCell = pSheet->Cell(row, col, false);
		
								if(pCell)
								{
									switch (pCell->Type())
									{
									case YExcel::BasicExcelCell::STRING:
									{
										if( pCol->GetLarvaField()->name == pCell->GetString() )
											m_ColumnMapping[pCol] = col;			
										break;
									}

									case YExcel::BasicExcelCell::WSTRING:
									{
										if( pCol->GetLarvaField()->name == pCell->GetWString() )
											m_ColumnMapping[pCol] = col;			
										break;
									}
									}
								} //if(pCell)	
							} //for
						}// if(excelColumnIndex != -1)
					}//if	
				}//for
			
			}//if(pColSet)
		
		}//if(pSheet)
	
	}//if(IsExcelDocOpen())

}

bool C_ExcelReader::Read()
{
	if(IsFileOpen())
	{	 										 
		YExcel::BasicExcelWorksheet* pSheet = m_pExcelDoc->GetWorksheet(GetSheet());
		if(pSheet)
		{	
			//For each row (excluding the first row with Column Labels)
			for(size_t rowIndex = GetMinRow(); rowIndex < GetMaxRow(); ++rowIndex)
			{
				bool dataAdded = false;

				Btl::Map<wxString, C_MutantValue> readRow;
				Btl::Map<const C_Column*, U32>::Iterator itCol;
				
				for(itCol = m_ColumnMapping.Begin(); itCol != m_ColumnMapping.End(); ++itCol)
				{
					const C_Column* pCol = itCol->GetKey();
					U32 colIndex = itCol->GetValue();

					if(pCol == NULL) 
						continue;
					
					YExcel::BasicExcelCell* pCell = pSheet->Cell(rowIndex, colIndex, false);
					
					if(pCell)
					{						
						switch (pCell->Type())
						{
							case YExcel::BasicExcelCell::INT:
							{
								readRow[pCol->GetKey()] = C_MutantValue(E_DataType_Int, pCell->GetInteger());
								dataAdded = true;
								break;
							}

							case YExcel::BasicExcelCell::DOUBLE:
							{
								readRow[pCol->GetKey()] = C_MutantValue(E_DataType_Double, pCell->GetDouble());
								dataAdded = true;
								break;
							}

							case YExcel::BasicExcelCell::STRING:
							{
								readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, wxString(pCell->GetString()).Trim(true).Trim(false));
								dataAdded = true;
								break;
							}

							case YExcel::BasicExcelCell::WSTRING:
							{
								const wxCharBuffer value = ToAscii(wxString(pCell->GetWString()));							
								readRow[pCol->GetKey()] = C_MutantValue(E_DataType_String, wxString((const char*)value).Trim(true).Trim(false));
								dataAdded = true;
								break;
							}
						}
					
					}
				}

				if(dataAdded)
				{
					m_readRows.PushBack(readRow);
				}
			
			} //...For each row
		
		} //...if(pSheet)

		return true;
	
	} //if(IsFileOpen())
	
	return false;	
}

} //namespace DMan
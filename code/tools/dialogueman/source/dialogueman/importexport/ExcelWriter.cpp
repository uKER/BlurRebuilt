#include "stdafx.h"

#include <dialogueman/ImportExport/ExcelWriter.h>

#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/Rowset.h>
#include <DialogueMan/Document/Row.h>

#include <DialogueMan/Misc/DomId.h>
#include <DialogueMan/Misc/Selection.h>
#include <Dialogueman/Misc/Utils.h>
#include <DialogueMan/Misc/StringUtil.h>



namespace DMan
{
C_ExcelWriter::C_ExcelWriter() :
	m_pItemType(NULL), 
	m_excelFilename(wxT("")),
	m_pExcelDoc(NULL),
	m_isExcelDocCreated(false),
	m_isExcelDocWritten(false),
	m_isExcelDocSaved(false),
	m_isExcelDocClosed(false)
{}

C_ExcelWriter::~C_ExcelWriter()
{
	CloseExcelDoc();
}

bool C_ExcelWriter::HasValidColumnMapping()
{ 
	return !m_ColumnMapping.Empty();
}

void C_ExcelWriter::ObtainColumnMapping()
{
	m_ColumnMapping.Clear();
	C_ColumnSet* pColSet = wxGetApp().GetDbManager().GetTypeColumnSet(GetItemType());
	if(pColSet)
	{
		int maxColumnIndex = -1;

		for(U32 i = 0; i < pColSet->GetNumColumns(); ++i)
		{
			C_Column* pCol = pColSet->GetColumn(i);
			if(pCol && pCol->IsImportable() &&
				(pCol->GetMetaData() == "" || pCol->GetMetaData() == "string")	
			   )
			{
				int excelColumnIndex = pCol->GetLarvaField()->attrs.GetInt(kXtSemanticAttr, "excelColumnIndex", -1);
				if(excelColumnIndex != -1)
				{
					m_ColumnMapping[pCol] = excelColumnIndex;
					if(excelColumnIndex > maxColumnIndex)
						maxColumnIndex = excelColumnIndex;
				}
			}
		}

		for(U32 i = 0; i < pColSet->GetNumColumns(); ++i)
		{
			C_Column* pCol = pColSet->GetColumn(i);
			if(pCol && pCol->IsImportable() &&
				!(pCol->GetMetaData() == "" || pCol->GetMetaData() == "string")	
			   )
			{
				m_ColumnMapping[pCol] = ++maxColumnIndex;
			}
		}
	}
}

bool C_ExcelWriter::CreateExcelDoc()
{
	if(!IsExcelDocCreated() || IsExcelDocClosed())
	{
		assert(!m_pExcelDoc);

		m_pExcelDoc = new YExcel::BasicExcel();

		m_pExcelDoc->New(1);
		m_pExcelDoc->DeleteWorksheet((size_t)0);
		
		m_isExcelDocCreated = true;
		m_isExcelDocWritten = false;
		m_isExcelDocSaved = false;
		m_isExcelDocClosed = false;

		return m_isExcelDocCreated;
	}
	return false;
}

bool C_ExcelWriter::Write()
{
	if(IsExcelDocCreated() && !IsExcelDocClosed())
	{
		assert(m_pExcelDoc);

		if(!HasValidColumnMapping())
			ObtainColumnMapping();

		YExcel::BasicExcelWorksheet* pSheet = m_pExcelDoc->AddWorksheet((const char*)GetItemType()->label);

		//Write column labels on first row
		Btl::Map<const C_Column*, U32>::Iterator itCol;
		for(itCol = m_ColumnMapping.Begin(); itCol != m_ColumnMapping.End(); ++itCol)
		{
			const C_Column* pCol = itCol->GetKey();
			U32 colIndex = itCol->GetValue();
			pSheet->Cell(0, colIndex)->SetString(pCol->GetLabel());
		}

		//Get data for every row
		C_Selection selection;
		wxGetApp().GetSelection(selection);
		
		C_RowSet* pRowSet = selection.GetCurrentRowSet();
		if (!pRowSet)
			return false;

		for(S32 rowNum = 0; rowNum < pRowSet->GetNumRows(); rowNum++)
		{
			C_Row* pRow = pRowSet->GetRow(rowNum);
			Xt::DomComponent* pComponent = Xt::AsComponent(pRow->GetXtData());
			Xt::DomBlock* pBlock = &pComponent->data;

			Btl::Map<const C_Column*, U32>::Iterator colPosIt;
			for(colPosIt = m_ColumnMapping.Begin(); colPosIt != m_ColumnMapping.End(); ++colPosIt)
			{
				const C_Column* pCol = colPosIt->GetKey();
				U32 colIndex = colPosIt->GetValue();

				const C_Row::ColumnMap& rowColMap = pRow->GetColumns();
				C_Row::ColumnMap::ConstIterator rowColMapIt = rowColMap.Find(pCol->GetKey());
				if (rowColMapIt != rowColMap.End())
				{
					C_MutantValue value = rowColMapIt->GetValue();
					switch(value.GetType())
					{
					case E_DataType_Bool:
					case E_DataType_Int:
					case E_DataType_UInt:
					case E_DataType_Long:
					case E_DataType_Pointer:
						pSheet->Cell(rowNum + 1, colIndex)->SetInteger(value);
						break;
					case E_DataType_Float:
					case E_DataType_Double:
						pSheet->Cell(rowNum + 1, colIndex)->SetDouble(value);
						break;
					case E_DataType_String:
						pSheet->Cell(rowNum + 1, colIndex)->SetString(wxString(value).c_str());
						break;
					}
				}
			}
		}
		
		m_isExcelDocWritten = true;
		m_isExcelDocSaved = false;

		return m_isExcelDocWritten;
	}
	return false;
}

bool C_ExcelWriter::SaveExcelDoc()
{
	if(IsExcelDocCreated() && !IsExcelDocClosed() && IsExcelDocWritten())
	{
		assert(m_pExcelDoc);

		const wxCharBuffer filenameAscii = ToAscii(GetFilename());
		m_isExcelDocSaved = m_pExcelDoc->SaveAs( (const char*)filenameAscii ) ;
		if(m_isExcelDocSaved)
			m_isExcelDocWritten = false;

		return m_isExcelDocSaved;
	}
	return false;
}

bool C_ExcelWriter::CloseExcelDoc()
{
	if(IsExcelDocCreated() && !IsExcelDocClosed())
	{
		assert(m_pExcelDoc);

		delete m_pExcelDoc;
		m_pExcelDoc = NULL;
		
		m_isExcelDocCreated = false;
		m_isExcelDocWritten = false;
		m_isExcelDocSaved = false;
		m_isExcelDocClosed = true;
		
		return m_isExcelDocClosed;
	}
	return false;
}

} //namespace DMan
#include "stdafx.h"

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Xt/Types/IXtType.h>
#include <DialogueMan/Xt/XtTypeDatabase.h>
#include <DialogueMan/App.h>

namespace DMan
{

C_DbManager::C_DbManager()
{
}

C_DbManager::~C_DbManager()
{
	Clear();
}

const C_DbManager::ColumnSetHashTable& C_DbManager::GetTypeColumnSets()
{
	return m_typeColumns;
}

C_ColumnSet* C_DbManager::GetTypeColumnSet(LarvaType* pType)
{
	ColumnSetHashTable::Iterator it = m_typeColumns.Find(pType);
	if (it == m_typeColumns.End())
	{
		assert(!"Failed to find column set for type.");
		return 0;
	}

	return it->GetValue();
}

C_Record* C_DbManager::GetRecord(XtId recordId)
{
	RecordRowsHashTable::Iterator rowIt = m_records.Find(recordId);
	if (rowIt == m_records.End())
	{
		return 0;
	}

	return rowIt->GetValue();
}

void C_DbManager::OnDbOpened()
{
	CreateTypeColumnSets();
	XtDeserialise();
}

void C_DbManager::OnDbClose()
{
	Clear();
}

void C_DbManager::Clear()
{
	for (RecordRowsHashTable::Iterator it = m_records.Begin(); it != m_records.End(); ++it)
	{
		it->GetValue()->XtUnbind();
		delete it->GetValue();
	}
	m_records.Clear();

	for (ColumnSetHashTable::Iterator it = m_typeColumns.Begin(); it != m_typeColumns.End(); ++it)
		delete it->GetValue();
	m_typeColumns.Clear();
}

void C_DbManager::CreateTypeColumnSets()
{
	//Create column layouts and storage for rows per layout using schema.

	for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&wxGetApp().GetDb()->schema->types); it; ++it)
	{
		LarvaType* pType = (LarvaType*)it;
		CreateTypeColumnSet(pType);
	}
}

void C_DbManager::CreateTypeColumnSet(LarvaType* pLarvaType, C_ColumnSet* pColumnSet)
{
	//Create a new column set!
	if (!pColumnSet)
	{
		E_XtType type = wxGetApp().GetXtTypeDatabase().GetType(pLarvaType);
		
		//Only looking for dialogue types...
		if (type != E_XtType_SxlRow)
			return;

		//Respect hidden types.
		if (pLarvaType->hidden)
			return;

		pColumnSet = new C_ColumnSet;
		assert(m_typeColumns.Find(pLarvaType) == m_typeColumns.End());
		m_typeColumns[pLarvaType] = pColumnSet;
	}

	pColumnSet->InsertColumn(
		BuildColumnKey(
			ToUnicode("__arrayindex"), ToUnicode("Array Index")),
			0).
		SetLabel(ToUnicode("Index")).
		SetImportable(true).
		SetComparable(false).
		SetDataType(E_DataType_Int).
		SetMetaData("arrayindex");

	//Process base first.
	LarvaBaseType* pBase = ((LtStruct*)pLarvaType)->base;
	if(pBase)
		CreateTypeColumnSet(pBase->type, pColumnSet);

	assert(pLarvaType->kind == kLtStruct);
	LtStruct* pStructType = (LtStruct*)pLarvaType;
	for(XtUt::List<LarvaField>::Iterator it(&pStructType->fields); it; ++it)
	{
		bool isImportable = it->attrs.GetBool(kXtSemanticAttr, "importexport", false);
		bool isComparable = it->attrs.GetBool(kXtSemanticAttr, "comparable", false);
		int rowWidth = it->attrs.GetInt(kXtSemanticAttr, "columnwidth", -1);
		
		if (it->kind == kLfLocString)
		{
			E_DataType dataType = E_DataType_String;

			pColumnSet->InsertColumn(
				BuildColumnKey(
					ToUnicode(pLarvaType->name.Text()), ToUnicode(it->name.Text()).Append(wxString::FromUTF8("__record"))),
					it).
				SetLabel(ToUnicode("Loc Record")).
				SetImportable(true).
				SetComparable(false).
				SetDataType(dataType).
				SetMetaData("record");

			pColumnSet->InsertColumn(
				BuildColumnKey(
					ToUnicode(pLarvaType->name.Text()), ToUnicode(it->name.Text()).Append(wxString::FromUTF8("__id"))),
					it).
				SetLabel(ToUnicode("Name")).
				SetImportable(true).
				SetComparable(false).
				SetDataType(dataType).
				SetMetaData("id");

			pColumnSet->InsertColumn(
				BuildColumnKey(
					ToUnicode(pLarvaType->name.Text()), ToUnicode(it->name.Text()).Append(wxString::FromUTF8("__string"))),
					it).
				SetLabel(ToUnicode("English")).
				SetImportable(true).  //always make this importable
				SetComparable(true).  //always make this comparable
				SetDataType(dataType).
				SetMetaData("string").
				SetWidth(400);
		}
		else
		{
			E_DataType dataType = E_DataType_String;
			switch(it->kind)
			{
			case(kLfInt):
			case(kLfEnum):
				{
				
				LfInt* pNativeType = (LfInt*)(LarvaField*)it;
				switch(pNativeType->type->atomType)
				{
				case(kXtI8):
				case(kXtI16):
				case(kXtI32):
					dataType = E_DataType_Int;
					break;
				case(kXtI64):
					dataType = E_DataType_Long;
					break;
				case(kXtU8):
				case(kXtU16):
				case(kXtU32):
					dataType = E_DataType_UInt;
					break;
				default:
					dataType = E_DataType_Long;
				}

				break;
				}
			case(kLfFloat):
				dataType = E_DataType_Float;
				break;
			case(kLfString):
			case(kLfWideString):
			case(kLfLocString):
				dataType = E_DataType_String;
				break;
			case(kLfBool):
				dataType = E_DataType_Bool;
				break;
			default:
				continue;
			}
			
			pColumnSet->InsertColumn(
				BuildColumnKey(
					ToUnicode(pLarvaType->name.Text()), ToUnicode(it->name.Text())),
					it).
				SetLabel(ToUnicode(it->label.Text())).
				SetImportable(isImportable).
				SetComparable(isComparable).
				SetDataType(dataType).
				SetWidth(rowWidth);
		}
	}  
}

C_Record* C_DbManager::AcquireRecord(LarvaType* pDialogueType, XtId recordId)
{
	RecordRowsHashTable::Iterator rowIt = m_records.Find(recordId);
	if (rowIt == m_records.End())
	{
		C_Record* p = new C_Record(recordId);
		XtShared::C_XtObjectImpl* pImpl = new XtShared::C_XtObjectImpl();
		p->AttachXtObjectImpl(pImpl);

		if (!p->XtInitialise())
		{
			delete p;
			return 0;
		}
		p->XtBind();
		m_records[recordId] = p;
		return p;
	}

	return rowIt->GetValue();
}

bool C_DbManager::RemoveRecord(XtId recordId)
{
	RecordRowsHashTable::Iterator rowIt = m_records.Find(recordId);
	if (rowIt != m_records.End())
	{
		rowIt->GetValue()->XtUnbind();
		delete rowIt->GetValue();
		return m_records.Erase(rowIt) > 0;
	}
	return false;
}

void C_DbManager::XtDeserialise()
{
	//Using the xt dom, populate tables of data per column layout.
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(wxGetApp().GetDb()->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&(pPack->records)); recordIt; ++recordIt)
		{
			Xt::DomRecord* pRec = recordIt;

			LarvaType* pDialogueType = Xt::TypeOf(wxGetApp().GetDb()->schema, pRec);
			if (!pDialogueType) 
				continue;

			I_XtType* pType = wxGetApp().GetXtTypeDatabase().FindType(pDialogueType->name.Text());
			if (!pType || !pType->IsA(E_XtType_SxlRecord))
				continue;

			//Initialise new row set.
			C_Record* pRecord = AcquireRecord(pDialogueType, pRec->id);
			if (!pRecord) 
				continue;

			pRecord->XtDeserialise();
		}
	}
}


}

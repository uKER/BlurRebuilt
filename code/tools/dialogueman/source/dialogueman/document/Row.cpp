#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/GlobalSignals.h>
#include <DialogueMan/App.h>

namespace DMan
{

C_Row::C_Row(C_RowSet* pRowSet, XtId xtId, S32 xtArrayIndex)
:
m_pRowSet(pRowSet),
m_xtId(xtId),
m_xtArrayIndex(xtArrayIndex)
{
	m_flags.dirty = false;
}

C_Row::~C_Row()
{
}

XtId C_Row::GetXtId() const
{
	return m_xtId;
}

void C_Row::SetXtArrayIndex(S32 index)
{
	m_xtArrayIndex = index;
}

S32 C_Row::GetXtArrayIndex() const
{
	return m_xtArrayIndex;
}

const C_Row::ColumnMap& C_Row::GetColumns() const
{
	return m_columns;
}

C_Row::ColumnMap& C_Row::GetColumns()
{
	return m_columns;
}

bool C_Row::XtInitialise(
	XtShared::C_XtType xtType, 
	bool isPolymorphic,
	Xt::DomElement* pData)
{
	if (!__super::XtInitialise(xtType, isPolymorphic, pData))
		return false;

	return true;
}

void C_Row::SetDirty(bool dirty)
{
	m_flags.dirty = dirty;
	if(dirty)
		wxGetApp().AddForDeferredUpdate(GetXtObjectImpl());
}

void C_Row::XtDeserialise(Xt::DomElement* pFrom)
{
	SetDirty();
}

void C_Row::XtDeserialiseDeferred()
{
	if (!m_flags.dirty)
		return;

	static const wxString kUnknown(wxT("####"));

	Xt::DomComponent* pData = Xt::AsComponent(GetXtData());
	
	//Loop thru columns.
	for (S32 iCol = 0; iCol < (S32)m_pRowSet->GetAllColumnSet()->GetNumColumns(); ++iCol)
	{
		C_Column* pColumn = m_pRowSet->GetAllColumnSet()->GetColumn(iCol);

		// Handle virtual fields
		if (pColumn->GetMetaData() == "arrayindex")
		{
			m_columns[pColumn->GetKey()] = C_MutantValue((int)GetXtArrayIndex());
			continue;
		}
		
		XtString baseTypeName = (const char*)ToAscii(XtTypeNameFromColumnKey(pColumn->GetKey()));
		LarvaType* pBaseType = FindTypeByName(wxGetApp().GetDb()->schema, baseTypeName);
		LarvaType* pMyType = GetXtType().AsType();

		//If we've no type, or aren't derived from a valid base type, then skip.
		if (!pMyType || !pBaseType || !IsBaseOf((LtStruct*)pBaseType, (LtStruct*)pMyType))
		{
			m_columns[pColumn->GetKey()] = kUnknown;
			continue;
		}
		
		Xt::DomElement* pElement = Xt::At(
			&pData->data, 
			pColumn->GetLarvaField()->name);

		if (!pElement)
			continue;

		//For the LocID field
		if (pColumn->GetLarvaField()->kind == kLfLocString)
		{
			
			Xt::DomBlock* pBlock	= Xt::AsBlock(pElement);
			Xt::DomField* pId		= Xt::AsField(Xt::At(pBlock, "id"));
			Xt::DomField* pRecord	= Xt::AsField(Xt::At(pBlock, "record"));

			if (!pId || !pRecord)
			{
				m_columns[pColumn->GetKey()] = wxString::FromUTF8("----");
				continue;
			}

			Xt::DomRecord* record = Xt::FindRecordById(wxGetApp().GetDb(), Xt::ParseId(pRecord->value));

			if (!record || record->root.type == kXtNullString)
			{
				m_columns[pColumn->GetKey()] = wxString::FromUTF8("----");
				continue;
			}

			LarvaType* recordType = wxGetApp().GetDb()->schema->types[record->root.type];

			if (!recordType)
			{
				m_columns[pColumn->GetKey()] = wxString::FromUTF8("----");
				continue;
			}

			Xt::DomArray* stringArray = Xt::AsArray(Xt::At(&record->root.data, "strings"));

			Xt::DomBlock* stringBlock = 0;

			XtId id = Xt::ParseId(Xt::AsText(pId));
			if (id)
			{
				stringBlock = Xt::AsBlock(XtUt::At(&stringArray->elements, id));
			}

			if (!stringBlock)
			{
				m_columns[pColumn->GetKey()] = wxString::FromUTF8("----");
				continue;
			}

			if (pColumn->GetMetaData() == "id")
			{
				Xt::DomField* nameField = Xt::AsField(Xt::At(stringBlock, "name"));

				if (!nameField)
				{
					m_columns[pColumn->GetKey()] = wxString::FromUTF8("----");
					continue;
				}

 				m_columns[pColumn->GetKey()] = wxString::FromUTF8(nameField->value.Text());
			}
			else if (pColumn->GetMetaData() == "record")
			{
				m_columns[pColumn->GetKey()] = wxString::FromUTF8(EvaluateSummaryText(wxGetApp().GetDb(), wxGetApp().GetDb()->schema, AsStruct(recordType), &record->root, "{__name}").Text());
			}
			else if (pColumn->GetMetaData() == "string")
			{
				Xt::DomField* stringField = Xt::AsField(Xt::At(stringBlock, "english"));

				if (!stringField)
				{
					m_columns[pColumn->GetKey()] = wxString::FromUTF8("----");
					continue;
				}

				m_columns[pColumn->GetKey()] = wxString(stringField->value.Text());
			}
			
			continue;
		}

		Xt::DomField* pField = Xt::AsField(pElement);
		if (pField)
		{
			m_columns[pColumn->GetKey()] = wxString::FromAscii(pField->value.Text());
		}
		else
		{
			assert(!"Could not find field!");
		}
	}
	m_flags.dirty = false;

	wxGetApp().GetGlobalSignals().rowDeserialised(this);
}

void C_Row::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent)
{
	XtDeserialise(pEvent->from);
}

}

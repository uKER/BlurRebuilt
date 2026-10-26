#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Misc/StringUtil.h>
#include <DialogueMan/Xt/Types/IXtType.h>
#include <DialogueMan/Xt/XtTypeDatabase.h>
#include <DialogueMan/GlobalSignals.h>
#include <DialogueMan/App.h>

namespace DMan
{

class ColumnSort
{
public:
	ColumnSort(S32 colKey, bool desc = false) : 
	  m_colKey(colKey), m_desc(desc)
	{}

	bool operator()(const wxString* lhs, const wxString* rhs)
	{
		if(m_desc) 
			return lhs[m_colKey] > rhs[m_colKey];
		else
			return lhs[m_colKey] < rhs[m_colKey];
	}

private:
	const S32 m_colKey;
	const bool m_desc;
};



C_RowSet::C_RowSet(LarvaField* pArrayField)
:
m_pArrayField(pArrayField),
m_pAllColumnSet(0)
{
}

C_RowSet::~C_RowSet()
{
	for (S32 i = 0; i < m_rows.Size(); ++i)
		delete m_rows[i];

	delete m_pAllColumnSet;
}

C_ColumnSet* C_RowSet::GetAllColumnSet()
{
	return m_pAllColumnSet;
}


S32 C_RowSet::GetNumRows()
{
	return m_rows.Size();
}

C_Row* C_RowSet::GetRow(U32 xtRowIndex)
{
	if((int)xtRowIndex >= m_rows.Size())
		return NULL;
	
	return m_rows[xtRowIndex];
}

C_RowSet::RowArray& C_RowSet::GetRows()
{
	return m_rows;
}

const C_RowSet::RowArray& C_RowSet::GetRows() const
{
	return m_rows;
}

void C_RowSet::XtBind() const
{
	__super::XtBind();

	for (S32 i = 0; i < m_rows.Size(); ++i)
		m_rows[i]->XtBind();
}

void C_RowSet::XtUnbind() const
{
	__super::XtUnbind();

	for (S32 i = 0; i < m_rows.Size(); ++i)
		m_rows[i]->XtUnbind();
}

bool C_RowSet::XtInitialise(XtShared::C_XtType xtType, bool isPolymorphic, Xt::DomElement* pData)
{
	if (!__super::XtInitialise(xtType, isPolymorphic, pData))
		return false;

	//Create all column set.
	{
		//Iterate column sets. For each column set derived from our base type grab its columns.
		const C_DbManager::ColumnSetHashTable& columnSets = 
			wxGetApp().GetDbManager().GetTypeColumnSets();

		m_pAllColumnSet = new C_ColumnSet;
		for (
			C_DbManager::ColumnSetHashTable::ConstIterator it = columnSets.Begin(); 
			it != columnSets.End(); 
			++it)
		{
			LarvaType* pDerivedItemType = it->GetKey();
			if (IsBaseOf((LtStruct*)xtType.AsField()->Type(), (LtStruct*)pDerivedItemType))
			{
				C_ColumnSet* pColumnSet = it->GetValue();
				for (S32 i = 0; i < (S32)pColumnSet->GetNumColumns(); ++i)
					m_pAllColumnSet->AddColumn(pColumnSet->GetColumn(i)->Clone());
			}
		}
	}

	return true;
}

void C_RowSet::XtDeserialise(Xt::DomElement* pFrom)
{
	LarvaType* pBaseItemType = GetXtType().AsType();

	if (!pFrom)
	{
		Xt::DomElement* pTest = GetXtData(); // TODO: Components
		Xt::DomArray* pArray = Xt::AsArray(GetXtData());
		LfArray* pLarvaArray = (LfArray*)GetXtType().AsField();

		//Find array item field type.
		LarvaType* pBaseItemType = pLarvaArray->field->Type();

		assert(pArray);
		//Loop thru rows.
		U32 count = Xt::Count(pArray);
		if (!count) return;

		m_rows.Reserve(count);
		for(Xt::DomArray::IndexIterator it(pArray); it; ++it)
		{
			//Xt row data.
			Xt::DomComponent* pXtRow = AsComponent(it);

			if (!AddNewRow(it.Index(), pXtRow))
				continue;
		}
	}
}

bool C_RowSet::AddNewRow(
	S32 xtArrayIndex,
	Xt::DomElement* pElement)
{
	Xt::DomComponent* pData = Xt::AsComponent(pElement);
	assert(pData);

	XtId xtId = pData->id;

	if (m_xtIdHashTable.Find(xtId) != m_xtIdHashTable.End())
	{
		assert(!"Id collision!");
		return false;
	}

	LfArray* pLarvaArray = (LfArray*)GetXtType().AsField();
	LarvaType* pBaseItemType = pLarvaArray->field->Type();
	LarvaType* pItemType = NULL;
	
	bool isPolymorphic;
	{
			//Attempt to find polymorphic dialogue item type.
		if (pLarvaArray->field->kind == kLfPtr)
		{
			if (pData->type)
			{
				pItemType = FindTypeByName(wxGetApp().GetDb()->schema, pData->type);
				if (pItemType)
				{
					//Check for unrecognised base type.
					E_XtType type = wxGetApp().GetXtTypeDatabase().GetType(pItemType);
					if (wxGetApp().GetXtTypeDatabase().GetType(pItemType) != E_XtType_SxlRow)
						//Null item.
						pItemType = 0;
				}

				isPolymorphic = true;
			}
		}
		else
		{
			pItemType = pBaseItemType;
			isPolymorphic = false;
		}
	}

	C_Row* pRow = new C_Row(this, xtId, xtArrayIndex);
	bool appended = xtArrayIndex >= m_rows.Size();
	if(appended) //Append
	{
		m_rows.PushBack(pRow);	
	}
	else //Insert in the middle
	{
		m_rows.InsertBefore(pRow, m_rows.Begin() + xtArrayIndex);
		//Fix C_Row indices below the insertion point
		for(S32 i = xtArrayIndex + 1; i < m_rows.Size(); ++i)
		{
			m_rows[i]->SetXtArrayIndex(m_rows[i]->GetXtArrayIndex() + 1);
			m_rows[i]->XtDeserialise();
		}
	}
	
	m_xtIdHashTable[xtId] = pRow;

	XtShared::C_XtObjectImpl* pImpl = new XtShared::C_XtObjectImpl();
	pRow->AttachXtObjectImpl(pImpl);
	if (!pRow->XtInitialise(pItemType, isPolymorphic, pData))
		return false;

	pRow->XtDeserialise();
	pRow->XtBind();
	if(appended)
		wxGetApp().GetGlobalSignals().rowSetRowAppended(this, pRow);
	else
		wxGetApp().GetGlobalSignals().rowSetRowInserted(this, pRow, xtArrayIndex);
	return true;
}

void C_RowSet::OnRowRemoved(Xt::DomElement* pFrom)
{
	Xt::DomArray* pArray = Xt::AsArray(pFrom);
	assert(pArray);

	XtIdHashSet rowsToPreserve;

	for(Xt::DomArray::IndexIterator arrayIt(pArray); arrayIt; ++arrayIt)
	{
		Xt::DomComponent* pData = Xt::AsComponent(arrayIt);
		
		XtId pId = pData->id;
		assert(pId);

		XtIdHashTable::Iterator hashIt = m_xtIdHashTable.Find(pId);
		if (hashIt == m_xtIdHashTable.End())
		{
			assert(!"Failed to find row!");
			return;
		}
		
		//Sync index.
		hashIt->GetValue()->SetXtArrayIndex(arrayIt.index);
		hashIt->GetValue()->XtDeserialise();
		//Mark as touched.
		rowsToPreserve.Insert(hashIt->GetKey());
	}

	//Should only be removing one row at a time.
	assert(rowsToPreserve.Size() == m_xtIdHashTable.Size()-1);

	//Remove entry in m_xtIdHashTable which is not in rowsToPreserve.
	for (XtIdHashTable::Iterator hashIt = m_xtIdHashTable.Begin(); hashIt != m_xtIdHashTable.End(); ++hashIt)
	{
		//If not found...
		if (rowsToPreserve.Find(hashIt->GetKey()) == rowsToPreserve.End())
		{
			wxGetApp().GetGlobalSignals().rowSetRowDelete(this, hashIt->GetValue());
		
			//Erase from row array.
			S32 idx = Btl::FindIndex(m_rows, hashIt->GetValue());
			assert(idx != m_rows.Size());
			m_rows.EraseByIndex(idx);

			delete hashIt->GetValue();
			m_xtIdHashTable.Erase(hashIt);

			//Only removing one row at a time.
			break;
		}
	}
}

bool C_RowSet::OnPotentialRowAdd(Xt::DomElement* pFrom, S32 index)
{
	Xt::DomComponent* pData = Xt::AsComponent(pFrom);
	XtId pId = pData->id;

	if (pId)
	{
		XtIdHashTable::Iterator it = m_xtIdHashTable.Find(pId);
		if (it != m_xtIdHashTable.End()) 
			return false;
	}

	return AddNewRow(index, pFrom);
}

bool C_RowSet::OnPotentialRowReorder(Xt::DomElement* pFrom, S32 index)
{
	Xt::DomComponent* pData = Xt::AsComponent(pFrom);
	XtId pId = pData->id;
	if (!pId)
		return false;
	XtIdHashTable::Iterator it = m_xtIdHashTable.Find(pId);
	if (it == m_xtIdHashTable.End()) 
		return false;

	C_Row* pRow = it->GetValue();
	pRow->SetXtArrayIndex(index);
	wxGetApp().GetGlobalSignals().rowSetRowReordered(this, pRow);
	return true;
}

void C_RowSet::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent)
{
	if (pEvent->code == Xt::kDomArrayChanged && pEvent->from->parentKey == "Items")
	{
		Xt::DomArrayChanged* pArrayChangedEvent = (Xt::DomArrayChanged*)pEvent;
		//Row removed?
		Xt::DomArray* pArray = Xt::AsArray(pEvent->from);
		assert(pArray);
		int	index = Xt::IndexOf(pArray, pArrayChangedEvent->id);

		if (index == -1)
			OnRowRemoved(pEvent->from);
		//Check for row being moved, otherwise row has been added.
		else 
		{
			Xt::DomElement*	el		= Xt::FindById(pArray, pArrayChangedEvent->id);

			if (OnPotentialRowReorder(el, index))
				return;

			OnPotentialRowAdd(el, index);
		}
	}
}

} 

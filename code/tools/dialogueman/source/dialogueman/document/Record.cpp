#include "stdafx.h"

#include <DialogueMan/Document/DbManager.h>
#include <DialogueMan/Document/ColumnSet.h>
#include <DialogueMan/Document/Row.h>
#include <DialogueMan/Document/RowSet.h>
#include <DialogueMan/Document/Record.h>
#include <DialogueMan/Xt/Types/IXtType.h>
#include <DialogueMan/Xt/XtTypeDatabase.h>
#include <DialogueMan/App.h>

namespace DMan
{

C_Record::C_Record(XtId xtRecordId)
:
m_xtRecordId(xtRecordId)
{
}

C_Record::~C_Record()
{
	for (S32 i = 0; i < m_rowSets.Size(); ++i)
		delete m_rowSets[i];
}

C_RowSet* C_Record::GetRowSet(S32 index)
{
	if (index == -1)
		return 0;

	return m_rowSets[index];
}

U32	C_Record::GetNumRowSets() const
{
	return (S32)m_rowSets.Size();
}

XtId C_Record::GetRecordId()
{
	return m_xtRecordId;
}

XtString C_Record::GetName() const
{
	Xt::DomRecord* pRec = FindRecordById(wxGetApp().GetDb(), m_xtRecordId);
	return pRec->name;
}

void C_Record::XtBind() const
{
	__super::XtBind();

	for (S32 i = 0; i < m_rowSets.Size(); ++i)
		m_rowSets[i]->XtBind();
}

void C_Record::XtUnbind() const
{
	__super::XtUnbind();

	for (S32 i = 0; i < m_rowSets.Size(); ++i)
		m_rowSets[i]->XtUnbind();
}

bool C_Record::XtInitialise()
{
	Xt::DomRecord* pRec = FindRecordById(wxGetApp().GetDb(), m_xtRecordId);
	LarvaType* pDialogueType = Xt::TypeOf(wxGetApp().GetDb()->schema, pRec);

	if (!__super::XtInitialise(pDialogueType, false, &pRec->root))
		return false;

	//Here we would loop thru all row sets. 
	for(XtUt::List<LarvaField>::Iterator it(&((LtStruct*)pDialogueType)->fields); it; ++it)
	{
		if (it->attrs.GetString(kXtSemanticAttr, kXtSemanticParam) != "SxlRows")
			continue;

		//Create and initialise row
		m_rowSets.PushBack(new C_RowSet(it));
		C_RowSet* pRowSet = m_rowSets.Back();

		XtShared::C_XtObjectImpl* pImpl = new XtShared::C_XtObjectImpl();
		pRowSet->AttachXtObjectImpl(pImpl);

		Xt::DomArray* pArray = Xt::AcquireArray(&Xt::AsComponent(GetXtData())->data, it->name);
		LarvaField* pField = it;
		pRowSet->XtInitialise(pField, false, Xt::AsArray(pArray));
	}
	return true;
}

void C_Record::XtDeserialise(Xt::DomElement* pFrom)
{
	for (S32 i = 0; i < m_rowSets.Size(); ++i)
	{
		assert(m_rowSets[i]);
		m_rowSets[i]->XtDeserialise(pFrom);
	}
}

void C_Record::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent)
{
}

}

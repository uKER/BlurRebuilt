#include "stdafx.h"

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Components/FinderQueries.h>
#include <EditorApp/Components/FindDialog.h>

namespace Editor
{

/////////////////////////// QeAllActors \\\\\\\\\\\\\\\\\\\\\\\\\\\

void QeAllActors::Process(Xt::QueryContext* pContext, const Xt::QueryResult& source)
{
	Xt::DomRecord* pLevelRecord = 
		EditorLib::C_Core::Instance().GetXtManager().FindRecord(
			EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel()->GetXtRecordId());
	
	assert(pLevelRecord);

	LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;

	LtStruct*  pRecordType = Xt::TypeOf(pSchema, pLevelRecord);

	Xt::FieldDom fd = AcquireBySemantic(
		pRecordType, 
		&pLevelRecord->root.data, 
		EditorLib::kXtSemanticAttr, 
		EditorLib::kXtSemanticParam, 
		EditorLib::kXtSemanticLabel_ActorArray);
	
	Xt::DomArray* pLevelActorArray = Xt::AsArray(fd.dom);

	assert(pLevelActorArray != 0);
	assert(fd.field->kind == kLfArray);

	LfArray* fieldArr = (LfArray*)fd.field;

	for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		if (!pComponent)
			continue;

		//This string will be null if __type doesn't exist or is null.
		LarvaType* pType = Xt::TypeOf(pSchema, fieldArr->field, pComponent);

		if (!pType)
			continue;

		Result(pContext, Xt::QueryResult(0, AsStruct(pType), pComponent, "/"));
	}
}

/////////////////////////// QeByActorName \\\\\\\\\\\\\\\\\\\\\\\\\\\

QeByActorName::QeByActorName(const XtString& name) : 
	m_name(name)
{
}


void QeByActorName::Process(Xt::QueryContext* pContext, const Xt::QueryResult& source)
{
	if (Xt::DomComponent* pComponent = ComponentOf(source.dom))
	{
		LtStruct*	pComponentType = source.type;

		Xt::DomField* pNameField = Xt::AsField(AcquireBySemantic(
			pComponentType,
			&pComponent->data,
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_Name).dom);

		if (IndexOfStringI<char>(pNameField->value, m_name) >= 0)
			Result(pContext, source);
	}
}

/////////////////////////// QeByActorType \\\\\\\\\\\\\\\\\\\\\\\\\\\

QeByActorType::QeByActorType(LtStruct* pType)  : 
	m_pType(pType) 
{
}


void QeByActorType::Process(Xt::QueryContext* pContext, const Xt::QueryResult& source)
{
	if (Xt::DomComponent* pComponent = ComponentOf(source.dom))
	{
		if (IsBaseOf(m_pType, source.type))
			Result(pContext, source);
	}

}

/////////////////////////// QeByActorFieldExpression \\\\\\\\\\\\\\\\\\\\\\\\\\\

QeByActorFieldExpression::QeByActorFieldExpression(LtStruct* type, Xt::FieldPath* path, Xt::FieldExpressionCmp cmp, const XtString& low, const XtString& high) :
	Xt::QeFieldExpression(type, path, cmp, low, high)
{
}

void QeByActorFieldExpression::Process(Xt::QueryContext* pContext, const Xt::QueryResult& source) 
{
	if (source.type)
	{
		Xt::DomBlock* block = NULL;
		if(source.dom->kind == Xt::kDomComponent)
			block = &Xt::AsComponent(source.dom)->data;
		else if (source.dom->kind == Xt::kDomBlock)
			block = Xt::AsBlock(source.dom);
		
		if(!block)
			return;

		DoObject(pContext, source.type, block);
	}
		
}

/////////////////////////// LogResults \\\\\\\\\\\\\\\\\\\\\\\\\\\

LogResults::LogResults()
{
}


void LogResults::Process(Xt::QueryContext* pContext, const Xt::QueryResult& source)
{
	LarvaSchema* pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::DomComponent* pComponent = ComponentOf(source.dom);

	LtStruct*	pComponentType = source.type;

	Xt::DomField* pNameField  = Xt::AsField(AcquireBySemantic(
		pComponentType,
		&pComponent->data,
		EditorLib::kXtSemanticAttr,
		EditorLib::kXtSemanticParam,
		EditorLib::kXtSemanticLabel_Name).dom);
	
	LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Found component: name = %s\n", (const char*)pNameField->value);  
}

/////////////////////////// SinkActorResults \\\\\\\\\\\\\\\\\\\\\\\\\\\

SinkActorResults::SinkActorResults(C_FinderDialog* pFinderDlg) :
	m_pFinderDlg(pFinderDlg)
{
}

void SinkActorResults::Process(Xt::QueryContext* pContext, const Xt::QueryResult& source)
{
	m_pFinderDlg->AddResult(&source);
}

}

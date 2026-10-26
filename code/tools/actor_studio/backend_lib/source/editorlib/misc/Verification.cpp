#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/Verification.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Core.h>

namespace EditorLib
{

C_VerificationResultDataActor::C_VerificationResultDataActor(Xt::DomComponent* pComponent)
{
	m_pComponent = pComponent;
}

Xt::DomComponent* C_VerificationResultDataActor::GetXtData() const
{
	return m_pComponent;
}

Btl::String C_VerificationResultDataActor::GetName() const
{
	Xt::DomField* pNameField  = Xt::AsField(AcquireBySemantic(
			AsStruct(FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, m_pComponent->type)),
			&m_pComponent->data,
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_Name).dom);

	if (pNameField)
		return pNameField->value.Text();
	else
		return "???";
}

C_SceneNode* C_VerificationResultDataActor::GetActor() const
{
	EditorLib::I_DgNode* dgNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(EditorLib::C_NodeId(m_pComponent->id, 0));

	if (dgNode->IsA(EditorLib::E_ObjType_SceneNode))
		return (C_SceneNode*)dgNode;
	else
		return 0;
}

C_VerificationResultDataXt::C_VerificationResultDataXt(Xt::DomElement* pElement)
{
	m_pElement = pElement;
}

Xt::DomElement* C_VerificationResultDataXt::GetXtData() const
{
	return m_pElement;
}

C_VerificationResultDataPosition::C_VerificationResultDataPosition(const Maths::Vector3& position)
{
	m_Position = position;
}

const Maths::Vector3& C_VerificationResultDataPosition::GetPosition() const
{
	return m_Position;
}

C_VerificationResult::C_VerificationResult(E_VerificationResultLevel errorLevel, const Btl::String& msg, LarvaField* field, Xt::DomElement* dom)
{
	m_ErrorLevel = errorLevel;

	m_Msg = field->name.Text();
	ParseRefLocation(m_Msg, dom);
	m_Msg = Btl::String::Format("%s (%s)", msg.Chars(), m_Msg.Chars());
	
	m_Data.PushBack(new C_VerificationResultDataXt(dom));
}

C_VerificationResult::C_VerificationResult(E_VerificationResultLevel errorLevel, const Btl::String& msg)
{
	m_ErrorLevel = errorLevel;

	m_Msg = msg;
}

C_VerificationResult::~C_VerificationResult()
{
	for (Btl::Vector<I_VerificationResultData*>::Iterator it = m_Data.Begin(); it != m_Data.End(); ++it)
	{
		delete *it;
	}
}

Btl::String C_VerificationResult::AsString() const
{
	return m_Msg;
}

E_VerificationResultLevel C_VerificationResult::GetErrorLevel() const
{
	return m_ErrorLevel;
}

void C_VerificationResult::AddData(EditorLib::I_VerificationResultData* data)
{
	m_Data.PushBack(data);
}

const Btl::Vector<I_VerificationResultData*>& C_VerificationResult::GetData() const
{
	return m_Data;
}

void C_VerificationResult::ParseRefLocation(Btl::String& location, Xt::DomElement* dom)
{
	if (dom == 0)
		return;

	if (dom->kind == Xt::kDomRecord)
	{
		location = location + " -> " + Xt::AsRecord(dom)->name.Text();
	}
	else if (dom->kind == Xt::kDomComponent)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(dom);
		location = location + " -> " + pComponent->type.Text();

		LarvaType* pActorType = FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, "ActorCreationData");
		LarvaType* pType = FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pComponent->type);
		
		if (IsBaseOf(AsStruct(pActorType), AsStruct(pType)))
		{
			m_Data.PushBack(new C_VerificationResultDataActor(pComponent));
		}

		Xt::DomField* pNameField  = Xt::AsField(AcquireBySemantic(
			AsStruct(FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pComponent->type)),
			&pComponent->data,
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_Name).dom);

		if (pNameField)
		{
			location = location + " (" + pNameField->value.Text() + ")";
		}
	}

	ParseRefLocation(location, dom->parent);
}

C_VerificationRecord::C_VerificationRecord(XtId xtId)
{
	m_XtId = xtId;

	Xt::DomRecord* pRecord = C_Core::Instance().GetXtManager().FindRecord(xtId);

	if (pRecord)
	{
		Xt::DomPack* pPack = Xt::PackOf(pRecord);
		if (pPack)
			m_Name = Btl::String::Format("%s.%s", pPack->name.Text(), pRecord->name.Text());
		else
			m_Name = pRecord->name.Text();
	} else {
		m_Name = "Default";
	}
}

C_VerificationRecord::~C_VerificationRecord()
{
	for (Btl::Vector<C_VerificationResult*>::Iterator it = m_Results.Begin(); it != m_Results.End(); ++it)
	{
		delete *it;
	}
}

XtId C_VerificationRecord::GetXtId() const
{
	return m_XtId;
}

void C_VerificationRecord::SetName(const Btl::String& name)
{
	m_Name = name;
}

const Btl::String& C_VerificationRecord::GetName() const
{
	return m_Name;
}

void C_VerificationRecord::AddResult(C_VerificationResult* result)
{
	m_Results.PushBack(result);
}

const Btl::Vector<C_VerificationResult*>& C_VerificationRecord::GetResults() const
{
	return m_Results;
}

void C_VerificationRecord::PrintToLog(bool detailed)
{
	LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Verification Group: %s", m_Name.Chars());

	if (detailed)
	{
		for (Btl::Vector<C_VerificationResult*>::Iterator it = m_Results.Begin(); it != m_Results.End(); ++it)
		{
			LOGINFO(EditorLib::C_Core::Instance().GetLogger(), (*it)->AsString().Chars());
		}
	}
}

C_VerificationGroup::C_VerificationGroup()
{
	m_Name = "Default";
}

C_VerificationGroup::~C_VerificationGroup()
{
	for (XtIdRecordHash::Iterator it = m_Records.Begin(); it != m_Records.End(); ++it)
	{
		delete it->GetValue();
	}
}

void C_VerificationGroup::SetName(const Btl::String& name)
{
	m_Name = name;
}

const Btl::String& C_VerificationGroup::GetName() const
{
	return m_Name;
}

C_VerificationRecord& C_VerificationGroup::GetRecord(XtId xtId)
{
	if (m_Records.Find(xtId) != m_Records.End())
		return *m_Records[xtId];
	else
	{
		C_VerificationRecord* record = new C_VerificationRecord(xtId);
		m_Records[xtId] = record;
		//record->SetName(name);
		return *record;
	}
}

const C_VerificationGroup::XtIdRecordHash& C_VerificationGroup::GetRecords() const
{
	return m_Records;
}

void C_VerificationGroup::PrintToLog(bool detailed)
{
	for (XtIdRecordHash::Iterator it = m_Records.Begin(); it != m_Records.End(); ++it)
	{
		it->GetValue()->PrintToLog(detailed);
	}
}

C_VerificationSystem::~C_VerificationSystem()
{
	Clear();
}

void C_VerificationSystem::RegisterCallback(Ut::Delegate<bool(void*, XtId)> pHandler, void* pData)
{
	m_Callbacks.PushBack(Btl::MakePair(pHandler, pData));
}

void C_VerificationSystem::Clear()
{
	for (Btl::OpenHashMap<Btl::String, C_VerificationGroup*, Btl::HashKeyOpBtlString>::Iterator it = m_Groups.Begin(); it != m_Groups.End(); ++it)
	{
		delete it->GetValue();
	}

	m_Groups.Clear();
}

C_Status C_VerificationSystem::VerifyLevel(XtId levelId, bool clearPreviousResults)
{
	C_Status status = E_Status_Success;

	if (clearPreviousResults)
		Clear();

	for (Btl::List<Btl::Pair<Ut::Delegate<bool(void*, XtId)>, void*> >::Iterator it = m_Callbacks.Begin(); it != m_Callbacks.End(); ++it)
	{
		if (!it->first(it->second, levelId))
			status = E_Status_Failure;
	}

	return status;
}

C_Status C_VerificationSystem::VerifyAllLevels()
{
	C_Status status = E_Status_Success;

	for (XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&EditorLib::C_Core::Instance().GetXtManager().GetDb()->packs); packIt; ++packIt)
	{
		for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&packIt->records); recordIt; ++recordIt)
		{
			if (recordIt->root.type == EditorLib::kXtLevelRecordType)
			{
				if (!VerifyLevel(recordIt->id, false))
					status = E_Status_Failure;
			}
		}
	}

	return status;
}

C_Status C_VerificationSystem::VerifyDatabase()
{
	return VerifyLevel(0);
}

C_VerificationGroup& C_VerificationSystem::GetGroup(const Btl::String& name)
{
	if (m_Groups.Find(name) != m_Groups.End())
		return *m_Groups[name];
	else
	{
		C_VerificationGroup* group = new C_VerificationGroup();
		m_Groups[name] = group;
		group->SetName(name);
		return *group;
	}
}

const C_VerificationSystem::StringGroupHash& C_VerificationSystem::GetGroups() const
{
	return m_Groups;
}

void C_VerificationSystem::PrintToLog(bool detailed)
{
	for (Btl::OpenHashMap<Btl::String, C_VerificationGroup*, Btl::HashKeyOpBtlString>::Iterator it = m_Groups.Begin(); it != m_Groups.End(); ++it)
	{
		it->GetValue()->PrintToLog(detailed);
	}
}

}
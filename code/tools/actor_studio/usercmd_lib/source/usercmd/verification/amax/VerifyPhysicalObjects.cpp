#include <stdafx.h>

#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Misc/Verification.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Core.h>

#include <UserCmd/Verification/Amax/VerifyPhysicalObjects.h>

namespace EditorUserCmd
{

C_VerifyPhysicalObjectsCmd::C_VerifyPhysicalObjectsCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_VerifyPhysicalObjectsCmd::~C_VerifyPhysicalObjectsCmd()
{
}

EditorLib::C_Status C_VerifyPhysicalObjectsCmd::Do()
{
	XtId inputId = 0;

	if (m_args.IsSet('x'))
	{
		const EditorLib::ArgumentDataArray* pData;

		Btl::String inputStr;
		pData = m_args.GetArgData('x');
		if (pData && pData->Size() != 0)
			(*pData)[0].GetValue(inputStr);

		inputId = Xt::ParseId(inputStr.Chars());

		if (!inputId)
			return EditorLib::E_Status_Failure;
	}

	Xt::DomElement* pElement;

	if (inputId == 0)
	{
		pElement = EditorLib::C_Core::Instance().GetXtManager().GetDb();
	} else {
		Xt::DomPack* pPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(inputId);
		if (pPack)
			pElement = pPack;
		else
		{
			Xt::DomRecord* pRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(inputId);
			if (pRecord && pRecord->root.type == EditorLib::kXtLevelRecordType)
				pElement = pRecord;
			else
				return EditorLib::E_Status_Failure;
		}
	}

	CachePhysicalObjects();

	return Verify(pElement);
}

bool C_VerifyPhysicalObjectsCmd::IsUndoable() const
{
	return true;
}

void C_VerifyPhysicalObjectsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_VerifyPhysicalObjectsCmd::Create(U32 type)
{
	return new C_VerifyPhysicalObjectsCmd(type);
}

EditorLib::C_Syntax* C_VerifyPhysicalObjectsCmd::CreateSyntax()
{
	EditorLib::C_Syntax* pSyntax = new EditorLib::C_Syntax();
	{
		{
			EditorLib::C_Argument arg(
				'x',
				"xtid",
				"XtId of pack/record to parse. Optional.",
				EditorLib::C_Argument::E_Required_False);
			EditorLib::VarTypeArray& dataTypes = arg.GetDataTypes();
			dataTypes.PushBack(Util::E_VarType_String);
			pSyntax->AddArg(arg);
		}
	}
	return pSyntax;
}

const char* C_VerifyPhysicalObjectsCmd::GetName()
{
	static char* p = "verifyPhysicalObjects";
	return p;
}

EditorLib::C_Status C_VerifyPhysicalObjectsCmd::Verify(Xt::DomElement* pElement)
{
	EditorLib::C_Status status = EditorLib::E_Status_Success;

	if (pElement->kind == Xt::kDomDb)
	{
		Xt::DomDb* pDb = Xt::AsDb(pElement);

		for (XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&pDb->packs); it; ++it)
		{
			if (!Verify(it))
				status = EditorLib::E_Status_Failure;
		}
	} else if (pElement->kind == Xt::kDomPack)
	{
		Xt::DomPack* pPack = Xt::AsPack(pElement);

		for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&pPack->records); it; ++it)
		{
			if (!Verify(it))
				status = EditorLib::E_Status_Failure;
		}
	} else if (pElement->kind == Xt::kDomRecord)
	{
		status = VerifyRecord(Xt::AsRecord(pElement));
	}
	
	return status;
}

EditorLib::C_Status C_VerifyPhysicalObjectsCmd::VerifyRecord(Xt::DomRecord* pRecord)
{
	EditorLib::C_VerificationGroup& group = EditorLib::C_Core::Instance().GetVerificationSystem().GetGroup("InvalidDynamicObject");
	group.SetName("Invalid Dynamic Objects");

	EditorLib::C_VerificationRecord& record = group.GetRecord(pRecord->id);
	
	Xt::DomArray* pActorArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pRecord), 
			&pRecord->root.data,
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_ActorArray).dom);

	for (size_t i=0; i<Xt::Count(pActorArray); i++)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pActorArray, i));
		if (!pComponent)
			continue;

		if (pComponent->type == "DynamicObjActor" || pComponent->type == "DynamicObjectActor")
		{
			Xt::DomField* pNameField = Xt::AsField(
				FindBySemantic(
					AsStruct(FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pComponent->type)), 
					&pComponent->data, 
					EditorLib::kXtSemanticAttr,
					EditorLib::kXtSemanticParam,
					EditorLib::kXtSemanticLabel_Name).dom);

			Xt::DomField* definitionField = 0;
			
			definitionField = Xt::AsField(Xt::At(&pComponent->data, "Definition"));
			if (!definitionField || !Xt::ParseId(definitionField->value)) definitionField = Xt::AsField(Xt::At(&pComponent->data, "DynamicObjType"));
			
			if (definitionField)
			{
				XtId physicalId = FindPhysicalObjectDefinition(Xt::ParseId(definitionField->value));

				if (!physicalId)
					continue;

				// Check if there's a match in the db, if there is, flag it up!
				EditorLib::C_VerificationResult* result = new EditorLib::C_VerificationResult(EditorLib::E_VerificationResultLevel_Error, Btl::String::Format("Invalid Dynamic Object"));
				result->AddData(new EditorLib::C_VerificationResultDataActor(pComponent));
				record.AddResult(result);
			}			
		}
	}

	return EditorLib::E_Status_Success;
}

void C_VerifyPhysicalObjectsCmd::CachePhysicalObjects()
{
	Xt::DomDb* pDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	for (XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&pDb->packs); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;

		for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&pPack->records); recordIt; ++recordIt)
		{
			Xt::DomRecord* pRecord = recordIt;

			if (pRecord->root.type == "ElementObjectRecord")
			{
				Btl::String definitionName = pRecord->name.Text();
				definitionName = definitionName.Replace("PhysicalObject", "DynamicObject");
				definitionName = definitionName.Lower();				
				
				m_PhysicalObjectNameMapping.Insert(definitionName, pRecord->id);

				Xt::DomField* pField = Xt::AsField(Xt::At(&pRecord->root.data, "Model"));
				if (pField)
				{
					XtId modelId = Xt::ParseId(pField->value);

					if (m_PhysicalObjectModelMapping.Find(modelId) != m_PhysicalObjectModelMapping.End())
					{
						m_PhysicalObjectModelMapping[modelId] = 0;
					}

					m_PhysicalObjectModelMapping.Insert(modelId, pRecord->id);
				}
			}
		}
	}
}

XtId C_VerifyPhysicalObjectsCmd::FindPhysicalObjectDefinition(XtId dynamicDefinition)
{
	if (m_ObjectMapping.Find(dynamicDefinition) != m_ObjectMapping.End())
		return m_ObjectMapping[dynamicDefinition];

	Xt::DomRecord* definitionRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(dynamicDefinition);

	if (!definitionRecord)
		return 0;

	XtId physicalDefinition = 0;

	Btl::String definitionName = definitionRecord->name.Text();
	definitionName = definitionName.Lower();

	if (m_PhysicalObjectNameMapping.Find(definitionName) != m_PhysicalObjectNameMapping.End())
		physicalDefinition = m_PhysicalObjectNameMapping[definitionName];

	if (!physicalDefinition)
	{
		Xt::DomField* pField = Xt::AsField(Xt::At(&definitionRecord->root.data, "Model"));
		if (pField)
		{
			XtId modelId = Xt::ParseId(pField->value);

			if (modelId && m_PhysicalObjectModelMapping.Find(modelId) != m_PhysicalObjectModelMapping.End())
				physicalDefinition = m_PhysicalObjectModelMapping[modelId];
		}
	}

	m_ObjectMapping.Insert(dynamicDefinition, physicalDefinition);

	return physicalDefinition;
}


}
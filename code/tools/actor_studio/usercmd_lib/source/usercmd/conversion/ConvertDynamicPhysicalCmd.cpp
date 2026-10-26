#include <stdafx.h>

#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Core.h>

#include <UserCmd/Conversion/ConvertDynamicPhysicalCmd.h>

namespace EditorUserCmd
{

C_ConvertDynamicPhysicalCmd::C_ConvertDynamicPhysicalCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_ConvertDynamicPhysicalCmd::~C_ConvertDynamicPhysicalCmd()
{
}

EditorLib::C_Status C_ConvertDynamicPhysicalCmd::Do()
{
	//Undo checkpoint.
	EditorLib::C_CheckpointHelper c(&m_pChange, GetName());

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

	int convertedCount = 0, dynamicObjectCount = 0;
	CachePhysicalGroups();
	CachePhysicalObjects();
	Convert(pElement, dynamicObjectCount, convertedCount);

	LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Successfully converted %d/%d dynamic objects", convertedCount, dynamicObjectCount);
		
	return EditorLib::E_Status_Success;
}

bool C_ConvertDynamicPhysicalCmd::IsUndoable() const
{
	return true;
}

void C_ConvertDynamicPhysicalCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_ConvertDynamicPhysicalCmd::Create(U32 type)
{
	return new C_ConvertDynamicPhysicalCmd(type);
}

EditorLib::C_Syntax* C_ConvertDynamicPhysicalCmd::CreateSyntax()
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

const char* C_ConvertDynamicPhysicalCmd::GetName()
{
	static char* p = "convertDynamicPhysical";
	return p;
}

bool C_ConvertDynamicPhysicalCmd::Convert(Xt::DomElement* pElement, int& dynamicObjectCount, int& convertedCount)
{
	if (pElement->kind == Xt::kDomDb)
	{
		Xt::DomDb* pDb = Xt::AsDb(pElement);

		for (XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&pDb->packs); it; ++it)
		{
			Convert(it, dynamicObjectCount, convertedCount);
		}
	} else if (pElement->kind == Xt::kDomPack)
	{
		Xt::DomPack* pPack = Xt::AsPack(pElement);

		for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&pPack->records); it; ++it)
		{
			Convert(it, dynamicObjectCount, convertedCount);
		}
	} else if (pElement->kind == Xt::kDomRecord)
	{
		Xt::DomRecord* pRecord = Xt::AsRecord(pElement);


		if (pRecord->root.type == EditorLib::kXtLevelRecordType)
		{
			Xt::DomArray* pArray = Xt::AsArray(
				FindBySemantic(
					Xt::TypeOf(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pRecord), 
					&pRecord->root.data, 
					EditorLib::kXtSemanticAttr,
					EditorLib::kXtSemanticParam,
					EditorLib::kXtSemanticLabel_ActorArray).dom);

			for (XtUt::Dictionary<XtId, Xt::DomElement>::Iterator it(&pArray->elements); it; ++it)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(it);
				if (!pComponent)
					continue;

				if (EditorLib::XtNodeHelpers::IsExternalActor(&pComponent->data))
					continue;

				if (pComponent->type == "DynamicObjectActor" || pComponent->type == "DynamicObjActor")
				{
					dynamicObjectCount++;

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

						LarvaType* type = FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, "PhysicalObjectActor");
						LtStruct* typeStruct = AsStruct(type);

						Xt::FieldDom definition;

						Xt::FieldDom object = AcquireBySemantic(typeStruct, &pComponent->data, "SceneView", "dynamicobject", "object");
						if (object)
						{
							definition = AcquireBySemantic(AsStruct(object.field->Type()), Xt::AsBlock(object.dom), "SceneView", "dynamicobject", "definition");
						} else {
							definition = AcquireBySemantic(typeStruct, &pComponent->data, "SceneView", "dynamicobject", "definition");
						}

						if (!definition)
							continue;

						Xt::DomField* pField = Xt::AsField(Xt::At(&pComponent->data, "ReplaceFromGroup"));
						if (pField && Xt::ParseId(pField->value))
						{
							XtId physicalGroupId = FindPhysicalGroupDefinition(Xt::ParseId(pField->value));
							
							if (!physicalGroupId)
								continue;

							Xt::Set(pField, Btl::String::Format("%X", physicalGroupId).Chars());
						}

						Xt::DomField* physicalField = Xt::AsField(definition.dom);
						BIZ_ASSERT(physicalField);

						Xt::SetType(pComponent, "PhysicalObjectActor");
						Xt::Set(physicalField, Btl::String::Format("%X", physicalId).Chars());
						
						convertedCount++;
					} else {
						LOGWARN(EditorLib::C_Core::Instance().GetLogger(), "DynamicObject (%s/%s) has no definition", pRecord->name.Text(), pNameField ? pNameField->value.Text() : "");
					}
				}
			}
		}		
	}

	return true;
}

bool C_ConvertDynamicPhysicalCmd::IsInteresting(EditorLib::I_DgNode* pNode)
{
	if (pNode && !pNode->IsReadOnly() && pNode->IsA(EditorLib::E_ObjType_SceneNode))
		return true;

	return false;
}

void C_ConvertDynamicPhysicalCmd::CachePhysicalGroups()
{
	Xt::DomDb* pDb = EditorLib::C_Core::Instance().GetXtManager().GetDb();

	for (XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&pDb->packs); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;

		for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator recordIt(&pPack->records); recordIt; ++recordIt)
		{
			Xt::DomRecord* pRecord = recordIt;

			if (pRecord->root.type == "PhysicalObjDefGroup")
			{
				Btl::String definitionName = pRecord->name.Text();
				definitionName = definitionName.Replace("PhysicalObject", "DynamicObject");
				definitionName = definitionName.Lower();
				
				m_PhysicalGroupNameMapping.Insert(definitionName, pRecord->id);
			}
		}
	}
}

void C_ConvertDynamicPhysicalCmd::CachePhysicalObjects()
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
						LOGWARN(EditorLib::C_Core::Instance().GetLogger(), "Model for object (%s) used in more than one location", definitionName.Chars());
					}

					m_PhysicalObjectModelMapping.Insert(modelId, pRecord->id);
				}
			}
		}
	}
}

XtId C_ConvertDynamicPhysicalCmd::FindPhysicalGroupDefinition(XtId dynamicDefinition)
{
	if (m_ObjectMapping.Find(dynamicDefinition) != m_ObjectMapping.End())
		return m_ObjectMapping[dynamicDefinition];

	Xt::DomRecord* definitionRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(dynamicDefinition);

	if (!definitionRecord)
		return 0;

	XtId physicalDefinition = 0;

	Btl::String definitionName = definitionRecord->name.Text();
	definitionName = definitionName.Lower();

	if (m_PhysicalGroupNameMapping.Find(definitionName) != m_PhysicalGroupNameMapping.End())
		physicalDefinition = m_PhysicalGroupNameMapping[definitionName];

	if (physicalDefinition)
	{
		Xt::DomRecord* physicalDefinitionRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(physicalDefinition);
		LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Mapping %s to physical group %s", definitionRecord->name.Text(), physicalDefinitionRecord ? physicalDefinitionRecord->name.Text() : "Unknown");
	}
	else
		LOGWARN(EditorLib::C_Core::Instance().GetLogger(), "Couldn't find mapping for %s", definitionRecord->name.Text());

	m_ObjectMapping.Insert(dynamicDefinition, physicalDefinition);

	return physicalDefinition;
}

XtId C_ConvertDynamicPhysicalCmd::FindPhysicalObjectDefinition(XtId dynamicDefinition)
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

	if (physicalDefinition)
	{
		Xt::DomRecord* physicalDefinitionRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(physicalDefinition);
		LOGINFO(EditorLib::C_Core::Instance().GetLogger(), "Mapping %s to physical object %s", definitionRecord->name.Text(), physicalDefinitionRecord ? physicalDefinitionRecord->name.Text() : "Unknown");
	}
	else
		LOGWARN(EditorLib::C_Core::Instance().GetLogger(), "Couldn't find mapping for %s", definitionRecord->name.Text());

	m_ObjectMapping.Insert(dynamicDefinition, physicalDefinition);

	return physicalDefinition;
}

}
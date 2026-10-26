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
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Core.h>

#include <UserCmd/Verification/Common/VerifyDynamicPositions.h>

namespace EditorUserCmd
{

C_VerifyDynamicPositionsCmd::C_VerifyDynamicPositionsCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_VerifyDynamicPositionsCmd::~C_VerifyDynamicPositionsCmd()
{
}

EditorLib::C_Status C_VerifyDynamicPositionsCmd::Do()
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

	return Verify(pElement);
}

bool C_VerifyDynamicPositionsCmd::IsUndoable() const
{
	return true;
}

void C_VerifyDynamicPositionsCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_VerifyDynamicPositionsCmd::Create(U32 type)
{
	return new C_VerifyDynamicPositionsCmd(type);
}

EditorLib::C_Syntax* C_VerifyDynamicPositionsCmd::CreateSyntax()
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

const char* C_VerifyDynamicPositionsCmd::GetName()
{
	static char* p = "verifyDynamicPositions";
	return p;
}

EditorLib::C_Status C_VerifyDynamicPositionsCmd::Verify(Xt::DomElement* pElement)
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

EditorLib::C_Status C_VerifyDynamicPositionsCmd::VerifyRecord(Xt::DomRecord* pRecord)
{
	EditorLib::C_VerificationGroup& group = EditorLib::C_Core::Instance().GetVerificationSystem().GetGroup("DynamicPositions");
	group.SetName("Dynamic Position Collisions");

	EditorLib::C_VerificationRecord& record = group.GetRecord(pRecord->id);
	
	Xt::DomArray* pActorArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pRecord), 
			&pRecord->root.data,
			EditorLib::kXtSemanticAttr,
			EditorLib::kXtSemanticParam,
			EditorLib::kXtSemanticLabel_ActorArray).dom);

	Btl::Vector<Btl::Pair<Xt::DomComponent*, EditorLib::Vector3> > dynamicActors;

	for (size_t i=0; i<Xt::Count(pActorArray); i++)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pActorArray, i));
		if (!pComponent)
			continue;

		if (pComponent->type == "DynamicObjActor" || pComponent->type == "DynamicObjectActor" || pComponent->type == "PhysicalObjectActor")
		{
			EditorLib::Matrix44 matrix;
			EditorLib::Vector3 position;

			if (EditorLib::XtHelpers::GetMatrix(matrix, pComponent))
			{
				position = matrix[3];
			}

			dynamicActors.PushBack(Btl::MakePair(pComponent, position));
		}
	}

	for (int i=0; i<dynamicActors.Size(); i++)
	{
		for (int j=i+1; j<dynamicActors.Size(); j++)
		{
			//if (Maths::Equal(dynamicActors[i].second, dynamicActors[j].second, Maths::Vector3(0.1f, 0.1f, 0.1f)))
			if (dynamicActors[i].second == dynamicActors[j].second)
			{
				EditorLib::C_VerificationResult* result = new EditorLib::C_VerificationResult(EditorLib::E_VerificationResultLevel_Warning, Btl::String::Format("Dynamic Collision at (%.2f, %.2f, %.2f)", dynamicActors[i].second.X(), dynamicActors[i].second.Y(), dynamicActors[i].second.Z()));//, field, dom)
				result->AddData(new EditorLib::C_VerificationResultDataActor(dynamicActors[i].first));
				result->AddData(new EditorLib::C_VerificationResultDataActor(dynamicActors[j].first));
				record.AddResult(result);
			}
		}
	}
	
	return EditorLib::E_Status_Success;
}

}
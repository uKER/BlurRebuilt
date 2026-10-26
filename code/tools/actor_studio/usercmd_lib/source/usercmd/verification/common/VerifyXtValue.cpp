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
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Core.h>

#include <UserCmd/Verification/Common/VerifyXtValue.h>

namespace EditorUserCmd
{

C_VerifyXtValueCmd::C_VerifyXtValueCmd(U32 type) : EditorLib::I_Command(type)
{
}

C_VerifyXtValueCmd::~C_VerifyXtValueCmd()
{
}

EditorLib::C_Status C_VerifyXtValueCmd::Do()
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

bool C_VerifyXtValueCmd::IsUndoable() const
{
	return true;
}

void C_VerifyXtValueCmd::GetResults(EditorLib::ResultsArray& results) const
{
}

EditorLib::I_Command* C_VerifyXtValueCmd::Create(U32 type)
{
	return new C_VerifyXtValueCmd(type);
}

EditorLib::C_Syntax* C_VerifyXtValueCmd::CreateSyntax()
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

const char* C_VerifyXtValueCmd::GetName()
{
	static char* p = "verifyNull";
	return p;
}

EditorLib::C_Status C_VerifyXtValueCmd::Verify(Xt::DomElement* pElement)
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

C_VerifyXtValueCmd::C_XtValueFieldVisitor::C_XtValueFieldVisitor(EditorLib::C_VerificationRecord& record) :
m_Record(record)
{
}

C_VerifyXtValueCmd::C_XtValueFieldVisitor::~C_XtValueFieldVisitor()
{
}

void C_VerifyXtValueCmd::C_XtValueFieldVisitor::GetRefLocation(Btl::String& location, Xt::DomElement* element)
{
	if (element == 0)
		return;

	if (element->kind == Xt::kDomRecord)
	{
		location = location + " -> " + Xt::AsRecord(element)->name.Text();
	}
	else if (element->kind == Xt::kDomComponent)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(element);
		location = location + " -> " + pComponent->type.Text();

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

	GetRefLocation(location, element->parent);
}

bool C_VerifyXtValueCmd::C_XtValueFieldVisitor::Visit(LfArray* field, Xt::DomArray* dom)
{
	for (size_t i=0; i < Xt::Count(dom); i++)
	{
		// ...
	}

	return true;
}

bool C_VerifyXtValueCmd::C_XtValueFieldVisitor::Visit(LfHandle* field, Xt::DomField* dom)
{
	return true;
}

bool C_VerifyXtValueCmd::C_XtValueFieldVisitor::Visit(LfWeakPtr* field, Xt::DomRef* dom)
{
	return true;
}

C_VerifyXtValueCmd::C_XtValueObjectVisitor::C_XtValueObjectVisitor(C_XtValueFieldVisitor& fieldVisitor):
m_FieldVisitor(fieldVisitor)
{
}

C_VerifyXtValueCmd::C_XtValueObjectVisitor::~C_XtValueObjectVisitor()
{
}

bool C_VerifyXtValueCmd::C_XtValueObjectVisitor::Visit(struct LtStruct* type, Xt::DomBlock* dom)
{
	VisitMembers(type, dom, &m_FieldVisitor, Xt::kIndexAt);

	return true;
}

EditorLib::C_Status C_VerifyXtValueCmd::VerifyRecord(Xt::DomRecord* pRecord)
{
	EditorLib::C_VerificationGroup& group = EditorLib::C_Core::Instance().GetVerificationSystem().GetGroup("XtValue");
	group.SetName("Invalid Xt Values");

	EditorLib::C_VerificationRecord& record = group.GetRecord(pRecord->id);

	C_XtValueFieldVisitor fieldVisitor(record);
	C_XtValueObjectVisitor objectVisitor(fieldVisitor);

	VisitAllObjects(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, Xt::TypeOf(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, pRecord), &pRecord->root.data, &objectVisitor, false);
	
	if (record.GetResults().Size())
		return EditorLib::E_Status_Failure;

	return EditorLib::E_Status_Success;
}

}
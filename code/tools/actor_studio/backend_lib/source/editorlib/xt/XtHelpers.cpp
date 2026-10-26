#include "stdafx.h"
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

namespace EditorLib
{
namespace XtHelpers
{

C_Status BuildLevelList(XtPackRecordsArray& packRecordsArray, Xt::DomDb* pDB)
{
	for(Xt::DomDb::Iterator packIt(pDB); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;

		C_PackRecords* pPackRecords = 0;
		for(Xt::DomPack::Iterator it(pPack); it; ++it)
		{
			Xt::DomRecord* rec = it;

			if (rec->root.type == kXtLevelRecordType)
			{
				//If we've not already done so, add a pack container.
				if (!pPackRecords)
				{
					packRecordsArray.PushBack();
					pPackRecords = &packRecordsArray.Back();
					pPackRecords->packId = pPack->id;
				}
				pPackRecords->levelRecords.PushBack(rec);
			}
		}
	}

	return E_Status_Success;
}

bool IsA(LarvaType* pLarvaType, const char* typeName)
{
	if (pLarvaType->kind != kLtStruct)
		return false;

	if (!strcmp(pLarvaType->name, typeName))
		return true;

	if (LarvaBaseType* base = ((LtStruct*)pLarvaType)->base)
	{
		return IsA(base->type, typeName);
	}
	return false;
}

bool IsChildOfOrEqualTo(const Xt::DomElement* pParent, const Xt::DomElement* pChild)
{
	if (pParent == pChild) 
		return true;

	if (!pParent || !pChild) 
		return false;

	for (; pChild->parent; pChild = pChild->parent)
	{
		if (pChild == pParent)
			return true;
	}

	return false;
}

bool AreRelated(const Xt::DomElement* l, const Xt::DomElement* r)
{
	if (l == r) 
		return true;

	if (!l || !r) 
		return false;

	return IsChildOfOrEqualTo(l, r) || IsChildOfOrEqualTo(r, l);
}


Xt::DomBlock* GetNearestBlock(Xt::DomElement* pData)
{
	if (pData->kind == Xt::kDomPack || pData->kind == Xt::kDomRecord || pData->kind == Xt::kDomDb)
		return 0;

	for (; pData; pData = pData->parent)
	{
		Xt::DomBlock* pBlock = Xt::AsBlock(pData);
		if (pBlock) 
			return pBlock;
	}

	return 0;
}

bool GetMatrix(Matrix44& matrix, Xt::DomComponent* pComponent)
{
	Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(&pComponent->data, "__transform"));
	if (pBlock)
	{
		Xt::DomBlock *pComponentElement = 0;
		Vector3 scale, shear, rotate, translate;

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "scale"));
		XtFieldHelper::GetVector(scale, pComponentElement);

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "shear"));
		XtFieldHelper::GetVector(shear, pComponentElement);

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "rotate"));
		XtFieldHelper::GetVector(rotate, pComponentElement);

		pComponentElement = Xt::AsBlock(Xt::At(pBlock, "translate"));
		XtFieldHelper::GetVector(translate, pComponentElement);

		Vector3 scalePivot(0,0,0);
		Vector3 scalePivotTranslation(0,0,0);
		Vector3 rotatePivot(0,0,0);
		Matrix44 rotateOrientation;
		UtMaths::SetIdentity(&rotateOrientation);
		Vector3 rotatePivotTranslation(0,0,0);

		Matrix44 rotateMatrix;
		Aftk::EulerToMatrix(rotateMatrix, rotate, (Aftk::E_RotationOrder)Aftk::E_RotationOrder_XYZ);

		Aftk::BuildMatrix(
			matrix,
			scalePivot,
			scale,
			shear,
			scalePivotTranslation,
			rotatePivot,
			rotateOrientation,
			rotateMatrix, 
			rotatePivotTranslation,
			translate);

		Xt::DomRef* pRef = Xt::AsRef(Xt::At(&pComponent->data, "__parent"));
		Xt::DomRecord* pRecord = Xt::RecordOf(pRef);
		Xt::DomDb* pDb = Xt::DbOf(pRef);
		if (pRef && pRef->value && pRecord && pDb)
		{
			Xt::DomArray* pActorArray = Xt::AsArray(
				AcquireBySemantic(
					Xt::TypeOf(pDb->schema, pRecord), 
					&pRecord->root.data,
					kXtSemanticAttr,
					kXtSemanticParam,
					kXtSemanticLabel_ActorArray).dom);

			Xt::DomComponent* pParent = FindComponentById(pDb->schema, pRecord, pRef->value).component;

			if (pParent)
			{
				Matrix44 parentMatrix;
				GetMatrix(parentMatrix, pParent);
				matrix = matrix * parentMatrix;
			} else {
				return false;
			}
		
		}

		return true;
	}

	return false;
}


}
}

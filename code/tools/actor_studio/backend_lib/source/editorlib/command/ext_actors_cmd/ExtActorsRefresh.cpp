#include "stdafx.h"

#include <System/Xml/Xml.h>

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtQuery.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Command/Ext_Actors_Cmd/ExtActorsRefresh.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Core.h>


namespace EditorLib
{

void ClearDb()
{
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();
	Btl::Vector<Xt::DomPack*> packsToProcess;
	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator packIt(&(pDb->packs)); packIt; ++packIt)
	{
		Xt::DomPack* pPack = packIt;
		if (!pPack) continue;

		ClearPack(pPack);
	}
}

void ClearPack(Xt::DomPack* pPack)
{
	for(Xt::DomPack::Iterator recordIt(pPack); recordIt; ++recordIt)
	{
		Xt::DomRecord* pRec = recordIt;
		if (!pRec) continue;

		ClearRecord(pRec);
	}
}

void ClearRecord(Xt::DomRecord* pRec)
{
	if (pRec->root.type != kXtLevelRecordType)
		return;

	//Acquire actor array.
	Xt::FieldDom fdArray = AcquireBySemantic(
		Xt::TypeOf(C_Core::Instance().GetXtManager().GetDb()->schema, pRec), 
		&pRec->root.data,
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_ActorArray);
	BIZ_ASSERT(fdArray.field->kind == kLfArray);
	LfArray* fieldArr = (LfArray*)fdArray.field;
	Xt::DomArray* pLevelActorArray = Xt::AsArray(fdArray.dom);

	//Remove any existing externally created actors.
	for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
	{
		Xt::DomComponent* pComponent = AsComponent(it);
		assert(pComponent);

		if (!XtNodeHelpers::IsExternalActor(&pComponent->data))
			continue;
		{
			Xt::Remove(pLevelActorArray, it.Index());

			//Bail if we've erased the last element.
			if (it.Index() == Xt::Count(pLevelActorArray))
				break;

			//Adjust iterator to compensate for deletion.
			it.index--;
		}
	}
}

C_Status CreateActor(
	LfArray* pLevelActorArrayField,
	Xt::DomArray* pLevelActorArray,
	ActorMap& actorsToMerge,
	const Btl::String& aftkNodeName,
	const Btl::String& aftkNodePath,
	const Btl::String& aftkRefFilePath,
	const Matrix44& aftkNodeWorldMatrix,
	const Btl::String& xml)
{
	//Create a single actor from incoming xml description of the actor's xt dom.
	Sys::C_XmlDom xmlDom;
	//Create a root node
	xmlDom.SetRootNodePtr(new Sys::C_XmlDomBaseNode());

	if (Sys::XmlParse(xmlDom, xml.Chars(), xml.Length()) != Sys::E_XmlResult_Success 
		||
		!xmlDom.GetRootNodePtr())
	{
		LOGERROR(
			C_Core::Instance().GetLogger(), 
			"Failed to parse embedded xml for %s", aftkNodeName.Chars());
		return E_Status_Failure;
	}

	//Find xml element representing the root block for our actor.
	Sys::C_XmlDomBaseNode* pRootXmlBlock = xmlDom.GetRootNodePtr();

	//Perform all operations upon xt data directly. Back-end lib will automagically sync up to this.
	bool created;

	Xt::DomComponent* pRootComponent;
	{
		pRootComponent = AcquireDomActor(actorsToMerge, aftkNodeName, aftkNodePath, aftkRefFilePath, created);
	}
	BIZ_ASSERT(pRootComponent);

	//Currently, xt auto population limited to processing only fields, and futhermore only fields
	//that reside at the root level of the block. Could go on to add full supports for arrays and nested
	//blocks etc if required.
	for (S32 i = 0; i < pRootXmlBlock->GetChildren().Size(); ++i)
	{
		if (pRootXmlBlock->GetChildren()[i]->GetType() != Sys::E_XmlDomNodeType_Element)
			continue;

		Sys::C_XmlDomElement* pElement = (Sys::C_XmlDomElement*)pRootXmlBlock->GetChildren()[i];
		const char* pElementType = pElement->GetName().Chars();

		if (!strcmp(pElementType, "field"))
		{
			if (pElement->GetAttributes().Elements() != 1)
			{
				LOGERROR(
					C_Core::Instance().GetLogger(), 
					"Invalid number of attributes for field of %s", aftkNodeName.Chars());
				continue;
			}

			const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[0];
			const char* pName = pAttr->name.Chars();
			const char* pValue = pAttr->value.Chars();
			if (!stricmp(pName, "__type"))
			{
				Xt::SetType(pRootComponent, pValue);
			}
			else
			{
				Xt::DomField* pField = Xt::AcquireField(&pRootComponent->data, pName);
				assert(pField);
				Xt::Set(pField, pValue);
			}
		}
	}

	//Examine __type field to find type.
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	LarvaType* pLarvaType = FindTypeByName(pSchema, pRootComponent->type);

	//Attempt to set data reliant upon schema.
	if (pLarvaType)
	{
		//Store the world matrix (for game use only)
		//There's no ideal way of doing this during in place construction at the moment.
		//Fix this.
		Matrix44 m;

		Xt::DomBlock* pMatrixBlock = Xt::AsBlock(AcquireBySemantic(
			AsStruct(pLarvaType),
			&pRootComponent->data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_WorldMatrix).dom);
		if (pMatrixBlock)
		{
			Matrix44 removeNegativeScale;
			UtMaths::SetIdentity(&removeNegativeScale);
			removeNegativeScale[2][2] = -1.f;
			m = removeNegativeScale*aftkNodeWorldMatrix;
			XtFieldHelper::SetMatrix(pMatrixBlock, m);
		}
	
		{
			Maths::HVec3 scale;
			Maths::HVec3 shear;
			Maths::HVec3 rotate;
			Maths::HVec3 translate;

			Maths::HMat43 temp;
			Aftk::TemplateToNightmareMatrix43(temp, m);

			//GetSRT grabs rotation values in roll, pitch, heading order. This is no good to us as legacy
			//animation curves are in XYZ order, and set matrix will be used on the nodes driven by these curves.
			//Therefore we must grab the rotation values in XYZ order from the given matrix.
			Aftk::GetSShRT(
				scale,
				shear,
				rotate,
				translate, 
				temp, 
				(Aftk::E_RotationOrder)E_RotationOrder_XYZ);

			Xt::DomBlock *pTransform = 
				Xt::AcquireBlock(&pRootComponent->data, "__transform");
			{
				Xt::DomBlock *pData = 0;
				pData = Xt::AcquireBlock(pTransform, "scale");
				XtFieldHelper::SetVector(pData, scale);

				pData = Xt::AcquireBlock(pTransform, "shear");
				XtFieldHelper::SetVector(pData, shear);

				pData = Xt::AcquireBlock(pTransform, "rotate");
				XtFieldHelper::SetVector(pData, rotate);

				pData = Xt::AcquireBlock(pTransform, "translate");
				XtFieldHelper::SetVector(pData, translate);
			}
		}
	}
	else
	{
		LOGERROR(
			C_Core::Instance().GetLogger(), 
			"Aftk actor node %s has no an invalid type \"%s\".", 
			aftkNodeName.Chars(),
			pRootComponent->type.Text());
	}

	//Append new actor into level actor array.
	if (created)
		Xt::Append(pLevelActorArray, pRootComponent);

	return E_Status_Success;
}

void BuildActorMap(
	ActorMap& actorsToMerge, 
	Xt::DomArray* pLevelActorArray)
{
	//Remove any existing externally created actors.
	for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
	{
		Xt::DomComponent* pComponent = AsComponent(it);

		if (!XtNodeHelpers::IsExternalActor(&pComponent->data))
			continue;

		Xt::DomField* pAftkPathField = Xt::AsField(Xt::At(&pComponent->data, "__extAftkPath"));
		if (pAftkPathField)
		{
			ActorMap::InsertionReturnType result = actorsToMerge.Insert(pAftkPathField->value.Text());
			result.iterator->GetValue().pRoot = pComponent;
		}
	}
}

Xt::DomComponent* AcquireDomActor(
	ActorMap& actorsToMerge, 
	const Btl::String& aftkNodeName,
	const Btl::String& aftkNodePath,
	const Btl::String& aftkRefFilePath,
	bool& created)
{
	ActorMap::Iterator it = actorsToMerge.Find(aftkNodePath);
	//Creating new actor.
	if (it == actorsToMerge.End())
	{
		Xt::DomComponent* pRootComponent = new Xt::DomComponent;

		//Assign xt id
		C_Status status = XtNodeHelpers::InitialiseXtId(pRootComponent);
		assert(status);

		//Store aftk path so that we can merge nodes together to minimise conflicts.
		{
			Xt::DomField* pField = Xt::AcquireField(&pRootComponent->data, "__extAftkPath");
			BIZ_ASSERT(pField);
			Xt::Set(pField, aftkNodePath.Chars());

			pField = Xt::AcquireField(&pRootComponent->data, "ExtAftkRefFilePath");
			BIZ_ASSERT(pField);
			Xt::Set(pField, Misc::FormatWindowsFileString(aftkRefFilePath.Lower()).Chars());
		}

		//Mark as being an external actor.
		{
			XtNodeHelpers::SetIsExternalActor(&pRootComponent->data, true);
		}

		//Set name, including prefix.
		{
			Xt::DomField* pField = Xt::AcquireField(&pRootComponent->data, "Name");
			BIZ_ASSERT(pField);
			Btl::String newName("Ext_");
			newName = newName + aftkNodeName;
			Xt::Set(pField, newName.Chars());
		}

		created = true;
		return pRootComponent;
	}

	created = false;
	Xt::DomComponent* pDomComponent = it->GetValue().pRoot;
	//Nolonger available for merging.
	actorsToMerge.Erase(it);
	return pDomComponent;
}

void RemoveUnmergedActors(
	Xt::DomArray* pLevelActorArray, 
	const Btl::String& originalRefFilePath,
	const ActorMap& actorsToMerge)
{
	//Remove any external actors from our list that are remaining in our toMerge map.
	//These are external actors that no longer exist.
	Btl::String formattedRefPath = Misc::FormatWindowsFileString(originalRefFilePath).Lower();
	for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
	{
		Xt::DomComponent* pComponent = AsComponent(it);
		assert(pComponent);

		if (!XtNodeHelpers::IsExternalActor(&pComponent->data))
			continue;

		Xt::DomField* pExtAftkField = Xt::AsField(Xt::At(&pComponent->data, "__extAftkPath"));
		if (!pExtAftkField)
			continue;

		Xt::DomField* pXtAftkRefFilePath = Xt::AsField(Xt::At(&pComponent->data, "ExtAftkRefFilePath"));
		if (!pXtAftkRefFilePath)
			continue;
		
		//Clear out actors with no path.
		if (pXtAftkRefFilePath->value != "")
		{
			if (Misc::FormatWindowsFileString(pXtAftkRefFilePath->value.Text()).Lower() != 
				formattedRefPath)
				//Not from current ref, ignore.
				continue;
		}

		Btl::String path = pExtAftkField->value.Text();
		ActorMap::ConstIterator mapIt = actorsToMerge.Find(path);
		if (mapIt == actorsToMerge.End())
			continue;

		Xt::Remove(pLevelActorArray, it.Index());

		//Bail if we've erased the last element.
		if (it.Index() == Xt::Count(pLevelActorArray))
			break;

		//Adjust iterator to compensate for deletion.
		it.index--;
	}
}

S32 CreateActors(
	Xt::DomRecord* pRec,
	const Btl::String& originalRefFilePath,
	const Btl::Vector<C_AftkActorPlaceholder>& aftkActorPlaceholderArray)
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	//Acquire actor array.
	Xt::FieldDom fdArray = AcquireBySemantic(
		Xt::TypeOf(pSchema, pRec), 
		&pRec->root.data,
		kXtSemanticAttr,
		kXtSemanticParam,
		kXtSemanticLabel_ActorArray);
	BIZ_ASSERT(fdArray.field->kind == kLfArray);
	LfArray* pLevelActorArrayField = (LfArray*)fdArray.field;
	Xt::DomArray* pLevelActorArray = Xt::AsArray(fdArray.dom);
	if (!pLevelActorArray)
		return -1;

	//Note that we compare against any filtering information present.
	ActorMap actorsToMerge;
	BuildActorMap(actorsToMerge, pLevelActorArray);

	U32 actorsCreated = 0;
	for (S32 i = 0; i < aftkActorPlaceholderArray.Size(); ++i)
	{
		const C_AftkActorPlaceholder& aftkPlaceholder = aftkActorPlaceholderArray[i];

		const Util::C_KeyValue* pAsDynamic(0);
		const Util::C_KeyValue* pAsActor(0);
		if (!AftkRefHelpers::GetAftkActorPlaceholderProperties(&pAsDynamic, &pAsActor,
			aftkPlaceholder.attrs, pRec->name.Text()))
			continue;

		if (pAsDynamic && pAsActor)
		{
			bool isDynamic;
			pAsDynamic->GetValue(isDynamic);
			if (isDynamic)
			{
				Btl::String xml;
				pAsActor->GetValue(xml);
				
				C_Status status = CreateActor(
					pLevelActorArrayField,
					pLevelActorArray,
					actorsToMerge,
					aftkPlaceholder.name,
					aftkPlaceholder.path,
					aftkPlaceholder.refFilePath,
					aftkPlaceholder.worldMatrix,
					xml);
				if (!status) 
					return -1;
				actorsCreated++;
			}
		}
	}

	RemoveUnmergedActors(pLevelActorArray, originalRefFilePath, actorsToMerge);
	return actorsCreated;
}

bool PackContainsActorLists(Xt::DomPack* pPack)
{
	for(Xt::DomPack::Iterator it(pPack); it; ++it)
	{
		Xt::DomRecord* pRec = it;
		if (!pRec) continue;
		//Only interested in levels.
		if (pRec->root.type != kXtLevelRecordType) continue;

		return true;
	}

	return false;
}

void DisplayRefreshSummary(const C_ExtActorsDbStats& dbStats)
{
	//Display summary.
	LOGINFO(C_Core::Instance().GetLogger(), 
		"Total unique ext actor maya scenes : %i.", 
		dbStats.uniqueRefFilePaths.Size());	
	for (UniqueRefSet::ConstIterator it = dbStats.uniqueRefFilePaths.Begin(); it != dbStats.uniqueRefFilePaths.End(); ++it)
	{
		LOGINFO(C_Core::Instance().GetLogger(), (Btl::String("\t") + it->GetKey()).Chars());
	}

	for (S32 i = 0; i < dbStats.packStatsArray.Size(); ++i)
	{
		const C_ExtActorsPackStats& packStats = dbStats.packStatsArray[i];
		S32 totalActorsCreated = 0;
		for (S32 j = 0; j < packStats.recordStatsArray.Size(); j++)
			totalActorsCreated += packStats.recordStatsArray[j].nActorsCreated;

		LOGINFO(C_Core::Instance().GetLogger(), 
			"ExtActors summary for pack %s : %i actors created in total.", 
			packStats.packName.Chars(), totalActorsCreated);
		for (S32 j = 0; j < packStats.recordStatsArray.Size(); j++)
		{
			C_ExtActorsRecordStats recordStats = packStats.recordStatsArray[j];
			if (recordStats.nActorsCreated > 0)
			{
				LOGINFO(C_Core::Instance().GetLogger(), 
					"\t%s : %i external actors created from the following refs:",
					recordStats.recordName.Chars(), recordStats.nActorsCreated);

				for (S32 k = 0; k < recordStats.refArray.Size(); ++k)
				{
					const C_ExtActorsRefStats& refStats = recordStats.refArray[k];
					LOGINFO(C_Core::Instance().GetLogger(), 
						"\t\t%s : %i actors",
						refStats.originalFilePath.Chars(), refStats.nActors);
				}
			}
		}
	}
}

}

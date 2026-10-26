#include "stdafx.h"

#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <Aftk/AftkEnum.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtLarvaHelper.h>
#include <EditorLib/Xt/XtVisualisation.h>

#include <EditorLib/Collision/Collision.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgXtGameDataNode.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/LevelHelpers.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{
namespace LevelHelpers
{
	I_DgNode*	CreateNodeInPlace(
		C_XtLevel* pTargetLevel, E_ObjType nodeType, Xt::DomComponent* pData);

	void		OnChangeXtType(C_XtLevel* pDstLevel, C_NodeId nodeId);

	I_DgNode*	ChangeXtType(C_XtLevel* pDstLevel, I_DgNode* pNode);

	Xt::DomArray*
				AcquireXtArray(XtId targetLevelId, I_DgNode* pNode);
}

C_XtLevel::C_XtLevel(C_LevelPack* pack)
{
	m_packId = kXtInvalidId;
	m_recordId = kXtInvalidId;
}

C_XtLevel::~C_XtLevel()
{
}

E_LevelType C_XtLevel::GetType() const
{
	return E_LevelType_XtLevel;
}

C_Status C_XtLevel::SetXtRecord(XtId recordId)
{
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(recordId);
	if (!pLevelRecord) return E_Status_Failure;

	Xt::DomPack* pPack = Xt::PackOf(const_cast<Xt::DomRecord*>(pLevelRecord));
	if (!pPack)	return E_Status_Failure;

	m_packId = pPack->id;
	m_recordId = recordId;

	return E_Status_Success;
}

C_Status C_XtLevel::LoadData()
{
	C_Status status = XtInitialise();
	if (!status) return status;

	XtBind();

	return E_Status_Success;
}

C_Status C_XtLevel::UnloadData()
{
	//Unbind ourselves from our Xt data.
	XtUnbind();

	{
		//Stop monitoring.
		Xt::Monitor(&m_xtActorArrayMonitor, 0);
		Xt::Monitor(&m_xtNodeArrayMonitor, 0);
		Xt::Monitor(&m_xtGameDataArrayMonitor, 0);
		Xt::Monitor(&m_xtAftkRefMonitor, 0);
	}

	// Unbind xt nodes
	for (DgIdPtrMap::Iterator it = m_dgNodes.Begin(); it != m_dgNodes.End(); ++it)
	{
		I_DgNode* pNode = it->GetValue();

		C_XtObject* pXtObject = pNode->AsXtObject();
		if (!pXtObject) continue;

		//Right, we've got an xt object. Make sure we unbind it prior to deletion otherwise
		//we're going to be erasing xt data!
		pXtObject->XtUnbind();
	}

	return E_Status_Success;
}

const C_AftkLevelRef* C_XtLevel::AddAftkRef(const Btl::String& _filePath)
{
	C_CheckpointDisabler cpDisable;
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());
	const C_AftkLevelRef* p = GetAftkRef(filePath);
	if (p) return p;

	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	if (!pLevelRecord) return 0;

	Xt::DomBlock* pBlock = AsBlock(m_xtAftkRefMonitor);
	if (!pBlock) return 0;

	Xt::DomArray* pXtAftkRefArray = Xt::AcquireArray(pBlock, "__aftkRefArray");

	Xt::DomBlock* pXtNewElement = new Xt::DomBlock;
	Xt::Append(pXtAftkRefArray, pXtNewElement);
	Xt::DomBlock* pXtAftkRef = Xt::AcquireBlock(pXtNewElement, "__aftkRef");
	Xt::Set(pXtAftkRef, "__filePath", XtString(filePath.Chars()));

	//Skip scenes which don't contain ext actor ref data.
	Xt::DomField* pField = Xt::AcquireField(pXtAftkRef, "__isExtActorRef");
	Xt::Set(pField, "0");

	C_AftkLevelRef* pAftkLevelRef = new C_AftkLevelRef();
	
	if (XtInitialiseAftkRef(*pAftkLevelRef, pXtNewElement))
	{
		m_aftkRefs.PushBack(pAftkLevelRef);
		C_Core::Instance().GetGlobalSignals().levelAftkRefAdded(filePath);
		return pAftkLevelRef;
	}

	SAFE_DELETE_PTR(pAftkLevelRef);

	return 0;
}

C_Status C_XtLevel::RemoveAftkRef(const Btl::String& _filePath)
{
	C_CheckpointDisabler cpDisable;
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());

	AftkLevelRefList::Iterator toEraseIt = m_aftkRefs.End();
	for (AftkLevelRefList::Iterator it = m_aftkRefs.Begin(); 
		it != m_aftkRefs.End(); 
		++it)
	{
		const C_AftkRef* pScene = (*it)->pAftkRef;
		if (pScene->GetOriginalFilePath() == filePath)
		{
			toEraseIt = it;
			break;
		}
	}

	if (toEraseIt == m_aftkRefs.End())
		return E_Status_Failure;

	//We've found the item in m_aftkRefs to erase, now find Xt dom version.
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	if (!pLevelRecord) return E_Status_Failure;

	Xt::DomBlock* pAftkRefs = Xt::AsBlock(Xt::At(&pLevelRecord->root.data, "__aftkRefs"));
	if (!pAftkRefs) return E_Status_Failure;

	Xt::DomArray* pXtAftkRefArray = Xt::AsArray(Xt::At(pAftkRefs, "__aftkRefArray"));
	if (!pXtAftkRefArray) return E_Status_Failure;

	S32 xtIndexToErase = -1;
	S32 i = 0;
	for(Xt::DomArray::IndexIterator it(pXtAftkRefArray); it; ++it, ++i)
	{
		Xt::DomBlock* pXtItemBlock = Xt::AsBlock(it);
		if (!pXtItemBlock) continue;

		Xt::DomBlock* pXtAftkRef = Xt::AsBlock(Xt::At(pXtItemBlock, "__aftkRef"));
		if (!pXtAftkRef) return E_Status_Failure;

		Btl::String xtFilePath = (const char*)Xt::AsText(Xt::At(pXtAftkRef, "__filePath"));
		xtFilePath = Misc::FormatUnixFileString(xtFilePath.Lower());
		if (filePath == xtFilePath)
		{
			xtIndexToErase = i;
			break;
		}
	}

	if (xtIndexToErase == -1)
		return E_Status_Failure;

	//If we've gotten this far then we know what elements we have to erase both locally and in xt.

	//Destroy ref.
	C_Status status = EditorLib::C_Core::Instance().GetAftkRefManager().DestroyRef(filePath);
	if (!status) return status;

	//Erase local data.
	SAFE_DELETE_PTR(*toEraseIt);
	m_aftkRefs.Erase(toEraseIt);
	Xt::Remove(pXtAftkRefArray, xtIndexToErase);

	return E_Status_Success;
}

//TODO: Better aftk ref / xt solution here.
void C_XtLevel::UpdateAftkRef(const Btl::String& filePath, bool isExtActor)
{
	C_CheckpointDisabler cpDisable;
	Xt::DomBlock* pAftkRefBlock = GetAftkRefData(filePath);
	if (!pAftkRefBlock)
		return;

	//Nasty, will get refactored soon anyhow.
	C_AftkLevelRef* pAftkLevelRef = 
		const_cast<C_AftkLevelRef*>(GetAftkRef(filePath));
	if (!pAftkLevelRef)
		return;

	//This is NOT the way to do things. The refs should be xtobject bound and
	//manage themselves.
	Xt::DomField* pField = Xt::AcquireField(pAftkRefBlock, "__isExtActorRef");
	Xt::Set(pField, isExtActor ? "1" : "0");
	pAftkLevelRef->isExtActor = isExtActor;
}


XtId C_XtLevel::GetXtPackId() const
{
	return m_packId;
}

XtId C_XtLevel::GetXtRecordId() const
{
	return m_recordId;
}

void C_XtLevel::SetName(const Btl::String& name)
{
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	if (!pLevelRecord) return;

	if (name.Length() == 0)
		return;

	pLevelRecord->name = name.Chars();

	C_LevelBase::SetName(name);
}

void C_XtLevel::BuildActorNodeArray(Btl::Vector<const I_DgNode*>& nodes) const
{
	Xt::DomRecord* pLevelRecord = 
		EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	BIZ_ASSERT(pLevelRecord);

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	//Accumulate node xt ids.
	Btl::Vector<XtId> xtIds;
	{
		//Actors.
		Xt::DomArray* pLevelActorArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data, 
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_ActorArray).dom);
		if (pLevelActorArray)
			for(Xt::DomArray::IndexIterator it(pLevelActorArray); it; ++it)
			{
				Xt::DomComponent* pComponent = AsComponent(it);
				if (pComponent)	xtIds.PushBack(pComponent->id);
			}

		//Scene nodes.
		Xt::DomArray* pLevelNodeArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data, 
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_NodeArray).dom);
		if (pLevelNodeArray)
			for(Xt::DomArray::IndexIterator it(pLevelNodeArray); it; ++it)
			{
				Xt::DomComponent* pComponent = AsComponent(it);
				if (pComponent)	xtIds.PushBack(pComponent->id);
			}

		//Data nodes.
		Xt::DomArray* pDataActorArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data, 
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_DataArray).dom);
		if (pDataActorArray)
			for(Xt::DomArray::IndexIterator it(pDataActorArray); it; ++it)
			{
				Xt::DomComponent* pComponent = AsComponent(it);
				if (pComponent)	xtIds.PushBack(pComponent->id);
			}
	}

	//Build node ptr list.
	for (S32 i = 0; i < xtIds.Size(); ++i)
	{
		C_NodeId nodeId(xtIds[i], kNoChildId);
		DgIdPtrMap::Iterator dgIt = m_dgNodes.Find(nodeId);
		if (dgIt == m_dgNodes.End())
			continue;

		if (!dgIt->GetValue())
			continue;
		nodes.PushBack(dgIt->GetValue());
	}
}

void C_XtLevel::OnDetachDgNode(I_DgNode* pNode)
{
	C_XtObject::C_Unbinder unbinder(this);

	//Bail if we're not bound to any Xt data, otherwise delete this node from Xt.
	if (!pNode || m_recordId == kXtInvalidId)
		return;

	//Bail if we're not an xt or xt bound object.
	if (pNode->AsXtObject() &&pNode->AsXtObject()->IsBound())
	{
		Xt::DomArray* pArray = LevelHelpers::AcquireXtArray(GetXtRecordId(), pNode);
		//If we're stored in on of our arrays...
		if (pArray)
		{
			XtId idToRemove = pNode->GetId().GetXtId();
			//Erase item from data list.
			S32 xtIndexToErase = -1;
			S32 i = 0;
			for(Xt::DomArray::IndexIterator it(pArray); it; ++it, ++i)
			{
				Xt::DomComponent* pComponent = AsComponent(it);
				XtId id = pComponent->id;

				if (id == idToRemove)
				{
					xtIndexToErase = i;
					break;
				}
			}

			if (xtIndexToErase != -1)
				Xt::Remove(pArray, xtIndexToErase);
			else
				BIZ_ASSERT(!"Failed to remove  node from xt array.");
		}
	}

	C_LevelBase::OnDetachDgNode(pNode);
}

C_Status C_XtLevel::XtInitialise()
{
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	if (!pLevelRecord) return E_Status_Failure;

	LtStruct* pLevelSchema = 
		XtLarvaHelpers::FindLarvaType(C_Core::Instance().GetXtManager().GetDb()->schema, kXtLevelRecordType, E_XtSchemaType_Record);
	if (!pLevelSchema)
		return E_Status_Failure;

	C_Status status = XtInitialise(this, pLevelSchema, &pLevelRecord->root);
	if (!status) return status;

	return E_Status_Success;
}

C_Status C_XtLevel::XtInitialise(
	C_XtLevel* pLevel, 
	LarvaType* pLarvaType, 
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(this, pLarvaType, pData);
	if (!status) return status;

	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	if (!pLevelRecord) return E_Status_Failure;

	LarvaSchema* pLarvaSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	//Actors
	Xt::DomArray* pLevelActorArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pLarvaSchema, pLevelRecord), 
			&pLevelRecord->root.data, 
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_ActorArray).dom);
	if (pLevelActorArray)
	{
		Xt::Monitor(&m_xtActorArrayMonitor, pLevelActorArray);
		m_xtActorArrayMonitor.SetCallback(
			&C_XtObject::StaticOnXtDomExternalMonitorEvent, (C_XtObject*)this);
	}

	//Scene nodes
	Xt::DomArray* pLevelNodeArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pLarvaSchema, pLevelRecord), 
			&pLevelRecord->root.data, 
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_NodeArray).dom);
	if (pLevelNodeArray)
	{
		Xt::Monitor(&m_xtNodeArrayMonitor, pLevelNodeArray);
		m_xtNodeArrayMonitor.SetCallback(
			&C_XtObject::StaticOnXtDomExternalMonitorEvent, (C_XtObject*)this);
	}

	//Data nodes
	{
		Xt::DomArray *pLevelDataArray = Xt::AsArray(
		AcquireBySemantic(
			Xt::TypeOf(pLarvaSchema, pLevelRecord), 
			&pLevelRecord->root.data, 
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_DataArray).dom);
		if (pLevelDataArray)
		{
			Xt::Monitor(&m_xtGameDataArrayMonitor, pLevelDataArray);
			m_xtGameDataArrayMonitor.SetCallback(
				&C_XtObject::StaticOnXtDomExternalMonitorEvent, (C_XtObject*)this);
		}
	}

	//Aftk ref proporties.
	{
		Xt::DomBlock* pBlock = Xt::AcquireBlock(
			&pLevelRecord->root.data, "__aftkRefs");
		BIZ_ASSERT(pBlock);
		Xt::Monitor(&m_xtAftkRefMonitor, pBlock);
	}

	m_Name = (const char*)pLevelRecord->name;

	//Read in dependency graph.
	XtInitialiseDgNodes(pLevelRecord);

	//Read in reference list.
	XtInitialiseAftkRefs(pLevelRecord);

	//Force an initial update of all nodes. This may itself create new nodes (e.g dynamic props)
	Btl::Vector<Sys::WeakPtr<I_DgNode> > initialUpdateList;
	initialUpdateList.Reserve(m_dgNodes.Size());
	for (DgIdPtrMap::Iterator it = m_dgNodes.Begin(); it != m_dgNodes.End(); ++it)
	{
		if (!it->GetValue())
			continue;
		initialUpdateList.PushBack(it->GetValue());
	}
	for (S32 i = 0; i < initialUpdateList.Size(); ++i)
	{
//		LOGDEBUG(C_Core::Instance().GetLogger(), "Updating %i/%i", i, initialUpdateList.Size());
		if (!initialUpdateList[i])
			continue;
		initialUpdateList[i]->Update();
	}

	return E_Status_Success;
}

void C_XtLevel::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	if (pEvent->code == Xt::kDomArrayChanged)
	{
		Xt::DomArrayChanged* pArrayChangedEvent = (Xt::DomArrayChanged*)pEvent;
		if (pArrayChangedEvent->id == 0)
			return;

		E_ObjType concreteBaseType = E_ObjType_Unknown;
		Xt::DomArray* pArray = 0;
		if (pMonitor == &m_xtActorArrayMonitor &&
			pEvent->from == m_xtActorArrayMonitor.element)
		{
			concreteBaseType = E_ObjType_SceneXtActorNode;
			pArray = AsArray(pArrayChangedEvent->from);
		}
		else if (pMonitor == &m_xtNodeArrayMonitor &&
				 pEvent->from == m_xtNodeArrayMonitor.element)
		{
			concreteBaseType = E_ObjType_SceneXtNode;
			pArray = AsArray(pArrayChangedEvent->from);
		}
		else if (pMonitor == &m_xtGameDataArrayMonitor &&
				 pEvent->from == m_xtGameDataArrayMonitor.element)
		{
			concreteBaseType = E_ObjType_SceneXtActorNode;
			pArray = AsArray(pArrayChangedEvent->from);
		}

		if (concreteBaseType != E_ObjType_Unknown)
		{
			S32 index = Xt::IndexOf(AsArray(pArrayChangedEvent->from), pArrayChangedEvent->id);
			if (index == -1)
				OnXtArrayRemove(pArray, concreteBaseType);
			else
				OnXtArrayAdd(pArray, concreteBaseType, index);
		}
	}

	if (pEvent->code == Xt::kDomFieldChanged ||
		pEvent->code == Xt::kDomRefChanged ||
		pEvent->code == Xt::kDomBlockChanged ||
		pEvent->code == Xt::kDomArrayChanged ||
		pEvent->code == Xt::kDomRecordChanged)
	{
		C_Core::Instance().DirtyViews();
	}
}

void C_XtLevel::OnXtArrayAdd(Xt::DomArray* pArray, E_ObjType concreteBaseType, S32 index)
{
	BIZ_ASSERT(index != -1);
	C_XtObject::C_Unbinder unbinder(this);

	BIZ_ASSERT(pArray);

	//Look for a node without a type. These are nodes that have just been created.
	if (index != -1)
	{
		Xt::DomComponent* pData = Xt::AsComponent(Xt::At(pArray, index));

		I_DgNode *pNode = 0;
		pNode = LevelHelpers::CreateNodeInPlace(this, concreteBaseType, pData);
	}
}

void C_XtLevel::OnXtArrayRemove(Xt::DomArray* pArray, E_ObjType baseFilterType)
{
	C_XtObject::C_Unbinder unbinder(this);

	BIZ_ASSERT(pArray);

	//Removing actor from array.
	{
		Ut::UtSet<XtId> xtIds;
		//Build a set of the current xt ids.
		for(Xt::DomArray::IndexIterator it(pArray); it; ++it)
		{
			Xt::DomComponent* pComponent = AsComponent(it);
			XtId id = pComponent->id;
			if (id)
				xtIds.Insert(id);
		}

		for (DgIdPtrMap::Iterator it = m_dgNodes.Begin(); it != m_dgNodes.End(); ++it)
		{
			I_DgNode* pNode = it->GetValue();
			if (!pNode || !pNode->AsXtObject() || !pNode->IsA(baseFilterType))
				continue;

			C_XtObject* pXtObject = pNode->AsXtObject();

			Ut::UtSet<XtId>::Iterator xtIdIt = xtIds.Find(pNode->GetId().GetXtId());
			//If we can't find an existing node within the current xt array...
			if (xtIdIt == xtIds.End())
			{
				pXtObject->XtUnbind();

				//Remove node.
				LevelHelpers::DestroyNode(pNode);
			}
		}
	}
}

void C_XtLevel::XtInitialiseDgNodes(Xt::DomArray* pArray, E_ObjType concreteBaseType)
{
	if (pArray)
	{
		for (Xt::DomArray::IndexIterator it(pArray); it; ++it)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(it);
			if (!pComponent)
				continue;

			I_DgNode *pNode = LevelHelpers::CreateNodeInPlace(
				this, concreteBaseType, pComponent);
			if (!pNode)
			{
				LOGWARN(C_Core::Instance().GetLogger(), "Failed to create node");
				continue;
			}
		}
	}
}

void C_XtLevel::XtInitialiseDgNodes(Xt::DomRecord* pLevelRecord)
{
	C_Status status;
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	//Actors
	{
		Xt::DomArray* pArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data, 
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_ActorArray).dom);
		XtInitialiseDgNodes(pArray, E_ObjType_SceneXtActorNode);
	}
	//Nodes
	{
		Xt::DomArray* pArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data, 
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_NodeArray).dom);
		XtInitialiseDgNodes(pArray, E_ObjType_SceneXtNode);
	}
	//Data
	{
		Xt::DomArray* pArray = Xt::AsArray(
			FindBySemantic(
				Xt::TypeOf(pSchema, pLevelRecord), 
				&pLevelRecord->root.data, 
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_DataArray).dom);
		XtInitialiseDgNodes(pArray, E_ObjType_DgXtGameDataNode);
	}
}

void C_XtLevel::XtInitialiseAftkRefs(Xt::DomRecord* pLevelRecord)
{
	Xt::DomBlock* pAftkRefs = Xt::AsBlock(Xt::At(&pLevelRecord->root.data, "__aftkRefs"));
	if (!pAftkRefs)
		//No references.
		return;

	Xt::DomArray* pXtAftkRefArray = Xt::AsArray(Xt::At(pAftkRefs, "__aftkRefArray"));
	if (!pXtAftkRefArray)
		//No references.
		return;

	for(Xt::DomArray::IndexIterator it(pXtAftkRefArray); it; ++it)
	{
		Xt::DomBlock* pXtItemBlock = Xt::AsBlock(it);
		if (!pXtItemBlock)
			continue;

		C_AftkLevelRef* pAftkLevelRef = new C_AftkLevelRef();
		if (!XtInitialiseAftkRef(*pAftkLevelRef, pXtItemBlock))
			return;

		m_aftkRefs.PushBack(pAftkLevelRef);
	}
}

bool C_XtLevel::XtInitialiseAftkRef(C_AftkLevelRef& aftkLevelRef, Xt::DomBlock* pDomBlock)
{
	Xt::DomBlock* pXtAftkRef = Xt::AsBlock(Xt::At(pDomBlock, "__aftkRef"));
	if (!pXtAftkRef)
		return false;

	XtString filePath = Xt::AsText(Xt::At(pXtAftkRef, "__filePath"));
	if (filePath.Length() == 0)
		return false;

	//Is this an external actor container?
	XtString isExtActorRef = Xt::AsText(Xt::At(pXtAftkRef, "__isExtActorRef"));
	if (isExtActorRef == "1")
		aftkLevelRef.isExtActor = true;
	else
		aftkLevelRef.isExtActor = false;

	C_AftkRef* pScene = 
		EditorLib::C_Core::Instance().GetAftkRefManager().CreateRef((const char*)filePath, false);
	if (!pScene)
		return false;

	//Todo - if we need to set other proporites we could do so here. Because the same reference can be used in multiple
	//levels nested within one another it probably doesn't make much sense to store too much info per level/ref combination.
	//Perhaps whether the ref should be loaded or not by default is enough. The primary level could override the 'is loaded by default'
	//flag if ambiguous.  

	aftkLevelRef.pAftkRef = pScene;
	return true;
}

Xt::DomBlock* C_XtLevel::GetAftkRefData(const Btl::String& _filePath)
{
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(m_recordId);
	if (!pLevelRecord) return 0;
	Xt::DomBlock* pAftkRefs = Xt::AsBlock(Xt::At(&pLevelRecord->root.data, "__aftkRefs"));
	if (!pAftkRefs) return 0;
	Xt::DomArray* pXtAftkRefArray = Xt::AsArray(Xt::At(pAftkRefs, "__aftkRefArray"));
	if (!pXtAftkRefArray) return 0;

	for(Xt::DomArray::IndexIterator it(pXtAftkRefArray); it; ++it)
	{
		Xt::DomBlock* pXtItemBlock = Xt::AsBlock(it);
		if (!pXtItemBlock) continue;
		Xt::DomBlock* pXtAftkRef = Xt::AsBlock(Xt::At(pXtItemBlock, "__aftkRef"));
		if (!pXtAftkRef) return 0;
		Btl::String xtFilePath = (const char*)Xt::AsText(Xt::At(pXtAftkRef, "__filePath"));
		xtFilePath = Misc::FormatUnixFileString(xtFilePath.Lower());
		if (filePath == xtFilePath)
			return pXtAftkRef;
	}

	return 0;
}

void C_XtLevel::OnNodeChangeXtType(const C_NodeId& id)
{
	LevelHelpers::OnChangeXtType(this, id);
}

C_XtObject* C_XtLevel::AsXtObject()
{
	return (C_XtObject*)this;
}

}

#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtFieldHelper.h>

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

namespace EditorLib
{

I_Layer::I_Layer(I_LayerSet* layerSet, I_Level* level)
{
	m_LayerSet = layerSet;
	m_Level = level;
	m_Flags.visible = true;
	m_Flags.selectable = true;
}

I_Layer::~I_Layer()
{
}

I_LayerSet* I_Layer::GetLayerSet()
{
	return m_LayerSet;
}

const I_Level* I_Layer::GetLevel() const
{
	return m_Level;
}

I_Level* I_Layer::GetLevel()
{
	return m_Level;
}

E_LayerType I_Layer::GetType() const
{
	return E_LayerType_Unknown;
}

void I_Layer::SetName(const Btl::String& name)
{
	m_Name = name;

	C_Core::Instance().GetGlobalSignals().layerEvent(*this, E_LayerEvent_Deserialised);
}

const Btl::String& I_Layer::GetName() const
{
	return m_Name;
}

void I_Layer::AddNode(I_DgNode *pNode)
{
	DgNodeSet::Iterator it = m_DgNodes.Find(pNode);
	if (it != m_DgNodes.End())
		return;

	DgNodeSet::InsertionReturnType r = m_DgNodes.Insert(DgNodeSet::DataType(pNode));
	assert(r.inserted);
	C_Core::Instance().GetGlobalSignals().layerNodeAdded(*this, *pNode);
}

void I_Layer::RemoveNode(const I_DgNode *pNode)
{
	DgNodeSet::Iterator it = m_DgNodes.Find(const_cast<I_DgNode*>(pNode));
	if (it == m_DgNodes.End())
		return;

	m_DgNodes.Erase(it);
	C_Core::Instance().GetGlobalSignals().layerNodeRemoved(*this, *pNode);
}

const I_Layer::DgNodeSet& I_Layer::GetNodes() const
{
	return m_DgNodes;
}

void I_Layer::SetDisplayColour(const Maths::C_RGBA_U8& colour)
{
	m_DisplayColour = colour;
	C_Core::Instance().DirtyViews();
}

Maths::C_RGBA_U8 I_Layer::GetDisplayColour() const
{
	return m_DisplayColour;
}

void I_Layer::SetLayerFlags(const I_Layer::Flags& flags)
{
	m_Flags = flags;
	C_Core::Instance().DirtyViews();
}

const I_Layer::Flags& I_Layer::GetLayerFlags() const
{
	return m_Flags;
}

C_Layer::C_Layer(I_LayerSet* layerSet, I_Level* level)
: I_Layer(layerSet, level)
{
}

C_Layer::~C_Layer()
{
}

C_RefLayer::C_RefLayer(I_LayerSet* layerSet, I_Level* level)
: C_Layer(layerSet, level)
{
}

C_RefLayer::~C_RefLayer()
{
}

E_LayerType C_RefLayer::GetType() const
{
	return E_LayerType_Reference;
}

void C_RefLayer::SetDisplayColour(const Maths::C_RGBA_U8& colour)
{
	m_DisplayColour = colour;
	C_Core::Instance().DirtyViews();
}

void C_RefLayer::AddNode(EditorLib::I_DgNode* pNode)
{
	if (pNode->AsXtObject())
		return;

	C_Layer::AddNode(pNode);
}

C_XtActorLayer::C_XtActorLayer(I_LayerSet* layerSet, I_Level* level)
: C_Layer(layerSet, level)
, C_XtObject()
{
	m_Flags.visible = true;
	m_Flags.selectable = true;
}

C_XtActorLayer::~C_XtActorLayer()
{
	Xt::Monitor(&m_xtActorArrayMonitor, 0);
}

E_LayerType C_XtActorLayer::GetType() const
{
	return E_LayerType_Actor;
}

void C_XtActorLayer::SetName(const Btl::String& name)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);
	Xt::DomField *pField = Xt::AsField(Xt::At(&pComponent->data, "Name"));
	if (pField)
		Xt::Set(pField, name.Chars());

	I_Layer::SetName(name);
}

C_Status C_XtActorLayer::XtInitialise(C_XtLevel *pLevel, LarvaType* pLarvaType, Xt::DomElement *pData)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(pData);
	assert(pComponent);

	Xt::DomBlock *pBlockData = &pComponent->data;

	//TODO: use component name instead of a separate name field
	Xt::DomField *pNameField = Xt::AsField(Xt::At(pBlockData, "Name"));
	if (!pNameField)
	{
		pNameField = Xt::AcquireField(pBlockData, "Name");
		pNameField->value = "NewActorStudioLayer";
		m_Name = "NewActorStudioLayer";
	}
	assert(pNameField);

	Xt::DomArray *pLayerActors = Xt::AsArray(Xt::At(pBlockData, "ActorArray"));
	if (!pLayerActors)
		pLayerActors = Xt::AcquireArray(pBlockData, "ActorArray");
	assert(pLayerActors);

	Xt::Monitor(&m_xtActorArrayMonitor, pLayerActors);
	m_xtActorArrayMonitor.SetCallback(
		&C_XtObject::StaticOnXtDomExternalMonitorEvent, (C_XtObject*)this);

	Xt::DomBlock *pColourBlock = Xt::AsBlock(Xt::At(pBlockData, "Colour"));
	if (!pColourBlock)
		pColourBlock = Xt::AcquireBlock(pBlockData, "Colour");
	assert(pColourBlock);

	Xt::DomField *pField;
	pField = Xt::AsField(Xt::At(pColourBlock, "r"));
	if (!pField)
	{
		pField = Xt::AcquireField(pColourBlock, "r");
		assert(pField); Xt::Set(pField, "0");
	}
	pField = Xt::AsField(Xt::At(pColourBlock, "g"));
	if (!pField)
	{
		pField = Xt::AcquireField(pColourBlock, "g");
		assert(pField); Xt::Set(pField, "0");
	}
	pField = Xt::AsField(Xt::At(pColourBlock, "b"));
	if (!pField)
	{
		pField = Xt::AcquireField(pColourBlock, "b");
		assert(pField); Xt::Set(pField, "0");
	}
	pField = Xt::AsField(Xt::At(pColourBlock, "a"));
	if (!pField)
	{
		pField = Xt::AcquireField(pColourBlock, "a");
		assert(pField); Xt::Set(pField, "0");
	}

	C_Status status = C_XtObject::XtInitialise(pLevel, pLarvaType, pData);
	if (!status)
		return status;

	return E_Status_Success;
}

void C_XtActorLayer::XtDeserialise(Xt::DomElement* pFrom)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	Xt::DomBlock* pDataBlock = pFrom ? Xt::AsBlock(pFrom) : &pComponent->data;

	bool updated = false;
	
	updated |= XtDeserialiseName(pFrom);
	updated |= XtDeserialiseColour(pFrom);

	Xt::DomArray *pActorArray = Xt::AsArray(Xt::At(pDataBlock, "ActorArray"));
	if (pActorArray)
	{
		for (Xt::DomArray::IndexIterator it(pActorArray); it; ++it)
		{
			Xt::DomRef* pField = Xt::AsRef(it);
			if (!pField)
				continue;

			C_DgNodeIterator dgIt = C_Core::Instance().GetDgManager().GetDgIterator();
			while ((I_DgNode*)dgIt)
			{
				if (dgIt->IsA(EditorLib::E_ObjType_SceneNode) &&
					((I_DgNode*)dgIt)->AsXtObject() && 
					((I_DgNode*)dgIt)->GetId().GetXtId() == pField->value)
				{
					I_DgNode* dgNode = ((I_DgNode*)dgIt);

					DgNodeSet::InsertionReturnType r = m_DgNodes.Insert(
						DgNodeSet::DataType(dgNode));

					C_Core::Instance().GetGlobalSignals().layerNodeAdded(*this, *dgNode);

					break;
				}
				++dgIt;
			}
		}
	}

	C_Core::Instance().GetGlobalSignals().layerEvent(*this, E_LayerEvent_Deserialised);
}

bool C_XtActorLayer::IsReadOnly() const
{
	return (C_Core::Instance().GetLevelManager()->GetActiveLevel() != (C_XtLevel*)m_Level);
}

void C_XtActorLayer::AddNode(I_DgNode *pNode)
{
	DgNodeSet::Iterator it = m_DgNodes.Find(pNode);
	if (it != m_DgNodes.End())
		return;

	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	Xt::DomArray *pActorArray = Xt::AsArray(Xt::At(&pComponent->data, "ActorArray"));
	if (pActorArray)
	{
		C_XtObject* pXtObject = pNode->AsXtObject();
		if (pXtObject)
		{
			Xt::DomRef *pField = new Xt::DomRef();
			pField->value = pNode->GetId().GetXtId();
			Xt::Append(pActorArray, pField);
		}
	}
}

void C_XtActorLayer::RemoveNode(const I_DgNode *pNode)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	Xt::DomArray *pActorArray = Xt::AsArray(Xt::At(&pComponent->data, "ActorArray"));
	if (pActorArray)
	{
		for (Xt::DomArray::IndexIterator it(pActorArray); it; ++it)
		{
			Xt::DomRef* pField = Xt::AsRef(it);
			if (!pField)
				continue;

			if (pNode->IsA(EditorLib::E_ObjType_SceneNode) &&
				pNode->AsXtObject() && 
				pNode->GetId().GetXtId() == pField->value)
			{
				Xt::Remove(pActorArray, it.index);
				C_Layer::RemoveNode(pNode);
			}
		}
	}
}

void C_XtActorLayer::SetDisplayColour(const Maths::C_RGBA_U8& colour)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	Xt::DomBlock *pColourBlock = Xt::AsBlock(Xt::At(&pComponent->data, "Colour"));
	if (pColourBlock)
	{
		C_XtObject::C_Unbinder unbinder(this);
		Xt::DomField *pField;

		pField = Xt::AsField(Xt::At(pColourBlock, "r"));
		XtFieldHelper::SetScalar(pField, colour.R());

		pField = Xt::AsField(Xt::At(pColourBlock, "g"));
		XtFieldHelper::SetScalar(pField, colour.G());

		pField = Xt::AsField(Xt::At(pColourBlock, "b"));
		XtFieldHelper::SetScalar(pField, colour.B());

		pField = Xt::AsField(Xt::At(pColourBlock, "a"));
		XtFieldHelper::SetScalar(pField, colour.A());
	}
	if (pColourBlock)
		XtDeserialiseColour();
}

bool C_XtActorLayer::XtDeserialiseName(Xt::DomElement* pFrom)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	Xt::DomField* pField;
	XT_HELPERS_AT_FIELD_CHECKED(pField, &pComponent->data, "Name");
	if (pField && pFrom ? XtHelpers::AreRelated(pField, pFrom) : true)
	{
		if (strcmp((const char*)pField->value, m_Name.Chars()))
		{
			I_Layer::SetName((const char*)pField->value);
			return true;
		}
	}

	return false;
}

bool C_XtActorLayer::XtDeserialiseColour(Xt::DomElement* pFrom)
{
	Xt::DomComponent *pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	Xt::DomBlock* pRoot = &pComponent->data;

	Xt::DomBlock* pBlock;
	XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pRoot, "Colour");
	if (pBlock && pFrom ? XtHelpers::AreRelated(pBlock, pFrom) : true)
	{
		Xt::DomField* pField;
		U8 colour;
		pField = Xt::AsField(Xt::At(pBlock, "r"));
		XtFieldHelper::GetScalar(colour, pField);
		m_DisplayColour.R(colour);

		pField = Xt::AsField(Xt::At(pBlock, "g"));
		XtFieldHelper::GetScalar(colour, pField);
		m_DisplayColour.G(colour);

		pField = Xt::AsField(Xt::At(pBlock, "b"));
		XtFieldHelper::GetScalar(colour, pField);
		m_DisplayColour.B(colour);

		pField = Xt::AsField(Xt::At(pBlock, "a"));
		XtFieldHelper::GetScalar(colour, pField);
		m_DisplayColour.A(colour);

		C_Core::Instance().DirtyViews();
		return true;
	}
	
	return false;
}

void C_XtActorLayer::OnXtDomMonitorEvent(Xt::DomMonitor *pMonitor, Xt::DomEvent *pEvent, bool typeChanged)
{
	bool updated = false;

	updated |= XtDeserialiseName(pEvent->from);
	updated |= XtDeserialiseColour(pEvent->from);

	if (pMonitor == &m_xtActorArrayMonitor &&
		pEvent->code == Xt::kDomArrayChanged &&
		pEvent->from == m_xtActorArrayMonitor.element)
	{
		Xt::DomArrayChanged* pArrayChangedEvent = (Xt::DomArrayChanged*)pEvent;
		int index = Xt::IndexOf(AsArray(pArrayChangedEvent->from), pArrayChangedEvent->id);
		if (index == -1)
			OnXtActorArrayRemove();
		else
			OnXtActorArrayAdd(index);
	}

	if (updated)
		C_Core::Instance().GetGlobalSignals().layerEvent(*this, E_LayerEvent_Deserialised);
}

void C_XtActorLayer::OnXtActorArrayAdd(S32 index)
{
	assert(index != -1);
	C_XtObject::C_Unbinder unbinder(this);

	Xt::DomArray* pLayerActorArray = Xt::AsArray(m_xtActorArrayMonitor);
	assert(pLayerActorArray);

	if (index != -1)
	{
		Xt::DomRef* pField = Xt::AsRef(Xt::At(pLayerActorArray, index));

		if (pField)
		{
			C_DgNodeIterator dgIt = C_Core::Instance().GetDgManager().GetDgIterator();
			while ((I_DgNode*)dgIt)
			{
				if (dgIt->IsA(EditorLib::E_ObjType_SceneNode) &&
					((I_DgNode*)dgIt)->AsXtObject() && 
					((I_DgNode*)dgIt)->GetId().GetXtId() == pField->value)
				{
					DgNodeSet::InsertionReturnType r = m_DgNodes.Insert(
						DgNodeSet::DataType((I_DgNode*)dgIt));
					BIZ_ASSERT(r.inserted);
					C_Core::Instance().GetGlobalSignals().layerNodeAdded(*this, *((I_DgNode*)dgIt));
					break;
				}
				dgIt++;
			}
		}
	}
}


void C_XtActorLayer::OnXtActorArrayRemove()
{
	C_XtObject::C_Unbinder unbinder(this);

	Xt::DomArray* pLayerActorArray = Xt::AsArray(m_xtActorArrayMonitor);
	assert(pLayerActorArray);

	Ut::UtSet<XtId> xtIds;

	{
		//Build a set of the current xt ids.
		for(Xt::DomArray::IndexIterator it(pLayerActorArray); it; ++it)
		{
			xtIds.Insert(Xt::AsId(it));
		}
	}

	//loop over our existing nodes
	DgNodeSet::Iterator it = m_DgNodes.Begin();
	while (it != m_DgNodes.End())
	{
		const I_DgNode* pNode = (const I_DgNode*)(*it);
		const C_XtObject* pXtObject = pNode->AsXtObject();
		if (pXtObject)
		{
			Ut::UtSet<XtId>::Iterator xtIdIt = xtIds.Find(pNode->GetId().GetXtId());
			if (xtIdIt == xtIds.End())
			{
				m_DgNodes.Erase(it);
				C_Core::Instance().GetGlobalSignals().layerNodeRemoved(*this, *pNode);
				break;
			}
		}
		++it;
	}
}

}

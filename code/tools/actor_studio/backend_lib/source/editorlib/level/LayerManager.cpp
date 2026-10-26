#include "stdafx.h"

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtLarvaHelper.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

using namespace EditorLib;

I_LayerSet::I_LayerSet(I_Level* level)
{
	m_Level = level;
}

I_LayerSet::~I_LayerSet()
{
	for (LayerList::Iterator it = m_Layers.Begin(); it != m_Layers.End(); ++it)
	{
		C_Core::Instance().GetGlobalSignals().layerEvent(**it, E_LayerEvent_Removed);
		delete (*it);
	}
}

I_Level* I_LayerSet::GetLevel()
{
	return m_Level;
}

C_Status I_LayerSet::AddLayer(EditorLib::I_Layer* layer)
{
	m_Layers.PushBack(layer);

	C_Core::Instance().GetGlobalSignals().layerEvent(*layer, E_LayerEvent_Added);

	return E_Status_Success;
}

C_Status I_LayerSet::RemoveLayer(EditorLib::I_Layer* layer)
{
	LayerList::Iterator it = m_Layers.Begin();
	while (it != m_Layers.End())
	{
		if ((*it) == layer)
			break;
		++it;
	}

	if (it == m_Layers.End())
		return E_Status_Failure;
	
	delete (*it);
	m_Layers.Erase(it);
	
	C_Core::Instance().GetGlobalSignals().layerEvent(*layer, E_LayerEvent_Removed);

	return E_Status_Success;
}

LayerList::ConstIterator I_LayerSet::GetLayerIterator() const
{
	return m_Layers.Begin();
}

void I_LayerSet::OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
	for (LayerList::Iterator it = m_Layers.Begin(); it != m_Layers.End(); ++it)
	{
		(*it)->RemoveNode(&dgNode);
	}
}

C_VirtualLayerSet::C_VirtualLayerSet(I_Level* level)
: I_LayerSet(level)
{
}

C_VirtualLayerSet::~C_VirtualLayerSet()
{
}

E_LayerSetType C_VirtualLayerSet::GetType()
{
	return E_LayerSetType_Virtual;
}

C_XtLayerSet::C_XtLayerSet(I_Level* level)
: I_LayerSet(level)
{
	Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(((C_XtLevel*)level)->GetXtRecordId());
	BIZ_ASSERT(pLevelRecord);

	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
	Xt::FieldDom fd = AcquireBySemantic(
			Xt::TypeOf(pSchema, pLevelRecord), 
			&pLevelRecord->root.data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_Layers);
	Xt::DomArray* layerArray = Xt::AsArray(fd.dom);
	BIZ_ASSERT(layerArray);

	BIZ_ASSERT(fd.field->kind == kLfArray);
	m_LayerFieldArray = (LfArray*)fd.field;

	XtInitialise((C_XtLevel*)level, fd.field->Type(), layerArray);
	XtBind();

	Populate();
}

C_XtLayerSet::~C_XtLayerSet()
{
	XtUnbind();
}

E_LayerSetType C_XtLayerSet::GetType()
{
	return E_LayerSetType_Xt;
}

C_Status C_XtLayerSet::AddLayer(I_Layer* layer)
{
	C_CheckpointDisabler cpDisable;

	if (layer->GetType() == E_LayerType_Actor)
	{
		if (GetLevel()->GetType() != E_LevelType_XtLevel)
			return E_Status_Failure;

		Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(((C_XtLevel*)GetLevel())->GetXtRecordId());
		if (!pLevelRecord)
			return E_Status_Failure;

		Xt::DomComponent* pXtNewElement = new Xt::DomComponent;
		Xt::Append(Xt::AsArray(GetXtData()), pXtNewElement);

		if (!GenerateIdForComponent(pXtNewElement, false))
			return E_Status_Failure;

		LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
		if (!layer || !((C_XtActorLayer*)layer)->XtInitialise((C_XtLevel*)GetLevel(), FindTypeByName(C_Core::Instance().GetXtManager().GetDb()->schema, "ActorLayer"), pXtNewElement))
		{
			delete layer;
			return E_Status_Failure;
		}
		((C_XtActorLayer*)layer)->XtBind();
	} else {
		BIZ_ASSERT("Adding anything other than an actor layer to an Xt layer set is unsupported");

		return E_Status_Failure;
	}

	I_LayerSet::AddLayer(layer);

	return E_Status_Success;
}

C_Status C_XtLayerSet::RemoveLayer(I_Layer *layer)
{
	C_CheckpointDisabler cpDisable;

	S32 xtIndexToErase = -1;
	Xt::DomArray* pLayers = 0;

	if (layer->GetType() == E_LayerType_Actor)
	{
		//We've found the item in m_layers to erase, now find Xt dom version.
		C_XtLevel* level = EditorLib::C_Core::Instance().GetLevelManager()->GetActiveLevel();
		if (!level)
			return E_Status_Failure;
		
		Xt::DomRecord* pLevelRecord = EditorLib::C_Core::Instance().GetXtManager().FindRecord(level->GetXtRecordId());
		if (!pLevelRecord)
			return E_Status_Failure;	

		LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
		pLayers = Xt::AsArray(AcquireBySemantic(
			Xt::TypeOf(pSchema, pLevelRecord), 
			&pLevelRecord->root.data, 
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_Layers).dom);

		if (!pSchema || !pLayers)
			return E_Status_Failure;

		
		for (Xt::DomArray::IndexIterator domIt(pLayers); domIt; ++domIt)
		{
			Xt::DomComponent* pXtComponent = Xt::AsComponent(domIt);
			if (!pXtComponent)
				continue;

			if (((C_XtActorLayer*)layer)->GetXtData() == pXtComponent)
			{
				xtIndexToErase = domIt.index;
				break;
			}
		}

		if (xtIndexToErase == -1)
			return E_Status_Failure;
	}

	if (!I_LayerSet::RemoveLayer(layer))
		return E_Status_Failure;

	if (xtIndexToErase != -1)
	{
		//Erase xt data.
		Xt::Remove(pLayers, xtIndexToErase);
	}

	return E_Status_Success;
}

void C_XtLayerSet::OnXtDomMonitorEvent(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, bool typeChanged)
{
	int a = 0;
}

void C_XtLayerSet::Populate()
{
	LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

	for(Xt::DomArray::IndexIterator it(Xt::AsArray(GetXtData())); it; ++it)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(it);
		if (!pComponent)
			continue;

		C_XtActorLayer* pLayer = new C_XtActorLayer(this, GetLevel());
		
		if (!pLayer || !pLayer->XtInitialise((C_XtLevel*)GetLevel(), Xt::TypeOf(pSchema, m_LayerFieldArray->field, pComponent), pComponent))
		{
			delete pLayer;
			continue;
		}

		I_LayerSet::AddLayer(pLayer); // Add before deserialisation, due to spurious UI assertion (TODO: move this once UI is cleaned up).

		pLayer->XtDeserialise();
		pLayer->XtBind();	
	}
}

C_LayerManager::C_LayerManager()
{
	// Create empty layer set
	C_VirtualLayerSet* layerSet = new C_VirtualLayerSet(0);
	m_LayerSetMap[0] = layerSet;

	m_Slots.Connect();
}

C_LayerManager::~C_LayerManager()
{
	m_Slots.Disconnect();

	for (LayerSetMap::Iterator it = m_LayerSetMap.Begin(); it != m_LayerSetMap.End(); ++it)
	{
		delete it->GetValue();
	}
	m_LayerSetMap.Clear();
}

void C_LayerManager::AddNodeToLayer(I_Layer* layer, I_DgNode* node)
{
	RemoveLayerNode(node);
	
	layer->AddNode(node);
}

void C_LayerManager::RemoveLayerNode(const I_DgNode* node)
{
	if (!node)
		return;

	DgLayerMap::Iterator it = m_DgLayerMap.Find(node);
	
	if (it == m_DgLayerMap.End())
		return;
	
	it->GetValue()->RemoveNode(node);
}

I_Layer* C_LayerManager::GetLayer(const I_DgNode* node)
{
	DgLayerMap::Iterator it = m_DgLayerMap.Find(node);
	
	if (it == m_DgLayerMap.End())
		return 0;
	
	return it->GetValue();
}

I_LayerSet* C_LayerManager::GetLayerSet(I_Level* level)
{
	if (m_LayerSetMap.Find(level) != m_LayerSetMap.End())
		return m_LayerSetMap[level];

	return 0;
}

void C_LayerManager::OnLevelOpened(const I_Level& level)
{
	I_LayerSet* layerSet = 0;

	if (level.GetType() == E_LevelType_XtLevel)
	{
		C_XtLevel* xtLevel = const_cast<C_XtLevel*>((const C_XtLevel*)&level);

		layerSet = new C_XtLayerSet(xtLevel);
	} else if (level.GetType() == E_LevelType_ReferenceLevel)
	{
		layerSet = new C_VirtualLayerSet(const_cast<I_Level*>(&level));
	}

	if (layerSet)
	{
		m_LayerSetMap[&level] = layerSet;
	}
}

void C_LayerManager::OnLevelClosing(const I_Level& level)
{
	LayerSetMap::Iterator it = m_LayerSetMap.Find(&level);

	if (it != m_LayerSetMap.End())
	{
		delete it->GetValue();
		m_LayerSetMap.Erase(it);
	}
}

void C_LayerManager::OnDgNodeDestroy(const I_DgNode& dgNode)
{
	for (LayerSetMap::Iterator setIt = m_LayerSetMap.Begin(); setIt != m_LayerSetMap.End(); ++setIt)
		setIt->GetValue()->OnDgNodeDestroy(dgNode);

	EditorLib::DgLayerMap::Iterator it = m_DgLayerMap.Find(&dgNode);

	if (it != m_DgLayerMap.End())
		m_DgLayerMap.Erase(it);
}

LayerSetMap::ConstIterator C_LayerManager::LayerSetIteratorBegin() const
{
	return m_LayerSetMap.Begin();
}

LayerSetMap::ConstIterator C_LayerManager::LayerSetIteratorEnd() const
{
	return m_LayerSetMap.End();
}

void C_LayerManager::C_Slots::Connect()
{
	C_GlobalSignals& s = C_Core::Instance().GetGlobalSignals();

	s.levelOpened.connect(this, &C_LayerManager::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &C_LayerManager::C_Slots::OnLevelClosingSlot);
	s.dgNodeDestroy.connect(this, &C_LayerManager::C_Slots::OnDgNodeDestroySlot);
	s.layerNodeAdded.connect(this, &C_LayerManager::C_Slots::OnLayerNodeAddedSlot);
	s.layerNodeRemoved.connect(this, &C_LayerManager::C_Slots::OnLayerNodeRemovedSlot);
}

void C_LayerManager::C_Slots::Disconnect()
{
	C_GlobalSignals& s = C_Core::Instance().GetGlobalSignals();

	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.dgNodeDestroy.disconnect(this);
	s.layerNodeAdded.disconnect(this);
	s.layerNodeRemoved.disconnect(this);
}

void C_LayerManager::C_Slots::OnLevelOpenedSlot(const I_Level& level)
{
	C_Core::Instance().GetLayerManager().OnLevelOpened(level);
}

void C_LayerManager::C_Slots::OnLevelClosingSlot(const I_Level& level)
{
	C_Core::Instance().GetLayerManager().OnLevelClosing(level);
}

void C_LayerManager::C_Slots::OnDgNodeDestroySlot(const I_DgNode& dgNode)
{
	C_Core::Instance().GetLayerManager().OnDgNodeDestroy(dgNode);
}

void C_LayerManager::C_Slots::OnLayerNodeAddedSlot(const I_Layer& layer, const I_DgNode& dgNode)
{
	C_Core::Instance().GetLayerManager().m_DgLayerMap[&dgNode] = const_cast<I_Layer*>(&layer);
}

void C_LayerManager::C_Slots::OnLayerNodeRemovedSlot(const I_Layer& layer, const I_DgNode& dgNode)
{
	DgLayerMap::Iterator it = C_Core::Instance().GetLayerManager().m_DgLayerMap.Find(&dgNode);
	BIZ_ASSERT(it != C_Core::Instance().GetLayerManager().m_DgLayerMap.End());

	C_Core::Instance().GetLayerManager().m_DgLayerMap.Erase(it);
}
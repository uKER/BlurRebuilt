#ifndef EDITORLIB_LAYER_MANAGER_H
#define EDITORLIB_LAYER_MANAGER_H

#include <sigslot/sigslot.h>

namespace EditorLib
{

class C_LayerManager;

class I_LayerSet
{
public:
	I_LayerSet(I_Level* level);
	virtual ~I_LayerSet();

	virtual E_LayerSetType GetType() = 0;

	I_Level* GetLevel();

	virtual C_Status AddLayer(I_Layer* layer);
	virtual C_Status RemoveLayer(I_Layer* layer);
	LayerList::ConstIterator GetLayerIterator() const;

protected:
	void OnDgNodeDestroy(const I_DgNode& dgNode);

private:
	I_Level* m_Level;
	LayerList m_Layers;

	FRIEND_CLASS C_LayerManager;
};

class C_VirtualLayerSet : public I_LayerSet
{
public:
	C_VirtualLayerSet(I_Level* level);
	virtual ~C_VirtualLayerSet();

	E_LayerSetType GetType();

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		void Connect();
		void Disconnect();
		
		FRIEND_CLASS C_VirtualLayerSet;
	};

	C_Slots m_Slots;
};

class C_XtLayerSet : public I_LayerSet, C_XtObject
{
public:
	C_XtLayerSet(I_Level* level);
	virtual ~C_XtLayerSet();

	virtual E_LayerSetType GetType();

	virtual C_Status AddLayer(I_Layer* layer);
	virtual C_Status RemoveLayer(I_Layer* layer);

private:
	void OnXtDomMonitorEvent(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, bool typeChanged);
	void Populate();

	LfArray* m_LayerFieldArray;
};
	
class C_LayerManager
{
public:
	C_LayerManager();
	~C_LayerManager();

	I_LayerSet* GetLayerSet(I_Level* level);

	void AddNodeToLayer(I_Layer* layer, I_DgNode* node);
	void RemoveLayerNode(const I_DgNode* node);
	I_Layer* GetLayer(const I_DgNode* node);

	void OnLevelOpened(const I_Level&);
	void OnLevelClosing(const I_Level&);
	void OnDgNodeDestroy(const I_DgNode&);

	LayerSetMap::ConstIterator LayerSetIteratorBegin() const;
	LayerSetMap::ConstIterator LayerSetIteratorEnd() const;

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		void Connect();
		void Disconnect();
		
		void OnLevelOpenedSlot(const I_Level&);
		void OnLevelClosingSlot(const I_Level&);
		void OnDgNodeDestroySlot(const I_DgNode&);
		void OnLayerNodeAddedSlot(const I_Layer& layer, const I_DgNode& dgNode);
		void OnLayerNodeRemovedSlot(const I_Layer& layer, const I_DgNode& dgNode);

		FRIEND_CLASS C_LayerManager;
	};

	C_Slots m_Slots;

	DgLayerMap m_DgLayerMap;
	LayerList m_Temp;
	LayerSetMap m_LayerSetMap;

	FRIEND_CLASS C_XtLayerSet;
};

}

#endif
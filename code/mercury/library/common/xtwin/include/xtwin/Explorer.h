#ifndef XTWIN_EXPLORER__H
#define XTWIN_EXPLORER__H

#ifdef _MSC_VER
#pragma once
#endif

#include <xtwin/TreeCtrl.h>

enum ExItemType
{
	kExItemTypeRoot,
	kExItemTypeRecord,
	kExItemTypeFilterPack,
	kExItemTypeFilterRecord,
	kExItemTypePack
};

class Explorer : public TreeCtrl
{
public:
	class ExplorerData : public TreeItem
	{
	public:
		ExplorerData(Explorer* explorer);
		ExplorerData(Explorer* explorer, Xt::DomRecord* pRecord);
		ExplorerData(Explorer* explorer, Xt::DomPack* pPack);
		ExplorerData(Explorer* explorer, const XtString& filterFolder, Xt::DomPack* pPack);

		virtual ~ExplorerData();

		ExItemType GetType() const;

		Xt::DomRecord* GetDomRecord();
		Xt::DomPack* GetDomPack();

		bool IsFilter() const;

		const XtString& GetFilter() const;

	public:
		virtual void OnContextMenu(const WinPoint& pt);
		virtual bool OnItemDeleting();
		virtual void OnItemDeleted() {}
		virtual bool OnItemSelectionChanged(bool selected) { return true; }
		virtual bool OnReparent(TreeItem* parent);
		virtual bool OnReorder(int newIndex) { return true; }
		virtual bool OnStartRename(Sz16& startName);
		virtual bool OnEndRename(Sz16& newName);
		virtual bool OnActivate();

		void UpdateName();

	private:
		void OnMonitorEvent(Xt::DomEvent* evt);
		static void StaticMonitorEvent(Xt::DomMonitor* monitor, Xt::DomEvent* evt, ExplorerData* data);

		Explorer* _Explorer;
		Xt::DomMonitor _Monitor;
		Xt::DomPack* _DomPack;
		Xt::DomRecord* _DomRecord;
		ExItemType _Type;
		XtString _Filter;
	};

public:
	Explorer();

	virtual ~Explorer();

	void SetDb(Xt::DomDb* db);
	void SetFilterEmpty(bool filter);
	void SetAllowRename(bool allowRename);
	void UpdateFilter();
	void GetFilterName(Explorer::ExplorerData* pData, XtString& filter);
	static void StaticMonitorEvent(Xt::DomMonitor* monitor, Xt::DomEvent* evt, Explorer* data);
	void OnMonitorEvent(Xt::DomEvent* evt);

protected:
	Xt::DomDb* GetDb();
	
private:
	void ParentChanged(Xt::DomElement* element);
	virtual void Populate();
	void AddPack(ExplorerData* pRootData, Xt::DomPack* pPack);
	void AddRecord(ExplorerData* pPackData, Xt::DomPack* pPack, Xt::DomRecord* pRecord);
	ExplorerData* GetExplorerData(Xt::DomElement* element);
	
	ExplorerData* GetFilter(ExplorerData* pack, const XtString& name, Xt::DomPack* pPack);

public:
	void Clear();

protected:
	virtual void FilterItem(ExplorerData* item);
	virtual void FilterEmpty(ExplorerData* item);
	virtual bool IsEmpty(ExplorerData* item);
	virtual bool OnCompareItems(TreeItem* a, TreeItem* b);

protected:
	Xt::DomPack* FindPack(const XtString& name);

private:
	void _GetFilterName(Explorer::ExplorerData* pData, XtString& filter);

protected:
	virtual void OnContextMenu(ExplorerData* item, const WinPoint& pt) {}
	virtual bool OnFilterItem(ExplorerData* pData);
	virtual void OnActivateItem(ExplorerData* pData) {};

	void Delete(ExplorerData* item);

private:
	void DeletePack(ExplorerData* pack);
	void DeletePackFolder(ExplorerData* pack);
	void DeleteRecord(ExplorerData* pack);
	void DeleteRecordFolder(ExplorerData* pack);
	
private:
	Xt::DomDb* _Db;
	Xt::DomMonitor _Monitor;
	XtUt::Dictionary<Xt::DomElement*, ExplorerData*> _DataMap;
	bool _ShouldFilterEmtpy;
	bool _AllowRename;
};

#endif
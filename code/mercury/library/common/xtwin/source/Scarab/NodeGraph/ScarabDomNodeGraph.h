#ifndef SCARABDOMNODEGRAPH_H
#define SCARABDOMNODEGRAPH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/DeskWindow.h"
#include "XtWin/NodeGraph.h"

class SnNgConnection;
class SnNgNode;
class SnNgPin;

class ScarabDomNodeGraph : public ChildWindow
{
public:
	ScarabDomNodeGraph();
	virtual ~ScarabDomNodeGraph();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void SetForm(ScarabForm* form);
	void SetDom(LtStruct* rootType, Xt::DomBlock* rootDom);
	void SetWatermark(XtString watermark);
	void Layout();

private:
	void PopulateRoots(LtStruct* type, Xt::DomBlock* dom);
	void PopulateNodes();
	void PopulatePins();

private:
	bool CreateNode(LarvaSchema* schema, LarvaField* field, Xt::DomComponent* component, Xt::Indexer* indexer, Xt::DomElement* parentDom);

private:
	void SyncControls();
	void SyncSelection();

private:
	struct RootElement
	{
		Xt::DomMonitor	dom;
		LarvaField*		field;

	public:
		RootElement(Xt::DomElement* dom_, LarvaField* field_) :
			field(field_)
		{
			Xt::Monitor(&dom, dom_);
		}
	};

private:
	NgDiagram* Diagram();
	SnNgNode* FindNode(XtId id);
	RootElement* FindBestRoot(LtStruct* want);
	RootElement* FindPossiblePlug(LtStruct* want);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ScarabDomNodeGraph* node);

private:
	void MegaUndoHack_CheckPoint();
	void MegaUndoHack_Clear();
	void MegaUndoHack_Store();
	void MegaUndoHack_Undo();
	void MegaUndoHack_Redo();

private:
	LtStruct*		_domType;
	Xt::DomMonitor	_domRoot;

private:
	ScarabForm*	_form;

private:
	DwHost		_host;
	DwSplit*	_splitter;
	bool		_firstLayout;
	bool		_resyncSelection;
	bool		_resyncControls;
	bool		_megaUndoHack;

private:
	NodeGraph*	_wndGraph;
	ScarabForm*	_wndForm;

private:
	XtUt::LinkList<SnNgNode> _nodes;

private:
	XtUt::List<RootElement> _rootElements;

private:
	struct UndoHack
	{
		XtUt::Link<UndoHack> link;

		Xt::Change* undo;
		Xt::Change* redo;

	public:
		inline UndoHack() :
			link(this),
			undo(0),
			redo(0)
		{
		}

		~UndoHack()
		{
			delete undo;
			delete redo;
		}
	};

private:
	Xt::DomElement*				_snapshot;
	XtUt::LinkList<UndoHack>	_undo;
	XtUt::LinkList<UndoHack>	_redo;

friend class SnNgConnection;
friend class SnNgPin;
friend class SnNgNode;
friend class ScarabDomNodeGraphContext;
friend class SnNgRootBuilder;
friend class SnNgPinBuilder;
};

#endif // SCARABDOMNODEGRAPH_H
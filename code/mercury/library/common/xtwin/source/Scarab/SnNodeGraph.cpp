#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DeskWindow.h"
#include "XtWin/NodeGraph.h"
#include "ScarabImpl.h"

//
// SnNodeGraph
//

SnNodeGraph::SnNodeGraph(LfControl* field) :
	SnControl(field),
	_panelSize(800, 600),
	_idGraph(this)
{
	_dom.SetCallback(DomCallback, this);

	_panelSize.cx = (int)GetInt("NodeGraph", "width", _panelSize.cx);
	_panelSize.cy = (int)GetInt("NodeGraph", "height", _panelSize.cy);
}

SnNodeGraph::~SnNodeGraph()
{
}

void SnNodeGraph::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnNodeGraph::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnNodeGraph::OnBuild()
{
	if (!_idGraph.Allocate())
		return false;

	if (_wndGraph == 0)
	{
		_wndGraph.SetForm(Panel()->Form());

		if (!_wndGraph.Create(*Panel()))
			return false;

		_wndGraph.SetDom(_field->container, AsBlock(_dom));

		_wndGraph.SetWatermark(GetString("NodeGraph", "watermark"));
	}

	return true;
}

void SnNodeGraph::OnMeasure(HDC ic)
{
	_size = _panelSize;
}

void SnNodeGraph::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndGraph, _client.left, _client.top, _panelSize.cx, _panelSize.cy, SWP_SHOWWINDOW);

	_wndGraph.Layout();
}

void SnNodeGraph::SetDom(Xt::DomBlock* block)
{
	SetEnabled(Monitor(&_dom, block) != 0);

	if (_wndGraph)
		_wndGraph.SetDom(_field->container, block);
}

void SnNodeGraph::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnNodeGraph* node)
{
	if (event->code == Xt::kDomRemoved)
		node->SetDom(0);
}
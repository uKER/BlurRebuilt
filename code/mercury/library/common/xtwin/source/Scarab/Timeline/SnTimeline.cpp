#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/Timeline.h"
#include "../ScarabImpl.h"

SnTimeline::SnTimeline(LfControl* field) :
	SnControl(field),
	_panelSize(800, 600),
	_idTimeline(this)
{
	_dom.SetCallback(DomCallback, this);

	_panelSize.cx = (int)GetInt("Timeline", "width", _panelSize.cx);
	_panelSize.cy = (int)GetInt("Timeline", "height", _panelSize.cy);
}

SnTimeline::~SnTimeline()
{
}

void SnTimeline::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnTimeline::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnTimeline::OnBuild()
{
	if (!_idTimeline.Allocate())
		return false;

	if (_wndTimeline == 0)
	{
		_wndTimeline.SetForm(Panel()->Form());

		if (!_wndTimeline.Create(*Panel()))
			return false;

		_wndTimeline.SetDom(Panel()->Db(), _field->container, AsBlock(_dom));
	}

	return true;
}

void SnTimeline::OnMeasure(HDC ic)
{
	_size = _panelSize;
}

void SnTimeline::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndTimeline, _client.left, _client.top, _panelSize.cx, _panelSize.cy, SWP_SHOWWINDOW);
}

void SnTimeline::SetDom(Xt::DomBlock* block)
{
	SetEnabled(Monitor(&_dom, block) != 0);

	if (_wndTimeline)
		_wndTimeline.SetDom(Panel()->Db(), _field->container, block);
}

void SnTimeline::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnTimeline* tl)
{
	if (event->code == Xt::kDomRemoved)
		tl->SetDom(0);
}
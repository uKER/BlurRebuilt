#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DopeSheet.h"
#include "../ScarabImpl.h"

SnDopeSheet::SnDopeSheet(LfControl* field) :
	SnControl(field),
	_panelSize(800, 600),
	_idDopeSheet(this)
{
	_dom.SetCallback(DomCallback, this);

	_panelSize.cx = (int)GetInt("DopeSheet", "width", _panelSize.cx);
	_panelSize.cy = (int)GetInt("DopeSheet", "height", _panelSize.cy);
}

SnDopeSheet::~SnDopeSheet()
{
}

void SnDopeSheet::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnDopeSheet::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnDopeSheet::OnBuild()
{
	if (!_idDopeSheet.Allocate())
		return false;

	if (_wndDopeSheet == 0)
	{
		_wndDopeSheet.SetForm(Panel()->Form());

		if (!_wndDopeSheet.Create(*Panel()))
			return false;

		_wndDopeSheet.SetDom(Panel()->Db(), _field->container, AsBlock(_dom));
	}

	return true;
}

void SnDopeSheet::OnMeasure(HDC ic)
{
	_size = _panelSize;
}

void SnDopeSheet::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndDopeSheet, _client.left, _client.top, _panelSize.cx, _panelSize.cy, SWP_SHOWWINDOW);
}

void SnDopeSheet::SetDom(Xt::DomBlock* block)
{
	SetEnabled(Monitor(&_dom, block) != 0);

	if (_wndDopeSheet)
		_wndDopeSheet.SetDom(Panel()->Db(), _field->container, block);
}

void SnDopeSheet::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnDopeSheet* tl)
{
	if (event->code == Xt::kDomRemoved)
		tl->SetDom(0);
}
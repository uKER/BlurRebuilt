#include "Core/StdAfx.h"
#include "SorceryView.h"
#include "LocForm/LocForm.h"
#include "Audio/WaveBankPane.h"
#include "DynamicForm/GridForm.h"
#include "DynamicForm/HyperView.h"

//
// SorceryView
//

SorceryView::SorceryView(SorceryApp& app, SorceryDocument* document, SorceryViewType type) :
	_linkWindow(this),
	_linkDocument(this),
	_app(app),
	_document(document),
	_type(type),
	_window(0)
{
	switch(_type)
	{
		case kSvtScarab:	_window = new DynamicFormMdiWindow(*this);	break;
		case kSvtLoc:		_window = new LocFormMdiWindow(*this);		break;
		case kSvtWaveBank:	_window = new WaveBankMdiWindow(*this);		break;
		case kSvtGrid:		_window = new GridMdiWindow(*this);			break;
		case kSvtHyper:		_window = new HyperViewMdiWindow(*this);	break;
	}
}

SorceryView::~SorceryView()
{
	delete _window;
}

void SorceryView::Create()
{
	static int cycle = 0;

	if (!_window->Create(WinPoint(cycle * 22, cycle * 22), WinSize(400, 400), _app._window.GetMdiWnd()))
		return;

	cycle = (cycle + 1) % 6;

	switch(_type)
	{
		case kSvtScarab:	((DynamicFormMdiWindow*)_window)->AdjustSizeToForm();	break;
		case kSvtWaveBank:	((WaveBankMdiWindow*)_window)->AdjustWindowSize();		break;
		case kSvtGrid:		((GridMdiWindow*)_window)->AdjustSizeToForm();			break;
		case kSvtHyper:		((HyperViewMdiWindow*)_window)->AdjustSizeToForm();		break;
	}
}

bool SorceryView::Close()
{
	Destroy();

	return true;
}

void SorceryView::Destroy(bool destroyDocument)
{
	SorceryDocument* document = destroyDocument ? _document : 0;

	if (document)
	{
		// Dont destroy the document if someone else is using it

		for(LinkIterator<SorceryView> it(_document->_views); it; ++it)
		{
			if (it != this)
			{
				document = 0;
				break;
			}
		}
	}

	if (_window != 0)
		SendMessage(_app._window.GetMdiWnd(), WM_MDIDESTROY, (WPARAM)(HWND)*_window, 0);

	delete this;

	if (document)
		delete document;
}

void SorceryView::OnDetailsUpdated()
{
	switch(_type)
	{
		case kSvtScarab:	((DynamicFormMdiWindow*)_window)->UpdateWindowTitle();	break;
		case kSvtLoc:		((LocFormMdiWindow*)_window)->UpdateWindowTitle();		break;
		case kSvtWaveBank:	((WaveBankMdiWindow*)_window)->UpdateWindowTitle();		break;
		case kSvtGrid:		((GridMdiWindow*)_window)->UpdateWindowTitle();			break;
		case kSvtHyper:		((HyperViewMdiWindow*)_window)->UpdateWindowTitle();		break;
	}
}
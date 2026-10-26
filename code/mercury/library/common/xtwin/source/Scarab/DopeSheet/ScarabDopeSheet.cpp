#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DopeSheet.h"
#include "../ScarabImpl.h"
#include "SnDSImpl.h"

//
// SnDSContext
//

class SnDSContext : public DopeSheetContext
{
public:
	SnDSContext(ScarabDopeSheet* sn) :
		_sn(sn)
	{
	}

public:
	virtual void SelectionChanged()
	{
		if (*_sn && !_sn->_resyncSelection)
		{
			_sn->_resyncSelection = true;

			PostMessage(*_sn, WM_APP + 1, 0, 0);
		}
	}

private:
	ScarabDopeSheet* _sn;
};

//
// SnDSSfc
//

class SnDSSfc : public ScarabFormContext
{
public:
	SnDSSfc(ScarabFormContext* sfc) :
		_sfc(sfc)
	{
	}

public:
	virtual void OpenRecord(Xt::DomRecord* record)
	{
		_sfc->OpenRecord(record);
	}

private:
	ScarabFormContext* _sfc;
};

//
// ScarabDopeSheet
//

ScarabDopeSheet::ScarabDopeSheet() :
	_db(0),
	_domType(0),
	_domRoot(DomCallback, this),
	_splitter(0),
	_wndDopeSheet(0),
	_wndForm(0),
	_resyncControls(false),
	_resyncSelection(false)
{
}

ScarabDopeSheet::~ScarabDopeSheet()
{
	delete _wndDopeSheet;
	delete _wndForm;
}

bool ScarabDopeSheet::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	return true;
}

LRESULT ScarabDopeSheet::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			if (ChildWindow::WndProc(msg, wParam, lParam) != 0)
				return -1;

			if ((_wndForm = new ScarabForm(new SnDSSfc(_form->FormContext()))) == 0)
				return -1;

			if ((_wndDopeSheet = new DopeSheetWnd(new SnDSContext(this))) == 0)
				return -1;

			if (!_wndForm->Create(*this))
				return -1;

			if (!_wndDopeSheet->Create(*this))
				return -1;

			_host.SetWindow(*this);

			_splitter = (DwSplit*)_host.Root()->SetChild(0, new DwSplit(&_host));
			_splitter->Create();
			_splitter->SetVertical(false);

			DwPane* pa0 = (DwPane*)_splitter->SetChild(0, new DwPane(&_host));
			DwPane* pa1 = (DwPane*)_splitter->SetChild(1, new DwPane(&_host));

			pa0->Add(new DwTab(&_host, *_wndDopeSheet));
			pa1->Add(new DwTab(&_host, *_wndForm));
		}
		return 0;

		case WM_DESTROY:
		{
			_host.SetWindow(0);

			delete _wndDopeSheet;
			delete _wndForm;
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);
			_host.Paint(ps.hdc, ps.rcPaint);
			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_SIZE:
		{
			WinRect rcClient(*this);

			_host.SetLocation(rcClient);
			_host.PerformLayout(true);
		}
		return 0;

		case WM_APP:
			SyncControls();
		return 0;

		case (WM_APP + 1):
			SyncSelection();
		return 0;
	}
	return ChildWindow::WndProc(msg, wParam, lParam);
}

void ScarabDopeSheet::SetForm(ScarabForm* form)
{
	_form = form;
}

void ScarabDopeSheet::SetDom(Xt::DomDb* db, LtStruct* rootType, Xt::DomBlock* rootDom)
{
	_db = db;
	_domType = rootType;

	Monitor(&_domRoot, rootDom);

	if (_wndDopeSheet)
		_wndDopeSheet->Diagram()->Root()->Clear();

	SyncControls();
	SyncSelection();
}

SnDsRow* ScarabDopeSheet::FindRow(XtId id, int index)
{
	if (!id)
		return 0;

	for(XtUt::LinkList<SnDsRow>::Iterator it(&_rows); it; ++it)
	{
		if ((it->_comId == id) && (it->_comIndex == index))
			return it;
	}

	return 0;
}

void ScarabDopeSheet::SyncControls()
{
	_resyncControls = false;

	if (Xt::DomBlock* root = AsBlock(_domRoot))
	{
		if (Xt::FieldDom fd = FindBySemantic(_domType, root, "DopeSheet", "length", "yes")) // HACK
		{
			float length = (float)atof(AsText(fd.dom));

			_wndDopeSheet->Diagram()->SetRange(0.0f, length);
		}
		else
			_wndDopeSheet->Diagram()->SetRange(0.0f, 1.0f);

		if (Xt::FieldDom fd = FindBySemantic(_domType, root, "DopeSheet", "quantisation", "yes")) // HACK
		{
			float q = (float)atof(AsText(fd.dom));

			_wndDopeSheet->Diagram()->SetQuantisation(true, 1.0f / q);
		}
		else
			_wndDopeSheet->Diagram()->SetQuantisation(false, 1.0f);
	}

	DoSnDSBuild(this, _db->schema, _wndDopeSheet->Diagram()->Root(), _domType, AsBlock(_domRoot));
}

void ScarabDopeSheet::SyncSelection()
{
	_resyncSelection = false;

	if (_wndForm)
	{
		SnDsRow* selectedRow = 0;
		SnDsKey* selectedKey = 0;

		for(XtUt::LinkList<DsRow>::Iterator it(_wndDopeSheet->Diagram()->SelectionRows()); it; ++it)
		{
			SnDsRow* row = (SnDsRow*)(DsRow*)it;

			if (!selectedRow)
				selectedRow = row;
			else
			{
				selectedRow = 0;
				break;
			}
		}

		for(XtUt::LinkList<DsKey>::Iterator it(_wndDopeSheet->Diagram()->SelectionKeys()); it; ++it)
		{
			SnDsKey* key = (SnDsKey*)(DsKey*)it;

			if (!selectedKey)
				selectedKey = key;
			else
			{
				selectedKey = 0;
				break;
			}
		}

		if (selectedRow && selectedKey)
		{
			selectedRow = 0;
			selectedKey = 0;
		}

		if (selectedRow)
		{
			if (Xt::DomComponent* component = AsComponent(selectedRow->_dom))
			{
				LarvaSchema* schema = _form->Db()->schema;

				_wndForm->SetDom(component, selectedRow->_lf);
			}
			else
				_wndForm->SetDom(0, (LarvaType*)0);
		}
		else if (selectedKey)
		{
			if (Xt::DomComponent* component = AsComponent(selectedKey->_dom))
			{
				LarvaSchema* schema = _form->Db()->schema;

				_wndForm->SetDom(component, selectedKey->_lf);
			}
			else
				_wndForm->SetDom(0, (LarvaType*)0);
		}
		else
			_wndForm->SetDom(0, (LarvaType*)0);
	}
}

void ScarabDopeSheet::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, ScarabDopeSheet* tl)
{
	if (*tl && !tl->_resyncControls)
	{
		tl->_resyncControls = true;

		PostMessage(*tl, WM_APP, 0, 0);
	}
}
#include "Core/XtWinPch.h"
#include "XtWin/WndCore.h"
#include "XtWin/Core.h"
#include "XtWin/NgMaths.h"
#include "XtWin/TreeCtrl.h"

#include <commctrl.h>

#define TC_EDIT_TIMER 1

TreeCtrl::TreeCtrl()
{
	_TreeData = 0;
	_ScrollX = 0;
	_ScrollY = 0;
	_TreeHeight = 0;
	_TreeWidth = 0;

	_FocusItem = 0;

	_IndentSize = 10;
	_ButtonSize = 8;
	_IconSize = 16;

	_Flags.HasIcons = false;
	_Flags.AllowMultipleSelection = true;
	_Flags.HighlightSelectedRow = false;
	_Flags.ToggleSelection = false;

	_IsFrozen = false;
	_CacheDirty = false;
	_BoundsDirty = false;

	_DragInfo.IsDragging = false;

	_DefaultFont = new Gdiplus::Font(Gdiplus::FontFamily::GenericSansSerif(), 8.0f, Gdiplus::FontStyleRegular);
	_DefaultBrush = new Gdiplus::SolidBrush(Gdiplus::Color(0, 0, 0));

	_OwnTreeData = false;
}

TreeCtrl::~TreeCtrl()
{
	delete _DefaultFont;
	delete _DefaultBrush;

	if (_OwnTreeData && _TreeData)
		delete _TreeData;
}

LRESULT TreeCtrl::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_MOUSEACTIVATE:
		SetFocus(*this);
		return MA_ACTIVATE;

	case WM_LBUTTONDBLCLK:
		LMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam, true);
		break;

	case WM_LBUTTONDOWN:
		LMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam, false);
		break;

	case WM_MBUTTONDOWN:
		MMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		break;

	case WM_RBUTTONDOWN:
		RMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		break;

	case WM_MBUTTONUP:
		MMouseUp(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		break;

	case WM_MOUSEMOVE:
		MouseMove(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		break;

	case WM_MOUSEWHEEL:
		{
			_ScrollY -= GET_WHEEL_DELTA_WPARAM(wParam)/6;
			_BoundsDirty = true;

			InvalidateWindow(_hwnd);
		}
		break;

	case WM_KEYDOWN:
		if (KeyDown(lParam, wParam))
			return 1;
		break;

	case WM_SIZE:
		_BoundsDirty = true;
		break;

	case WM_PAINT:
		if (!_IsFrozen)
			PaintWindow();
		break;

	case WM_HSCROLL:
		{
			switch (LOWORD(wParam))
			{
			case(SB_PAGEUP):
				{
					RECT rc;
					GetClientRect(_hwnd, &rc);
					_ScrollX -= (rc.right-rc.left);
				}
				break;

			case(SB_PAGEDOWN):
				{
					RECT rc;
					GetClientRect(_hwnd, &rc);
					_ScrollX += (rc.right-rc.left);
				}
				break;

			case(SB_LINEUP):
				_ScrollX -= 20;
				break;

			case(SB_LINEDOWN):
				_ScrollX += 20;
				break;

			case(SB_THUMBPOSITION):
			case(SB_THUMBTRACK):
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				GetScrollInfo(_hwnd, SB_HORZ, &si);
				_ScrollX = si.nTrackPos;
				break;
			}

			_BoundsDirty = true;

			InvalidateWindow(_hwnd);

			break;
		}

	case WM_VSCROLL:
		{
			switch (LOWORD(wParam))
			{
			case(SB_PAGEUP):
				{
					RECT rc;
					GetClientRect(_hwnd, &rc);
					_ScrollY -= (rc.bottom-rc.top);
				}
				break;

			case(SB_PAGEDOWN):
				{
					RECT rc;
					GetClientRect(_hwnd, &rc);
					_ScrollY += (rc.bottom-rc.top);
				}
				break;

			case(SB_LINEUP):
				_ScrollY -= 20;
				break;

			case(SB_LINEDOWN):
				_ScrollY += 20;
				break;

			case(SB_THUMBPOSITION):
			case(SB_THUMBTRACK):
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				GetScrollInfo(_hwnd, SB_VERT, &si);
				_ScrollY = si.nTrackPos;
				break;
			}

			_BoundsDirty = true;

			InvalidateWindow(_hwnd);

			break;
		}

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case (EN_UPDATE):
			{
				char tmp[512];
				if (Edit_GetText(_EditControl, tmp, 512))
				{
					HDC hdc = GetDC(_EditControl);
					TEXTMETRICW textMetric;
					SIZE sz;

					GetTextExtentPoint32A(hdc, tmp, strlen(tmp), &sz);
					
					RECT rc;
					GetClientRect(_hwnd, &rc);

					GetTextMetricsW(hdc, &textMetric);
					sz.cx += (textMetric.tmMaxCharWidth * 2);
					sz.cx = max(sz.cx, textMetric.tmMaxCharWidth * 3);
					sz.cx = min(sz.cx, rc.right-rc.left - GetItemOffsetX(_RenameItem));

					SetWindowPos(_EditControl, HWND_TOP, 0, 0, sz.cx, 18, SWP_NOMOVE | SWP_DRAWFRAME);

					ReleaseDC(_EditControl, hdc);
				}
			}
		}
		break;

	case WM_TIMER:
		KillTimer(_hwnd, TC_EDIT_TIMER);
		if (_FocusItem)
			StartRename(_FocusItem);
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}	

	return ChildWindow::WndProc(msg, wParam, lParam);
}

LRESULT TreeCtrl::EditWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KILLFOCUS:
		EndRename(true);
		return 1;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case (VK_ESCAPE):
			EndRename(false);
			return 1;
		case (VK_RETURN):
			EndRename(true);
			return 1;
		}
		break;
	case WM_MOUSEACTIVATE:
		return 1;
	case WM_MOUSEWHEEL:
		return 1;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	}

	return CallWindowProc(_EditWndProc, _EditControl, msg, wParam, lParam);
}

bool TreeCtrl::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style |= WS_HSCROLL | WS_VSCROLL;

	return true;
}

void TreeCtrl::PaintWindow()
{
	OnPrePaintWindow();

	PAINTSTRUCT ps;
	BeginPaint(_hwnd, &ps);

	{
		Gdiplus::Graphics g(ps.hdc);

		_DisplayRect = ps.rcPaint;

		Gdiplus::Bitmap bm(_DisplayRect.Width(), _DisplayRect.Height(), &g);

		{
			Gdiplus::Graphics g2(&bm);

			g2.Clear(Gdiplus::Color::White);

			g2.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
			g2.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			g2.SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);

			g2.TranslateTransform(-(float)_DisplayRect.left, -(float)_DisplayRect.top);

			WinRect rcClient(*this);

			DrawTree(&g2);
		}

		Gdiplus::CachedBitmap cbm(&bm, &g);

		g.DrawCachedBitmap(&cbm, _DisplayRect.left, _DisplayRect.top);
	}

	EndPaint(_hwnd, &ps);
}

void TreeCtrl::DrawTree(Gdiplus::Graphics* g)
{
	g->SetSmoothingMode(Gdiplus::SmoothingModeNone);

	if (!_TreeData || !_TreeData->GetNumChildren())
		return;

	RECT rc;
	GetClientRect(_hwnd, &rc);

	int height=0;
	int i=0;
	for (; i < _TreeData->GetNumChildren() && height <= _ScrollY;)
	{
		height += _TreeData->GetChild(i)->GetTotalHeight();
		i++;
	}

	i--;
	height -= _TreeData->GetChild(i)->GetTotalHeight();

	// TEMP
	int initialHeight = height;
	int finalHeight = initialHeight;
	int tempI;
	for (tempI = i; tempI < _TreeData->GetNumChildren() && finalHeight < _ScrollY + rc.bottom-rc.top; tempI++)
		finalHeight += _TreeData->GetChild(tempI)->GetTotalHeight();
	// Subtract the height of the last visible item
	if (tempI == _TreeData->GetNumChildren())
		tempI--;
	for (; tempI >= 0; tempI--)
	{
		finalHeight -= _TreeData->GetChild(tempI)->GetTotalHeight();

		if (!_TreeData->GetChild(tempI)->IsHidden())
			break;
	}
	Gdiplus::Pen dotted(Gdiplus::Color::Gray, 1);
	dotted.SetDashStyle(Gdiplus::DashStyleDot);
	g->DrawLine(&dotted, 5+GetButtonSize()/2-_ScrollX, initialHeight-_ScrollY+GetButtonSize(), 5+GetButtonSize()/2-_ScrollX, finalHeight-_ScrollY+GetButtonSize());
	// END TEMP

	do
	{
		_TreeData->GetChild(i)->Draw(g, 5-_ScrollX, height-_ScrollY);
		height += _TreeData->GetChild(i)->GetTotalHeight();
		i++;
	} while (height < _ScrollY + rc.bottom-rc.top && i < _TreeData->GetNumChildren());

	if (_DragInfo.IsDragging && _DragInfo.NewParent)
	{
		Gdiplus::SolidBrush transGrey(Gdiplus::Color(64, 128, 128, 128));
		if (_DragInfo.SourceType == kTcSourceItem)
		{
			int sourceOffset = GetItemOffsetY(_DragInfo.SourceItem)-_ScrollY;
			int sourceHeight = _DragInfo.SourceItem->GetItemHeight();
			g->FillRectangle(&transGrey, 0, sourceOffset, rc.right-rc.left, sourceHeight);
		} else if (_DragInfo.SourceType == kTcSourceSelection)
		{
			for (XtUt::Array<TreeItem*>::ConstIterator it(&_Selection); it; ++it)
			{
				int sourceOffset = GetItemOffsetY(*it)-_ScrollY;
				int sourceHeight = (*it)->GetItemHeight();
				g->FillRectangle(&transGrey, 0, sourceOffset, rc.right-rc.left, sourceHeight);
			}
		}

		if (_DragInfo.DestinationType == kTcDestinationBetween)
		{
			int itemOffset = GetItemOffsetY(_DragInfo.NewParent->GetChild(_DragInfo.NewIndex-1))-_ScrollY;
			int itemHeight = _DragInfo.NewParent->GetChild(_DragInfo.NewIndex-1)->GetItemHeight();

			g->DrawLine(&dotted, 0, itemOffset+itemHeight, rc.right-rc.left, itemOffset+itemHeight);
		} else if (_DragInfo.DestinationType == kTcDestinationOver)
		{
			int itemOffset = GetItemOffsetY(_DragInfo.NewParent)-_ScrollY;
			int itemHeight = _DragInfo.NewParent->GetItemHeight();

			Gdiplus::SolidBrush transBlue(Gdiplus::Color(128, 128, 128, 255));
			g->FillRectangle(&transBlue, 0, itemOffset, rc.right-rc.left, itemHeight);
			g->DrawRectangle(&dotted, 0, itemOffset, rc.right-rc.left, itemHeight);
		}
	}
}

void TreeCtrl::LMouseDown(const WinPoint &pt, WPARAM wParam, bool isDoubleClick)
{
	TcHitResult hit = HitTest(pt);

	if (hit.HitType != kTcHitNone)
	{
		if (hit.HitType == kTcHitRowButton)
		{
			hit.Item->SetExpanded(!hit.Item->IsExpanded());
		}
		else if (hit.HitType == kTcHitRowLabel || hit.HitType == kTcHitRowIcon || hit.HitType == kTcHitRowEmpty)
		{
			if (_Flags.ToggleSelection)
			{
				hit.Item->SetSelected(!hit.Item->IsSelected());
			}
			else
			{
				if (wParam & MK_SHIFT)
				{
					if (!_FocusStart)
						_FocusStart = hit.Item;

					if (_FocusItem)
						ClearSelectionRange(_FocusStart, _FocusItem, true);

					SetSelectionRange(_FocusStart, hit.Item);
				}
				else if (wParam & MK_CONTROL)
				{
					_FocusStart = hit.Item;
					hit.Item->SetSelected(!hit.Item->IsSelected());
				}
				else
				{
					if (hit.HitType == kTcHitRowLabel && _FocusItem == hit.Item)
						SetTimer(_hwnd, TC_EDIT_TIMER, GetDoubleClickTime(), 0);

					_FocusStart = hit.Item;
					SetSelection(hit.Item);
				}
			}

			_FocusItem = hit.Item;
			EnsureVisible(_FocusItem);
		}

		if (isDoubleClick && hit.HitType != kTcHitRowButton)
		{
			if (_FocusItem && !_FocusItem->OnActivate())
				_FocusItem->SetExpanded(!_FocusItem->IsExpanded());
		}
	} else {
		if (!(wParam & MK_SHIFT) && !(wParam & MK_CONTROL) && !_Flags.ToggleSelection)
			ClearSelection();
	}

	if (isDoubleClick)
		KillTimer(_hwnd, TC_EDIT_TIMER);

	InvalidateWindow(_hwnd);
}

void TreeCtrl::MMouseDown(const WinPoint &pt, WPARAM wParam)
{
	TcHitResult result = HitTest(pt);

	if (result.HitType != kTcHitRowIcon && result.HitType != kTcHitRowLabel)
		return;

	_DragInfo.SourceType = kTcSourceUnknown;
	_DragInfo.SourceItem = 0;
	_DragInfo.NewParent = 0;

	for (XtUt::Array<TreeItem*>::ConstIterator it(&_Selection); it; ++it)
	{
		if (result.Item == *it)
		{
			_DragInfo.SourceType = kTcSourceSelection;
			break;
		}
	}

	if (_DragInfo.SourceType == kTcSourceUnknown)
	{
		_DragInfo.SourceType = kTcSourceItem;
		_DragInfo.SourceItem = result.Item;
	}

	_DragInfo.IsDragging = true;
	_DragInfo.DestinationType = kTcDestinationUnknown;

	InvalidateWindow(_hwnd);
}

void TreeCtrl::RMouseDown(const WinPoint &pt, WPARAM wParam)
{
	TcHitResult hit = HitTest(pt);

	if (hit.HitType != kTcHitNone)
	{
		if (!hit.Item->IsSelected())
		{
			_FocusStart = _FocusItem = hit.Item;

			if (_Flags.ToggleSelection)
				hit.Item->SetSelected(true);
			else
				SetSelection(hit.Item);
		}
		else
		{
			_FocusItem = hit.Item;
		}		
	} else
		return;

	EnsureVisible(_FocusItem);

	InvalidateWindow(_hwnd);

	if (_FocusItem)
		_FocusItem->OnContextMenu(pt);
}

void TreeCtrl::DragReparent(TreeItem* source)
{
	if (_DragInfo.DestinationType == kTcDestinationBetween)
	{
		if (_DragInfo.NewParent != source->GetParent())
		{
			if (source->OnReparent(_DragInfo.NewParent))
				source->SetParent(_DragInfo.NewParent);
		}
		
		if (source->OnReorder(_DragInfo.NewIndex))
		{
			// TODO: Implement re-ordering
		}
	} else if (_DragInfo.DestinationType == kTcDestinationOver) {
		if (source->OnReparent(_DragInfo.NewParent))
				source->SetParent(_DragInfo.NewParent);
		source->OnReorder(0);
	}
}

void TreeCtrl::MMouseUp(const WinPoint &pt, WPARAM wParam)
{
	InvalidateWindow(_hwnd);

	if (!_DragInfo.IsDragging || !_DragInfo.NewParent)
		return;

	_DragInfo.IsDragging = false;
	
	if (_DragInfo.SourceType == kTcSourceItem && _DragInfo.NewParent != _DragInfo.SourceItem && !_DragInfo.NewParent->IsChildOf(_DragInfo.SourceItem))
	{
		DragReparent(_DragInfo.SourceItem);
	}
	else if (_DragInfo.SourceType == kTcSourceSelection)
	{
		bool validParent = true;
		for (XtUt::Array<TreeItem*>::ConstIterator it(&_Selection); it; ++it)
		{
			if (_DragInfo.NewParent->IsChildOf(*it))
			{
				validParent = false;
				break;
			}
		}

		if (!validParent)
			return;

		for (XtUt::Array<TreeItem*>::ConstIterator it(&_Selection); it; ++it)
		{
			DragReparent(*it);
		}
	}
}

void TreeCtrl::MouseMove(const WinPoint &pt, WPARAM wParam)
{
	if (_DragInfo.IsDragging && wParam == MK_MBUTTON)
	{
		_DragInfo.DestinationType = kTcDestinationUnknown;

		TcHitResult result = HitTest(pt);

		if (result.HitType == kTcHitNone)
		{
			_DragInfo.NewParent = 0;

			WinPoint tmpPt = pt;
			
			while (tmpPt.y >= 0 && result.HitType == kTcHitNone)
			{
				result = HitTest(tmpPt);
				tmpPt.y--;
			}

			if (result.HitType != kTcHitNone)
			{
				_DragInfo.NewParent = result.Item->GetParent();
				_DragInfo.NewIndex = result.Item->GetIndex()+1;
				_DragInfo.DestinationType = kTcDestinationBetween;
			}
			
		} else {
			_DragInfo.DestinationType = kTcDestinationOver;
			_DragInfo.NewParent = result.Item;
		}

		InvalidateWindow(_hwnd);
	}
}

bool TreeCtrl::KeyDown(LPARAM lParam, WPARAM wParam)
{
	if (_FocusItem == 0)
		return false;

	TreeItem* oldFocusItem = _FocusItem;

	switch (wParam)
	{
	case VK_UP:
		while (_FocusItem->_GetPrev())
		{
			_FocusItem = _FocusItem->_GetPrev();
			if (_FocusItem->IsVisible())
				break;
		}
		if (!_FocusItem->IsVisible() || !_FocusItem->GetParent())
			_FocusItem = oldFocusItem;
		break;

	case VK_DOWN:
		while (_FocusItem->_GetNext())
		{
			_FocusItem = _FocusItem->_GetNext();
			if (_FocusItem->IsVisible())
				break;
		}
		if (!_FocusItem->IsVisible() || !_FocusItem->GetParent())
			_FocusItem = oldFocusItem;
		break;

	case VK_PRIOR:
		{
			RECT rc;
			GetClientRect(_hwnd, &rc);
			int windowHeight = rc.bottom-rc.top-30;
			int skippedHeight = 0;
			TreeItem* farthestVisible = oldFocusItem;
			while ((skippedHeight < windowHeight || !_FocusItem->IsVisible()) && _FocusItem->_GetPrev())
			{
				if (_FocusItem->IsVisible())
				{
					skippedHeight += _FocusItem->GetItemHeight();
					farthestVisible = _FocusItem;
				}
				
				_FocusItem = _FocusItem->_GetPrev();
			}
			if (!_FocusItem->IsVisible() || !_FocusItem->GetParent())
				_FocusItem = farthestVisible;
		}
		break;

	case VK_NEXT:
		{
			RECT rc;
			GetClientRect(_hwnd, &rc);
			int windowHeight = rc.bottom-rc.top-30;
			int skippedHeight = 0;
			TreeItem* farthestVisible = oldFocusItem;
			while ((skippedHeight < windowHeight || !_FocusItem->IsVisible()) && _FocusItem->_GetNext())
			{
				if (_FocusItem->IsVisible())
				{
					skippedHeight += _FocusItem->GetItemHeight();
					farthestVisible = _FocusItem;
				}
				
				_FocusItem = _FocusItem->_GetNext();
			}
			if (!_FocusItem->IsVisible() || !_FocusItem->GetParent())
				_FocusItem = farthestVisible;
		}
		break;

	case VK_LEFT:
		if (_FocusItem->IsExpanded())
			_FocusItem->SetExpanded(false);
		else if (_FocusItem->GetParent() && _FocusItem->GetParent()->GetParent())
			_FocusItem = _FocusItem->GetParent();
		break;

	case VK_RIGHT:
		if (!_FocusItem->IsExpanded())
			_FocusItem->SetExpanded(true);
		else if (_FocusItem->GetChild(0))
			_FocusItem = _FocusItem->GetChild(0);
		break;

	case VK_HOME:
		if (_TreeData->GetNumChildren())
			_FocusItem = _TreeData->GetChild(0);
		else
			_FocusItem = 0;

		break;

	case VK_END:
		{
			TreeItem* lastChild = _TreeData->GetLastChild();

			while(1)
			{
				if (lastChild && lastChild->GetNumChildren())
					lastChild = lastChild->GetLastChild();
				else
					break;
			}

			while(lastChild && !lastChild->IsVisible())
				lastChild = lastChild->_GetPrev();

			_FocusItem = lastChild;
		}

		break;

	case VK_SPACE:
	case VK_MULTIPLY:
	case VK_ADD:
	case VK_SUBTRACT:
	case VK_DELETE:
	case VK_F2:
		break;

	default:
		return false;
	}

	switch (wParam)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
		EnsureVisible(_FocusItem);
		if (!_FocusStart)
			_FocusStart = _FocusItem;

		if (!_Flags.ToggleSelection)
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				ClearSelectionRange(_FocusStart, oldFocusItem, true);
				SetSelectionRange(_FocusStart, _FocusItem);
			}
			else if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				_FocusStart = _FocusItem;
			}
			else
			{
				_FocusStart = _FocusItem;
				SetSelection(_FocusItem);
			}
		}

		break;

	case VK_SPACE:
		_FocusItem->SetSelected(!_FocusItem->IsSelected());
		OnSelectionChanged(_Selection, kTcSelectionToggle);
		break;

	case VK_MULTIPLY:
		_FocusItem->SetExpanded(true, true);
		break;

	case VK_ADD:
		_FocusItem->SetExpanded(true);
		break;

	case VK_SUBTRACT:
		_FocusItem->SetExpanded(false);
		break;

	case VK_DELETE:
		{
			if (!OnRemoveItems(_Selection))
				return true;

			size_t skip = 0;
			while (_Selection.count > skip)
			{
				if (_Selection[skip]->GetParent())
				{
					if (!_Selection[skip]->GetParent()->RemoveChild(_Selection[skip], true, false))
						skip++;
				}
				else
					skip++;
			}
		}
		break;

	case VK_F2:
		StartRename(_FocusItem);
	}

	InvalidateWindow(_hwnd);

	return true;
}

void TreeCtrl::UpdateBounds()
{
	if (_IsFrozen)
		return;

	if (!_BoundsDirty && !_CacheDirty)
		return;

	_BoundsDirty = false;

	if (!_TreeData)
	{
		_TreeWidth = 0;
		_TreeHeight = 0;
	}

	// Calculate tree data size
	if (_CacheDirty && _TreeData)
	{
		_TreeHeight = _TreeData->GetTotalHeight();
		_TreeWidth = _TreeData->GetMaxWidth();
		_CacheDirty = false;
	}

	RECT rc;
	GetClientRect(_hwnd, &rc);

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);

	si.nMin = 0;
	si.nMax = _TreeHeight;
	si.fMask = SIF_RANGE;	
	SetScrollInfo(_hwnd, SB_VERT, &si, true);

	si.nMin = 0;
	si.nMax = _TreeWidth;
	si.fMask = SIF_RANGE;
	SetScrollInfo(_hwnd, SB_HORZ, &si, true);

	si.nPage = rc.bottom-rc.top;
	si.fMask = SIF_PAGE;
	SetScrollInfo(_hwnd, SB_VERT, &si, true);

	si.nPage = rc.right-rc.left;
	si.fMask = SIF_PAGE;
	SetScrollInfo(_hwnd, SB_HORZ, &si, true);

	if (_ScrollY < 0)
		_ScrollY = 0;

	int maxScrollY = max(0,_TreeHeight - (rc.bottom-rc.top));
	if (_ScrollY > maxScrollY)
		_ScrollY = maxScrollY;

	if (_ScrollX < 0)
		_ScrollX = 0;

	int maxScrollX = _TreeWidth;
	if (_ScrollX > maxScrollX)
		_ScrollX = maxScrollX;

	si.fMask = SIF_POS;
	si.nPos = _ScrollX;
	SetScrollInfo(_hwnd, SB_HORZ, &si, true);

	si.fMask = SIF_POS;
	si.nPos = _ScrollY;
	SetScrollInfo(_hwnd, SB_VERT, &si, true);
}

const XtUt::Array<TreeItem*>& TreeCtrl::GetSelection() const
{
	return _Selection;
}

void TreeCtrl::ClearSelection(bool supressEvent)
{
	while (_Selection.count)
		_Selection[0]->SetSelected(false, true);

	if (!supressEvent)
		OnSelectionChanged(_Selection, kTcSelectionClean);
}

void TreeCtrl::ClearSelectionRange(TreeItem* start, TreeItem* end, bool supressEvent)
{
	bool found = false;

	TreeItem* current = start;
	while (current)
	{
		if (current == end)
		{
			found = true;
			break;
		}
		current = current->_GetNext();
	}

	if (found)
	{
		current = start;
		while (current && current != end)
		{
			if (current->IsVisible())
				current->SetSelected(false);
			current = current->_GetNext();
		}
		end->SetSelected(false);
	} else
	{
		current = start;
		while (current && current != end)
		{
			if (current->IsVisible())
				current->SetSelected(false);
			current = current->_GetPrev();
		}
		end->SetSelected(false);
	}

	if (!supressEvent)
		OnSelectionChanged(_Selection, kTcSelectionClean);
}

void TreeCtrl::SetSelection(TreeItem* item)
{
	ClearSelection(true);
	
	item->SetSelected(true);

	OnSelectionChanged(_Selection, kTcSelectionClean);
}

void TreeCtrl::SetSelection(const XtUt::Array<TreeItem*>& selection)
{
	ClearSelection(true);

	for (size_t i=0; i<selection.count; i++)
		selection[i]->SetSelected(true);

	OnSelectionChanged(_Selection, kTcSelectionClean);
}

void TreeCtrl::SetSelectionRange(TreeItem* start, TreeItem* end)
{
	bool found = false;

	TreeItem* current = start;
	while (current)
	{
		if (current == end)
		{
			found = true;
			break;
		}

		current = current->_GetNext();
	}

	if (found)
	{
		current = start;
		while (current && current != end)
		{
			if (current->IsVisible())
				current->SetSelected(true);
			current = current->_GetNext();
		}
		end->SetSelected(true);
	} else
	{
		current = start;
		while (current && current != end)
		{
			if (current->IsVisible())
				current->SetSelected(true);
			current = current->_GetPrev();
		}
		end->SetSelected(true);
	}

	OnSelectionChanged(_Selection, kTcSelectionToggle);
}

TreeCtrl::TreeCtrlFlags& TreeCtrl::GetFlags()
{
	return _Flags;
}

const TreeCtrl::TreeCtrlFlags& TreeCtrl::GetFlags() const
{
	return _Flags;
}

void TreeCtrl::SetRootNode(TreeItem* data, bool takeOwnership)
{
	Freeze();

	ClearSelection(true);

	if (_OwnTreeData && _TreeData)
		delete _TreeData;

	_TreeData = data;
	_OwnTreeData = takeOwnership;

	if (_TreeData)
	{
		_TreeData->SetTreeCtrl(this);
		_TreeData->SetExpanded(true);
	}

	Thaw();
}

TreeItem* TreeCtrl::GetRootNode()
{
	return _TreeData;
}

const TreeItem* TreeCtrl::GetRootNode() const
{
	return _TreeData;
}

int TreeCtrl::GetIndentSize() const
{
	return _IndentSize;
}

void TreeCtrl::SetIndentSize(int indentSize)
{
	_IndentSize = indentSize;
}

int TreeCtrl::GetButtonSize() const
{
	return _ButtonSize;
}

void TreeCtrl::SetButtonSize(int buttonSize)
{
	_ButtonSize = buttonSize;
}

int TreeCtrl::GetIconSize() const
{
	return _IconSize;
}

void TreeCtrl::SetIconSize(int iconSize)
{
	_IconSize = iconSize;
}

bool TreeCtrl::IsSorted() const
{
	return _IsSorted;
}

void TreeCtrl::SetSorted(bool isSorted)
{
	_IsSorted = isSorted;
}

void TreeCtrl::AddToSelection(TreeItem* item, int index)
{
	if (_Flags.AllowMultipleSelection || !_Selection.count)
		AppendEx(&_Selection, item);
	else
		item->SetSelected(false, true);

	InvalidateWindow(_hwnd);
}

void TreeCtrl::RemoveFromSelection(TreeItem* item)
{
	for (size_t i=0; i<_Selection.count; i++)
	{
		if (_Selection[i] == item)
		{
			Remove(&_Selection, i);
			InvalidateWindow(_hwnd);
			return;
		}
	}
}

void TreeCtrl::OnPrePaintWindow()
{
	UpdateBounds();
}

void TreeCtrl::OnAddItem(TreeItem* item)
{
	_CacheDirty = true;

	InvalidateWindow(_hwnd);
}

void TreeCtrl::OnRemoveItem(TreeItem* item)
{
	_CacheDirty = true;

	if (item == _RenameItem)
		EndRename(false);

	for (int i=0; i<item->GetNumChildren(); i++)
	{
		OnRemoveItem(item->GetChild(i));
	}

	for (size_t i=0; i<_Selection.count; i++)
	{
		if (_Selection[i] == item)
		{
			XtUt::Remove(&_Selection, i);
			break;
		}
	}

	if (_FocusItem == item)
		_FocusItem = 0;

	if (_FocusStart == item)
		_FocusStart = 0;

	_BoundsDirty = true;

	InvalidateWindow(_hwnd);
}

bool TreeCtrl::OnRemoveItems(const XtUt::Array<TreeItem*>& items)
{
	return true;
}

void TreeCtrl::OnHideItem(TreeItem* item)
{
	_CacheDirty = true;

	InvalidateWindow(_hwnd);
}

void TreeCtrl::OnLabelChanged(TreeItem* item)
{
	_CacheDirty = true;

	InvalidateWindow(_hwnd);
}

bool TreeCtrl::OnCompareItems(TreeItem* a, TreeItem* b)
{
	return a->GetLabel() < b->GetLabel();
}

int TreeCtrl::LoadIcon(const Sz8& location, int id)
{
	if (id==-1)
	{
		do
		{
			id = rand();
		} while(At(&_Icons, id) != 0);
	}

	Gdiplus::Bitmap* image = new Gdiplus::Bitmap(MbToWc(location).Text());

	Insert(&_Icons, id, image);

	return id;
}

void TreeCtrl::RemoveIcon(int iconId)
{
	//Remove(&_Icons, iconId);
}

Gdiplus::Bitmap* TreeCtrl::GetIconImage(int iconId)
{
	return At(&_Icons, iconId);
}

const TreeItem* TreeCtrl::GetFocusItem() const
{
	return _FocusItem;
}

TcHitResult TreeCtrl::HitTest(const WinPoint& pt)
{
	TcHitResult hit;
	hit.HitType = kTcHitNone;
	hit.Item = 0;

	if (!_TreeData || !_TreeData->GetNumChildren())
		return hit;

	RECT rc;
	GetClientRect(_hwnd, &rc);

	int height=0;
	int i=0;

	do
	{
		height += _TreeData->GetChild(i)->GetTotalHeight();
		i++;
	} while (height < _ScrollY && i < _TreeData->GetNumChildren());

	i--;
	height -= _TreeData->GetChild(i)->GetTotalHeight();

	do
	{
		WinPoint offsetPt;
		offsetPt.x = pt.x + _ScrollX - 5;
		offsetPt.y = pt.y - height + _ScrollY;
		hit = _TreeData->GetChild(i)->HitTest(offsetPt);

		if (hit.HitType != kTcHitNone)
			break;

		height += _TreeData->GetChild(i)->GetTotalHeight();
		i++;
	} while (height < _ScrollY + rc.bottom-rc.top && i < _TreeData->GetNumChildren());

	return hit;
}

bool TreeCtrl::RemoveItem(TreeItem* item)
{
	TreeItem* parent = item->GetParent();
	if (parent)
		return parent->RemoveChild(item);

	return false;
}

void TreeCtrl::Freeze()
{
	_IsFrozen++;
}

void TreeCtrl::Thaw()
{
	if (_IsFrozen == 0)
		return;
	else
	{
		_IsFrozen--;

		if (_IsFrozen == 0)
			InvalidateWindow(_hwnd);
	}
}

LRESULT WINAPI EditWndProcHandler(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	TreeCtrl* treeCtrl = (TreeCtrl*)GetProp(hWnd, "TreeCtrl");
	return treeCtrl->EditWndProc(message, wParam, lParam);	
}

void TreeCtrl::StartRename(TreeItem* item)
{
	if (!item)
		return;

	Sz16 startName = item->GetLabel();

	if (!item->OnStartRename(startName))
		return;

	_RenameItem = item;

	EnsureVisible(item);

	int itemOffsetX = GetItemOffsetX(item);

	HINSTANCE instance = GetModuleHandle(0);
	_EditControl = CreateWindow("EDIT", WcToMb(startName).Text(),
                        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
						GetItemOffsetX(item)+GetIndentSize()+(GetButtonSize()/2)+4, GetItemOffsetY(item)-_ScrollY-2, 80, 18,
                        _hwnd,
                        NULL,
                        instance,
                        NULL);

	SetProp(_EditControl, "TreeCtrl", (HANDLE)this);
	
	_EditWndProc = (WNDPROC)SetWindowLong(_EditControl, GWL_WNDPROC, (DWORD)EditWndProcHandler);

	SetFocus(_EditControl);

	HFONT hFont=CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, \
		  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, \
		  DEFAULT_PITCH | FF_SWISS, "Arial");
	SendMessage(_EditControl, WM_SETFONT, (WPARAM)hFont, true);

	HDC hdc = GetDC(_EditControl);
	TEXTMETRICW textMetric;
	SIZE sz;

	GetTextExtentPoint32W(hdc, item->GetLabel().Text(), item->GetLabel().Length(), &sz);

	RECT rc;
	GetClientRect(_hwnd, &rc);

	GetTextMetricsW(hdc, &textMetric);
	sz.cx += (textMetric.tmMaxCharWidth * 2);
	sz.cx = max(sz.cx, textMetric.tmMaxCharWidth * 3);
	sz.cx = min(sz.cx, rc.right-rc.left - GetItemOffsetX(item));

	SetWindowPos(_EditControl, HWND_TOP, 0, 0, sz.cx, 18, SWP_NOMOVE | SWP_DRAWFRAME);

	Edit_SetSel(_EditControl, 0, item->GetLabel().Length());

	ReleaseDC(_EditControl, hdc);
}

void TreeCtrl::EndRename(bool apply)
{
	Sz16 newName;
	char tmp[512];

	if (apply)
	{
		if (Edit_GetText(_EditControl, tmp, 512))
			newName = MbToWc(tmp);

		if (_RenameItem->OnEndRename(newName))
		{
			_RenameItem->SetLabel(newName);
		}
	}

	SetWindowLong(_EditControl, GWL_WNDPROC, (DWORD) _EditWndProc);
	DestroyWindow(_EditControl);
	_EditControl = 0;
	_EditWndProc = 0;
}

Gdiplus::SolidBrush* TreeCtrl::GetDefaultBrush()
{
	return _DefaultBrush;
}

Gdiplus::Font* TreeCtrl::GetDefaultFont()
{
	return _DefaultFont;
}

WinRect& TreeCtrl::GetDisplayRect()
{
	return _DisplayRect;
}

void TreeCtrl::EnsureVisible(TreeItem* item)
{
	TreeItem* vis = item->GetParent();
	while (vis)
	{
		vis->SetExpanded(true);
		vis = vis->GetParent();
	}

	int offset = GetItemOffsetY(item);

	RECT rc;
	GetClientRect(_hwnd, &rc);

	if (offset < _ScrollY)
		_ScrollY = offset;

	if (offset > _ScrollY + (rc.bottom-rc.top) - item->GetItemHeight())
		_ScrollY = offset - (rc.bottom-rc.top) + item->GetItemHeight();

	_BoundsDirty = true;

}

int TreeCtrl::GetItemOffsetX(TreeItem* item)
{
	int offset = 0;

	TreeItem* itemParent = item->GetParent();
	if (itemParent)
		offset += GetItemOffsetXInternal(itemParent);

	offset += GetButtonSize()/2+GetIndentSize()+8;

	return offset;
}

int TreeCtrl::GetItemOffsetXInternal(TreeItem* item)
{
	TreeItem* itemParent = item->GetParent();
	if (itemParent)
		return GetItemOffsetXInternal(itemParent) + GetIndentSize() + (_Flags.HasIcons ? GetIconSize() : 0);
	else
		return 0;
}

int TreeCtrl::GetItemOffsetY(TreeItem* item)
{
	TreeItem* itemParent = item->GetParent();
	if (itemParent && itemParent->IsVisible())
	{
		int offset = GetItemOffsetY(itemParent);

		offset += itemParent->GetItemHeight();

		for (int i=0; i<itemParent->GetNumChildren(); i++)
		{
			TreeItem* child = itemParent->GetChild(i);
			
			if (child == item)
				break;

			offset += child->GetTotalHeight();
		}
		
		return offset;
	} else {
		return -item->GetItemHeight();
	}
}

TreeItem::TreeItem() :
_MinItemWidth(5)
{
	_Label = MbToWc("");
	_IsHidden = false;
	_IsExpanded = false;
	_IsSelected = false;
	_TotalHeight = 0;
	_TotalHeightDirty = true;
	_TreeCtrl = 0;
	_Parent = 0;
	_IconId = -1;
}

TreeItem::~TreeItem()
{
	if (_Parent)
		_Parent->RemoveChild(this, false, true);

	while (_Children.count)
		RemoveChild(_Children[0], true, true);
}

const TreeItem* TreeItem::GetParent() const
{
	return _Parent;
}

TreeItem* TreeItem::GetParent()
{
	return _Parent;
}

void TreeItem::SetParent(TreeItem* parent)
{
	if (_Parent == parent)
		return;

	if (_Parent)
		_Parent->UnlinkChild(this);

	if (parent)
		parent->AddChild(this);

	_Parent = parent;
}

int TreeItem::GetNumChildren() const
{
	return _Children.count;
}

TreeItem* TreeItem::GetChild(int index)
{
	if ((size_t)index >= _Children.count)
		return 0;

	return _Children[index];
}

void TreeItem::AddChild(TreeItem* child, TreeItem* before)
{
	if (before != 0)
		assert(!"Not Currently supported");

	AppendEx(&_Children, child);
	child->SetTreeCtrl(_TreeCtrl);
	child->_Parent = this;
	
	if (_TreeCtrl)
		_TreeCtrl->OnAddItem(child);

	child->_Index = _Children.count-1;
}

bool TreeItem::UnlinkChild(TreeItem* child)
{
	if (child->_Parent != this)
		return false;

	child->_Parent = 0;
	XtUt::Remove(&_Children, child->GetIndex());
	ResetChildIndices();

	return true;
}

bool TreeItem::RemoveChild(TreeItem* child, bool freeMemory, bool force, bool skipEvents)
{
	for (size_t i=0; i<_Children.count; i++)
	{
		if (_Children[i] == child)
		{
			if ((!skipEvents && !child->OnItemDeleting()) && !force)
				return false;

			child->_Parent = 0;
			child->SetSelected(false);

			_TreeCtrl->OnRemoveItem(child);

			XtUt::Remove(&_Children, i);

			ResetChildIndices();

			if (freeMemory)
			{
				if (!skipEvents)
					child->OnItemDeleted();
				delete child;
			}

			return true;
		}
	}

	return false;
}

void TreeItem::SetIcon(int iconId)
{
	_IconId = iconId;
}

void TreeItem::SetColour(int r, int g, int b)
{
	_Colour.SetValue(Gdiplus::Color(r, g, b).GetValue());
}

const Sz16& TreeItem::GetLabel() const
{
	return _Label;
}

void TreeItem::SetLabel(const Sz16& label)
{
	_Label = label;

	if (_TreeCtrl)
		_TreeCtrl->OnLabelChanged(this);
}

void TreeItem::SetLabel(const XtString& label)
{
	_Label = MbToWc(label);

	if (_TreeCtrl)
		_TreeCtrl->OnLabelChanged(this);
}

bool TreeItem::IsSelected() const
{
	return _IsSelected;
}

void TreeItem::SetSelected(bool isSelected, bool force)
{
	if (_IsSelected != isSelected && OnItemSelectionChanged(isSelected) && _Parent || force)
	{
		_IsSelected = isSelected;

		if (isSelected)
			_TreeCtrl->AddToSelection(this);
		else
			_TreeCtrl->RemoveFromSelection(this);
	}
}

bool TreeItem::IsHidden() const
{
	return _IsHidden;
}

void TreeItem::SetHidden(bool isHidden)
{
	_IsHidden = isHidden;

	if (_TreeCtrl)
		_TreeCtrl->OnHideItem(this);
}

bool TreeItem::IsExpanded() const
{
	return _Children.count && _IsExpanded;
}

void TreeItem::SetExpanded(bool isExpanded, bool recursive)
{
	if (_TreeCtrl)
		_TreeCtrl->_CacheDirty = true;

	_IsExpanded = isExpanded;

	if (recursive)
	{
		for (size_t i=0; i<_Children.count; i++)
			_Children[i]->SetExpanded(isExpanded, recursive);
	}
}

bool TreeItem::IsVisible() const
{
	if (_Parent && !_Parent->IsExpanded())
		return false;

	if (_Parent && !_Parent->IsVisible())
		return false;

	if (IsHidden())
		return false;

	return true;
}

int TreeItem::GetItemHeight()
{
	if (IsHidden())
		return 0;
	
	return 16;
}

int TreeItem::GetItemWidth()
{
	HDC hdc = GetDC(_TreeCtrl->_hwnd);
	Gdiplus::Graphics g(hdc);
	Gdiplus::RectF rect;
	g.MeasureString(_Label, _Label.Length(), _TreeCtrl->GetDefaultFont(), Gdiplus::PointF((Gdiplus::REAL)_TreeCtrl->GetIndentSize()+(_TreeCtrl->GetButtonSize()/2)+4+(_TreeCtrl->_Flags.HasIcons?_TreeCtrl->GetIconSize():0), 0), 0, &rect);
	rect.Width = max(_MinItemWidth, rect.Width);
	ReleaseDC(_TreeCtrl->_hwnd, hdc);

	return (int)rect.Width + _TreeCtrl->GetIndentSize() + (_TreeCtrl->GetButtonSize()/2) + 4 + (_TreeCtrl->_Flags.HasIcons?_TreeCtrl->GetIconSize():0);
}

int TreeItem::GetTotalHeight()
{
	if (_TotalHeightDirty)
	{
		_TotalHeight = GetItemHeight();

		if (IsExpanded() && !IsHidden())
		{
			for (size_t i=0; i<_Children.count; i++)
				_TotalHeight += _Children[i]->GetTotalHeight();
		}

		//_TotalHeightDirty = false;
	}

	return _TotalHeight;
}

int TreeItem::GetMaxWidth()
{
	if (IsHidden())
		return 0;

	int maxWidth = GetItemWidth();

	if (IsExpanded())
	{
		for (size_t i=0; i<_Children.count; i++)
		{
			int childWidth = (_TreeCtrl->_Flags.HasIcons?_TreeCtrl->GetIconSize():0) + _TreeCtrl->GetIndentSize() + _Children[i]->GetMaxWidth();
			if (childWidth > maxWidth)
				maxWidth = childWidth;
		}
	}

	return maxWidth;
}

int TreeItem::GetIndex()
{
	return _Index;
}

TreeItem* TreeItem::GetPrevSibling()
{
	if (!_Parent)
		return 0;
	else if (_Index-1 >= 0)
		return _Parent->GetChild(_Index-1);
	else
		return 0;
}

TreeItem* TreeItem::GetNextSibling()
{
	if (!_Parent)
		return 0;
	else if (_Index+1 < _Parent->GetNumChildren())
		return _Parent->GetChild(_Index+1);
	else
		return 0;
}

TreeItem* TreeItem::_GetNext()
{
	if (_Children.count)
		return _Children[0];
	else if (_Parent)
	{
		if (_Index+1 < _Parent->GetNumChildren())
			return _Parent->GetChild(_Index+1);
		else
		{
			TreeItem* parent = _Parent;
			TreeItem* parentSibling = 0;
			while (parent && !parentSibling)
			{
				parentSibling = parent->GetNextSibling();
				parent = parent->GetParent();
			}

			return parentSibling;
		}
	}
	else
		return 0;
}

TreeItem* TreeItem::_GetPrev()
{
	if (_Parent)
	{
		if (_Index-1 >= 0)
		{
			TreeItem* lastChild = _Parent->GetChild(_Index-1);

			while(1)
			{
				if (lastChild->GetNumChildren())
					lastChild = lastChild->GetLastChild();
				else
					break;
			}

			return lastChild;
		}
		else
			return _Parent;
	}
	else
		return 0;
}

void TreeItem::SetTreeCtrl(TreeCtrl* tree)
{
	_TreeCtrl = tree;

	for (size_t i=0; i<_Children.count; i++)
		_Children[i]->SetTreeCtrl(tree);
}

class TreeItemSortPredicate
{
public:
	TreeItemSortPredicate(TreeCtrl* treeCtrl) { _TreeCtrl = treeCtrl; }
	bool operator()(TreeItem* a, TreeItem* b) { return _TreeCtrl->OnCompareItems(a, b); }

private:
	TreeCtrl* _TreeCtrl;
};

void TreeItem::SortChildren(bool recursive)
{
	if (!_Children.count)
		return;

	TreeItemSortPredicate predicate(_TreeCtrl);

	XtUt::MergeSort(&_Children, predicate);

	ResetChildIndices();

	if (recursive)
	{
		for (size_t i=0; i<_Children.count; i++)
		{
			_Children[i]->SortChildren(true);
		}
	}
}

void TreeItem::ResetChildIndices()
{
	for (size_t i=0; i<_Children.count; i++)
	{
		_Children[i]->_Index = i;
	}
}

void TreeItem::Draw(Gdiplus::Graphics* g, int offsetX, int offsetY)
{
	Gdiplus::Font* font = _TreeCtrl->GetDefaultFont();
	Gdiplus::SolidBrush* fontBrush = _TreeCtrl->GetDefaultBrush();
	
	if (IsHidden())
		return;

	int bs = _TreeCtrl->GetButtonSize();
	int hbs = bs/2;

	int ics = _TreeCtrl->GetIconSize();

	int is;
	if (_TreeCtrl->GetFlags().HasIcons)
		is = _TreeCtrl->GetIndentSize() + ics;
	else
		is = _TreeCtrl->GetIndentSize();

	Gdiplus::Pen dotted(Gdiplus::Color::Gray, 1);
	dotted.SetDashStyle(Gdiplus::DashStyleDot);

	Gdiplus::Pen p(Gdiplus::Color::Black, 1);

	Gdiplus::Pen buttonPen(Gdiplus::Color::Gray, 1);
	Gdiplus::LinearGradientBrush buttonBrush(Gdiplus::RectF((Gdiplus::REAL)offsetX, (Gdiplus::REAL)offsetY+2, (Gdiplus::REAL)bs, (Gdiplus::REAL)bs), Gdiplus::Color::White, Gdiplus::Color::LightGray, Gdiplus::LinearGradientModeForwardDiagonal);

	bool hasVisibleChildren = false;

	for (size_t i=0; i<_Children.count; i++)
	{
		if (!_Children[i]->IsHidden())
		{
			hasVisibleChildren = true;
			break;
		}
	}

	// If we have children, draw the expansion button
	if (hasVisibleChildren)
	{
		g->DrawLine(&dotted, offsetX+bs, offsetY+2+hbs, offsetX+is+bs, offsetY+2+hbs);

		g->FillRectangle(&buttonBrush, offsetX, offsetY+2, bs, bs);

		g->DrawRectangle(&buttonPen, offsetX, offsetY+2, bs, bs);

		g->DrawLine(&p, offsetX+2, offsetY+2+hbs, offsetX+bs-2, offsetY+2+hbs);

		if (!IsExpanded())
			g->DrawLine(&p, offsetX+hbs, offsetY+4, offsetX+hbs, offsetY+bs);
	} else {
		g->DrawLine(&dotted, offsetX+4, offsetY+2+hbs, offsetX+is+bs, offsetY+2+hbs);
	}

	fontBrush->SetColor(_Colour);
	g->DrawString(_Label, _Label.Length(), font, Gdiplus::PointF((Gdiplus::REAL)offsetX+is+hbs+4, (Gdiplus::REAL)offsetY), 0, fontBrush);
	
	// If this is the focus item, draw the marquee
	if (this == _TreeCtrl->GetFocusItem())
	{
		Gdiplus::RectF rect;
		g->MeasureString(_Label, _Label.Length(), font, Gdiplus::PointF((Gdiplus::REAL)offsetX+is+hbs+4, (Gdiplus::REAL)offsetY), 0, &rect);
		rect.Width = max(_MinItemWidth, rect.Width);
		g->DrawRectangle(&dotted, rect);
	}

	// If the item is selected, draw the coloured selection rect
	if (IsSelected())
	{
		Gdiplus::RectF rect;

		if (_TreeCtrl->_Flags.HighlightSelectedRow)
		{
			rect.X = 0;
			rect.Y = (Gdiplus::REAL)offsetY;
			rect.Height = (Gdiplus::REAL)this->GetItemHeight();
			rect.Width = (Gdiplus::REAL)_TreeCtrl->GetDisplayRect().Width();
		} else {
			g->MeasureString(_Label, _Label.Length(), font, Gdiplus::PointF((Gdiplus::REAL)offsetX+is+hbs+4, (Gdiplus::REAL)offsetY), 0, &rect);
			rect.Width = max(_MinItemWidth, rect.Width);
		}
		
		Gdiplus::SolidBrush selBrush(Gdiplus::Color(80,0,0,255));
		g->FillRectangle(&selBrush, rect);
	}

	if (hasVisibleChildren && IsExpanded())
	{
		// Draw the verticle line connecting the children
		int initialOffsetY = offsetY;
		int finalOffsetY = initialOffsetY + GetItemHeight();

		for (size_t i=0; i<_Children.count; i++)
			finalOffsetY += _Children[i]->GetTotalHeight();

		// Subtract the height of the last visible item
		for (size_t i=_Children.count-1; i >= 0; i--)
		{
			finalOffsetY -= _Children[i]->GetTotalHeight();

			if (!_Children[i]->IsHidden())
				break;
		}

		g->DrawLine(&dotted, offsetX+is+4, initialOffsetY+2+hbs, offsetX+is+4, finalOffsetY+2+hbs);

		// Draw the children
		int tempOffsetY = offsetY + GetItemHeight();

		for (size_t i=0; i<_Children.count; i++)
		{
			_Children[i]->Draw(g, offsetX+is, tempOffsetY);
			tempOffsetY += _Children[i]->GetTotalHeight();
		}
	}	

	if (_TreeCtrl->GetFlags().HasIcons && _IconId != -1)
		g->DrawImage(_TreeCtrl->GetIconImage(_IconId), (Gdiplus::REAL)offsetX+is+hbs+4-ics, (Gdiplus::REAL)offsetY, (Gdiplus::REAL)ics, (Gdiplus::REAL)ics);
}

TcHitResult TreeItem::HitTest(const WinPoint& point)
{
	TcHitResult result;
	result.HitType = kTcHitNone;

	if (point.y > 0 && point.y < GetItemHeight())
	{
		if (_Children.count && point.y > 0 && point.y < 4+_TreeCtrl->GetButtonSize() && point.x > -2 && point.x < _TreeCtrl->GetButtonSize()+2)
			result.HitType = kTcHitRowButton;
		else 
		{
			HDC hdc = GetDC(_TreeCtrl->_hwnd);
			Gdiplus::Graphics g(hdc);
			Gdiplus::RectF rect;
			g.MeasureString(_Label, _Label.Length(), _TreeCtrl->GetDefaultFont(), Gdiplus::PointF((Gdiplus::REAL)_TreeCtrl->GetIndentSize()+(_TreeCtrl->GetButtonSize()/2)+4+(_TreeCtrl->_Flags.HasIcons?_TreeCtrl->GetIconSize():0), 0), 0, &rect);
			rect.Width = max(_MinItemWidth, rect.Width);
			ReleaseDC(_TreeCtrl->_hwnd, hdc);

			if (point.x > rect.GetLeft() && point.x < rect.GetRight())
			{
				result.HitType = kTcHitRowLabel;
			} else {
				result.HitType = kTcHitRowEmpty;
			}
		}

		result.Item = this;
	}
	else if (IsExpanded())
	{
		WinPoint childPoint = point;
		childPoint.x -= _TreeCtrl->GetIndentSize();
		if (_TreeCtrl->GetFlags().HasIcons)
			childPoint.x -= _TreeCtrl->GetIconSize();
		childPoint.y -= GetItemHeight();

		for (size_t i=0; i<_Children.count; i++)
		{
			result = _Children[i]->HitTest(childPoint);
			childPoint.y -= _Children[i]->GetTotalHeight();

			if (result.HitType != kTcHitNone)
				return result;
		}
	}

	return result;
}

TreeItem* TreeItem::GetFirstChild()
{
	if (_Children.count)
		return _Children[0];

	return 0;
}

TreeItem* TreeItem::GetLastChild()
{
	if (_Children.count)
		return _Children[_Children.count-1];

	return 0;
}

bool TreeItem::IsChildOf(TreeItem* item)
{
	TreeItem* parent = _Parent;
	
	while (parent)
	{
		if (parent == item)
			return true;

		parent = parent->GetParent();
	}

	return false;
}
#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

/*namespace Jinx
{
	class PropertyDescription
	{
	private:
		const char* _name;
	};

	enum PropertyType
	{
		kPropertyBool,
		kPropertyInt32,
		kPropertyFloat,
		kPropertyString
	};

	class Property
	{
		PropertyDescription* _desc;
	};
	
	class PropertyFloat : public Property
	{
		float _value;
	};

	class Element
	{
	public:
		void Measure(const WinSize& availableSize);
		void Arrange(const WinRect& finalRect);

	protected:
		virtual WinSize OnMeasure(const WinSize& availableSize);
		virtual WinSize OnArrange(const WinSize& finalSize);

	private:
		Element* _parent;

	private:
		WinSize _desiredSize;
		WinRect _finalRect;
	};
}*/

SnGroup::SnGroup(ScarabForm* form, const XtString& groupName, bool isCollapsed, XtU32 colour) :
	_groupName(groupName),
	_rgb(colour),
	_collapsed(isCollapsed),
	_idGroup(this),
	_spc(this),
	_wndGroup(form, &_spc)
{
	if (!*_groupName)
		_groupName = "General";

	if (form)
		_collapsed = form->IsGroupCollapsed(_groupName, isCollapsed);

	SetEnabled(true);
}

SnGroup::~SnGroup()
{
}

ScarabNode* SnGroup::Body()
{
	return _wndGroup.Root();
}

bool SnGroup::OnPopulate()
{
	if (!ScarabNode::OnPopulate())
		return false;

	ScarabNode* body = Body();

	if (!body)
		return false;

	return body->Populate();
}

bool SnGroup::OnBuild()
{
	if (!Body())
		return false;

	if (!_idGroup.Allocate())
		return false;

	if (!_wndGroup && !_wndGroup.Create((HWND)*Panel(), (HMENU)(intptr_t)_idGroup))
		return false;

	SetWindowPos(_wndGroup, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

	return true;
}

void SnGroup::OnBind(Xt::DomBlock* parentDom)
{
	ScarabNode::OnBind(parentDom);

	if (ScarabNode* node = Body())
		node->Bind(parentDom);
}

void SnGroup::OnBind(Xt::DomArray* parentDom, int index)
{
	ScarabNode::OnBind(parentDom, index);

	if (ScarabNode* node = Body())
		node->Bind(parentDom, index);
}

void SnGroup::OnMeasure(HDC ic)
{
	if (_collapsed)
	{
		WinSize labelSize;
		GetTextExtentPoint32(ic, _groupName, _groupName.Length(), &labelSize);

		_size.cx = labelSize.cx + 16;
		_size.cy = 0;

		_padding = WinRect(8, 16, 8, 0);
	}
	else
	{
		_padding = WinRect(8, 24, 8, 8);

		_wndGroup.LayoutNow();

		WinRect rc(_wndGroup);

		_size = rc.Size();
	}
}

void SnGroup::OnLayout(ScarabLayoutContext* layoutContext)
{
	if (_wndGroup)
	{
		if (_collapsed)
		{
			layoutContext->SetWindowPos(_wndGroup, _client.left, _client.top, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
			EnableWindow(_wndGroup, FALSE);
		}
		else
		{
			layoutContext->SetWindowPos(_wndGroup, _client.left, _client.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
			EnableWindow(_wndGroup, TRUE);
		}
	}
}

void SnGroup::OnLeftButton(const WinPoint& pt)
{
	int y = _client.top + (_collapsed ? 8 : 0);

	WinRect rc(_client.left - 8, y - 24, _client.right + 8, y - 8);

	if (PtInRect(&rc, pt))
	{
		_collapsed = !_collapsed;

		if (ScarabForm* form = Panel()->Form())
			form->SetGroupCollapsed(_groupName, _collapsed);

		Panel()->DirtyLayout();
	}
}

void SnGroup::OnPaint(HDC hdc, const WinRect& rcPaint)
{
	int y = _client.top + (_collapsed ? 8 : 0);

	// Box

	HPEN pen		= CreatePen(PS_SOLID, 2, _rgb);
	HPEN oldPen		= (HPEN)SelectObject(hdc, pen);

	SelectObject(hdc, GetStockObject(NULL_BRUSH));

	if (!_collapsed)
		Rectangle(hdc, _client.left - 8, y - 8, _client.right + 8, _client.bottom + 8);

	HBRUSH brush	= CreateSolidBrush(_rgb);
	HBRUSH oldBrush	= (HBRUSH)SelectObject(hdc, brush);

	Rectangle(hdc, _client.left - 8, y - 24, _client.right + 8, y - 8);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	SelectObject(hdc, GetStockObject(WHITE_PEN));

	Rectangle(hdc, _client.left - 4, y - 17, _client.left + 4, y - 15);

	if (!_collapsed)
		Rectangle(hdc, _client.left - 1, y - 20, _client.left + 1, y - 12);

	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
	DeleteObject(pen);
	DeleteObject(brush);

	// Label

	SelectObject(hdc, Panel()->DrawContext()->boldFont);
	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, TRANSPARENT);

	WinSize labelSize;
	GetTextExtentPoint32(hdc, _groupName, _groupName.Length(), &labelSize);

	ExtTextOut(hdc, _client.left + 7, y - 16 - (labelSize.cy / 2), 0, 0, _groupName, _groupName.Length(), 0);

	// Children

	ScarabNode::OnPaint(hdc, rcPaint);
}

ScarabNode* SnGroup::HitTestLabel(const WinPoint& pt)
{
	ScarabNode* node = ScarabNode::HitTestLabel(pt);

	if (node)
		return node;

	if (PtInRect(&_location, pt))
		return this;

	return 0;
}
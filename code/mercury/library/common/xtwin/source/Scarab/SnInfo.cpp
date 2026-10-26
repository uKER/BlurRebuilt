#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"
#include <ColorDlg.h>

SnInfo::SnInfo(LfControl* field) :
	SnControl(field)
{
}

SnInfo::~SnInfo()
{
}

void SnInfo::OnMeasure(HDC ic)
{
	XtString msg = GetMessage();

	GetTextExtentPoint32(ic, msg, msg.Length(), &_size);

	if (_size.cy > 0)
		_size.cy += 7;
}

void SnInfo::OnPaint(HDC hdc, const WinRect& rcPaint)
{
	ScarabNode::OnPaint(hdc, rcPaint);

	XtString msg = GetMessage();

	if (*msg)
	{
		SelectObject(hdc, Panel()->DrawContext()->labelFont);
		SetTextColor(hdc, RGB(0, 0, 0));
		ExtTextOut(hdc, _client.left, _client.top + 3, 0, 0, msg, msg.Length(), 0);
	}
}

XtString SnInfo::GetMessage()
{
	return GetString("Info", "message", "");
}
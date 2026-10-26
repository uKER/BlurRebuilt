#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/SendEmail.h"
#include "ScarabImpl.h"

const int kLabelSpacing = 4;

ScarabClipTarget::ScarabClipTarget(Xt::DomElement* element_, LarvaField* field_) :
	element(element_),
	field(field_)
{
}

ScarabLayoutContext::ScarabLayoutContext() :
	_dwp(0),
	_lastLayoutWnd(HWND_TOP)
{
}

void ScarabLayoutContext::Begin()
{
	_dwp = BeginDeferWindowPos(32);
}

void ScarabLayoutContext::End()
{
	if (_dwp)
		EndDeferWindowPos(_dwp);
}

void ScarabLayoutContext::SetWindowPos(HWND wnd, int x, int y, int cx, int cy, UINT flags)
{
	if (!wnd || !_dwp)
		return;

	if ((_dwp = DeferWindowPos(_dwp, wnd, _lastLayoutWnd, x, y, cx, cy, flags | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOOWNERZORDER)) == 0)
	{
		DebugLn("DeferWindowPos failed :(");
	}

	_lastLayoutWnd = wnd;
}

ScarabNode::ScarabNode(ScarabPanel* panel) :
	_panel(panel),
	_parent(0),
	_enabled(false),
	_built(false),
	_labelVisible(true),
	_labelOffset(0),
	_borderVisible(false),
	_borderColour(0),
	_labelAlignMargin(0),
	_spacing(7),
	_layoutMethod(kSlVertical),
	_whiteBackground(true)
{
}

ScarabNode::~ScarabNode()
{
}

bool ScarabNode::OnPopulate()
{
	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		if (!it->Populate())
			return false;

	return true;
}

void ScarabNode::OnBind(Xt::DomField* parentDom)
{
	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Bind(parentDom);
}

void ScarabNode::OnBind(Xt::DomBlock* parentDom)
{
	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Bind(parentDom);
}

void ScarabNode::OnBind(Xt::DomArray* parentDom, int index)
{
	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Bind(parentDom, index);
}

void ScarabNode::OnBind(Xt::DomComponent* parentDom)
{
	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Bind(parentDom);
}

bool ScarabNode::OnBuild()
{
	return true;
}

void ScarabNode::OnMeasure(HDC ic)
{
	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Measure(ic);

	MeasurePassContext ctx;

	MeasurePass_Margin(&ctx, this);
	MeasurePass_Position(&ctx, this);
}

void ScarabNode::MeasurePass_Margin(MeasurePassContext* ctx, ScarabNode* node)
{
	if (_layoutMethod == kSlVertical)
	{
		for(XtUt::List<ScarabNode>::Iterator it(&node->_nodes); it; ++it)
		{
			int requiredMargin = it->_labelVisible ? (int)it->_labelSize.cx : 0;

			if (requiredMargin > 0)
				requiredMargin += kLabelSpacing;

			requiredMargin += (int)it->_anchor.x;

			_labelAlignMargin = Max(_labelAlignMargin, requiredMargin);

			if (it->_layoutMethod == kSlInline)
				MeasurePass_Margin(ctx, it);
		}
	}
	else if (_layoutMethod == kSlHorizontal)
	{
		for(XtUt::List<ScarabNode>::Iterator it(&node->_nodes); it; ++it)
		{
			ctx->margin = Max(ctx->margin, (int)it->_anchor.y);

			if (it->_layoutMethod == kSlInline)
				MeasurePass_Margin(ctx, it);
		}
	}
	else if (_layoutMethod == kSlInline)
	{
	}
}

void ScarabNode::MeasurePass_Position(MeasurePassContext* ctx, ScarabNode* node)
{
	if (_layoutMethod == kSlVertical)
	{
		int width	= 0;
		int y		= 0;

		bool requiresSpacing = false;

		for(XtUt::List<ScarabNode>::Iterator it(&node->_nodes); it; ++it)
		{
			if (it->_layoutMethod == kSlInline)
				MeasurePass_Position(ctx, it);

			if (requiresSpacing && (it->_size.cy > 0))
				y += node->_spacing;

			it->_labelOffset	= 0;
			it->_position.x		= _labelAlignMargin - it->_anchor.x;
			it->_position.y		= y;

			if (it->_layoutMethod == kSlInline)
				it->_position.x -= _labelAlignMargin;

			width = Max(width, (int)it->_position.x + (int)it->_size.cx);

			int height = Max((int)it->_size.cy, it->_labelVisible ? (int)it->_labelSize.cy : 0);

			requiresSpacing = (height > 0);

			y += height;
		}

		node->_size.cx = width;
		node->_size.cy = Max(y, node->_labelVisible ? (int)node->_labelSize.cy : 0);
	}
	else if (_layoutMethod == kSlHorizontal)
	{
		// TODO: This layout method doesn't cope with inline blocks yet

		int x		= 0;
		int height	= 0;

		bool requiresSpacing = false;

		for(XtUt::List<ScarabNode>::Iterator it(&node->_nodes); it; ++it)
		{
			if (it->_layoutMethod == kSlInline)
				MeasurePass_Position(ctx, it);

			if (requiresSpacing)
				x += _spacing;

			it->_labelOffset = x;

			if (it->_labelVisible && (it->_labelSize.cx > 0))
			{
				x += it->_labelSize.cx + kLabelSpacing;

				if (it.index <= 0)
					_labelAlignMargin = x;
			}

			it->_position.x = x;
			it->_position.y = ctx->margin - it->_anchor.y;

			x += it->_size.cx;

			requiresSpacing = it->_size.cx > 0;

			height = Max(height, (int)it->_size.cy);
		}

		node->_size.cx = x;
		node->_size.cy = Max(height, node->_labelVisible ? (int)node->_labelSize.cy : 0);
	}
	else if (_layoutMethod == kSlInline)
	{
	}
}

void ScarabNode::OnLayout(ScarabLayoutContext* layoutContext)
{
	WinPoint origin(_client.left, _client.top);

	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Layout(layoutContext, origin);
}

LRESULT ScarabNode::OnNotify(NMHDR* hdr)
{
	return 0;
}

void ScarabNode::OnCommand(int id, int code)
{
}

void ScarabNode::OnScroll(int id, int msg, int method, HWND scrollBar)
{
}

namespace
{
	bool DoSetTypeMeta(Xt::DomBlock* block, LarvaType* type)
	{
		if (!type)
			return false;

		return Set(block, "type", type->name) != 0;
	}

	bool DoSetFieldMeta(Xt::DomBlock* block, LarvaField* field)
	{
		switch(field->kind)
		{
			case kLfStruct:		return DoSetTypeMeta(block, ((LfStruct*)field)->type);
			case kLfInt:		return DoSetTypeMeta(block, ((LfInt*)field)->type);
			case kLfFloat:		return DoSetTypeMeta(block, ((LfFloat*)field)->type);
			case kLfString:		return DoSetTypeMeta(block, ((LfString*)field)->type);
			case kLfWideString:	return DoSetTypeMeta(block, ((LfWideString*)field)->type);
			case kLfBool:		return DoSetTypeMeta(block, ((LfBool*)field)->type);
			case kLfEnum:		return DoSetTypeMeta(block, ((LfEnum*)field)->type);
			case kLfFlags:		return DoSetTypeMeta(block, ((LfFlags*)field)->type);
			case kLfStrictPtr:	return DoSetTypeMeta(block, ((LfStrictPtr*)field)->type);
			case kLfPtr:		return Set(block, "ptr", "1") && DoSetTypeMeta(block, ((LfPtr*)field)->baseType);
			case kLfWeakPtr:	return Set(block, "weak", "1") && DoSetTypeMeta(block, ((LfWeakPtr*)field)->baseType);
			case kLfHandle:		return Set(block, "handle", "1") && DoSetTypeMeta(block, ((LfHandle*)field)->baseType);
			case kLfArray:		return Set(block, "array", "1") && DoSetFieldMeta(block, ((LfArray*)field)->field);
			case kLfLocString:	return DoSetTypeMeta(block, ((LfLocString*)field)->type);
		}

		return false;
	}

	bool IsTypeMetaCompatible(Xt::DomBlock* block, LarvaType* type)
	{
		if (!type)
			return false;

		return AsText(At(block, "type")) == type->name;
	}

	bool IsFieldMetaCompatible(Xt::DomBlock* block, LarvaField* field)
	{
		switch(field->kind)
		{
			case kLfStruct:		return IsTypeMetaCompatible(block, ((LfStruct*)field)->type);
			case kLfInt:		return IsTypeMetaCompatible(block, ((LfInt*)field)->type);
			case kLfFloat:		return IsTypeMetaCompatible(block, ((LfFloat*)field)->type);
			case kLfString:		return IsTypeMetaCompatible(block, ((LfString*)field)->type);
			case kLfWideString:	return IsTypeMetaCompatible(block, ((LfWideString*)field)->type);
			case kLfBool:		return IsTypeMetaCompatible(block, ((LfBool*)field)->type);
			case kLfEnum:		return IsTypeMetaCompatible(block, ((LfEnum*)field)->type);
			case kLfFlags:		return IsTypeMetaCompatible(block, ((LfFlags*)field)->type);
			case kLfStrictPtr:	return IsTypeMetaCompatible(block, ((LfStrictPtr*)field)->type);
			case kLfPtr:		return atoi(AsText(At(block, "ptr"))) && IsTypeMetaCompatible(block, ((LfPtr*)field)->baseType);
			case kLfWeakPtr:	return atoi(AsText(At(block, "weak"))) && IsTypeMetaCompatible(block, ((LfWeakPtr*)field)->baseType);
			case kLfHandle:		return atoi(AsText(At(block, "handle"))) && IsTypeMetaCompatible(block, ((LfHandle*)field)->baseType);
			case kLfArray:		return atoi(AsText(At(block, "array"))) && IsFieldMetaCompatible(block, ((LfArray*)field)->field);
			case kLfLocString:	return IsTypeMetaCompatible(block, ((LfLocString*)field)->type);
		}

		return false;
	}
}

void ScarabNode::OnLeftButton(const WinPoint& pt)
{
}

void ScarabNode::OnContextMenu(const WinPoint& pt)
{
	HMENU menu = CreatePopupMenu();

	ScarabClipTarget ct = ClipTarget();

	AppendMenu(menu, MF_STRING | (ct.element ? 0 : (MF_GRAYED | MF_DISABLED)), 100, "&Copy");
	AppendMenu(menu, MF_STRING | (ct.element ? 0 : (MF_GRAYED | MF_DISABLED)), 101, "&Paste");

	if (ct.field)
	{
		Sz8 menuText = "Schema: " + SplitFileExt<char>(ct.field->container->sourceFile) + Format(" (%i)", ct.field->sourceLine);

		AppendMenu(menu, MF_SEPARATOR, 0, 0);
		AppendMenu(menu, MF_STRING, 102, menuText);
	}

	int result = TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY, pt.x, pt.y, *Panel(), 0);

	if (result == 102)
	{
		ShellExecute(0, "open", ct.field->container->sourceFile, 0, 0, SW_SHOW);
	}
	else if (result == 100)
	{
		ScarabClipTarget ct = ClipTarget();

		if (ct.element && ct.field)
		{
			Xt::DomBlock block;

			if (DoSetFieldMeta(&block, ct.field) && Set(&block, "name", ct.field->name) && Set(&block, "data", Clone(ct.element)))
			{
				XtString data;

				if (Xt::Serialise2(&block, data))
					SetClipboardText(*Panel(), data);
			}
		}
	}
	else if (result == 101)
	{
		ScarabClipTarget ct = ClipTarget();
		Xt::DomDb* db = _panel->Db();

		bool success = false;

		if (ct.element && ct.field)
		{
			XtString data;

			if (GetClipboardText(*Panel(), data))
			{
				Xt::DomBlock block;

				if (Xt::Parse2(&block, &XtUt::WeakBlock((XtU8*)data.Text(), (size_t)data.Length()), 0, 0))
				{
					if (IsFieldMetaCompatible(&block, ct.field))
					{
						if (Xt::DomField* dst = AsField(ct.element))
						{
							if ((success = Xt::Clone(dst, AsField(At(&block, "data")))) != false)
							{
								if (Xt::DomRecord* rec = RecordOf(dst))
									Xt::SetDirty(rec, false);
							}
						}
						else if (Xt::DomRef* dst = AsRef(ct.element))
						{
							if ((success = Xt::Clone(dst, AsRef(At(&block, "data")))) != false)
							{
								if (Xt::DomRecord* rec = RecordOf(dst))
									Xt::SetDirty(rec, false);
							}
						}
						else if (Xt::DomBlock* dst = AsBlock(ct.element))
						{
							if ((success = Xt::Clone(dst, AsBlock(At(&block, "data")))) != false)
							{
								if (Xt::DomRecord* rec = RecordOf(dst))
									Xt::SetDirty(rec, false);
							}
						}
						else if (Xt::DomComponent* dst = AsComponent(ct.element))
						{
							if ((success = Xt::Clone(dst, AsComponent(At(&block, "data")))) != false)
							{
								Xt::IdMap map;

								RegenerateComponentIds(db->schema, ct.field, dst, &map);
								MapWeakPtrs(db->schema, ct.field, dst, &map);

								if (Xt::DomRecord* rec = RecordOf(dst))
									Xt::SetDirty(rec, false);
							}
						}
						else if (Xt::DomArray* dst = AsArray(ct.element))
						{
							if ((success = Xt::Clone(dst, AsArray(At(&block, "data")))) != false)
							{
								if (ct.field->kind == kLfArray)
								{
									LfArray* lf = (LfArray*)ct.field;

									Xt::IdMap map;

									for(Xt::DomArray::IndexIterator it(dst); it; ++it)
									{
										if (Xt::DomComponent* com = AsComponent(it))
											RegenerateComponentIds(db->schema, lf->field, com, &map);
									}

									for(Xt::DomArray::IndexIterator it(dst); it; ++it)
									{
										if (Xt::DomComponent* com = AsComponent(it))
											MapWeakPtrs(db->schema, lf->field, com, &map);
									}
								}

								if (Xt::DomRecord* rec = RecordOf(dst))
									Xt::SetDirty(rec, false);
							}
						}
					}
					else
					{
						DebugLn("Paste Error: Incompatible types");
					}
				}
				else
				{
					DebugLn("Paste Import Error");
				}
			}
		}

		if (!success)
			MessageBeep(MB_ICONHAND);
	}

	DestroyMenu(menu);
}

void ScarabNode::OnPaint(HDC hdc, const WinRect& rcPaint)
{
	if (_labelVisible && _parent && (_layoutMethod != kSlInline))
	{
		SelectObject(hdc, Panel()->DrawContext()->labelFont);
		SetTextColor(hdc, _enabled ? RGB(0, 0, 0) : Panel()->DrawContext()->borderColour);
		ExtTextOut(hdc, _parent->_client.left + _labelOffset, _client.top + 3, 0, 0, _labelText, _labelText.Length(), 0);
	}

	if (_borderVisible && (_layoutMethod != kSlInline))
	{
		HPEN pen	= CreatePen(PS_SOLID, _borderColour ? 2 : 0, _borderColour ? _borderColour : RGB(169, 169, 169));
		HPEN oldPen	= (HPEN)SelectObject(hdc, pen);

		SelectObject(hdc, GetStockObject(NULL_BRUSH));

		Rectangle(hdc, _location.left, _location.top, _location.right, _location.bottom);

		SelectObject(hdc, oldPen);
		DeleteObject(pen);
	}

	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->OnPaint(hdc, rcPaint);
}

HBRUSH ScarabNode::OnControlColour(int id, int msg, HDC hdc, HWND hwnd)
{
	if ((msg == WM_CTLCOLORSTATIC) && _whiteBackground)
		return Panel()->DrawContext()->backgroundBrush;

	return 0;
}

bool ScarabNode::MeasureItem(int id, MEASUREITEMSTRUCT* mis)
{
	return false;
}

bool ScarabNode::DrawItem(int id, DRAWITEMSTRUCT* dis)
{
	return false;
}

ScarabClipTarget ScarabNode::ClipTarget()
{
	return ScarabClipTarget(0, 0);
}

bool ScarabNode::Populate()
{
	return OnPopulate();
}

bool ScarabNode::Build()
{
	if (!_built)
	{
		_built = true;

		if (!OnBuild())
			return false;
	}

	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
	{
		if (!it->Build())
			return false;
	}

	return true;
}

void ScarabNode::Bind(Xt::DomField* parentDom)
{
	OnBind(parentDom);
}

void ScarabNode::Bind(Xt::DomBlock* parentDom)
{
	OnBind(parentDom);
}

void ScarabNode::Bind(Xt::DomArray* parentDom, int index)
{
	OnBind(parentDom, index);
}

void ScarabNode::Bind(Xt::DomComponent* parentDom)
{
	OnBind(parentDom);
}

void ScarabNode::Measure(HDC ic)
{
	_labelAlignMargin = 0;

	if (_layoutMethod != kSlInline)
		GetTextExtentPoint32(ic, _labelText, _labelText.Length(), &_labelSize);
	else
		_labelSize = WinSize();

	OnMeasure(ic);

	if (_borderVisible && (_layoutMethod != kSlInline))
	{
		_size.cx += 16;
		_size.cy += 16;
	}

	_size.cx += _padding.left + _padding.right;
	_size.cy += _padding.top + _padding.bottom;
}

void ScarabNode::Layout(ScarabLayoutContext* layoutContext, const WinPoint& parentOrigin)
{
	_location.left		= parentOrigin.x + _position.x;
	_location.top		= parentOrigin.y + _position.y;
	_location.right		= _location.left + _size.cx;
	_location.bottom	= _location.top + _size.cy;

	_client = _location;

	if (_borderVisible && (_layoutMethod != kSlInline))
		_client.Shrink(8);

	_client.left	+= _padding.left;
	_client.right	-= _padding.right;
	_client.top		+= _padding.top;
	_client.bottom	-= _padding.bottom;

	OnLayout(layoutContext);
}

ScarabNode* ScarabNode::Add(ScarabNode* node)
{
	if (node)
	{
		if (node->_parent)
			return 0;

		if (!AppendEx(&_nodes, node))
			return 0;

		node->_panel	= _panel;
		node->_parent	= this;
	}

	return node;
}

ScarabNode* ScarabNode::Insert(int index, ScarabNode* node)
{
	if (node)
	{
		if (node->_parent)
			return 0;

		if (!InsertEx(&_nodes, index, node))
			return 0;

		node->_panel	= _panel;
		node->_parent	= this;
	}

	return node;
}

void ScarabNode::Clear()
{
	Free(&_nodes);
}

void ScarabNode::SetEnabled(bool enabled)
{
	if (enabled == _enabled)
		return;

	if (_panel)
		InvalidateWindow(*_panel);

	_enabled = enabled;
}

Xt::DomDb* ScarabNode::Db()
{
	return _panel ? _panel->Db() : 0;
}

LarvaSchema* ScarabNode::Schema()
{
	if (Xt::DomDb* db = Db())
		return db->schema;

	return 0;
}

bool ScarabNode::IsEnabled() const
{
	return _enabled;
}

ScarabNode* ScarabNode::HitTestLabel(const WinPoint& pt)
{
	if (_labelVisible && _parent && (_layoutMethod != kSlInline))
	{
		WinRect rc;

		rc.left		= _parent->_client.left + _labelOffset;
		rc.top		= _client.top + 3;
		rc.right	= rc.left + _labelSize.cx;
		rc.bottom	= rc.top + _labelSize.cy;

		if (PtInRect(&rc, pt))
			return this;
	}

	if (PtInRect(&_location, pt))
	{
		for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		{
			if (ScarabNode* node = it->HitTestLabel(pt))
				return node;
		}
	}

	return 0;
}
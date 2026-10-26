#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

//
// SnFlatArray
//

SnFlatArray::SnFlatArray(LfArray* field) :
	_field(field),
	_msgId(this)
{
	_dom.SetCallback(DomCallback, this);
}

SnFlatArray::~SnFlatArray()
{
}

void SnFlatArray::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireArray(parentDom, _field->name));
}

void SnFlatArray::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireArray(parentDom, index));
}

bool SnFlatArray::OnBuild()
{
	if (!_msgId.Allocate())
		return false;

	return ScarabNode::OnBuild();
}

void SnFlatArray::OnMeasure(HDC ic)
{
	ScarabNode::OnMeasure(ic);

	_anchor.x = _labelAlignMargin;
}

ScarabClipTarget SnFlatArray::ClipTarget()
{
	return ScarabClipTarget(AsArray(_dom), _field);
}

void SnFlatArray::OnCommand(int id, int code)
{
	if (id == _msgId)
	{
		if (code == 0)
			UpdateControls();
	}
}

void SnFlatArray::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFlatArray* node)
{
	//if (event->code == Xt::kDomRemoved)
	//	PostMessage(*node->Panel(), WM_COMMAND, MAKELONG(node->_msgId, 0), 0);
}

void SnFlatArray::SetDom(Xt::DomArray* arr)
{
	SetEnabled(Monitor(&_dom, arr) != 0);

	UpdateControls();
}

void SnFlatArray::PopulateBody()
{
	Xt::DomArray* arr = AsArray(_dom);

	Clear();

	if (arr)
	{
		for(size_t i = 0; i < Count(arr); i++)
			Add(new SnFlatArrayItem(_field, this, (int)i));

		Add(new SnFlatArrayItem(0, this, (int)Count(arr)));

		Populate();
	}
}

void SnFlatArray::UpdateControls()
{
	Xt::DomArray* arr = AsArray(_dom);

	PopulateBody();

	for(XtUt::List<ScarabNode>::Iterator it(&_nodes); it; ++it)
		it->Bind(arr, 0);

	Panel()->DirtyLayout();
}

//
// SnFlatArrayItem
//

SnFlatArrayItem::SnFlatArrayItem(LfArray* field, SnFlatArray* array, int index) :
	_field(field),
	_index(index),
	_idAdd(this),
	_idRemove(this),
	_array(array),
	_originalWidth(0)
{
	_dom.SetCallback(DomCallback, this);

	_labelText = Format("%i:", _index);
}

SnFlatArrayItem::~SnFlatArrayItem()
{
}

bool SnFlatArrayItem::OnPopulate()
{
	if (_field && !ScarabBuild(this, _field->field))
		return false;

	return ScarabNode::OnPopulate();
}

void SnFlatArrayItem::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(AcquireArray(parentDom, _field->name));
}

void SnFlatArrayItem::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(parentDom);
}

bool SnFlatArrayItem::OnBuild()
{
	if (!_idAdd.Allocate() || !_idRemove.Allocate())
		return false;

	if (_add == 0)
	{
		if (!_add.Create(0, WC_BUTTON, _T("+"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idAdd))
			return false;

		SendMessage(_add, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	if (_field && (_remove == 0))
	{
		if (!_remove.Create(0, WC_BUTTON, _T("-"), BS_FLAT | WS_CHILD | WS_TABSTOP, *Panel(), _idRemove))
			return false;

		SendMessage(_remove, WM_SETFONT, (WPARAM)Panel()->DrawContext()->labelFont, FALSE);
	}

	return true;
}

void SnFlatArrayItem::OnMeasure(HDC ic)
{
	ScarabNode::OnMeasure(ic);

	if (_size.cx > 0)
		_size.cx += 4;

	_originalWidth = _size.cx;

	_size.cx += 16;

	if (_field)
		_size.cx += 20;

	_size.cy = Max((int)_size.cy, 20);
}

void SnFlatArrayItem::OnLayout(ScarabLayoutContext* layoutContext)
{
	int y = _client.top;// + (_client.Height() - 16) / 2;

	ScarabNode::OnLayout(layoutContext);

	layoutContext->SetWindowPos(_add, _client.left + _originalWidth, y, 16, 16, SWP_SHOWWINDOW);

	if (_field)
		layoutContext->SetWindowPos(_remove, _client.left + _originalWidth + 20, y, 16, 16, SWP_SHOWWINDOW);
}

ScarabClipTarget SnFlatArrayItem::ClipTarget()
{
	if (_field)
		return ScarabClipTarget(At(AsArray(_dom), _index), _field->field);
	else
		return ScarabClipTarget(0, 0);
}

void SnFlatArrayItem::OnCommand(int id, int code)
{
	LarvaSchema*	schema	= Panel()->Db()->schema;
	Xt::DomArray*	array	= AsArray(_dom);

	if (array)
	{
		if ((id == _idAdd) && (code == BN_CLICKED))
		{
			Xt::Insert(array, _index, new Xt::DomComponent);

			if (LtStruct* type = AsStruct(_array->_field->Type()))
			{
				if (Xt::DomComponent* com = AsComponent(At(array, _index)))
				{
					GenerateIdForComponentAndChildren(Schema(), type, com, true);
					SetDefaults(schema, type, com);
				}
			}

			_array->UpdateControls();
		}
		else if ((id == _idRemove) && (code == BN_CLICKED))
		{
			Xt::Remove(array, _index);

			_array->UpdateControls();
		}
	}
}

void SnFlatArrayItem::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFlatArrayItem* node)
{
	//if (event->code == Xt::kDomRemoved)
	//	PostMessage(*node->Panel(), WM_COMMAND, MAKELONG(node->_array->_msgId, 0), 0);
}

void SnFlatArrayItem::SetDom(Xt::DomArray* arr)
{
	SetEnabled(Monitor(&_dom, arr) != 0);

	ScarabNode::OnBind(arr, _index);

	if (arr)
		GenerateIdForComponent(AsComponent(At(arr, _index)), false);
}
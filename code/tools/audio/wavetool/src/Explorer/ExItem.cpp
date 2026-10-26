#include "Core/StdAfx.h"
#include "Explorer/Explorer.h"

//
// Lifetime
//

ExItem::ExItem(ExItemType type, ExNode* node) :
	_type(type),
	_node(node)
{
	if (_node->_item)
		delete _node->_item;

	_node->_item = this;
}

ExItem::~ExItem()
{
	_node->_item = 0;
}

void ExItem::SetId(const XtString& id)
{
	ExNode* parent = _node ? _node->Container() : 0;

	if (parent)
		_node->DetachSelf();

	_node->_id = id;

	if (parent)
		parent->Add(_node);
}

void ExItem::SetIcon(ExNodeIcon open, ExNodeIcon closed)
{
	if ((_node->_iconOpen != open) || (_node->_iconClosed != closed))
	{
		_node->_iconOpen	= open;
		_node->_iconClosed	= closed;

		_node->UpdateIconLabel();
	}
}

void ExItem::SetIcon(LarvaType* type)
{
	ExplorerView* view = Node()->Explorer();

	ExNodeIcon icon = view->RegisterTypeForIcon(type);

	SetIcon(icon, icon);
}

void ExItem::SetOverlayIcon(ExNodeOverlayIcon icon)
{
	if (_node->_iconOverlay != icon)
	{
		_node->_iconOverlay = icon;

		_node->UpdateIconLabel();
	}
}

void ExItem::SetLabel(const XtString& label)
{
	_node->_label = label;

	_node->UpdateIconLabel();
}

bool ExItem::OnEnter()
{
	return false;
}

bool ExItem::OnDoubleClick()
{
	return false;
}

bool ExItem::OnRightClick()
{
	return false;
}

bool ExItem::OnBeginEditLabel()
{
	return false;
}

bool ExItem::OnEndEditLabel(const char* newText)
{
	return false;
}

void ExItem::OnDeletePressed()
{
}

void ExItem::OnDuplicateRecord(ExItem* parent, bool renameRecord)
{
}

void ExItem::Selected()
{
}

void ExItem::Delete()
{
}

void ExItem::Repath(const XtString& oldPath, const XtString& newPath)
{
}

void ExItem::MoveToPack(Xt::DomPack* pack)
{
}

void ExItem::GenerateFilters(XtUt::Set<XtString>* filters, const XtString& path)
{
}

void ExItem::RepathPack(const XtString& oldPath, const XtString& newPath)
{
}

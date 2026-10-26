#ifndef RECORD_GRAPH_HIDE_ICON_H
#define RECORD_GRAPH_HIDE_ICON_H

#pragma once

#include "src/NodeInterface.h"

namespace RecordGraph
{

class C_HideIcon : public I_Node
{
public:
	C_HideIcon();
	virtual ~C_HideIcon() {};

	void	Draw(wxDC& dc) const;
	bool	MouseClick();
	bool	SetSelected(const bool bSelected);
};

} // RecordGraph

#endif // RECORD_GRAPH_HIDE_ICON_H

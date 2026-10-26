#ifndef RECORD_GRAPH_CLOSE_ICON_H
#define RECORD_GRAPH_CLOSE_ICON_H

#pragma once

#include "src/NodeInterface.h"

namespace RecordGraph
{

class C_CloseIcon : public I_Node
{
public:
	C_CloseIcon();
	virtual ~C_CloseIcon() {};

	void	Draw(wxDC& dc) const;
	bool	MouseClick();
};

} // RecordGraph

#endif // RECORD_GRAPH_CLOSE_ICON_H

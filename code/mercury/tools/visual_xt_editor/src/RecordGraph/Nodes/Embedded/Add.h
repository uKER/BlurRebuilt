#ifndef RECORD_GRAPH_ADD_ICON_H
#define RECORD_GRAPH_ADD_ICON_H

#pragma once

#include "src/NodeInterface.h"

namespace RecordGraph
{

class C_AddIcon : public I_Node
{
public:
	C_AddIcon();
	virtual ~C_AddIcon() {};

	void	Draw(wxDC& dc) const;
	bool	MouseClick();
};

} // RecordGraph

#endif // RECORD_GRAPH_ADD_ICON_H

#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/NodeGraph.h"
#include "../ScarabImpl.h"
#include "SnNgImpl.h"

SnNgConnection::SnNgConnection(ScarabDomNodeGraph* ng) :
	_ng(ng)
{
	_ng->MegaUndoHack_CheckPoint();
}

SnNgConnection::~SnNgConnection()
{
	_ng->MegaUndoHack_CheckPoint();
}

bool SnNgConnection::IsConflicted()
{
	if (!Diagram()->IsConnectionDragActive())
		return false;

	SnNgPin* dragFrom	= (SnNgPin*)Diagram()->ConnectionDragFrom();
	SnNgPin* dragTo		= (SnNgPin*)Diagram()->ConnectionDragTo();

	if (!dragFrom || !dragTo)
		return false;

	if (!dragFrom->OnTryConnect(dragTo))
		return false;

	SnNgPin* from	= (SnNgPin*)From();
	SnNgPin* to		= (SnNgPin*)To();

	return (from == dragTo) && !(to && (to == dragFrom));
}

bool SnNgConnection::IsHot()
{
	if (NgPin* hot = Diagram()->Hot())
		return (From() == hot) || (To() == hot);

	return false;
}
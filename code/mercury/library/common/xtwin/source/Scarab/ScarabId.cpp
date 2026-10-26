#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

bool ScarabId::Allocate()
{
	if (!_id)
	{
		ScarabPanel* panel = _node->Panel();

		if (!panel)
			return false;

		_id = panel->AllocateControlId(_node);
	}

	return _id != 0;
}

void ScarabId::Free()
{
	ScarabPanel* panel = _node->Panel();

	if (panel && _id)
	{
		panel->FreeControlId(_id);
		_id = 0;
	}
}
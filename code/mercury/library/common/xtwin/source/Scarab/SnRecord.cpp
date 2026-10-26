#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnRecord::SnRecord(LtStruct* type, const XtString& filter) :
	_type(type),
	_filter(filter)
{
}

SnRecord::~SnRecord()
{
}

bool SnRecord::OnPopulate()
{
	return ScarabBuild(this, _type, true, _filter) && ScarabNode::OnPopulate();
}
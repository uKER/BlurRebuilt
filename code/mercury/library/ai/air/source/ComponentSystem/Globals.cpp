#include "stdafx.h"
#include <ComponentSystem/Globals.h>
#include <ComponentSystem/CObjectManager.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{
	CObjectManager &GlobalsComponent::GetObjectManager()
	{
		static CObjectManager obj;
		return obj;
	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif








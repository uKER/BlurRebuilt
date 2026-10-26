#include "stdafx.h"
#include <ComponentSystem/IComponent.h>
#include <ComponentSystem/Globals.h>
#include <ComponentSystem/CObjectManager.h>

namespace airacingtech
{
	IComponent::IComponent()
	{
	}

	IComponent::~IComponent()
	{
	}

	EMessageResult IComponent::HandleMessage(const CMessage &)
	{
		return MR_IGNORED;
	}
}


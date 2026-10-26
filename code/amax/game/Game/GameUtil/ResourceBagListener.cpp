#include "stdafx.h"
#include "ResourceBagListener.h"

#include "Game/GameUtil/ResourceBag.h"


C_ResourceBagListener::C_ResourceBagListener()
	: m_Listenees()
{
}

C_ResourceBagListener::~C_ResourceBagListener()
{
	while (!m_Listenees.Empty())
	{
		m_Listenees.Front()->RemoveListener(this);
	}
}

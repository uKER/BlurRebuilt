#include "stdafx.h"
#include <tools_shared/guid/CryptRandom.h>

#include <EditorLib/Misc/GuidManager.h>

namespace EditorLib
{

C_GuidManager::C_GuidManager()
{
}

C_GuidManager::~C_GuidManager()
{
}

bool C_GuidManager::CreateGuid(U32& id)
{
	Tools::C_CryptRandom cr;
	if (!cr.Initialise())
	{
		assert(!"Could not initialise CryptRandom!");
		return false;
	}

	const U32 maxTries = 32;
	for (U32 i = 0; i < maxTries; i++)
	{
		id = (U32)cr();
		IdSet::Iterator it = m_ids.Find(id);
		if (it == m_ids.End())
		{
			m_ids.Insert(id);
			return true;
		}
	}

	return false;
}

bool C_GuidManager::RegisterGuid(U32& id)
{
	IdSet::Iterator it = m_ids.Find(id);
	if (it == m_ids.End())
	{
		m_ids.Insert(id);
		return true;
	}

	return false;
}

void C_GuidManager::DestroyGuid(U32 id)
{
	IdSet::Iterator it = m_ids.Find(id);
	if (it != m_ids.End())
	{
		m_ids.Erase(it);
		return;
	}

	assert(!"Could not find id to destroy!");
}

}

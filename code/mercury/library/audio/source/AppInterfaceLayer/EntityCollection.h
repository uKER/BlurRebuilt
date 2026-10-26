#ifndef _ENTITY_COLLECTION_H_
#define _ENTITY_COLLECTION_H_

#ifdef _MSC_VER
#pragma once
#endif


#include <btl/Containers/list.h>


namespace Audio
{

	class EntityCollection
	{

	public:
		EntityCollection() { }
		~EntityCollection() { m_aInstances.Clear(); }

		virtual bool						AddEntityInstanceAsListElement(EntityInstanceHandle hEntity)
		{
			m_aInstances.PushBack(hEntity);

			return true;
		}

		virtual bool						RemoveEntityInstanceAsListElement(EntityInstanceHandle hInstance)
		{
			Btl::List<EntityInstanceHandle>::Iterator i;

			for (i = m_aInstances.Begin(); i < m_aInstances.End(); ++i)
			{
				if (*(i) == hInstance)
				{
					m_aInstances.Erase(i);
					return true;
				}
			}

			return false;
		}

		virtual EntityInstanceHandle		GetEntityInstance()
		{
			EntityInstanceHandle hRet;
	
			BIZ_ASSERT(!m_aInstances.Empty());
			if (!m_aInstances.Empty())
			{
				hRet = m_aInstances.Front();
				m_aInstances.PopFront();
				m_aInstances.PushBack(hRet);
			}
			return hRet;

		}

		virtual bool						ReleaseEntityInstance(EntityInstanceHandle hEntity)
		{
			if (RemoveEntityInstanceAsListElement(hEntity))
			{
				m_aInstances.PushBack(hEntity);
				return true;
			}

			return false;
		}

	private:
		Btl::List<EntityInstanceHandle>		m_aInstances;
	};
}

#endif	//	_ENTITY_COLLECTION_H_

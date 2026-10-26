#include "SharedActorsPch.h"

#define SCENEGRAPH_ALLOW_INCLUDE // only for VisibilityDetection.cpp

#include "shared_actors/Utility/VisibilityDetection.h"
#include <scenegraph/VisibleDetect.h>
#include <scenegraph/SceneGraph.h>
#include <actor_system\ActorList.h>
#include "btl/containers/algorithm.h"

//#pragma optimize("", off)

namespace Actors
{

Debug::Logger &g_VisDetectLog = Debug::GetLogger("gamesys.actors.visdetect");

/// list of allocated C_VisibleDetect structures
Btl::Vector<C_VisibleDetect*> C_VisibilityDetector::m_VisDetectList;
Btl::DynamicPool<C_VisibleDetect> C_VisibilityDetector::s_visDetectPool;
Btl::Vector<C_VisibleDetect*> C_VisibilityDetector::s_objectsToDelete;

/// constructor
C_VisibilityDetector::C_VisibilityDetector(Actor::C_ActorList *pActorList)
: m_pActorList( pActorList )
{
	BIZ_ASSERT(m_VisDetectList.Size() == 0);
}

/// create a C_VisibleDetect
C_VisibleDetect *C_VisibilityDetector::CreateVisDetect(Actor::C_ActorHandle handle, Component::C_ComponentInterfaceId id)
{
	C_VisibleDetect *p = s_visDetectPool.Allocate();

	p->m_Handle = handle;
	p->m_ComponentId = id;
	m_VisDetectList.PushBack(p);
	return p;	
}

/// release a C_VisibleDetect
void C_VisibilityDetector::ReleaseVisDetect(C_VisibleDetect *p)
{
	if (!p)
		return;

	BIZ_ASSERT(Btl::Find(m_VisDetectList, p) != m_VisDetectList.End());
	BIZ_ASSERT(Btl::Find(s_objectsToDelete, p) == s_objectsToDelete.End());

	s_objectsToDelete.PushBack(p);
}

/// destructor
C_VisibilityDetector::~C_VisibilityDetector()
{
	for (int i=0; i<m_VisDetectList.Size(); i++)
		s_visDetectPool.Free(m_VisDetectList[i]);

	m_VisDetectList.Destroy();
	s_objectsToDelete.Destroy();
}

template <class T, class P>
inline void RemoveAll_NonOrdered(Btl::Vector<T, P>& s, const Btl::Vector<T, P>& elements)
{
	if (s.Empty() || elements.Empty())
		return;

	typename Btl::Vector<T, P>::Iterator i = s.Begin();

	while (i != s.End())
	{
		if (Btl::Find(elements, *i) != elements.End())
		{
			if (s.Size() > 1)
			{
				using Btl::Swap;
				Swap(*i, s.Back());
				s.PopBack();
			}
			else
			{
				s.Clear();
				break;
			}
		}
		else
		{
			++i;
		}
	};
}

/// update visibility detection, send messages to actors
void C_VisibilityDetector::Update(SceneGraph::C_SceneGraph *pSceneGraph)
{
	// Remove deleted items from the currently visible list.
	RemoveAll_NonOrdered(m_CurrentlyVisibleList, s_objectsToDelete);

	// process currently visible list, check if anything objects have stopped being visible
	for (int i=0; i<m_CurrentlyVisibleList.Size(); i++)
	{
		C_VisibleDetect *p = (C_VisibleDetect*)m_CurrentlyVisibleList[i];

		int x = p->flag;
		if (!(x&SceneGraph::E_VisDetectFlag_Visible))
		{
			m_pActorList->SendMessageToActorComponent(p->m_Handle, p->m_ComponentId, E_VisibilityMsg_OnHidden);
			p->flag &= ~SceneGraph::E_VisDetectFlag_AddedToVisibleList;

			m_CurrentlyVisibleList.EraseByIndexNonOrdered(i);
			i--;
		}

		p->flag &= ~(SceneGraph::E_VisDetectFlag_Visible|SceneGraph::E_VisDetectFlag_AddedToNewList|SceneGraph::E_VisDetectFlag_DefaultVisible);
	}

	// process newly visible list
	Btl::Vector<C_VisibleDetect*> nvList = (Btl::Vector<C_VisibleDetect*>&) SceneGraph::GetNewlyVisibleList(pSceneGraph);

	// Remove deleted items from the newly visible list.
	RemoveAll_NonOrdered(nvList, s_objectsToDelete);

	// Send messages to new objects
	for (int i=0; i<nvList.Size(); i++)
	{
		C_VisibleDetect *p = (C_VisibleDetect*)nvList[i];

		m_CurrentlyVisibleList.PushBack(p);
		p->flag |= SceneGraph::E_VisDetectFlag_AddedToVisibleList;
		m_pActorList->SendMessageToActorComponent(p->m_Handle, p->m_ComponentId, E_VisibilityMsg_OnVisible);
	}

	// Remove deleted items from the global list.
	if (!s_objectsToDelete.Empty())
	{
		RemoveAll_NonOrdered(m_VisDetectList, s_objectsToDelete);

		// Delete items from the pool.
		for (int i = 0; i < s_objectsToDelete.Size(); i++)
			s_visDetectPool.Free(s_objectsToDelete[i]);

		s_objectsToDelete.Clear();
	}
}

}

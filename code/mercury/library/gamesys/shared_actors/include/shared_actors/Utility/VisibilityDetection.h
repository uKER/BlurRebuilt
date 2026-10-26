#ifndef __ACTOR_VISIBILITYDETECTION_H__
#define __ACTOR_VISIBILITYDETECTION_H__

#include <scenegraph/VisibleDetect.h>
#include <actor_system/ActorDeclare.h>
#include <misc_shared\UID\FourCC.h>
#include "btl/allocators/pool.h"

namespace SceneGraph
{
class C_SceneGraph;
}

namespace Actors
{

/// visibility detection probe
class C_VisibleDetect : public SceneGraph::C_VisibleDetect
{
	friend class C_VisibilityDetector;
	friend class Btl::DynamicPool<C_VisibleDetect>;
protected:
	/// constructor
	C_VisibleDetect() : m_ComponentId(-1) {}

public:
	/// destructor
	~C_VisibleDetect() {};

	Actor::C_ActorHandle				m_Handle;
	Component::C_ComponentInterfaceId	m_ComponentId;
};

class C_VisibilityDetector
{
public:
	/// constructor
	C_VisibilityDetector(Actor::C_ActorList *pActorList);

	/// destructor
	~C_VisibilityDetector();

	/// update visibility detection, send messages to actors
	void Update(SceneGraph::C_SceneGraph *pSceneGraph);

	/// create a C_VisibleDetect to associate with a modelinstance
	static C_VisibleDetect *CreateVisDetect(Actor::C_ActorHandle handle, Component::C_ComponentInterfaceId id = -1);

	/// release a previously created C_VisibleDetect
	static void ReleaseVisDetect(C_VisibleDetect *p);

private:
	/// list of currently visible objects
	Btl::Vector<C_VisibleDetect*> m_CurrentlyVisibleList;

	/// list of allocated C_VisibleDetect structures
	static Btl::Vector<C_VisibleDetect*> m_VisDetectList;

	// Pool to store the C_VisibleDetect objects.
	static Btl::DynamicPool<C_VisibleDetect> s_visDetectPool;

	static Btl::Vector<C_VisibleDetect*> s_objectsToDelete;

	Actor::C_ActorList *m_pActorList;
};

enum E_VisibilityMsg
{
	E_VisibilityMsg_HiBits = FOURCC_HI20('VBLM'), 

	E_VisibilityMsg_OnVisible, 
	E_VisibilityMsg_OnHidden,

	E_VisibilityMsg_End, 
	E_VisibilityMsg_Begin = E_VisibilityMsg_HiBits + 1, 
	E_VisibilityMsg_Count = E_VisibilityMsg_End - E_VisibilityMsg_Begin
};

}

#endif

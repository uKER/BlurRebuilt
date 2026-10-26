#include "stdafx.h"

#include <Gravity/NarrowPhase/Primitive.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>

#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionStructs.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionManager.h>
#include <EditorLib/Collision/CollisionScene.h>

#include <EditorLib/Aftk/AftkCollisionBuilder.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

//TODO: Visitor pattern here.

static const F32 kDefaultCollisionRadius = 0.4f;

namespace EditorLib
{

C_CollisionManager::C_CollisionManager()
{
	m_slots.Connect();
	m_pGlobalCollisionScene = new C_CollisionScene;
	m_collisionScenes.PushBack(m_pGlobalCollisionScene);

	//Create a collision proxy for locators.
	m_pDefaultCollisionSource = new C_CollisionProxy;
	//Set proxy bounds.
	((C_CollisionProxy*)m_pDefaultCollisionSource)->boundingSphere = Maths::C_Sphere(
		Maths::C_Vertex(0,0,0), kDefaultCollisionRadius);

	C_Status status = m_pGlobalCollisionScene->AddCollisionSource(
		m_pDefaultCollisionSource);
}

C_CollisionManager::~C_CollisionManager()
{
	//Everything should have been nicely unlinked by now.
	assert(m_aftkCollisionScenes.Size() == 0);
	assert(m_levelCollisionScenes.Size() == 0);
	//Should only contain global scene.
	assert(m_collisionScenes.Size() == 1);
	delete m_pGlobalCollisionScene;
	//Deleted with collision scene.
	m_pDefaultCollisionSource = 0;
	m_slots.Disconnect();
}

C_CollisionScene& C_CollisionManager::GetGlobalCollisionScene()
{
	return *m_pGlobalCollisionScene;
}

const Btl::List<C_CollisionScene*>& C_CollisionManager::GetCollisionScenes()
{
	return m_collisionScenes;
}

void C_CollisionManager::AddCollisionScene(C_CollisionScene* pCollisionScene, const char* aftkFilePath)
{
	if (aftkFilePath)
	{
		C_CollisionManager::AftkCollisionHashMap::InsertionReturnType result = 
			m_aftkCollisionScenes.Insert(aftkFilePath);
		if (!result.inserted)
		{
			assert(!"Failed to insert new collision scene!");
			return;
		}
		result.iterator->GetValue() = pCollisionScene;
	}

	m_collisionScenes.PushBack(pCollisionScene);
}

void C_CollisionManager::RemoveCollisionScene(C_CollisionScene* pCollisionScene)
{
	if (m_pGlobalCollisionScene == pCollisionScene)
	{
		assert(!"Cannot delete global collision scene!");
		return;
	}

	for (C_CollisionManager::AftkCollisionHashMap::Iterator it = m_aftkCollisionScenes.Begin(); 
		it != m_aftkCollisionScenes.End(); ++it)
	{
		if (it->GetValue() == pCollisionScene)
		{
			m_aftkCollisionScenes.Erase(it);
			break;
		}
	}

	for (Btl::List<C_CollisionScene*>::Iterator it = m_collisionScenes.Begin(); 
		it != m_collisionScenes.End(); ++it)
	{
		if (*it == pCollisionScene)
		{
			m_collisionScenes.Erase(it);
			delete pCollisionScene;
			return;
		}
	}

	assert(!"Failed to remove collision scene!");
}

//Find all intersections (at most one per object).
U32 C_CollisionManager::Intersect(
	C_RayResult* pResults,
	U32 resultCapacity,
	const C_Ray& ray,
	E_CollisionQueryType queryType,
	U32 sourceTypeFilter) const
{
	U32 nResults = 0;

	for (Btl::List<C_CollisionScene*>::ConstIterator it = m_collisionScenes.Begin();
		it != m_collisionScenes.End(); ++it)
	{
		const C_CollisionScene* pScene = *it;
		U32 nIntersectResults = 
			pScene->Intersect(pResults, resultCapacity, ray, queryType, sourceTypeFilter);
		if (nIntersectResults > 0)
		{
			//Adjust offset and alter remaining capacity.
			pResults = pResults + nIntersectResults;
			nResults += nIntersectResults;
			resultCapacity -= nResults;
		}

		if (resultCapacity == 0)
			break;
	}

	return nResults;
}

//Find all intersections (at most one per object).
bool C_CollisionManager::Intersect(
	C_RayResult& result,
	const C_Ray& ray,
	E_CollisionQueryType queryType,
	U32 sourceTypeFilter) const
{
	bool intersects = false;

	C_RayResult tmp;
	for (Btl::List<C_CollisionScene*>::ConstIterator it = m_collisionScenes.Begin();
		it != m_collisionScenes.End(); ++it)
	{
		const C_CollisionScene* pScene = *it;
		if (pScene->Intersect(tmp, ray, queryType, sourceTypeFilter) &&
			tmp.distanceSquared < result.distanceSquared)
		{
			intersects = true;
			result = tmp;
		}
	}

	return intersects;
}

bool C_CollisionManager::Intersect(
	C_RayResult& result,
	const C_Ray& ray,
	I_DgNode* pDgNode,
	E_CollisionQueryType queryType) const
{
	for (Btl::List<C_CollisionScene*>::ConstIterator it = m_collisionScenes.Begin();
		it != m_collisionScenes.End(); ++it)
	{
		C_CollisionScene* pCol = *it;
		C_CollisionScene::CollisionInstanceMapping::ConstIterator instIt =
			pCol->GetCollisionInstanceMapping().Find(const_cast<I_DgNode*>(pDgNode));
		if (instIt == pCol->GetCollisionInstanceMapping().End()) continue;

		I_CollisionInstance* pInstance = instIt->GetValue();
		if (pCol->Intersect(result, ray, pInstance, queryType))
			return true;
	}

	return false;
}

bool C_CollisionManager::NearestPoint(
	C_NearestPointResult& result,
	const C_Ray& ray,
	U32 sourceTypeFilter) const
{
	bool foundPoint = false;
	result.distanceSquared = FLT_MAX;

	for (Btl::List<C_CollisionScene*>::ConstIterator it = m_collisionScenes.Begin();
		it != m_collisionScenes.End(); ++it)
	{
		C_NearestPointResult tmpResult;
		tmpResult.distanceSquared = FLT_MAX;
		C_CollisionScene* pCol = *it;
		if (pCol->NearestPoint(tmpResult, ray, sourceTypeFilter))
		{
			if (tmpResult.distanceSquared < result.distanceSquared)
			{
				result = tmpResult;
				foundPoint = true;
			}
		}
	}

	return foundPoint;
}

bool C_CollisionManager::NearestPoint(
	C_NearestPointResult& result, 
	const Vector3& point) const
{
	bool foundPoint = false;
	result.distanceSquared = FLT_MAX;

	for (Btl::List<C_CollisionScene*>::ConstIterator it = m_collisionScenes.Begin();
		it != m_collisionScenes.End(); ++it)
	{
		C_NearestPointResult tmpResult;
		tmpResult.distanceSquared = FLT_MAX;
		C_CollisionScene* pCol = *it;
		if (pCol->NearestPoint(tmpResult, point))
		{
			if (tmpResult.distanceSquared < result.distanceSquared)
			{
				result = tmpResult;
				foundPoint = true;
			}
		}
	}

	return foundPoint;
}

void C_CollisionManager::OnAddCollisionSource(I_CollisionSource* pSource, const char* aftkPath)
{
}

void C_CollisionManager::OnRemoveCollisionSource(I_CollisionSource* pSource)
{
}

const C_CollisionManager::AftkCollisionHashMap& C_CollisionManager::GetAftkCollisionSceneMap() const
{
	return m_aftkCollisionScenes;
}

I_CollisionInstance* C_CollisionManager::FindCollisionInstance(I_DgNode* pNode)
{
	for (Btl::List<C_CollisionScene*>::ConstIterator it = m_collisionScenes.Begin(); 
		it != m_collisionScenes.End(); ++it)
	{
		const C_CollisionScene::CollisionInstanceMapping& mapping = (*it)->GetCollisionInstanceMapping();
		C_CollisionScene::CollisionInstanceMapping::ConstIterator mappingIt = mapping.Find(
			pNode);
		if (mappingIt != mapping.End())
		{
			//Found our node.
			return mappingIt->GetValue();
		}
	}

	return 0;
}

I_CollisionSource& C_CollisionManager::GetDefaultCollisionSource()
{
	return *m_pDefaultCollisionSource;
}

void C_CollisionManager::OnLevelClosed(const I_Level& level)
{
}

////// SLOTS //////

void C_CollisionManager::C_Slots::Connect()
{
	C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelClosed.connect(this, &C_CollisionManager::C_Slots::OnLevelClosedSlot);

	s.aftkRefLoaded.connect(this, &C_CollisionManager::C_Slots::OnAftkRefLoadedSlot);
	s.aftkRefUnloaded.connect(this, &C_CollisionManager::C_Slots::OnAftkRefUnloadedSlot);
}

void C_CollisionManager::C_Slots::Disconnect()
{
	C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelClosed.disconnect(this);

	s.aftkRefLoaded.disconnect(this);
	s.aftkRefUnloaded.disconnect(this);
}

void C_CollisionManager::C_Slots::OnLevelClosedSlot(const I_Level& level)
{
	C_Core::Instance().GetCollisionManager().OnLevelClosed(level);
}

void C_CollisionManager::C_Slots::OnAftkRefLoadedSlot(
	const Btl::String& originalFilePath, 
	const Aftk::C_Scene& aftkScene, 
	const EditorLib::C_SceneNode& sceneNode, 
	const EditorLib::AftkDgMap& aftkDgMap)
{
	C_Status status;

	const C_CollisionManager::AftkCollisionHashMap& aftkMap = 
		C_Core::Instance().GetCollisionManager().GetAftkCollisionSceneMap();
	if (aftkMap.Find(originalFilePath.Chars()) != aftkMap.End())
	{
		assert(!"Attempted to insert duplicate collision scene. AftkRef system likley to be broken!");
		return;
	}

	C_CollisionScene* pCollisionScene = new C_CollisionScene();
	C_Core::Instance().GetCollisionManager().AddCollisionScene(
		pCollisionScene, originalFilePath.Chars());

	C_AftkCollisionBuilder collisionBuilder;
	status = collisionBuilder.Build(*pCollisionScene, originalFilePath, aftkDgMap);

}

void C_CollisionManager::C_Slots::OnAftkRefUnloadedSlot(
	const Btl::String& originalFilePath)
{
	const C_CollisionManager::AftkCollisionHashMap& aftkMap = 
		C_Core::Instance().GetCollisionManager().GetAftkCollisionSceneMap();
	C_CollisionManager::AftkCollisionHashMap::ConstIterator it = aftkMap.Find(originalFilePath.Chars());
	if (it == aftkMap.End())
	{
		assert(!"Failed to find collision scene. AftkRef system likley to be broken!");
		return;
	}

	C_Core::Instance().GetCollisionManager().RemoveCollisionScene(it->GetValue());
}

void C_CollisionManager::RegisterCollidableHandler(Ut::Delegate<bool(const I_DgNode*)> pHandler)
{
	m_collidable.PushBack(pHandler);
}

void C_CollisionManager::UnregisterCollidableHandler(Ut::Delegate<bool(const I_DgNode*)> pHandler)
{
	for (Btl::List<Ut::Delegate<bool(const I_DgNode*)> >::Iterator it = m_collidable.Begin(); 
		it != m_collidable.End(); ++it)
	{
		if (*it == pHandler)
		{
			m_collidable.Erase(it);
			break;
		}
	}
}

bool C_CollisionManager::IsCollidable(const I_DgNode* pDgNode)
{
	for (Btl::List<Ut::Delegate<bool(const I_DgNode*)> >::Iterator it = m_collidable.Begin(); 
		it != m_collidable.End(); ++it)
	{
		if (!(*it)(pDgNode))
			return false;
	}

	return true;
}

}

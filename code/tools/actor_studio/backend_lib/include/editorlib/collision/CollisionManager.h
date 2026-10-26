#ifndef EDITOR_LIB_COLLISION_MANAGER_H
#define EDITOR_LIB_COLLISION_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <sigslot/sigslot.h>

#include <Btl/Containers/List.h>
#include <Btl/Containers/openhashmap.h>
#include <Btl/Utility/string.h>
#include <Btl/Utility/Hashops.h>

//Implementation note:
//Dg objects do not store a reference to their collision data. Instead the collision manager
//maps between dg nodes and collision data.

namespace EditorLib
{
	struct I_CollisionSource;
	struct I_CollisionInstance;
	class C_CollisionScene;

	class C_CollisionManager
	{
	public:
		typedef Btl::OpenHashMap<Btl::String, C_CollisionScene*, Btl::HashKeyOpBtlString> 
			AftkCollisionHashMap;

		C_CollisionManager();
		~C_CollisionManager();

		//Global collision scene used for collidable objects that are not from an aftk scene.
		//For example: box or sphere property shapes.
		C_CollisionScene& GetGlobalCollisionScene();

		const Btl::List<C_CollisionScene*>& GetCollisionScenes();

		void AddCollisionScene(C_CollisionScene* pCollisionScene, const char* aftkFilePath = 0);
		void RemoveCollisionScene(C_CollisionScene* pCollisionScene);

		I_CollisionSource&	GetDefaultCollisionSource();

		const AftkCollisionHashMap& GetAftkCollisionSceneMap() const;

		I_CollisionInstance* FindCollisionInstance(I_DgNode* pNode);

		//Find all intersections (at most one per object). Returns num. results.
		U32 Intersect(
			C_RayResult* pResults,		//OUT
			U32 resultCapacity,			//result array capacity initially.
			const C_Ray& ray,
			E_CollisionQueryType queryType,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		bool Intersect(
			C_RayResult& result,
			const C_Ray& ray,
			E_CollisionQueryType queryType,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		//Find nearest object intersection.
		bool Intersect(
			C_RayResult& result,
			const C_Ray& ray,
			I_DgNode* pNode,
			E_CollisionQueryType queryType) const;

		bool NearestPoint(
			C_NearestPointResult& result,
			const C_Ray& ray,
			U32 sourceTypeFilter = kCollisionSourceType_MaskAll) const;

		bool NearestPoint(
			C_NearestPointResult& result, 
			const Vector3& point) const;

		bool IsCollidable(const I_DgNode* pDgNode);

		void RegisterCollidableHandler(Ut::Delegate<bool(const I_DgNode*)> pHandler);
		void UnregisterCollidableHandler(Ut::Delegate<bool(const I_DgNode*)> pHandler);

	private:
		void OnAddCollisionSource(I_CollisionSource* pSource, const char* aftkPath);
		void OnRemoveCollisionSource(I_CollisionSource* pSource);

		void OnLevelClosed(const I_Level& level);

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			void Connect();
			void Disconnect();

			void OnLevelClosedSlot(const I_Level& level);

			void OnAftkRefLoadedSlot(
				const Btl::String& originalFilePath, 
				const Aftk::C_Scene& aftkScene, 
				const EditorLib::C_SceneNode& sceneNode, 
				const EditorLib::AftkDgMap& aftkDgMap);
			void OnAftkRefUnloadedSlot(
				const Btl::String& originalFilePath);

			FRIEND_CLASS C_CollisionManager;
		};

		C_Slots m_slots;
		//Hash-table of aftk collision scenes keyed by the original aftk filename.
		AftkCollisionHashMap m_aftkCollisionScenes;
		//Hash-table of xt level collision scenes keyed by Xt Id of the level record.
		typedef Btl::OpenHashMap<XtId, C_CollisionScene*> LevelCollisionHashMap;
		LevelCollisionHashMap m_levelCollisionScenes;
		C_CollisionScene* m_pGlobalCollisionScene;

		//A flat array containing all collision scenes.
		Btl::List<C_CollisionScene*> m_collisionScenes;
		Btl::List<Ut::Delegate<bool(const I_DgNode*)> > m_collidable;

		//Collision source for objects such as locators.
		I_CollisionSource*	m_pDefaultCollisionSource;

		FRIEND_CLASS C_CollisionScene;
	};
}

//Collision with dynamic objects: locators, splines and mesh (e.g. instances of actor mesh)
//update with transform
//		Btl::List<C_CollisionDef> defs;

#endif

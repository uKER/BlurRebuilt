#ifndef EDITOR_LIB_AFTK_COLLISION_BUILDER_H
#define EDITOR_LIB_AFTK_COLLISION_BUILDER_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <btl/utility/string.h>

#include <EditorLib/Misc/Status.h>

namespace EditorLib
{
	class C_AftkCollisionBuilder
	{
	public:
		C_Status Build(
			C_CollisionScene& collisionScene, 
			const Btl::String& originalFilePath,
			const AftkDgMap& aftkDgMap);

	private:
		typedef Ut::OpenHashMap<
			const Aftk::C_SceneNode*, 
			I_CollisionSource*, Ut::HashKeyOpPtr> AftkCollisionSourceMap;

		//Build collision sources as well as a mapping from Aftk node -> source
		C_Status BuildSources(
			C_CollisionScene& collisionScene, 
			AftkCollisionSourceMap& collisionSourceMap,
			const Btl::String& originalFilePath,
			const AftkDgMap& aftkDgMap);

		C_Status AddMeshSource(
			C_CollisionScene& collisionScene, 
			AftkCollisionSourceMap& collisionSourceMap,
			const Btl::String& originalFilePath,
			const Aftk::C_SceneMeshNode* pAftkMesh);

		C_Status AddSplineSource(
			C_CollisionScene& collisionScene, 
			AftkCollisionSourceMap& collisionSourceMap,
			const Btl::String& originalFilePath,
			const Aftk::C_SceneSplineNode* pAftkSpline);

		C_Status AddPointSource(
			C_CollisionScene& collisionScene, 
			AftkCollisionSourceMap& collisionSourceMap,
			const Aftk::C_SceneNode* pAftkNode);

		//Build mesh/spline refs and point collision
		C_Status BuildCollisionInstances(
			C_CollisionScene& collisionScene, 
			const AftkCollisionSourceMap& collisionSourceMap,
			const AftkDgMap& aftkDgMap);
	};
}

#endif

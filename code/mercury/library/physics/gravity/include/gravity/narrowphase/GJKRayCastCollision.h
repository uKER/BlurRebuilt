#ifndef GJK_RAYCAST_COLLISION
#define GJK_RAYCAST_COLLISION

//#include "gravity/narrowphase/Primitive.h"
//
//#include "gravity/narrowphase/CollisionResult.h"
//#include "gravity/narrowphase/CollisionManager.h"
//#include "gravity/narrowphase/GJK.h"

namespace Collision
{

	//Fat ray cast
	bool GJKRayCastCapsule(const C_Shape* const __restrict shape, C_Ray& ray);

	bool GJKRayCastCylinder(const C_Shape* const __restrict shape, C_Ray& ray);

	bool GJKRayCastConvex(const C_Shape* const __restrict shape, C_Ray& ray);

	bool GJKRayCastCone(const C_Shape* const __restrict shape, C_Ray& ray);

	bool GJKRayCastBox(const C_Shape* const __restrict shape, C_Ray& ray);

	bool GJKRayCastSphere(const C_Shape* const __restrict shape, C_Ray& ray);

	bool GJKRayCastComposite(const C_Shape* const __restrict shape, C_Ray& ray,  SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);


	//Fat transparent ray cast
	bool GJKRayCastCapsule(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);

	bool GJKRayCastCylinder(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);

	bool GJKRayCastConvex(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);

	bool GJKRayCastCone(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);

	bool GJKRayCastBox(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);

	bool GJKRayCastSphere(const C_Shape* const __restrict shape, C_TransparentRay& ray, SharedPhysics::C_GraphNode* pGraphNode);

	bool GJKRayCastComposite(const C_Shape* const composite, C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, C_CollisionFilter* pRayFilter, SharedPhysics::C_SmallSPUCache& spuCache,
		SharedPhysics::C_GraphNode* pGraphNode);
}




#endif
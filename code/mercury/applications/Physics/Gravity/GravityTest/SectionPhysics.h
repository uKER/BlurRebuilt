#ifndef SAMPLES_SECTIONPHYSICS_H
#define SAMPLES_SECTIONPHYSICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include <Render/Sky/SkyDome.h>
#include <Render/Light/Light.h>
#include <Render/Shadow/ShadowMapStandard.h>
#include <Render/Shadow/ShadowMapVariance.h>
#include <Render/Texture/TextureUtils.h>
#include "gravity/narrowphase/Primitive.h"
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/treebuilder/KDTree.h"
#include "gravity/narrowphase/OptimisedAABBTree.h"
#include "gravity/narrowphase/OptimisedKDTree.h"
#include "ConstraintGraphTest.h"



namespace Render
{
namespace Debug
{
class C_DebugRenderSys;
}
namespace Font
{
	class C_FontSystem;
}
};

namespace Samples
{

class C_SectionPhysics : public C_Section
{
public:
	
	C_SectionPhysics(const Util::C_String& name);
	~C_SectionPhysics();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);

protected:
	void CollideDebugRender();
	void TestCollideCapsuleCapsule();
	void TestCapsuleTriangle();
	void TestCapsuleSphere();
	void TestCollidedCapsuleCapsule();
	void TestCollidedCapsuleSphere();
	void TestCollidedCapsuleTriangle();
	void TestCapsuleRay();
	void TestTriangleOBB();
	void TestCollidedOBBTriangle();
	void TestCollidedOBBCapsule();
	void TestOBBSegment();
	void TestOBBRay();
	void TestCollidedOBBSphere();
	void TestCollidedShpereTriangle();
	void TestIntersectSegmentTriangle();
	void TestClosestPtSegmentTriangle();
	void TestSphereRay();
	void TestHeapsort();
	void GJKCapsuleCapsule();
	void GJKAABBCapsule();
	void GJKAABBsTest();
	void GJKOBBsTest();
	void GJKCylinderCyliner();
	void GJKConvexTriangle();
	void GJKConeSphere();
	void GJKConeCylinder();
	void GJKConeCapsule();
	void GJKConeBox();
	void GJKConeTriangle();
	void GJKMarginCylinderTriangle();
	void GJKMarginCylinderCapsule();
	void GJKMargin();
	void GJKRayCastCapsuleTriangleTest();
	void GJKRayCastOBBTriangleTest();
	void GJKRayCastCapsuleOBBTest();
	void OBBsIntersectionTest();
	void TestBuildAABBTree();
	void TestAABBTreeRay();
	void TestKDTreeRay();
	void TestAABBTreeSphere();
	void TestAABBTreeOBB();
	void TestBuildKDTree();
	void BuildKDTree();
	void TestOBBOBB();
	void OBBOBBCollisionTest();
	void DrawAABBTreeNode(Baking::AABBTreeNode* node, Baking::AABBTree& tree, U32 depth, U32 maxDepth,
		const Collision::C_CRAxisBox& box);
	//void DrawKDTreeNode(Baking::KDTreeNode* node, Baking::KDTree& tree, U32 depth, U32 maxDepth);
	//void SweepAndPruneTest();
	void TestPhysicsEngine(F32 timer);
	void TestConvexHull();
	void TestTriangleTriangle();
	void TestCylinderTriangle();
	void TestCylinderRay();
	void TestCylinderSphere();
	void TestConvexHullRay();
	void TestConeRay();
	void TestConeTriangle();
	void TestConeSphere();
	//void TestGJKConvexHullTriangle();

	void TestSweptRayTriangle();
	void TestSweptSphereSphere();
	void TestSweptSphereOBB();
	void TestSweptSphereCapsule();
	void TestSweptSphereCylinder();
	
	/*void TestFinitePlaneSphere();
	void TestFinitePlaneOBB();*/

	void TestXenoCollideBoxBox();

	void BuildAABBTree();
	bool OnCreate();
	bool OnCreateVolatile();
	void OnDestroyVolatile();
	void OnDestroy();

private:
	void PhysicsTestUpdate(const C_Input& input, const C_Timer& timer);

private:

	//C_Level							m_Level;
	Render::Sky::C_SkyDome			m_SkyDome;
	bool							m_bDebounceSpace;
	bool							m_bDoMovement;
	bool							m_backward;
	//Ut::Vector<C_RayInfo>			m_RayInfoList;
	Render::Font::C_FontSystem*		m_pFontSystem;
	Render::Debug::C_DebugRenderSys*m_pDebugRenderSys;

	U64								m_iTimer;
	C_Input							m_input;
	Collision::OptimisedAABBTree	m_optTree;//optimised tree
	Baking::AABBTree				m_tree;	//no optimised tree 
//	Baking::KDTree					m_kdtree; 
	Collision::OptimisedKDTree		m_OptKdTree;
	C_TestPhysics					m_TestPhysics;

#ifdef PLATFORM_XENON
	HANDLE							m_hMainThread;
#endif
};

}; // namespace Samples

#endif // SAMPLES_SECTIONPHYSICS_H

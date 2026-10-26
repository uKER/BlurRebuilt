#ifndef COMPOSITESHAPECOLLISION_INL
#define COMPOSITESHAPECOLLISION_INL

//#include "gravity/narrowphase/CompositeShapeCollision.h"
#include "gravity/narrowphase/CollisionManager.h"

namespace Collision
{
/*
	both _compositeShape and shape are ppu pointer
*/
template <int MaxPatches, int MaxContacts>
inline bool CollideCompositeShape(C_CollisionManager& manager, const Collision::C_Shape* const  _pCompositeShape, const C_Shape* const  pShape, C_CollisionResult<MaxPatches, MaxContacts>& results,
								  SharedPhysics::C_SafeStack<true>& stack, EPA& m_Epa, bool bFlip, Collision::C_ContactPatch<MaxContacts>& patch)
{

	Spu::MemBaseMarker _marker;
	//DebugBreak();
	const Maths::HF32 closeNormal(SharedPhysics::Constants::CloseNormal());
	C_CompositeShape* pCompositeShape=(C_CompositeShape* )_pCompositeShape;
	//C_ShapeWrapper* pShapeWrapperPtr = (C_ShapeWrapper*)Spu::Get((U32*)&pCompositeShape->m_pShapes);
	//const U32 numShapes = Spu::Get(&pCompositeShape->m_NumShapes);

	//OK. Test bounding box of pair of objects to see if they hit each-other...To do this, 
	//I'm going to work one of the bounding boxes into the local space of the other and test for intersection!

	
	//const Maths::HMat43 relativeMatrix(pShape->m_PosRot * pCompositeShape->m_PosRot.InverseRotTrans());
	Collision::C_CRAxisBox localBoxB;
	/*pShape->m_LocalBoundingBox.Transform(relativeMatrix, localBoxB);
	if(!localBoxB.Intersects(pCompositeShape->m_LocalBoundingBox))
	{
		return false;
	}*/

	Collision::C_CompositeShapeIterator iter(pCompositeShape);
	
	bool collided=false;
//#ifndef PLATFORM_SPU
//	C_CollisionResult<MaxPatches, MaxContacts> temp;
//#else
//	C_CollisionResult<MaxPatches, MaxContacts>* _temp = (C_CollisionResult<MaxPatches, MaxContacts>*)Spu::Alloc(sizeof(C_CollisionResult<MaxPatches, MaxContacts>));
//	C_CollisionResult<MaxPatches, MaxContacts>& temp = *_temp;
//#endif

	//Collision::C_CollisionFilter* pShapeFilter = (Collision::C_CollisionFilter*)Spu::FetchAlloc(&pShape->m_Filter, sizeof(Collision::C_CollisionFilter));

	//for(U32 i=0; i< numShapes; ++i)
	while(iter.HasNextShape())
	{
		Spu::MemBaseMarker marker;
		iter.NextShape();
		Collision::C_Shape* pSubShape = iter.GetCurrentShape<C_Shape>();
		//DebugBreak();
		//const C_ShapeWrapper* shapeWrapper =&compositeShape->m_pShapes[i];
		//const C_Shape* const pShape=shapeWrapper->m_pShape;
		//const C_ShapeWrapper* pWrapperPtr = (const C_ShapeWrapper*)Spu::Get((U32*)(&(pCompositeShape->m_pShapes + i)));
		//const C_ShapeWrapper* pShapeWrapper=(const C_ShapeWrapper*)Spu::FetchAlloc(&pShapeWrapperPtr[i], sizeof(C_ShapeWrapper));
		//const C_Shape* const pCompositeShape=(const Collision::C_Shape*)Spu::FetchAlloc(pShapeWrapper->m_pShape, sizeof(C_Shape) );
		
		//const Collision::C_CollisionFilter* pCompositeFilter=(const C_CollisionFilter*)Spu::FetchAlloc(&pShapeWrapper->m_pShape->m_Filter, sizeof(Collision::C_CollisionFilter));
	
		/*Collision::C_CollisionFilter* pShapeFilter=(C_CollisionFilter*)Spu::FetchAlloc((void*)Spu::Get((U32*)&pShape->m_pFilter), sizeof(Collision::C_CollisionFilter));*/

		//Base shape may have been allowed to collide, but sub-shape may not (think shootable mesh attached
		//to a collidable body)

		const Maths::HMat43 relativeMatrix2(pShape->m_PosRot * pSubShape->m_PosRot.InverseRotTrans());
		pShape->m_LocalBoundingBox.Transform(relativeMatrix2, localBoxB);

		if(localBoxB.Intersects(pSubShape->m_LocalBoundingBox) && pSubShape->m_Filter.CanCollide(&pShape->m_Filter))
		{
			collided = manager.Collide(pSubShape, pShape, results, stack, m_Epa, bFlip, patch) || collided;
		}
	}
	return collided;
}

};

#endif

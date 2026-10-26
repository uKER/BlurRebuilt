#include "Stdafx.h"
#include "gravity/sdk/AnimatedShapeController.h"
#include "gravity/sdk/scene.h"
#include "gravity/narrowphase/Shapes.h"


namespace Gravity
{

	C_AnimatedShapeController::C_AnimatedShapeController(Gravity::C_Body* pBody): m_pBody(pBody), m_pScene(NULL), m_pPrev(NULL), m_pNext(NULL), m_Active(false),
		m_Transforms(NULL)
#ifndef CFG_SPEED
		, m_nTransforms(0)
#endif
	{
		if(m_pBody)
		{
			Collision::C_CompositeShape* pShape = (Collision::C_CompositeShape*)m_pBody->GetShape();
			m_Transforms = new Maths::HMat43[pShape->m_NumShapes];
#ifndef CFG_SPEED
			m_nTransforms = pShape->m_NumShapes;
#endif
			m_pBody->GetGraphNode()->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_AnimatedShape);
		}
	}

	C_AnimatedShapeController::~C_AnimatedShapeController()
	{
		if(m_pBody)
			m_pBody->GetGraphNode()->ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_AnimatedShape);
		m_pBody = NULL;
		if(m_Transforms)
			delete[] m_Transforms;
		m_Transforms = NULL;
#ifndef CFG_SPEED
		m_nTransforms = 0;
#endif
	}

	void C_AnimatedShapeController::SetScene(C_Scene* pScene)
	{
		m_pScene = pScene;
	}

	C_Body* C_AnimatedShapeController::GetBody()
	{
		return m_pBody;
	}

	Collision::C_CompositeShape* C_AnimatedShapeController::GetCompositeShape()
	{
		return (Collision::C_CompositeShape*)Spu::Get((INTPTR*)&m_pBody->m_pShape);	// [Blur2PC][x64] pointer-wide
	}

	bool C_AnimatedShapeController::operator < (const C_AnimatedShapeController& controller)
	{
		return *m_pBody < *controller.m_pBody;
	}
	bool C_AnimatedShapeController::operator <= (const C_AnimatedShapeController& controller)
	{
		return *m_pBody <= *controller.m_pBody;
	}
	bool C_AnimatedShapeController::operator > (const C_AnimatedShapeController& controller)
	{
		return *m_pBody > *controller.m_pBody;
	}
	bool C_AnimatedShapeController::operator >= (const C_AnimatedShapeController& controller)
	{
		return *m_pBody >= *controller.m_pBody;
	}
	bool C_AnimatedShapeController::operator == (const C_AnimatedShapeController& controller)
	{
		return *m_pBody == *controller.m_pBody;
	}
	bool C_AnimatedShapeController::operator != (const C_AnimatedShapeController& controller)
	{
		return *m_pBody != *controller.m_pBody;
	}
	
	void C_AnimatedShapeController::Activate()
	{
		if(m_Active || m_pScene == NULL)
			return;

		m_Active = true;
		m_pNext = m_pScene->m_pActiveAnimatedShapeControllers;
		if(m_pScene->m_pActiveAnimatedShapeControllers)
			m_pScene->m_pActiveAnimatedShapeControllers->m_pPrev = this;
		m_pPrev = NULL;
		m_pScene->m_pActiveAnimatedShapeControllers = this;

	}
	void C_AnimatedShapeController::Deactivate()
	{
		if(!m_Active || m_pScene == NULL)
			return;
		m_Active = false;
		if(m_pScene->m_pActiveAnimatedShapeControllers == this)
			m_pScene->m_pActiveAnimatedShapeControllers = m_pNext;
		if(m_pNext)
			m_pNext->m_pPrev = m_pPrev;
		if(m_pPrev)
			m_pPrev->m_pNext = m_pNext;
		m_pNext = m_pPrev = NULL;
	}


	void C_AnimatedShapeController::UpdateTransform(Collision::C_CompositeShape* pSpuCompShape)
	{
		using namespace Maths;
		const HF32 half(Constants<HF32>::Half());
		
		Collision::C_CompositeShapeIterator iter(pSpuCompShape);
		S32 parentIndex = pSpuCompShape->GetIndex();

		HVec3 vMin(Constants<HVec3>::Max());
		HVec3 vMax(-Constants<HVec3>::Max());
		Collision::C_CRAxisBox axisBox;

		while(iter.HasNextShape())
		{
			Spu::MemBaseMarker marker;
			iter.NextShape();
			Collision::C_Shape* pSubShape = iter.GetCurrentShape<Collision::C_Shape>();
			const S32 index = pSubShape->GetIndex();

			if(index >= 0 && index != parentIndex)
			{
				const HMat43* elementMat = (HMat43*)Spu::FetchAlloc(&m_Transforms[index], sizeof(HMat43));
				const HVec3 elementOffset = pSubShape->m_ElementOffset * elementMat->Rotation();
				pSubShape->m_RelativeTransformation = HMat43(elementMat->Rotation(), elementMat->Translation() + elementOffset - pSpuCompShape->m_ElementOffset);
#ifdef PLATFORM_SPU
				Spu::Store(&pSubShape->m_pPpuPtr->m_RelativeTransformation, &pSubShape->m_RelativeTransformation, sizeof(HMat43));
#endif
				if (pSubShape->GetShape() == Collision::E_Composite_Shape)
				{
					UpdateTransform((Collision::C_CompositeShape*)pSubShape);
				}
			}

			//Recalculate local bounding box...	
			pSubShape->m_LocalBoundingBox.Transform(pSubShape->m_RelativeTransformation, axisBox);

			vMin = Min(vMin, axisBox.centre - axisBox.extents);
			vMax = Max(vMax, axisBox.centre + axisBox.extents);
		}
		pSpuCompShape->m_LocalBoundingBox.centre = (vMin + vMax) * half;
		pSpuCompShape->m_LocalBoundingBox.extents = (vMax - vMin) * half;
#ifdef PLATFORM_SPU
		Spu::Store(&pSpuCompShape->m_pPpuPtr->m_LocalBoundingBox, &pSpuCompShape->m_LocalBoundingBox, sizeof(Collision::C_CRAxisBox) );
#endif
	}
}
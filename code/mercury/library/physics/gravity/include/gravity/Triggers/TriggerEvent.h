#ifndef TRIGGEREVENT_H
#define TRIGGEREVENT_H

#include "gravity/narrowphase/Shapes.h"

namespace Gravity
{

class C_TriggerShape
{
public:
	enum E_TriggerType
	{
		E_TriggerBox,
		E_TriggerSphere
	};
	//Matrix and local bounding box!
	Maths::HMat43 m_PosRot;
	Maths::HVec3 m_LocalExtents;
	Collision::C_CollisionFilter m_Filter;
	U32 m_TriggerType;

	//Could be virtual, but we want to be able to use the SPU to detect collision, so don't do it!
	//virtual bool isInsideTriggerShape(Collision::C_Convex* pShape) = 0;

	C_TriggerShape(E_TriggerType type, Collision::C_CollisionFilter* pFilter) : m_TriggerType(type), m_Filter(*pFilter),
		m_LocalExtents(Maths::UNDEFINED), m_PosRot(Maths::UNDEFINED)
	{
	}

	virtual ~C_TriggerShape()
	{
	}

	void SetFilter(Collision::C_CollisionFilter* pFilter)
	{
		m_Filter = *pFilter;
	}

	virtual void CalculateLocalBoundingBox() = 0;

	virtual void Transform(const Maths::HMat43 PCREF matrix) = 0;

	void GetTransformedBoundingBox(Collision::C_CRAxisBox& box);

	__forceinline bool CanCollide(C_TriggerShape* pOtherShape)
	{
		return m_Filter.CanCollide(&pOtherShape->m_Filter);
	}

	__forceinline bool CanCollide(Collision::C_Shape* pOtherShape)
	{
		return m_Filter.CanCollide(&pOtherShape->m_Filter);
	}
};

class C_TriggerBox : public C_TriggerShape
{
public:

	C_TriggerBox(const Maths::HVec3 PCREF halfExtents, Collision::C_CollisionFilter* pFilter): C_TriggerShape(E_TriggerBox, pFilter)
	{
		m_Box.Extent(halfExtents);
		CalculateLocalBoundingBox();
		/*m_Box.Margin(0.04f);
		m_Box.Epsilon(Maths::Constants<Maths::HF32>::Epsilon2());*/
	}

	virtual ~C_TriggerBox()
	{
	}

	NOALIAS inline void SetTriggerBox(Collision::C_OBB& pBox)
	{
		m_Box = pBox;
	}

	bool isInsideTriggerShape(Collision::C_Shape* pShape);

	void CalculateLocalBoundingBox();

	void Transform(const Maths::HMat43 PCREF matrix);

	Collision::C_OBB m_Box;
};

class C_TriggerSphere : public C_TriggerShape
{
public:

	C_TriggerSphere(const Maths::HF32 PCREF radius, Collision::C_CollisionFilter* pFilter): C_TriggerShape(E_TriggerSphere, pFilter)
	{
		m_Sphere.Radius(radius);
		/*m_Sphere.Margin(0.f);
		m_Sphere.Epsilon(Maths::Constants<Maths::HF32>::Epsilon2());*/
		CalculateLocalBoundingBox();
	}

	virtual ~C_TriggerSphere()
	{
	}


	NOALIAS inline void SetTriggerSphere(Collision::C_Sphere& pSphere)
	{
		m_Sphere = pSphere;
	}

	bool isInsideTriggerShape(Collision::C_Shape* pShape);

	void CalculateLocalBoundingBox();

	void Transform(const Maths::HMat43 PCREF matrix);

	Collision::C_Sphere m_Sphere;
};
}


#endif
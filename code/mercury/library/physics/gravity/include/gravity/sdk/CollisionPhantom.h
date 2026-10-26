#ifndef COLLISION_PHANTOM_H
#define COLLISION_PHANTOM_H

#include "gravity/broadphase/Object.h"
#include "gravity/sharedphysics/graphnode.h"

namespace Collision
{
	class C_Shape;
}

namespace Gravity
{
	class C_CollisionPhantom
	{
		friend class C_Scene;
		Gravity::C_Object m_Object;
		SharedPhysics::C_GraphNode m_GraphNode;
		Maths::HVec3 m_OffsetToCentre;
		Collision::C_Shape* m_pShape;
		void* m_pUserData;

	public:
		C_CollisionPhantom(Collision::C_Shape* pShape);

		void Transform(const Maths::HMat43 PCREF transform);

		C_Object* GetObject();
		
		SharedPhysics::C_GraphNode* GetGraphNode();

		Collision::C_Shape* GetShape();

		__forceinline Maths::HVec3 GetOffsetToCentre() const
		{
			return m_OffsetToCentre;
		}

		void SetUserData(void* pData);

		void* GetUserData();

		void DebugRender(DbUtil::I_DebugRender* pRender);
	};
}

#endif

#ifndef SPU_MANAGER_H
#define SPU_MANAGER_H

#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/Shape.h"

namespace Collision
{
	class SPUManager
	{
	public:

		SPUManager()
		{
		}

		~SPUManager()
		{
		}

		Maths::HVec3 Support(const C_Shape* pShape, const Maths::HVec3 PCREF dir);
		Maths::HVec3 Support(const C_Convex* pConvex, const Maths::HVec3 PCREF dir);
		Maths::HVec3 SupportMargin(const C_Convex* pConvex, const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin);
		/*Maths::HVec3 Centre(const C_Convex* pConvex);
		void WarmStart(const C_Convex* pConvex, const Maths::HVec3 PCREF warmStart);
		Maths::HVec3 WarmStart(const C_Convex* pConvex);*/
		bool QuadraticShape(const C_Convex* pConvex);
	};

}
#endif
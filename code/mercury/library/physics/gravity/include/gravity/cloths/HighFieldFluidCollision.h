#ifndef HIGHFIELDFLUIDCOLLISION_H
#define HIGHFIELDFLUIDCOLLISION_H

#include "gravity/Cloths/HighFieldFluid.h"



namespace Collision
{
	class C_CollisionObject;

	class C_HFFCollisionManager
	{
	public:
		C_HFFCollisionManager()
		{
		}

		~C_HFFCollisionManager()
		{
		}
		
		bool Collided(C_Fluid* fluid, const C_CollisionObject* pObject, Maths::HF32& averageHeight);


	};
}

#endif

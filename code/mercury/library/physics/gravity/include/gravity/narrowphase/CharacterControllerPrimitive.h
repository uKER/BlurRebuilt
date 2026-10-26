#ifndef CHARACTER_CONTROLLER_PRIMITIVE_H
#define CHARACTER_CONTROLLER_PRIMITIVE_H

#include "gravity/narrowphase/primitive.h"
#include "gravity/narrowphase/ray.h"

namespace Collision
{
	class C_CharacterControllerPrimitive
	{
	public:
		C_Capsule m_Capsule;
		C_Capsule m_EnclosingCapsule;
		C_Ray m_Ray;
		Maths::HF32 m_RayLength;
		Maths::HVec3 m_Position;
		Maths::HF32 m_CosMaxAngle;
		Maths::HF32 m_MinPenetration;
		Maths::HF32 m_DeadZone;
		U16 m_FeetMaterial;
		U16 m_EffectMaterial;
		bool m_bContactWrittenToStream;
		U32 m_CapsuleCollisionFlags;

	

		C_CharacterControllerPrimitive() : m_RayLength(Maths::UNDEFINED), m_Position(Maths::UNDEFINED),
			m_CosMaxAngle(Maths::UNDEFINED), m_MinPenetration(Maths::UNDEFINED), m_DeadZone(Maths::Constants<Maths::HF32>::Zero()), 
			m_bContactWrittenToStream(false), m_CapsuleCollisionFlags(0)
		{

		}

		C_CharacterControllerPrimitive(const Maths::HF32 PCREF capsuleHeight, const Maths::HF32 PCREF capsuleRadius, 
			const Maths::HF32 PCREF rayLength, const Maths::HF32 PCREF cosMaxAngle, const Maths::HF32 PCREF traceRadius,
			const Maths::HF32 PCREF minPen, const U16 feetMaterial, const U16 effectMaterial, const Collision::C_CollisionFilter* pRayFilter,
			const U32 capsuleCollisionFlags=0) : 
											m_RayLength(rayLength),m_Position(Maths::UNDEFINED), m_CosMaxAngle(cosMaxAngle),
											m_MinPenetration(minPen), m_DeadZone(Maths::UNDEFINED), m_FeetMaterial(feetMaterial), m_EffectMaterial(effectMaterial),
											m_bContactWrittenToStream(false), m_CapsuleCollisionFlags(capsuleCollisionFlags)
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			//Maths::HF32 capsuleHalfHeight(capsuleHeight * Maths::Constants<Maths::HF32>::Half());
			m_Capsule.LocalStart(Maths::HVec3(zero, capsuleHeight, zero));
			m_Capsule.LocalEnd(Maths::HVec3(zero, -capsuleHeight, zero));
			m_Capsule.Radius(capsuleRadius);

			m_Capsule.Epsilon(Maths::Max(capsuleHeight, capsuleRadius) * Maths::HF32(0.005f));
			m_Capsule.Margin(capsuleRadius);

			m_Ray.m_Filter = *pRayFilter;
			m_Ray.m_Radius = traceRadius;
			m_Ray.m_bFatTrace = true;

			m_EnclosingCapsule.LocalStart(m_Capsule.LocalStart());
			Maths::HVec3 end(m_Capsule.LocalEnd());
			end.Y(end.Y() - (m_RayLength));
			m_EnclosingCapsule.LocalEnd(end);
			m_EnclosingCapsule.Radius(capsuleRadius);
			m_EnclosingCapsule.Epsilon(Maths::Max(capsuleHeight, capsuleRadius) * Maths::HF32(0.005f));
			m_EnclosingCapsule.Margin(capsuleRadius);
			m_DeadZone = (m_RayLength + traceRadius) * Maths::Constants<Maths::HF32>::Quarter();
			
		}

		void SetCapsuleCollisionFlags(U32 capsuleCollisionFlags)
		{
			m_CapsuleCollisionFlags = capsuleCollisionFlags;
		}

		Collision::C_RayTracingResult& GetRayResult()
		{
			return m_Ray.m_result;
		}

		void Translate(const Maths::HVec3 PCREF translation)
		{
			using namespace Maths;
			const HF32 half(Constants<HF32>::Half());
			m_Position = translation;
			/*m_Capsule.m_End = m_Capsule.LocalEnd() + translation;
			m_Capsule.m_Start = m_Capsule.LocalStart() + translation;
			m_EnclosingCapsule.m_End = m_EnclosingCapsule.LocalEnd() + translation;
			m_EnclosingCapsule.m_Start = m_EnclosingCapsule.LocalStart() + translation; */
			const HVec3 capsuleEnd(m_Capsule.LocalEnd() + translation);
			const HVec3 capsuleStart(m_Capsule.LocalStart() + translation);
			const HVec3 capsuleCentre((capsuleStart + capsuleEnd)*half);
			m_Capsule.End(capsuleEnd);
			m_Capsule.Start(capsuleStart);
			m_Capsule.Centre(capsuleCentre);
			m_Capsule.WarmStart(capsuleCentre);

			const HVec3 enclosingCapsuleEnd(m_EnclosingCapsule.LocalEnd() + translation);
			const HVec3 enclosingCapsuleStart(m_EnclosingCapsule.LocalStart() + translation);
			const HVec3 enclosingCapsuleCentre((enclosingCapsuleStart + enclosingCapsuleEnd)*half);
			m_EnclosingCapsule.End(enclosingCapsuleEnd);
			m_EnclosingCapsule.Start(enclosingCapsuleStart); 
			m_EnclosingCapsule.Centre(enclosingCapsuleCentre);
			m_EnclosingCapsule.WarmStart(enclosingCapsuleCentre);
		}

		void SetCollisionFilter(Collision::C_CollisionFilter* pFilter)
		{
			m_Ray.m_Filter = *pFilter;
		}


		void ResetContacts()
		{
			/*const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());*/
			//Maths::HVec3 start(m_Capsule.m_End);
			//Maths::HVec3 start(m_Capsule.End());

			/*m_Ray.InitialiseRay(start, Maths::C_Normal(Maths::HVec3(zero, -one, zero), Maths::C_Normal::UNIT_LENGTH_VECTOR), m_RayLength + m_MinPenetration,
				&m_Ray.m_Filter, m_Ray.m_Radius);*/
			m_Ray.InitialiseRay(m_Capsule.End(), Maths::C_Normal(Maths::Constants<Maths::HVec3>::UnitAxisNegativeY(), Maths::C_Normal::UNIT_LENGTH_VECTOR), m_RayLength + m_MinPenetration,
				&m_Ray.m_Filter, m_Ray.m_Radius);
			m_Ray.SetMaxCosAngle(-m_CosMaxAngle);
		}

		C_Capsule& GetCapsule(const Collision::C_Shape* pShape)
		{
			if(pShape->CanCollide(m_CapsuleCollisionFlags))
				return m_Capsule;

			return m_EnclosingCapsule;

		}
	};
};



#endif

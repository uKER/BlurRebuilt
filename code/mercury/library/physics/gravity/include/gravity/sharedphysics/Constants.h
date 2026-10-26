#ifndef GRAVITY_CONSTANTS_H
#define GRAVITY_CONSTANTS_H

namespace SharedPhysics
{

	struct Constants
	{
		static const F32 m_Eightieth; 
		static const F32 m_Tenth;
		static const F32 m_Ten;
		static const F32 m_Twenty;
		static const F32 m_CloseContact;
		static const F32 m_CloseNormal;
		static const F32 m_Third;
		static const F32 m_BigNumber;
		static const F32 m_Thousand;
		static const F32 m_DefaultMaxSpeed;
		static const F32 m_DefaultMaxAngularSpeed;
		static const F32 m_Ninety;
		static const F32 m_CosFortyFive;
		static const F32 m_CosSixty;
		static const F32 m_CosThirty;
		static const F32 m_RestitutionVelocityThreshold;
		static const F32 m_TwoPiOverThree;
		static const F32 m_SinSixty;
		static const F32 m_Hundredth;
		static const F32 m_PointNine;
		static const F32 m_Epsilon;
		NOALIAS FORCE_INLINE static const Maths::HF32 Eightieth() { return Maths::Load<Maths::HF32>(&m_Eightieth);}
		NOALIAS FORCE_INLINE static const Maths::HF32 Tenth() { return Maths::Load<Maths::HF32>(&m_Tenth);}
		NOALIAS FORCE_INLINE static const Maths::HF32 Twenty() { return Maths::Load<Maths::HF32>(&m_Twenty);}
		NOALIAS FORCE_INLINE static const Maths::HF32 Ten() { return Maths::Load<Maths::HF32>(&m_Ten);}
		NOALIAS FORCE_INLINE static const Maths::HF32 CloseContact() { return Maths::Load<Maths::HF32>(&m_CloseContact);}
		NOALIAS FORCE_INLINE static const Maths::HF32 CloseNormal() { return Maths::Load<Maths::HF32>(&m_CloseNormal);}
		NOALIAS FORCE_INLINE static const Maths::HF32 Third() { return Maths::Load<Maths::HF32>(&m_Third);}
		NOALIAS FORCE_INLINE static const Maths::HF32 BigNumber() { return Maths::Load<Maths::HF32>(&m_BigNumber);}
		NOALIAS FORCE_INLINE static const Maths::HF32 Thousand() { return Maths::Load<Maths::HF32>(&m_Thousand);}
		NOALIAS FORCE_INLINE static const Maths::HF32 DefaultMaxSpeed() { return Maths::Load<Maths::HF32>(&m_DefaultMaxSpeed);}
		NOALIAS FORCE_INLINE static const Maths::HF32 DefaultMaxAngularSpeed() { return Maths::Load<Maths::HF32>(&m_DefaultMaxAngularSpeed);}
		NOALIAS FORCE_INLINE static const Maths::HF32 Ninety() { return Maths::Load<Maths::HF32>(&m_Ninety);}
		NOALIAS FORCE_INLINE static const Maths::HF32 CosFortyFive() { return Maths::Load<Maths::HF32>(&m_CosFortyFive);}
		NOALIAS FORCE_INLINE static const Maths::HF32 CosSixty() { return Maths::Load<Maths::HF32>(&m_CosSixty);}
		NOALIAS FORCE_INLINE static const Maths::HF32 CosThirty() { return Maths::Load<Maths::HF32>(&m_CosThirty);}
		NOALIAS FORCE_INLINE static const Maths::HF32 RestitutionVelocityThreshold() { return Maths::Load<Maths::HF32>(&m_RestitutionVelocityThreshold); }
		NOALIAS FORCE_INLINE static const Maths::HF32 TwoPiOverThree() { return Maths::Load<Maths::HF32>(&m_TwoPiOverThree); }
		NOALIAS FORCE_INLINE static const Maths::HF32 SinSixty() { return Maths::Load<Maths::HF32>(&m_SinSixty); }
		NOALIAS FORCE_INLINE static const Maths::HF32 Hundredth() { return Maths::Load<Maths::HF32>(&m_Hundredth); }
		NOALIAS FORCE_INLINE static const Maths::HF32 PointNine() { return Maths::Load<Maths::HF32>(&m_PointNine); }
		NOALIAS FORCE_INLINE static const Maths::HF32 Epsilon() { return Maths::Load<Maths::HF32>(&m_Epsilon); }



	};
};

#endif

#ifndef ANIMATION_POSEFIELDS_INL
#define ANIMATION_POSEFIELDS_INL

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(ANIMATION_ALLOW_INCLUDE) && defined(ANIMATION_SHOW_INCLUDES)
#pragma message("******* Including PoseFields.inl outside of animation library")
#endif


//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>

#include <animation\pose\PoseFields.h>

namespace Animation
{

	//
	// C_ScalarField
	// 
	
	inline C_ScalarField::C_ScalarField()
	{
	}
	
	inline C_ScalarField::C_ScalarField( const Maths::VF32& s ):
		m_valueAndWeight( Maths::MakeVec4F32( s, s, s, Maths::Const<Maths::VF32>::Zero() ) )
	{
	}
		
	inline C_ScalarField::operator F32 () const
	{
		return *reinterpret_cast<const F32*>(&m_valueAndWeight);
	}

	inline C_ScalarField::operator Maths::VF32 () const
	{
		return Maths::X(m_valueAndWeight);
	}
	
	inline void C_ScalarField::SetZero()
	{
		m_valueAndWeight = Maths::Const<Maths::Vec4F32>::Zero();
	}
	
	inline void C_ScalarField::operator = ( const F32 rhs )
	{
		Maths::Vec4F32 v = Maths::MakeVec4F32(rhs);
		Maths::SetW(&v, Maths::Const<Maths::VF32>::Zero() );
		m_valueAndWeight = v;
	}
	
	inline void C_ScalarField::operator = ( const Maths::Vec4F32& rhs )
	{
		m_valueAndWeight = rhs;
	}
	
	inline void C_ScalarField::operator = ( const Maths::VF32& rhs )
	{
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		m_valueAndWeight = Maths::MakeVec4F32( rhs, rhs, rhs, zero );
	}


	//
	// C_Vector3Field
	//
	
	inline C_Vector3Field::C_Vector3Field()
	{
	}

	inline C_Vector3Field::C_Vector3Field( const Maths::Vector3& v ):
		m_valueAndWeight(Maths::MakeVec4F32( v, Maths::Const< Maths::VF32 >::One()))
	{
	}

	inline C_Vector3Field::operator Maths::Vector3 () const
	{
		return Maths::MakeVector3(m_valueAndWeight);
	}
	
	inline void C_Vector3Field::SetZero()
	{
		m_valueAndWeight = Maths::MakeVec4F32(Maths::Const<Maths::Vector3>::Zero(), Maths::Const<Maths::VF32>::One());
	}
	
	inline void C_Vector3Field::SetOne()
	{
		m_valueAndWeight = Maths::Const<Maths::Vec4F32>::One();
	}

	inline Maths::VF32 C_Vector3Field::GetWeight() const
	{
		return Maths::W(m_valueAndWeight);
	}
	
	inline void C_Vector3Field::SetWeight( const Maths::VF32& weight )
	{
		Maths::SetW(&m_valueAndWeight, weight);
	}
	
	inline Maths::Vec4F32 C_Vector3Field::GetValueAndWeight() const
	{
		return m_valueAndWeight;
	}
	
	inline void C_Vector3Field::Lerp( const Maths::Vector3& dest, const Maths::VF32& amount )
	{
		const Maths::Vector3 result = Maths::Lerp( Maths::MakeVector3(m_valueAndWeight), dest, amount );
		m_valueAndWeight = Maths::MakeVec4F32( result, Maths::Const<Maths::VF32>::Zero() );
	}
	
	inline void C_Vector3Field::Normalise()
	{
		const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
		const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
		const Maths::Vec4F32 valueAndWeight = m_valueAndWeight;
		const Maths::VF32 weight = Maths::W( valueAndWeight );
		const Maths::VF32 invWeight = Maths::Select( weight > zero, one / weight, one );
		m_valueAndWeight = valueAndWeight * invWeight;
	}
	
	inline void C_Vector3Field::operator = ( const Maths::Vector3& rhs )
	{
		m_valueAndWeight = Maths::MakeVec4F32( rhs, Maths::Const<Maths::VF32>::One() );
	}

	inline void C_Vector3Field::operator = ( const Maths::Vec4F32& rhs )
	{
		m_valueAndWeight = rhs;
	}


	//
	// C_QuaternionField
	//
	
	inline C_QuaternionField::C_QuaternionField()
	{
	}

	inline C_QuaternionField::C_QuaternionField( const Maths::Quaternion& q ):
		m_value(q)
	{
	}

	inline C_QuaternionField::operator Maths::Quaternion () const
	{
		return m_value;
	}
	
	inline void C_QuaternionField::SetIdentity()
	{
		m_value = Maths::Const< Maths::Quaternion >::Identity();
	}
	
	inline void C_QuaternionField::SetZero()
	{
		m_value = Maths::Const< Maths::Quaternion >::Zero();
	}
	
	inline	void C_QuaternionField::NearestLerpNormalised( const Maths::Quaternion& dest, const Maths::VF32& amount )
	{
		m_value = Maths::NearestLerpNormalised( m_value, dest, amount );
	}
		
	inline void C_QuaternionField::Normalise()
	{
		const Maths::Quaternion q = m_value;
		const Maths::Quaternion qn = Maths::Normalise( q );
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::Quaternion zeroQuat = Maths::Const<Maths::Quaternion>::Identity();
		m_value = Maths::Select( Maths::LengthSquared( q ) > zero, qn, zeroQuat );
	}
	
	inline void C_QuaternionField::operator = ( const Maths::Quaternion& rhs )
	{
		m_value = rhs;
	}
	
	//
	// C_Vector4Field
	//
	
	inline C_Vector4Field::C_Vector4Field()
	{
	}
	
	inline C_Vector4Field::C_Vector4Field( const Maths::Vector4& v ):
		m_xyz( Maths::MakeVector3( v ) ),
		m_w( Maths::W( v ) )
	{
	}
			
	inline C_Vector4Field::operator Maths::Vector4 () const
	{
		return Maths::MakeVector4( static_cast<Maths::Vector3>(m_xyz), static_cast<Maths::VF32>(m_w) );
	}
		
	inline void C_Vector4Field::SetZero()
	{
		m_xyz.SetZero();
		m_w.SetZero();
	}
		
	//inline void C_Vector4Field::Normalise()
	//{
	//	m_xyz.Normalise();
	//	m_w.Normalise();
	//}
		
	inline void C_Vector4Field::operator = ( const Maths::Vector4& rhs )
	{
		m_xyz = Maths::MakeVector3( rhs );
		m_w = Maths::W( rhs );
	}
}

#endif

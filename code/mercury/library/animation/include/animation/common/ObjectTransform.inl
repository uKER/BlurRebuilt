#ifndef ANIMATION_OBJECTTRANSFORM_INL
#define ANIMATION_OBJECTTRANSFORM_INL

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(ANIMATION_ALLOW_INCLUDE) && defined(ANIMATION_SHOW_INCLUDES)
#pragma message("******* Including ObjectTransform.inl outside of animation library")
#endif

#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <animation/pose/PoseFields.inl>

namespace Animation
{

	inline C_ObjectTransform::C_ObjectTransform()
	{
	}

	inline C_ObjectTransform::C_ObjectTransform(
		const Maths::Quaternion& rotation,
		const Maths::Vector3& translation,
		const Maths::Vector3& scale ) :
		m_rotation(rotation),
		m_translation(translation),
		m_scale(scale)
	{
	}
	
	inline Maths::Mat43 C_ObjectTransform::AsMat43() const
	{
		// scale * (shear) * rotation * translation
		const Maths::ScaleMat3 scale = Maths::MakeScaleMat3( GetScale() );
		const Maths::Mat33 rotation = Maths::MakeMat33( GetRotation() );
		const Maths::Vector3 translation = GetTranslation();

		return Maths::MakeMat43( scale * rotation, translation );
	}	
	
	inline const Maths::Vector3 C_ObjectTransform::GetScale() const
	{
		return m_scale;
	}

	inline void C_ObjectTransform::SetScale( const Maths::Vector3& scale )
	{
		m_scale = scale;
	}

	inline const Maths::Quaternion C_ObjectTransform::GetRotation() const
	{
		return m_rotation;
	}
	
	inline void C_ObjectTransform::SetRotation( const Maths::Quaternion& rotation )
	{
		m_rotation = rotation;
	}

	inline const Maths::Vector3 C_ObjectTransform::GetTranslation() const
	{
		return m_translation;
	}
	
	inline void C_ObjectTransform::SetTranslation( const Maths::Vector3& translation )
	{
		m_translation = translation;
	}
	
	inline const Maths::Point3 C_ObjectTransform::GetPosition() const
	{
		return Maths::MakePoint3(m_translation);
	}

	inline void C_ObjectTransform::SetIdentity()
	{
		m_rotation.SetIdentity();
		m_translation.SetZero();
		m_scale.SetOne();
	}
	
	inline void C_ObjectTransform::SetZero()
	{
		m_rotation.SetZero();
		m_translation.SetZero();
		m_scale.SetZero();
	}

	inline const C_ObjectTransform C_ObjectTransform::Inverse() const
	{
		const Maths::Quaternion invR = !GetRotation();
		const Maths::Vector3 invT = -GetTranslation() * invR;
		const Maths::Vector3 invS = Maths::Rcp( GetScale() );
		return C_ObjectTransform( invR, invT, invS );
	}
	
	inline const C_ObjectTransform C_ObjectTransform::operator * ( const C_ObjectTransform& rhs ) const
	{
		const Maths::Quaternion q1 = GetRotation();		
		const Maths::Quaternion q2 = rhs.GetRotation();
		const Maths::Vector3 t1 = GetTranslation();
		const Maths::Vector3 t2 = rhs.GetTranslation();
		const Maths::Vector3 s1 = GetScale();
		const Maths::Vector3 s2 = rhs.GetScale();
		return C_ObjectTransform( q1 * q2, t1 * q2 + t2, s1 * s2 );
	}
	
	inline void C_ObjectTransform::operator *= ( const C_ObjectTransform& rhs )
	{
		const Maths::Quaternion q1 = GetRotation();
		const Maths::Quaternion q2 = rhs.GetRotation();
		const Maths::Vector3 t1 = GetTranslation();
		const Maths::Vector3 t2 = rhs.GetTranslation();
		const Maths::Vector3 s1 = GetScale();
		const Maths::Vector3 s2 = rhs.GetScale();
		SetRotation( q1 * q2 );
		SetTranslation( t1 * q2 + t2 );
		SetScale( s1 * s2 );
	}
	
	//
	// Blending.
	//
	
	inline void C_ObjectTransform::Normalise()
	{
		m_rotation.Normalise();
		m_translation.Normalise();
		m_scale.Normalise();
	}
	
	//
	// Transformations of points, vectors and normals.
	//
	
	inline const Maths::Point3 C_ObjectTransform::TransformPoint( const Maths::Point3& p ) const
	{
		const Maths::Quaternion q = GetRotation();
		const Maths::Vector3 t = GetTranslation();
		const Maths::Vector3 s = GetScale();
		return Maths::MakePoint3( t + ( s * Maths::MakeVector3( p ) ) * q );
	}
	
	inline const Maths::Vector3 C_ObjectTransform::TransformVector( const Maths::Vector3& v ) const
	{
		const Maths::Quaternion q = GetRotation();
		const Maths::Vector3 s = GetScale();
		return ( s * v ) * q;
	}
	
	inline const Maths::Normal3 C_ObjectTransform::TransformNormal( const Maths::Normal3& n ) const
	{
		const Maths::Quaternion q = GetRotation();
		return n * q;
	}

	inline const Maths::Point3 C_ObjectTransform::InverseTransformPoint( const Maths::Point3& p ) const
	{
		const Maths::Quaternion invQ = !GetRotation();
		const Maths::Vector3 invT = -GetTranslation() * invQ;
		const Maths::Vector3 invS = Maths::Rcp( GetScale() );
		return Maths::MakePoint3( invT + ( invS * Maths::MakeVector3( p ) ) * invQ );
	}
	
	inline const Maths::Vector3 C_ObjectTransform::InverseTransformVector( const Maths::Vector3& v ) const
	{
		const Maths::Quaternion invQ = !GetRotation();
		const Maths::Vector3 invS = Maths::Rcp( GetScale() );
		return ( invS * v ) * invQ;
	}
	
	inline const Maths::Normal3 C_ObjectTransform::InverseTransformNormal( const Maths::Normal3& n ) const
	{
		return n * !GetRotation();
	}

} // end Animation

#endif

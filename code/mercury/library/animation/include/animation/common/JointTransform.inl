#ifndef ANIMATION_JOINTTRANSFORM_INL
#define ANIMATION_JOINTTRANSFORM_INL

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(ANIMATION_ALLOW_INCLUDE) && defined(ANIMATION_SHOW_INCLUDES)
#pragma message("******* Including JointTransform.inl outside of animation library")
#endif
#include <animation/common/JointTransform.h>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>
#include <animation/pose/PoseFields.inl>

namespace Animation
{

	
	inline C_JointTransform::C_JointTransform(E_Identity i):
		m_rotation( Maths::Const<Maths::Quaternion>::Identity() ),
		m_translation( Maths::Const<Maths::Vector3>::Zero() ),
		m_scale( Maths::Const<Maths::Vector3>::One() )
	{
	}

	
	FORCE_INLINE C_JointTransform::C_JointTransform( const Maths::Quaternion& rotation, const Maths::Vector3& translation ):
		m_rotation( rotation ),
		m_translation( translation ),
		m_scale( Maths::Const<Maths::Vector3>::One() )
	{
	}
	
	FORCE_INLINE C_JointTransform::C_JointTransform( const Maths::Quaternion& rotation, const Maths::Vector3& translation, const Maths::Vector3& scale ):
		m_rotation( rotation ),
		m_translation( translation ),
		m_scale( scale )
	{
	}	
	
	inline Maths::Mat43 C_JointTransform::AsMat43() const
	{
		// scale * (shear) * rotation * translation
		// could be optimised...
		const Maths::ScaleMat3 scale = Maths::MakeScaleMat3( GetScale() );
		const Maths::Mat33 rotation = Maths::MakeMat33( GetRotation() );
		const Maths::Vector3 translation = GetTranslation();
		return Maths::MakeMat43( scale * rotation, translation );
	}
	
	
	FORCE_INLINE const Maths::Vector3 C_JointTransform::GetScale() const
	{
		return m_scale;
	}

	FORCE_INLINE void C_JointTransform::SetScale( const Maths::Vector3& scale )
	{
		m_scale = C_Vector3Field(scale);
	}

	FORCE_INLINE const Maths::Quaternion C_JointTransform::GetRotation() const
	{
		return m_rotation;
	}
	
	FORCE_INLINE void C_JointTransform::SetRotation( const Maths::Quaternion& rotation )
	{
		m_rotation = rotation;
	}

	FORCE_INLINE const Maths::Vector3 C_JointTransform::GetTranslation() const
	{
		return m_translation;
	}
	
	FORCE_INLINE void C_JointTransform::SetTranslation( const Maths::Vector3& translation )
	{
		m_translation = C_Vector3Field(translation);
	}
	
	FORCE_INLINE const Maths::Point3 C_JointTransform::GetPosition() const
	{
		return Maths::MakePoint3(m_translation);
	}
	
	inline Maths::VF32 C_JointTransform::GetTranslationWeight() const
	{
		return m_translation.GetWeight();
	}
	
	inline void C_JointTransform::SetTranslationWeight( const Maths::VF32& weight )
	{
		m_translation.SetWeight( weight );
	}

	inline void C_JointTransform::SetIdentity()
	{
		m_rotation.SetIdentity();
		m_translation.SetZero();
		m_scale.SetOne();
	}
	
	inline void C_JointTransform::SetZero()
	{
		m_rotation.SetZero();
		m_translation.SetZero();
		m_scale.SetZero();
	}

	FORCE_INLINE const C_JointTransform C_JointTransform::Inverse() const
	{
		const Maths::Quaternion invR = !GetRotation();
		const Maths::Vector3 invT = -GetTranslation() * invR;
		const Maths::Vector3 invS = Maths::Rcp( GetScale() );
		return C_JointTransform( invR, invT, invS );
	}
	
	FORCE_INLINE const C_JointTransform C_JointTransform::operator * ( const C_JointTransform& rhs ) const
	{
		const Maths::Quaternion q1 = GetRotation();		
		const Maths::Quaternion q2 = rhs.GetRotation();
		const Maths::Vector3 t1 = GetTranslation();
		const Maths::Vector3 t2 = rhs.GetTranslation();
		const Maths::Vector3 s1 = GetScale();
		const Maths::Vector3 s2 = rhs.GetScale();
		return C_JointTransform( q1 * q2, t1 * q2 + t2, s1 * s2 );
	}
	
	FORCE_INLINE void C_JointTransform::operator *= ( const C_JointTransform& rhs )
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
	
	inline void C_JointTransform::Normalise()
	{
		m_rotation.Normalise();
		m_translation.Normalise();
		m_scale.Normalise();
	}
	
	//
	// Transformations of points, vectors and normals.
	//
	
	inline const Maths::Point3 C_JointTransform::TransformPoint( const Maths::Point3& p ) const
	{
		const Maths::Quaternion q = GetRotation();
		const Maths::Vector3 t = GetTranslation();
		const Maths::Vector3 s = GetScale();
		return Maths::MakePoint3( t + ( s * Maths::MakeVector3( p ) ) * q );
	}

	inline const Maths::Vector3 C_JointTransform::TransformPoint( const Maths::Vector3& p ) const
	{
		const Maths::Quaternion q = GetRotation();
		const Maths::Vector3 t = GetTranslation();
		const Maths::Vector3 s = GetScale();
		return ( t + ( ( s * p ) * q ) );
	}
	
	inline const Maths::Vector3 C_JointTransform::TransformVector( const Maths::Vector3& v ) const
	{
		const Maths::Quaternion q = GetRotation();
		const Maths::Vector3 s = GetScale();
		return ( s * v ) * q;
	}
	
	inline const Maths::Normal3 C_JointTransform::TransformNormal( const Maths::Normal3& n ) const
	{
		const Maths::Quaternion q = GetRotation();
		return n * q;
	}

	inline const Maths::Point3 C_JointTransform::InverseTransformPoint( const Maths::Point3& p ) const
	{
		const Maths::Quaternion invQ = !GetRotation();
		const Maths::Vector3 invT = -GetTranslation() * invQ;
		const Maths::Vector3 invS = Maths::Rcp( GetScale() );
		return Maths::MakePoint3( invT + ( invS * Maths::MakeVector3( p ) ) * invQ );
	}
	
	inline const Maths::Vector3 C_JointTransform::InverseTransformVector( const Maths::Vector3& v ) const
	{
		const Maths::Quaternion invQ = !GetRotation();
		const Maths::Vector3 invS = Maths::Rcp( GetScale() );
		return ( invS * v ) * invQ;
	}
	
	inline const Maths::Normal3 C_JointTransform::InverseTransformNormal( const Maths::Normal3& n ) const
	{
		return n * !GetRotation();
	}
	
	template <typename T>
	inline void C_JointTransform::Serialise( T& stream ) const
	{
		Maths::StorageVec4F32 q;
		Maths::Store( Maths::GetVec4F32( Maths::Quaternion(m_rotation) ), &q );
		stream << q.m_Data.x << q.m_Data.y << q.m_Data.z << q.m_Data.w;

		Maths::Vector3 t = m_translation;
		Maths::Serialise( t, stream );
		
		Maths::Vector3 s = m_scale;
		Maths::Serialise( s, stream );
	}
	
	template <typename T>
	inline void C_JointTransform::Deserialise( T& stream )
	{
		Maths::Vec4F32 q;
		Maths::Deserialise( stream, &q );
		m_rotation = C_QuaternionField( Maths::MakeQuaternion( q ) );
		
		Maths::Vector3 t;
		Maths::Deserialise( stream, &t );
		m_translation = C_Vector3Field( t );
		
		Maths::Vector3 s;
		Maths::Deserialise( stream, &s );
		m_scale = C_Vector3Field( s );
	}
	
	template <typename T>
	inline void C_JointTransform::DeserialiseWithoutScale( T& stream )
	{
		Maths::Vec4F32 q;
		Maths::Deserialise( stream, &q );
		m_rotation = C_QuaternionField( Maths::MakeQuaternion( q ) );
		
		Maths::Vector3 t;
		Maths::Deserialise( stream, &t );
		m_translation = C_Vector3Field( t );
		
		m_scale.SetOne();
	}
}


#endif



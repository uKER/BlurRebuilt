#ifndef ANIMATION_JOINTTRANSFORM_H
#define ANIMATION_JOINTTRANSFORM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/pose/PoseFields.h>

//
// Include JointTransform.inl to use inline functions.
//

namespace Animation
{
	class C_ObjectTransform;
	
	/**
		A transform comprising rotation, translation and scale, usually associated with
		a joint (bone) but can apply to any kind of object.
		
		Shear is not supported. This transform has some peculiarities with its
		multiplication operator as scale is multiplied separately to rotation
		and translation. This is because of the potential for a scale transform
		multiplied by a rotation to introduce shear, which cannot be represented.
		The upshot is that a local space C_JointTransform won't necessarily match
		its corresponding local space matrix. For this reason, it's best to avoid
		calculations with local space C_JointTransforms and work in model space instead.
		
		Each C_JointTransform is also a compound pose field meaning that it can
		be stored in a C_Pose and have its scale, rotation and translation components
		individually blended.
		
		@ingroup compoundfields
		
		@see C_ObjectTransform
	*/

	class C_JointTransform
	{
	public:
	
		enum E_Identity
		{
			IDENTITY
		};
	
		/// Construct an uninitialised transform.
		C_JointTransform() {}

		/// Construct an transform initialised to identity.
		inline C_JointTransform(E_Identity i);
		
		/// Construct transform from rotation quaternion and translation vector.
		inline C_JointTransform( const Maths::Quaternion& rotation, const Maths::Vector3& translation );
	
		/// Construct transform from rotation quaternion, translation vector and axis-aligned scale vector.
		inline C_JointTransform( const Maths::Quaternion& rotation, const Maths::Vector3& translation, const Maths::Vector3& scale );
		
		/// Constructs a joint transform from a C_ObjectTransform.
		C_JointTransform( const C_ObjectTransform& objectTransform );
		
		/// Construct transform from a Mat43.
		C_JointTransform( const Maths::Mat43& m );
		
		/// Convert to a Mat43.
		inline Maths::Mat43 AsMat43() const;

		//
		// Scale, rotation and translation.
		//

		/// Get scale component.
		inline const Maths::Vector3 GetScale() const;
		
		/// Set scale component.
		inline void SetScale( const Maths::Vector3& scale );
		
		/// Get rotation component.
		inline const Maths::Quaternion GetRotation() const;
		
		/// Set rotation component.
		inline void SetRotation( const Maths::Quaternion& rotation );

		/// Get translation component.
		inline const Maths::Vector3 GetTranslation() const;
		
		/// Set translation component.
		inline void SetTranslation( const Maths::Vector3& translation );

		/// Gets translation interpreted as a position.
		inline const Maths::Point3 GetPosition() const;
	
		//
		// Field-based interface.
		//

		/// Gets rotation component as a C_QuaternionField.
		const C_QuaternionField& GetRotationField() const		{ return m_rotation; }

		/// Gets rotation component as a C_QuaternionField.
		C_QuaternionField& GetRotationField()					{ return m_rotation; }

		/// Gets translation component as a C_Vector3Field.
		const C_Vector3Field& GetTranslationField() const		{ return m_translation; }

		/// Gets translation component as a C_Vector3Field.
		C_Vector3Field& GetTranslationField()					{ return m_translation; }

		/// Gets scale component as a C_Vector3Field.
		const C_Vector3Field& GetScaleField() const			{ return m_scale; }

		/// Gets scale component as a C_Vector3Field.
		C_Vector3Field& GetScaleField()							{ return m_scale; }
		
		inline Maths::VF32 GetTranslationWeight() const;
		
		inline void SetTranslationWeight( const Maths::VF32& weight );
		

		//
		// Transform operations.
		//
	
		/// Set transform to identity.
		inline void SetIdentity();

		/// Returns true if transform is identity.
		bool IsIdentity() const;
		
		/// Set transform to zero before first accumulation.
		inline void SetZero();

		/// Get inverse.
		inline const C_JointTransform Inverse() const;
		
		/// Multiply.
		inline const C_JointTransform operator * ( const C_JointTransform& rhs ) const;

		/// Multiply in-place.
		inline void operator *= ( const C_JointTransform& rhs );
		
		//
		// Blending.
		//

		/// Normalise rotation and translation after weighted accumulation.
		inline void Normalise();
		
		void Normalise(const C_JointTransform& basePoseTransform);
		
		static void Normalise(
			C_JointTransform* __restrict transforms,
			U32 count,
			const C_JointTransform* __restrict basePoseTransforms
		);	

		static void Normalise(
			C_JointTransform* __restrict transforms,
			U32 count,
			bool unitiseZeroWeights = true
		);
	
		
		//
		// Transformation of points, vectors and normals.
		//
		
		/// Transform a point.
		inline const Maths::Point3 TransformPoint( const Maths::Point3& p ) const;
		inline const Maths::Vector3 TransformPoint( const Maths::Vector3& p ) const;
		
		/// Transform a vector.
		inline const Maths::Vector3 TransformVector( const Maths::Vector3& v ) const;
		
		/// Transform a normal.
		inline const Maths::Normal3 TransformNormal( const Maths::Normal3& n ) const;

		/// Inverse transform a point.
		inline const Maths::Point3 InverseTransformPoint( const Maths::Point3& p ) const;
		
		/// Inverse transform a vector.
		inline const Maths::Vector3 InverseTransformVector( const Maths::Vector3& v ) const;
		
		/// Inverse transform a normal.
		inline const Maths::Normal3 InverseTransformNormal( const Maths::Normal3& n ) const;
		
		//
		// Serialisation.
		//
		template <typename T>
		inline void Serialise( T& stream ) const;
		
		template <typename T>
		inline void Deserialise( T& stream );

		template <typename T>
		inline void DeserialiseWithoutScale( T& stream );
		
	private:
		C_QuaternionField	m_rotation;
		C_Vector3Field		m_translation;
		C_Vector3Field		m_scale;	
		
		static Maths::Vector4 s_normalisationConstants;
	};
	
	
}

#endif

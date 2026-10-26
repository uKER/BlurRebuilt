#ifndef ANIMATION_OBJECTTRANSFORM_H
#define ANIMATION_OBJECTTRANSFORM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/pose/PoseFields.h>

//
// Include ObjectTransform.inl to use inline functions.
//

namespace Animation
{

	class C_JointTransform;

	/**
		A transform representing rotation, translation and scale of an object.
		
		Shear is not supported. This transform has some peculiarities with its
		multiplication operator as scale is multiplied separately to rotation
		and translation. This is because of the potential for a scale transform
		multiplied by a rotation to introduce shear, which cannot be represented.
		The upshot is that a local space C_ObjectTransform won't necessarily match
		its corresponding local space matrix. For this reason, it's best to avoid
		calculations with local space C_ObjectTransforms and work in model space instead.
		
		Each C_ObjectTransform is also a compound pose field meaning that it can
		be stored in a C_Pose and have its scale, rotation and translation components
		individually blended.
		
		@ingroup compoundfields
		
		@deprecated Use C_JointTransform instead.
			
		@see C_JointTransform
	*/
	class C_ObjectTransform
	{
	public:
	
		/// Construct an uninitialised transform.
		C_ObjectTransform();
		
		/// Construct transform from rotation quaternion, translation vector and axis-aligned scale vector.
		C_ObjectTransform( const Maths::Quaternion& rotation, const Maths::Vector3& translation, const Maths::Vector3& scale );
		
		/// Construct transform from a C_JointTransform.
		C_ObjectTransform( const C_JointTransform& jointTransform );
		
		/// Construct transform from a Mat43.
		C_ObjectTransform( const Maths::Mat43& m );
		
		/// Convert to a Mat43.
		Maths::Mat43 AsMat43() const;

		//
		// Scale, rotation and translation.
		//

		/// Get scale component.
		const Maths::Vector3 GetScale() const;
		
		/// Set scale component.
		void SetScale( const Maths::Vector3& scale );
		
		/// Get rotation component.
		const Maths::Quaternion GetRotation() const;
		
		/// Set rotation component.
		void SetRotation( const Maths::Quaternion& rotation );

		/// Get translation component.
		const Maths::Vector3 GetTranslation() const;
		
		/// Set translation component.
		void SetTranslation( const Maths::Vector3& translation );

		/// Gets translation interpreted as a position.
		const Maths::Point3 GetPosition() const;
	
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

		//
		// Transform operations.
		//
	
		/// Set transform to identity.
		void SetIdentity();
		
		/// Set transform to zero before first accumulation.
		void SetZero();

		/// Get inverse.
		inline const C_ObjectTransform Inverse() const;
		
		/// Multiply.
		const C_ObjectTransform operator * ( const C_ObjectTransform& rhs ) const;

		/// Multiply in-place.
		void operator *= ( const C_ObjectTransform& rhs );
		
		//
		// Blending.
		//

		/// Normalise rotation and translation after weighted accumulation.
		void Normalise();
		
		static void Normalise(
			C_ObjectTransform* __restrict transforms,
			U32 count,
			const C_ObjectTransform* __restrict basePoseTransforms
		);	

		static void Normalise(
			C_ObjectTransform* __restrict transforms,
			U32 count,
			const C_JointTransform* __restrict basePoseTransforms
		);	

		
		static void Normalise(
			C_ObjectTransform* __restrict transforms,
			U32 count
		);
		
		//
		// Transformation of points, vectors and normals.
		//
		
		/// Transform a point.
		const Maths::Point3 TransformPoint( const Maths::Point3& p ) const;
		
		/// Transform a vector.
		const Maths::Vector3 TransformVector( const Maths::Vector3& v ) const;
		
		/// Transform a normal.
		const Maths::Normal3 TransformNormal( const Maths::Normal3& n ) const;

		/// Inverse transform a point.
		const Maths::Point3 InverseTransformPoint( const Maths::Point3& p ) const;
		
		/// Inverse transform a vector.
		const Maths::Vector3 InverseTransformVector( const Maths::Vector3& v ) const;
		
		/// Inverse transform a normal.
		const Maths::Normal3 InverseTransformNormal( const Maths::Normal3& n ) const;
		
		//
		// Serialisation.
		//
		template <typename T>
		void Serialise( T& stream ) const;
		
		template <typename T>
		void Deserialise( T& stream );
		
	private:
		C_QuaternionField	m_rotation;
		C_Vector3Field		m_translation;
		C_Vector3Field		m_scale;	
		
		static Maths::Vector4 s_normalisationConstants;
	};
}

#endif

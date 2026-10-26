#ifndef ANIMATION_ATOMICPOSEFIELDS_H
#define ANIMATION_ATOMICPOSEFIELDS_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <core/maths/hvec/new/IncludeGeneral.h>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>

//
// Include PoseFields.inl for inline functions.
//

namespace Animation
{

	/**
		@defgroup atomicfields Atomic Field Types
		
		Atomic fields represent the most basic animated data types that can be evaluated from a cycle and blended
		together in a Animation::C_Pose. Fields can be blended and therefore often include
		a blend weight parameter for normalisation after a series of accumulated blends.
		
		Currently, the animation system supports the following three basic data types:
		
			- Scalars (#Animation::C_ScalarField)
			- 3D Vectors (#Animation::C_Vector3Field)
			- Quaternions (#Animation::C_QuaternionField)
			
		2D vectors may be included in the future.
		
		These basic field types can be combined to create @ref compoundfields "compound fields".
	*/
	
	/**
		@defgroup compoundfields Compound Field Types
		
		Compound pose fields are structures that consist of multiple @ref atomicfields "atomic fields".
		Each atomic field inside a compound field is called a "field element". Each field element can be
		blended individually via field member accessors in the compound field classes, or the 
		low-level Animation::C_Pose::GetFieldElement() functions.
		
		There is only one compound field type at present, the joint transform, which contains rotation, translation
		and scale fields (Animation::C_JointTransform).
			
		Further compound field types may be added in the future for things such as cameras and material properties.
	*/

	/**
		One-dimensional scalar field.
		
		Includes a weight parameter to track accumulated blending weights.
		
		@ingroup atomicfields
	*/
	class C_ScalarField
	{
	public:
		/// Default uninitialised constructor.
		C_ScalarField();
		
		/// Construct from Maths::VF32.
		explicit C_ScalarField( const Maths::VF32& s );
		
		/// Convert to F32.
		operator F32 () const;
		
		/// Convert to Maths::VF32.
		operator Maths::VF32 () const;
		
		/// Set to zero.
		void SetZero();
		
		/// F32 assignment operator. Weight will be set to zero.
		void operator = ( const F32 rhs );
		
		/// Vec4F32 assignment operator, where XYZ = scalar value and W = weight.
		void operator = ( const Maths::Vec4F32& rhs );

		/// Maths::VF32 assignment operator.
		void operator = ( const Maths::VF32& rhs );

		
	private:
		Maths::Vec4F32 m_valueAndWeight;
	};
	
	
	/**
		Three-dimensional vector field.
		
		Includes a weight parameter to track accumulated blending weights.
		
		@ingroup atomicfields
	*/
	class C_Vector3Field
	{
	public:
		/// Default uninitialised constructor.
		C_Vector3Field();
		
		/// Maths::Vector3 constructor.
		explicit C_Vector3Field( const Maths::Vector3& v );
	
		/// Converts to Maths::Vector3.
		operator Maths::Vector3 () const;
		
		/// Sets value and weight to zero.
		void SetZero();
		
		/// Sets value to one and weight to zero.
		void SetOne();
		
		/// Get weight.
		Maths::VF32 GetWeight() const;
		
		/// Set weight.
		void SetWeight( const Maths::VF32& weight );
		
		/// Get value in XYZ and weight in W.
		Maths::Vec4F32 GetValueAndWeight() const;
		
		/// Linearly interpolate towards @a dest. Weight is set to zero.
		void Lerp( const Maths::Vector3& dest, const Maths::VF32& amount );
				
		/// Normalise after accumulative blending.
		void Normalise();
		
		/// Maths::Vector3 assignment operator. Weight is set to zero.
		void operator = ( const Maths::Vector3& rhs );

		/// Maths::Vec4F32 assignment operator. Weight is set to the contents of the W component.
		void operator = ( const Maths::Vec4F32& rhs );

	
	private:
		Maths::Vec4F32 m_valueAndWeight;
	};
	
	/**
		Quaternion normalised 4D vector field.
		
		Quaternion fields do not have a separate weight parameter as
		quaternions can be linearly combined and renormalised without
		any need for a separate weight.
		
		@ingroup atomicfields
	*/
	class C_QuaternionField
	{
	public:
		/// Default uninitialised constructor.
		C_QuaternionField();
		
		/// Maths::Quaternion constructor.
		explicit C_QuaternionField( const Maths::Quaternion& q );
	
		/// Convert to Maths::Quaternion.
		operator Maths::Quaternion () const;
		
		/// Set to identity.
		void SetIdentity();
		
		/// Set to zero.
		void SetZero();
		
		/// Linearly interpolate and renormalise along the shortest rotation path towards @a dest.
		void NearestLerpNormalised( const Maths::Quaternion& dest, const Maths::VF32& amount );
		
		/// Normalise after accumulative blending.
		void Normalise();
		
		/// Maths::Quaternion assignment operator. Weight is set to zero.
		void operator = ( const Maths::Quaternion& rhs );
		
	private:
		Maths::Quaternion m_value;
	};
	
	/**
		4D vector field.
		
		Includes a separate weight parameter.
		
		@ingroup atomicfields
	*/
	class C_Vector4Field
	{
	public:
		/// Default uninitialised constructor.
		C_Vector4Field();
		
		/// Maths::Vector4 constructor.
		explicit C_Vector4Field( const Maths::Vector4& v );
		
		/// Convert to Maths::Vector4.
		operator Maths::Vector4 () const;
		
		/// Set value and weight to zero.
		void SetZero();
		
		/// Normalise after accumulative blending.
		//void Normalise();
		
		/// Maths::Vector4 assignment operator. Weight is set to zero.
		void operator = ( const Maths::Vector4& rhs );
		
	private:
		C_Vector3Field	m_xyz;
		C_ScalarField	m_w;
	};
}

#endif


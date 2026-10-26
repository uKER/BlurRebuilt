#ifndef ANIMATION_TOOLS_SCENETOGAMETRANSFORM_H
#define ANIMATION_TOOLS_SCENETOGAMETRANSFORM_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>

namespace AnimationTools
{

	/// \addtogroup Animation_Tools
	//! @{

	///
	/// Helper class that transforms matrices and vectors from "scene" space (Maya/XSI coordinate system)
	/// to game space.
	///
	class C_SceneToGameTransform
	{
	public:
		C_SceneToGameTransform():
			m_hasReflection( false ),
			m_flipX(false),
			m_flipY(false),
			m_flipZ(false)
		{
		}
		
		bool FlipX() const		{ return m_flipX; }
		bool FlipY() const		{ return m_flipY; }
		bool FlipZ() const		{ return m_flipZ; }
		
		bool FlipXRotation() const		{ return m_flipX ^ m_hasReflection; }
		bool FlipYRotation() const		{ return m_flipY ^ m_hasReflection; }
		bool FlipZRotation() const		{ return m_flipZ ^ m_hasReflection; }
		
		Maths::HVec3 GetFlipVector() const
		{
			return Maths::HVec3(m_flipX ? -1.f : 1.f, m_flipY ? -1.f : 1.f, m_flipZ ? -1.f : 1.f);
		}
		
		Maths::HVec3 GetRotationFlipVector() const
		{
			return Maths::HVec3(FlipXRotation() ? -1.f : 1.f, FlipYRotation() ? -1.f : 1.f, FlipZRotation() ? -1.f : 1.f);
		}		
		
		void SetSceneToGameMatrix( F32 uniformScale, bool flipX, bool flipY, bool flipZ  )
		{
			m_flipX = flipX;
			m_flipY = flipY;
			m_flipZ = flipZ;
			m_scale = uniformScale;
			Maths::HMat44 sceneToGame;
			
			Maths::HMat44 m;
		
			F32 x = flipX ? -1.f : 1.f;
			F32 y = flipY ? -1.f : 1.f;
			F32 z = flipZ ? -1.f : 1.f;
				
			sceneToGame.AX( Maths::HVec4( x * uniformScale, 0.f, 0.f, 0.f ) );
			sceneToGame.AY( Maths::HVec4( 0.f, y * uniformScale, 0.f, 0.f ) );
			sceneToGame.AZ( Maths::HVec4( 0.f, 0.f, z * uniformScale, 0.f ) );

			m_sceneToGame = Maths::HMat33(sceneToGame);
			m_gameToScene = Maths::HMat33(sceneToGame).Inverse();
			m_hasReflection = 0 != ( m_sceneToGame.Determinant() < Maths::Constants< Maths::HF32 >::Zero() );

			m_sceneToGameNormal = m_sceneToGame.Inverse().Transposed();
		}
		
		F32 GetUniformScale() const
		{
			return m_scale;
		}
		
		// FIXME: differentiate between points and vectors. If scene to game matrix has no translation then this is not too
		// much of a problem.
		
		// Vector to game space.
		Maths::HVec3 ToGameSpace( const Maths::HVec3& v ) const
		{
			return v * m_sceneToGame;
		}

		// Vector to game space.
		MathsStorage::Float3 ToGameSpace( const MathsStorage::Float3& v ) const
		{
			Maths::HVec3 result = Maths::Load< Maths::HVec3 >( &v ) * m_sceneToGame;
			MathsStorage::Float3 tmp;
			Maths::Store( result, &tmp );
			return tmp;
		}
		
		// Point to game space.
		MathsStorage::Float3 PointToGameSpace( const MathsStorage::Float3& v ) const
		{
			// assume scene to game matrix has no translation
			return ToGameSpace( v );
			//Maths::HVec4 result = Maths::HVec4(Maths::Load<Maths::HVec3>(&v)) * m_sceneToGame;
			//return MathsStorage::Float3( result.X(), result.Y(), result.Z() );
		}

		// Point to scene space.
		MathsStorage::Float3 PointToSceneSpace( const MathsStorage::Float3& v ) const
		{
			Maths::HVec3 result = Maths::Load< Maths::HVec3 >( &v ) * m_gameToScene;
			MathsStorage::Float3 tmp;
			Maths::Store( result, &tmp );
			return tmp;
		}

		Maths::Point3 PointToGameSpace( const Maths::Point3& p ) const
		{
			return p * static_cast<Maths::Mat33>(m_sceneToGame);
		}

		// Normal to game space.
		MathsStorage::Float3 NormalToGameSpace( const MathsStorage::Float3& v ) const
		{
			Maths::HVec3 result = Maths::Load< Maths::HVec3 >( &v ) * Maths::HMat33(m_sceneToGameNormal);
			result.SafeNormalise();
			MathsStorage::Float3 tmp;
			Maths::Store( result, &tmp );
			return tmp;
		}
		
		Maths::HVec3 NormalToGameSpace( const Maths::HVec3& v ) const
		{
			Maths::HVec3 result = v * Maths::HMat33(m_sceneToGameNormal);
			result.SafeNormalise();
			return result;
		}
		
		Maths::Normal3 NormalToGameSpace( const Maths::Normal3& v ) const
		{
			Maths::Vector3 result = v * static_cast<Maths::Mat33>(m_sceneToGameNormal);
			return Maths::Normalise(result);
		}		
		
		Maths::HMat43 ToGameSpace( const Maths::HMat43& m ) const
		{
			return Maths::HMat43( Maths::HMat43( m_gameToScene, Maths::HVec3() ) * m * Maths::HMat43( m_sceneToGame, Maths::HVec3() ) );
		}
		
		Maths::Mat43 ToGameSpace( const Maths::Mat43& m ) const
		{
			Maths::Mat43 gameToScene = Maths::MakeMat43( (Maths::Mat33) m_gameToScene, Maths::Const<Maths::Vector3>::Zero() );
			Maths::Mat43 sceneToGame = Maths::MakeMat43( (Maths::Mat33) m_sceneToGame, Maths::Const<Maths::Vector3>::Zero() );
		
			return gameToScene * m * sceneToGame;
		}		
		
		Maths::HMat33 ToGameSpace( const Maths::HMat33& m ) const
		{
			return Maths::HMat33( m_gameToScene * m * m_sceneToGame );
		}
		
		bool HasReflection() const
		{
			return m_hasReflection;
		}

	private:
		Maths::HMat33 m_sceneToGame;
		Maths::HMat33 m_gameToScene;
		Maths::HMat33 m_sceneToGameNormal;	// Tranpose of the inverse.
		F32 m_scale;
		bool m_hasReflection;
		bool m_flipX;
		bool m_flipY;
		bool m_flipZ;
	};
	
	//! @}
}



#endif


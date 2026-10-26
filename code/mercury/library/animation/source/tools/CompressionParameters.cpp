#include "stdafx.h"
#include <Animation/Tools/CompressionParameters.h>

namespace AnimationTools
{

C_CompressionParameters::C_CompressionParameters( U32 numJoints )
{
	m_jointParams.Resize( numJoints );

	SetGlobalSingleAxisRotationThreshold( 1.f / 8192.f );
	SetGlobalMaxRotationError( 0.1f );
	SetGlobalMaxTranslationError( 0.005f );
}

void C_CompressionParameters::SetGlobalSingleAxisRotationThreshold( F32 singleAxisRotationThreshold )
{
	m_singleAxisRotationThreshold = singleAxisRotationThreshold;
}

void C_CompressionParameters::SetGlobalMaxRotationError( F32 maxRotationErrorInDegrees )
{
	for ( U32 i = 0, count = m_jointParams.Size(); i < count; ++i )
	{
		m_jointParams[ i ].m_maxRotationErrorInDegrees = maxRotationErrorInDegrees;
	}
}

void C_CompressionParameters::SetGlobalMaxTranslationError( F32 maxTranslationError )
{
	for ( U32 i = 0, count = m_jointParams.Size(); i < count; ++i )
	{
		m_jointParams[ i ].m_maxTranslationError = maxTranslationError;
	}
}

void C_CompressionParameters::SetJointMaxRotationError( U32 jointIndex, F32 maxRotationErrorInDegrees )
{
	m_jointParams[ jointIndex ].m_maxRotationErrorInDegrees = maxRotationErrorInDegrees;
}

void C_CompressionParameters::SetJointMaxTranslationError( U32 jointIndex, F32 maxTranslationError )
{
	m_jointParams[ jointIndex ].m_maxTranslationError = maxTranslationError ;
}

F32 C_CompressionParameters::GetJointMaxRotationError( U32 jointIndex ) const
{
	return m_jointParams[ jointIndex ].m_maxRotationErrorInDegrees;
}

F32 C_CompressionParameters::GetJointMaxTranslationError( U32 jointIndex ) const
{
	return m_jointParams[ jointIndex ].m_maxTranslationError;
}

F32 C_CompressionParameters::GetSingleAxisRotationThreshold( U32 jointIndex ) const
{
	return m_singleAxisRotationThreshold;
}

F32 C_CompressionParameters::GetJointMaxScaleError( U32 jointIndex ) const
{
	return m_jointParams[ jointIndex ].m_maxScaleError;
}

void C_CompressionParameters::SetJointMaxScaleError( U32 jointIndex, F32 maxError )
{
	m_jointParams[ jointIndex ].m_maxScaleError = maxError;
}


} // namespace Animation

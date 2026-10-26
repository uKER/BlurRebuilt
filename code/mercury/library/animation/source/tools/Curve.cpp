#include "stdafx.h"
#include <animation/tools/Curve.h>
#include <animation/tools/CompressedCurveUtils.h>

namespace AnimationTools
{

C_Curve::C_Curve():
	m_numKeys( 0 ),
	m_type( E_CurveType_Invalid ),
	m_keyType( E_KeyType_Invalid )
{
}

C_Curve::~C_Curve()
{
}

void C_Curve::Clear()
{
	m_numKeys = 0;
	m_type = E_CurveType_Invalid;
	m_keyType = E_KeyType_Invalid;
}

void C_Curve::CreateBasePoseRotationCurve()
{
	m_numKeys = 0;
	m_type = E_CurveType_BasePoseRotation;
	m_keyType = E_KeyType_Invalid;
}

void C_Curve::CreateBasePoseTranslationCurve()
{
	m_numKeys = 0;
	m_type = E_CurveType_BasePoseTranslation;
	m_keyType = E_KeyType_Invalid;
}

void C_Curve::CreateIdentityRotationCurve()
{
	m_numKeys = 0;
	m_type = E_CurveType_IdentityQuaternion;
	m_keyType = E_KeyType_Invalid;
}

void C_Curve::CreateZeroTranslationCurve()
{
	m_numKeys = 0;
	m_type = E_CurveType_ZeroVector3;
	m_keyType = E_KeyType_Invalid;
}

bool C_Curve::CreateQuaternionCurve(
	E_KeyType keyType, 
	U32 numKeys,
	const U32* keyTimes,
	const Maths::C_Quaternion* quatKeys
)
{
	Clear();
	
	BIZ_ASSERT( IsQuaternionKeyType( keyType ) );
	
	if ( !IsQuaternionKeyType( keyType ) )
	{
		return false;
	}
	
	SetKeyTypeAndDeriveCurveType( keyType, numKeys );
		
	m_numKeys = numKeys;
	
	CopyInKeyTimes( keyTimes, numKeys );
	
	if ( numKeys == 0 )
	{
		return true;
	}
	
	m_keyValues.Resize( numKeys );
	
	switch ( keyType )
	{
		case E_KeyType_Quaternion32:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressQuaternion32( quatKeys[ i ], m_keyValues[ i ].m_u32 );
			}
			break;
			
		case E_KeyType_Quaternion48:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressQuaternion48( quatKeys[ i ], m_keyValues[ i ].m_vec3U16 );
			}
			break;
			
		case E_KeyType_QuaternionX16:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressSingleAxisQuaternion16( quatKeys[ i ], CompressedCurveUtils::E_QuaternionAxis_X, m_keyValues[ i ].m_u16 );
			}
			break;

		case E_KeyType_QuaternionY16:

			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressSingleAxisQuaternion16( quatKeys[ i ], CompressedCurveUtils::E_QuaternionAxis_Y, m_keyValues[ i ].m_u16 );
			}
			break;
			
		case E_KeyType_QuaternionZ16:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressSingleAxisQuaternion16( quatKeys[ i ], CompressedCurveUtils::E_QuaternionAxis_Z, m_keyValues[ i ].m_u16 );
			}
			break;
			
		case E_KeyType_QuaternionF32:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				Maths::Store< Maths::C_Quaternion, MathsStorage::Float4 >( quatKeys[ i ], (MathsStorage::Float4*) m_keyValues[ i ].m_vec4F32 );
			}
			break;
			
		default:
			BIZ_ASSERT( !"Quaternion curve type needs implementing!" );
			break;
	}

	return true;
}

bool C_Curve::CreateVector3Curve(
	E_KeyType keyType,
	U32 numKeys,
	const U32* keyTimes,
	const MathsStorage::Float3* vector3Keys,
	const F32 offset[ 3 ],
	const F32 scale[ 3 ]
)
{
	Clear();
	
	BIZ_ASSERT( IsVector3KeyType( keyType ) );
	
	if ( !IsVector3KeyType( keyType ) )
	{
		return false;
	}
	
	SetKeyTypeAndDeriveCurveType( keyType, numKeys );
	m_numKeys = numKeys;
	
	CopyInKeyTimes( keyTimes, numKeys );
	
	if ( numKeys == 0 )
	{
		return true;
	}
	
	m_keyValues.Resize( numKeys );
	
	F32 mins[ 3 ];
	F32 maxs[ 3 ];

	if ( offset && scale )
	{
		m_offset.x = offset[ 0 ];
		m_offset.y = offset[ 1 ];
		m_offset.z = offset[ 2 ];
		m_scale.x = scale[ 0 ];
		m_scale.y = scale[ 1 ];
		m_scale.z = scale[ 2 ];
	}
	else
	{
		CalculateBounds( numKeys, vector3Keys->v, 3, mins, maxs );
		SetScaleAndOffset( mins, maxs );
	}
	
	switch ( keyType )
	{
	
		case E_KeyType_Vector3F32:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				m_keyValues[ i ].m_vec3F32[ 0 ] = vector3Keys[ i ].x;
				m_keyValues[ i ].m_vec3F32[ 1 ] = vector3Keys[ i ].y;
				m_keyValues[ i ].m_vec3F32[ 2 ] = vector3Keys[ i ].z;
			}
			break;
		
		case E_KeyType_Vector3U16:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressVector3U16( vector3Keys[ i ].v, m_offset.v, m_scale.v, m_keyValues[ i ].m_vec3U16 );
			}
			break;
		
		case E_KeyType_Vector3U8:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				CompressedCurveUtils::CompressVector3U8( vector3Keys[ i ].v, m_offset.v, m_scale.v, m_keyValues[ i ].m_vec3U8 );
			}
			break;
			
		default:
			BIZ_ASSERT( !"Vector3 curve type needs implementing!" );
			break;
	}

	return true;
}

bool C_Curve::CreateScalarCurve(
	E_KeyType keyType,
	U32 numKeys,
	const U32* keyTimes,
	const F32* scalarKeys
)
{
	Clear();
	
	if ( !IsScalarKeyType( keyType ) )
	{
		BIZ_ASSERT( false );
		return false;
	}
	
	SetKeyTypeAndDeriveCurveType( keyType, numKeys );
	m_numKeys = numKeys;
	
	CopyInKeyTimes( keyTimes, numKeys );
	
	if ( numKeys == 0 )
	{
		return true;
	}
	
	m_keyValues.Resize( numKeys );
	
	switch ( keyType )
	{
	
		case E_KeyType_ScalarF32:
			for ( U32 i = 0; i < numKeys; ++i )
			{
				m_keyValues[ i ].m_f32 = scalarKeys[ i ];
			}
			break;
		
		default:
			BIZ_ASSERT( !"Scalar curve type needs implementing!" );
			return false;
			break;
	}

	return true;
}

E_CurveType C_Curve::GetType() const
{
	return m_type;
}

E_KeyType C_Curve::GetKeyType() const
{
	return m_keyType;
}

void C_Curve::SetKeyTypeAndDeriveCurveType( E_KeyType keyType, U32 numKeys )
{
	m_keyType = keyType;
	
	bool isConstant = false;
	if ( numKeys == 1 )
	{
		isConstant = true;
	}
	
	E_CurveType curveType = E_CurveType_Invalid;
	
	static const S32 constMappings[][2] =
	{
		{ E_KeyType_Quaternion48, E_CurveType_ConstantQuaternion48 },
		{ E_KeyType_QuaternionF32, E_CurveType_ConstantQuaternionF32 },
//		{ E_KeyType_Quaternion32, E_CurveType_ConstantQuaternion32 },
//		{ E_KeyType_QuaternionX16, E_CurveType_ConstantQuaternionX16 },
//		{ E_KeyType_QuaternionY16, E_CurveType_ConstantQuaternionY16 },
//		{ E_KeyType_QuaternionZ16, E_CurveType_ConstantQuaternionZ16 },
		{ E_KeyType_Vector3F32, E_CurveType_ConstantVector3F32 },
		{ E_KeyType_ScalarF32, E_CurveType_ConstantScalarF32 }
	};
	static const S32 varyingMappings[][2] = 
	{
		{ E_KeyType_Quaternion32, E_CurveType_VaryingQuaternion32 },
		{ E_KeyType_Quaternion48, E_CurveType_VaryingQuaternion48 },
		{ E_KeyType_QuaternionX16, E_CurveType_VaryingQuaternionX16 },
		{ E_KeyType_QuaternionY16, E_CurveType_VaryingQuaternionY16 },
		{ E_KeyType_QuaternionZ16, E_CurveType_VaryingQuaternionZ16 },
		{ E_KeyType_QuaternionF32, E_CurveType_VaryingQuaternionF32 },
		{ E_KeyType_Vector3F32,	E_CurveType_VaryingVector3F32 },
		{ E_KeyType_Vector3U16,	E_CurveType_VaryingVector3U16 },
		{ E_KeyType_Vector3U8, E_CurveType_VaryingVector3U8 },
		{ E_KeyType_ScalarF32, E_CurveType_VaryingScalarF32 }
	};
	
	
	if ( isConstant )
	{
		for ( U32 i = 0, count = sizeof( constMappings ) / sizeof( constMappings[ 0 ] ); i < count; ++i )
		{
			if ( constMappings[ i ][ 0 ] == keyType )
			{
				curveType = static_cast< E_CurveType >( constMappings[ i ][ 1 ] );
				break;
			}
		}
	}
	
	if ( curveType == E_CurveType_Invalid )
	{
		for ( U32 i = 0, count = sizeof( varyingMappings ) / sizeof( varyingMappings[ 0 ] ); i < count; ++i )
		{
			if ( varyingMappings[ i ][ 0 ] == keyType )
			{
				curveType = static_cast< E_CurveType >( varyingMappings[ i ][ 1 ] );
				break;
			}
		}
	}
	
	BIZ_ASSERT( curveType != E_CurveType_Invalid );
	m_type = curveType;
}

U32 C_Curve::GetNumKeys() const
{
	return m_numKeys;
}

U32 C_Curve::GetKeyTime( U32 keyIndex ) const
{
	BIZ_ASSERT( keyIndex < m_numKeys );
	return m_keyTimes[ keyIndex ];
}

void C_Curve::FindKeyIndexForTime( F32 timeInFrames, U32& outLowerKeyIndex, F32& outInterpolationFraction ) const
{
	U32 nextKeyIndex = 0;
	
	CompressedCurveUtils::FindKeyIndexForTime(
		&m_keyTimes[ 0 ],
		m_keyTimes.Size(),
		timeInFrames,
		nextKeyIndex,
		outInterpolationFraction
	);
	
	if ( nextKeyIndex == 0 )
	{
		outLowerKeyIndex = 0;
	}
	else if ( nextKeyIndex == m_keyTimes.Size() )
	{
		outLowerKeyIndex = Maths::Max( m_keyTimes.Size() - 2, 0 );
	}
	else
	{
		outLowerKeyIndex = nextKeyIndex - 1;
	}
}

S32 C_Curve::FindFirstKeyIndexForTime(U32 timeInFrames) const
{
	for (U32 i = 0, count = m_keyTimes.Size(); i < count; ++i)
	{
		if (m_keyTimes[i] == timeInFrames)
		{
			return i;
		}
		else if (m_keyTimes[i] > timeInFrames)
		{
			return -1;
		}
		
	}
	return -1;
}


Maths::C_Quaternion C_Curve::GetQuaternionKey( U32 keyIndex ) const
{
	BIZ_ASSERT( IsQuaternionCurve() );
	
	Maths::C_Quaternion result;
	
	switch ( m_keyType )
	{
		case E_KeyType_Quaternion32: CompressedCurveUtils::DecompressQuaternion32( m_keyValues[ keyIndex ].m_u32, result ); break;
		case E_KeyType_Quaternion48: CompressedCurveUtils::DecompressQuaternion48( m_keyValues[ keyIndex ].m_vec3U16, result ); break;
		case E_KeyType_QuaternionX16: CompressedCurveUtils::DecompressSingleAxisQuaternion16( m_keyValues[ keyIndex ].m_u16, CompressedCurveUtils::E_QuaternionAxis_X, result ); break;
		case E_KeyType_QuaternionY16: CompressedCurveUtils::DecompressSingleAxisQuaternion16( m_keyValues[ keyIndex ].m_u16, CompressedCurveUtils::E_QuaternionAxis_Y, result ); break;
		case E_KeyType_QuaternionZ16: CompressedCurveUtils::DecompressSingleAxisQuaternion16( m_keyValues[ keyIndex ].m_u16, CompressedCurveUtils::E_QuaternionAxis_Z, result ); break;		
		default: break;
	}
	// {{{
	return !result;
}

Maths::HVec3 C_Curve::GetVector3Key( U32 keyIndex ) const
{
	MathsStorage::Float3 result;
	
	switch ( m_keyType )
	{
		case E_KeyType_Vector3F32:
			result.x = m_keyValues[ keyIndex ].m_vec3F32[ 0 ];
			result.y = m_keyValues[ keyIndex ].m_vec3F32[ 1 ];
			result.z = m_keyValues[ keyIndex ].m_vec3F32[ 2 ];
			break;
			
		case E_KeyType_Vector3U16:
			CompressedCurveUtils::DecompressVector3U16( m_keyValues[ keyIndex ].m_vec3U16, m_offset.v, m_scale.v, result.v );
			break;
			
		case E_KeyType_Vector3U8:
			CompressedCurveUtils::DecompressVector3U8( m_keyValues[ keyIndex ].m_vec3U8, m_offset.v, m_scale.v, result.v );
			break;
			
		default:
			BIZ_ASSERT( !"Not a vector3 curve" );
			result.x = 0.f;
			result.y = 0.f;
			result.z = 0.f;
			break;
	}
	
	return Maths::Load< Maths::HVec3, MathsStorage::Float3 >( &result );
}

F32 C_Curve::GetScalarKey( U32 keyIndex ) const
{
	F32 value = 0.f;
	switch ( m_keyType )
	{
		case E_KeyType_ScalarF32:
			value = m_keyValues[keyIndex].m_f32;
		break;
		
		default:
			BIZ_ASSERT(!"Not a scalar curve");
			break;
	}
	return value;
}

bool C_Curve::IsConstant() const
{
	return m_numKeys <= 1;
}

bool C_Curve::IsVarying() const
{
	return m_numKeys > 1;
}

bool C_Curve::IsInvalid() const
{
	return m_type == E_CurveType_Invalid;
}

bool C_Curve::IsValid() const
{
	return m_type != E_CurveType_Invalid;
}

bool C_Curve::IsBasePose() const
{
	return m_type == E_CurveType_BasePoseRotation || m_type == E_CurveType_BasePoseTranslation;
}

//bool C_Curve::IsNonBasePose() const
//{
//	return m_type != E_CurveType_BasePoseRotation && m_type != E_CurveType_BasePoseTranslation;
//}

bool C_Curve::HasKeys() const
{
	return m_numKeys > 0;
}


const F32* C_Curve::GetScale() const
{
	return m_scale.v;
}

const F32* C_Curve::GetOffset() const
{
	return m_offset.v;
}

void C_Curve::GetKeyDataVec4F32( U32 keyIndex, F32 outKey[ 4 ] ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_QuaternionF32 );
	const F32* key = m_keyValues[ keyIndex ].m_vec4F32;
	outKey[ 0 ] = key[ 0 ];
	outKey[ 1 ] = key[ 1 ];
	outKey[ 2 ] = key[ 2 ];
	outKey[ 3 ] = key[ 3 ];
}

void C_Curve::GetKeyDataVec3F32( U32 keyIndex, F32 outKey[ 3 ] ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_Vector3F32 );
	const F32* key = m_keyValues[ keyIndex ].m_vec3F32;
	outKey[ 0 ] = key[ 0 ];
	outKey[ 1 ] = key[ 1 ];
	outKey[ 2 ] = key[ 2 ];
}

void C_Curve::GetKeyDataF32( U32 keyIndex, F32& outKey ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_ScalarF32 );
	outKey = m_keyValues[ keyIndex ].m_f32;
}

void C_Curve::GetKeyDataU32( U32 keyIndex, U32& outKey ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_Quaternion32 );
	outKey = m_keyValues[ keyIndex ].m_u32;
}

void C_Curve::GetKeyDataU16( U32 keyIndex, U16& outKey ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_QuaternionX16 || m_keyType == E_KeyType_QuaternionY16 || m_keyType == E_KeyType_QuaternionZ16 );
	outKey = m_keyValues[ keyIndex ].m_u16;

}

void C_Curve::GetKeyDataVec3U16( U32 keyIndex, U16 outKey[ 3 ] ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_Quaternion48 || m_keyType == E_KeyType_Vector3U16 );
	const U16* key = m_keyValues[ keyIndex ].m_vec3U16;
	outKey[ 0 ] = key[ 0 ];
	outKey[ 1 ] = key[ 1 ];
	outKey[ 2 ] = key[ 2 ];
}


void C_Curve::GetKeyDataVec3U8( U32 keyIndex, U8 outKey[ 3 ] ) const
{
	BIZ_ASSERT( m_keyType == E_KeyType_Vector3U8 );
	const U8* key = m_keyValues[ keyIndex ].m_vec3U8;
	outKey[ 0 ] = key[ 0 ];
	outKey[ 1 ] = key[ 1 ];
	outKey[ 2 ] = key[ 2 ];
}

void C_Curve::GetKeyData( U32 keyIndex, KeyValue& outKeyValue ) const
{
	outKeyValue = m_keyValues[ keyIndex ];
}

void C_Curve::CopyInKeyTimes( const U32* keyTimes, U32 numKeys )
{
	m_numKeys = numKeys;
	m_keyTimes.Resize( numKeys );
	
	if ( numKeys > 0 )
	{
		memcpy( &m_keyTimes[ 0 ], keyTimes, sizeof( U32 ) * numKeys );
	}
}

bool C_Curve::IsQuaternionCurve() const
{
	return IsQuaternionKeyType( m_keyType );
}

bool C_Curve::IsVector3Curve() const
{
	return IsVector3KeyType( m_keyType );
}

bool C_Curve::IsScalarCurve() const
{
	return IsScalarKeyType( m_keyType );
}

void C_Curve::CalculateBounds( U32 numVectors, const F32* vectors, U32 numDimensions, F32* outMins, F32* outMaxs ) const
{
	const F32* pVec = vectors;
	
	if ( 0 == numVectors )
	{
		for ( U32 i = 0; i < numDimensions; ++i )
		{
			outMins[ i ] = 0.f;
			outMaxs[ i ] = 0.f;
		}
		return;
	}
	
	for ( U32 i = 0; i < numDimensions; ++i )
	{
		outMins[ i ] = pVec[ i ];
		outMaxs[ i ] = pVec[ i ];
	}
		
	pVec += numDimensions;
	
	for ( U32 i = 1; i < numVectors; ++i )
	{
		for ( U32 j = 0; j < numDimensions; ++j )
		{
			if ( pVec[ j ] < outMins[ j ] ) outMins[ j ] = pVec[ j ];
			if ( pVec[ j ] > outMaxs[ j ] ) outMaxs[ j ] = pVec[ j ];
		}
		pVec += numDimensions;
	}
}

void C_Curve::SetScaleAndOffset( const F32 minBounds[ 3 ], const F32 maxBounds[ 3 ] )
{
	m_offset.x = minBounds[ 0 ];
	m_offset.y = minBounds[ 1 ];
	m_offset.z = minBounds[ 2 ];
	m_scale.x = maxBounds[ 0 ] - minBounds[ 0 ];
	m_scale.y = maxBounds[ 1 ] - minBounds[ 1 ];
	m_scale.z = maxBounds[ 2 ] - minBounds[ 2 ];
}

//
// Curve utility functions
//

using namespace Animation;

bool IsQuaternionKeyType( E_KeyType keyType )
{
	return keyType >= E_KeyType_Quaternion32 && keyType <= E_KeyType_QuaternionF32;
}

bool IsVector3KeyType( E_KeyType keyType )
{
	return keyType >= E_KeyType_Vector3F32 && keyType <= E_KeyType_Vector3U8;
}

bool IsScalarKeyType( E_KeyType keyType )
{
	return keyType == E_KeyType_ScalarF32;
}


const CHAR8* ToString( E_CurveType curveType )
{
	switch ( curveType )
	{
		//case E_CurveType_ConstantQuaternion32: return "ConstQuat32";
		case E_CurveType_ConstantQuaternion48: return "ConstQuat48";
		//case E_CurveType_ConstantQuaternionX16: return "ConstQuatX16";
		//case E_CurveType_ConstantQuaternionY16: return "ConstQuatY16";
		//case E_CurveType_ConstantQuaternionZ16: return "ConstQuatZ16";
		case E_CurveType_ConstantVector3F32: return "ConstVec3F32";
		case E_CurveType_ConstantScalarF32: return "ConstScalarF32";

		case E_CurveType_VaryingQuaternion32: return "VarQuat32";
		case E_CurveType_VaryingQuaternion48: return "VarQuat48";
		case E_CurveType_VaryingQuaternionX16: return "VarQuatX16";
		case E_CurveType_VaryingQuaternionY16: return "VarQuatY16";
		case E_CurveType_VaryingQuaternionZ16: return "VarQuatZ16";
		case E_CurveType_VaryingVector3F32: return "VarVec3F32";
		case E_CurveType_VaryingVector3U16: return "VarVec3U16";
		case E_CurveType_VaryingVector3U8: return "VarVec3U8";
		case E_CurveType_VaryingScalarF32: return "VarScalarF32";
		
		case E_CurveType_BasePoseRotation: return "BasePoseRot";
		case E_CurveType_BasePoseTranslation: return "BasePoseTrans";
		
		case E_CurveType_IdentityQuaternion: return "IdentQuat";
		case E_CurveType_ZeroVector3: return "ZeroVec3";
				
		default: break;
	}
	return "<Unknown>";
}

const CHAR8* ToString( E_KeyType keyType )
{
	switch ( keyType )
	{
		case E_KeyType_Quaternion32: return "Quat32";
		case E_KeyType_Quaternion48: return "Quat48";
		case E_KeyType_QuaternionX16: return "QuatX16:";
		case E_KeyType_QuaternionY16: return "QuatY16:";
		case E_KeyType_QuaternionZ16: return "QuatZ16:";
		case E_KeyType_QuaternionF32: return "QuatF32";
		case E_KeyType_Vector3F32: return "Vector3F32";
		case E_KeyType_Vector3U16: return "Vector3U16";
		case E_KeyType_Vector3U8: return "Vector3U8";
		case E_KeyType_ScalarF32: return "ScalarF32";
		case E_KeyType_Invalid: return "Invalid";
		default: break;
	}
	return "<Unknown>";
}

Maths::C_Quaternion SampleQuaternionCurve( const C_Curve& curve, F32 timeInFrames )
{
	if ( curve.IsConstant() )
	{
		return GetFirstQuaternionKey( curve );
	}

	U32 lowerKeyIndex;
	F32 lerpFrac;
	curve.FindKeyIndexForTime( timeInFrames, lowerKeyIndex, lerpFrac );

	Maths::C_Quaternion q1 = curve.GetQuaternionKey( lowerKeyIndex );
	Maths::C_Quaternion q2 = curve.GetQuaternionKey( lowerKeyIndex + 1);

	return q1.ShortestLerpNormalised( q2, Maths::HF32( lerpFrac ) );
}

Maths::HVec3 SampleVector3Curve( const C_Curve& curve, F32 timeInFrames )
{
	if ( curve.IsConstant() )
	{
		return GetFirstVector3Key( curve );
	}

	U32 lowerKeyIndex;
	F32 lerpFrac;
	curve.FindKeyIndexForTime( timeInFrames, lowerKeyIndex, lerpFrac );

	Maths::HVec3 v1 = curve.GetVector3Key( lowerKeyIndex );
	Maths::HVec3 v2 = curve.GetVector3Key( lowerKeyIndex + 1);

	return v1.Lerp( v2, Maths::HF32( lerpFrac ) );
}

F32 SampleScalarCurve( const C_Curve& curve, F32 timeInFrames )
{
	if ( curve.IsConstant() )
	{
		return GetFirstScalarKey( curve );
	}

	U32 lowerKeyIndex;
	F32 lerpFrac;
	curve.FindKeyIndexForTime( timeInFrames, lowerKeyIndex, lerpFrac );

	F32 s1 = curve.GetScalarKey( lowerKeyIndex );
	F32 s2 = curve.GetScalarKey( lowerKeyIndex + 1);

	return Maths::Lerp( s1, s2, lerpFrac );
}

Maths::C_Quaternion GetFirstQuaternionKey( const C_Curve& curve )
{
	return curve.GetQuaternionKey( 0 );
}

Maths::C_Quaternion GetLastQuaternionKey( const C_Curve& curve )
{
	return curve.GetQuaternionKey( curve.GetNumKeys() - 1 );
}

Maths::HVec3 GetFirstVector3Key( const C_Curve& curve )
{
	return curve.GetVector3Key( 0 );
}

Maths::HVec3 GetLastVector3Key( const C_Curve& curve )
{
	return curve.GetVector3Key( curve.GetNumKeys() - 1 );
}

F32 GetFirstScalarKey( const C_Curve& curve )
{
	return curve.GetScalarKey( 0 );
}

F32 GetLastScalarKey( const C_Curve& curve )
{
	return curve.GetScalarKey( curve.GetNumKeys() - 1 );
}

U32 GetCurveSizeInBytes( const C_Curve& curve )
{
	U32 n = curve.GetNumKeys();
	U32 keyTimeSize = 4;
	U32 keyValueSize = GetCurveKeyValueSize( curve );
	U32 size = n * ( keyTimeSize + keyValueSize );
	
	// Add additional overhead required by certain curve types.
	switch ( curve.GetType() )
	{
		case E_CurveType_VaryingVector3U16: size += 24; break;
		case E_CurveType_VaryingVector3U8: size = 24; break;
		default: break;
	}
	
	return size;
}

U32 GetKeyTypeValueSize( E_KeyType keyType )
{
	U32 size = 0;
	switch ( keyType )
	{
		case E_KeyType_Quaternion32: size = 4; break;
		case E_KeyType_Quaternion48: size = 6; break;
		case E_KeyType_QuaternionX16: size = 2; break;
		case E_KeyType_QuaternionY16: size = 2; break;
		case E_KeyType_QuaternionZ16: size = 2; break;
		case E_KeyType_QuaternionF32: size = 16; break;
		case E_KeyType_Vector3F32: size = 12; break;
		case E_KeyType_Vector3U16: size = 6; break;
		case E_KeyType_Vector3U8: size = 3; break;
		case E_KeyType_ScalarF32: size = 4; break;
		default: break;
	}
	return size;
}

U32 GetCurveKeyValueSize( const C_Curve& curve )
{
	return GetKeyTypeValueSize( curve.GetKeyType() );
}

E_KeyType GetKeyTypeFromCurveType( E_CurveType curveType )
{
	E_KeyType keyType = E_KeyType_Invalid;
	switch ( curveType )
	{
		case E_CurveType_ConstantQuaternion48: keyType = E_KeyType_Quaternion48; break;
		case E_CurveType_ConstantQuaternionF32: keyType = E_KeyType_QuaternionF32; break;
		case E_CurveType_ConstantVector3F32: keyType = E_KeyType_Vector3F32; break;
		case E_CurveType_ConstantScalarF32: keyType = E_KeyType_ScalarF32; break;

		case E_CurveType_VaryingQuaternion32: keyType = E_KeyType_Quaternion32; break;
		case E_CurveType_VaryingQuaternion48: keyType = E_KeyType_Quaternion48; break;
		case E_CurveType_VaryingQuaternionF32: keyType = E_KeyType_QuaternionF32; break;
		case E_CurveType_VaryingQuaternionX16: keyType = E_KeyType_QuaternionX16; break;
		case E_CurveType_VaryingQuaternionY16: keyType = E_KeyType_QuaternionY16; break;
		case E_CurveType_VaryingQuaternionZ16: keyType = E_KeyType_QuaternionZ16; break;
		case E_CurveType_VaryingVector3F32: keyType = E_KeyType_Vector3F32; break;
		case E_CurveType_VaryingVector3U16: keyType = E_KeyType_Vector3U16; break;
		case E_CurveType_VaryingVector3U8: keyType = E_KeyType_Vector3U8; break;
		case E_CurveType_VaryingScalarF32: keyType = E_KeyType_ScalarF32; break;
		
		case E_CurveType_BasePoseRotation: break;
		case E_CurveType_BasePoseTranslation: break;
		
		case E_CurveType_IdentityQuaternion: break;
		case E_CurveType_ZeroVector3: break;
				
		default: break;
	}

	return keyType;
}

E_VaryingCurveType GetVaryingCurveType( E_CurveType curveType )
{
	E_VaryingCurveType varyingType = E_VaryingCurveType_Invalid;
	switch ( curveType )
	{
		case E_CurveType_ConstantQuaternion48: break;
		case E_CurveType_ConstantQuaternionF32: break;
		case E_CurveType_ConstantVector3F32: break;
		case E_CurveType_ConstantScalarF32: break;

		case E_CurveType_VaryingQuaternion32: varyingType = E_VaryingCurveType_VaryingQuaternion32; break;
		case E_CurveType_VaryingQuaternion48: varyingType = E_VaryingCurveType_VaryingQuaternion48; break;
		case E_CurveType_VaryingQuaternionF32: break;
		case E_CurveType_VaryingQuaternionX16: break;
		case E_CurveType_VaryingQuaternionY16: break;
		case E_CurveType_VaryingQuaternionZ16: break;
		case E_CurveType_VaryingVector3F32: varyingType = E_VaryingCurveType_VaryingVector3F32; break;
		case E_CurveType_VaryingVector3U16: varyingType = E_VaryingCurveType_VaryingVector3U16; break;
		case E_CurveType_VaryingVector3U8: varyingType = E_VaryingCurveType_VaryingVector3U8; break;
		case E_CurveType_VaryingScalarF32: varyingType = E_VaryingCurveType_VaryingScalarF32; break;
		
		case E_CurveType_BasePoseRotation: break;
		case E_CurveType_BasePoseTranslation: break;
		
		case E_CurveType_IdentityQuaternion: break;
		case E_CurveType_ZeroVector3: break;
				
		default: break;
	}
	BIZ_ASSERT( varyingType != E_VaryingCurveType_Invalid );
	return varyingType;
}

E_ConstantCurveType GetConstantCurveType( E_CurveType curveType )
{
	E_ConstantCurveType constType = E_ConstantCurveType_Invalid;
	
	switch ( curveType )
	{
		case E_CurveType_ConstantQuaternion48: constType = E_ConstantCurveType_ConstantQuaternion48; break;
		case E_CurveType_ConstantQuaternionF32: break;
		case E_CurveType_ConstantVector3F32: constType = E_ConstantCurveType_ConstantVector3F32; break;
		case E_CurveType_ConstantScalarF32: constType = E_ConstantCurveType_ConstantScalarF32; break;

		case E_CurveType_VaryingQuaternion32: break;
		case E_CurveType_VaryingQuaternion48: break;
		case E_CurveType_VaryingQuaternionF32: break;
		case E_CurveType_VaryingQuaternionX16: break;
		case E_CurveType_VaryingQuaternionY16: break;
		case E_CurveType_VaryingQuaternionZ16: break;
		case E_CurveType_VaryingVector3F32: break;
		case E_CurveType_VaryingVector3U16: break;
		case E_CurveType_VaryingVector3U8: break;
		case E_CurveType_VaryingScalarF32: break;
		
		case E_CurveType_BasePoseRotation: ; break;
		case E_CurveType_BasePoseTranslation: ; break;
		
		case E_CurveType_IdentityQuaternion: break;
		case E_CurveType_ZeroVector3: break;
				
		default: break;
	}

	BIZ_ASSERT( constType != E_ConstantCurveType_Invalid );
	return constType;
}

void SampleCurveAsKeyValue( const C_Curve& curve, F32 timeInFrames, KeyValue& outKeyValue )
{
	U32 lowerKeyIndex;
	F32 lerpFrac;
	curve.FindKeyIndexForTime( timeInFrames, lowerKeyIndex, lerpFrac );

	if ( curve.IsConstant() )
	{
		curve.GetKeyData( 0, outKeyValue );
		return;
	}
	
	KeyValue prevKey;
	KeyValue nextKey;
	
	curve.GetKeyData( lowerKeyIndex, prevKey );
	curve.GetKeyData( lowerKeyIndex + 1, nextKey );
	
	if ( lerpFrac == 0.f )
	{
		outKeyValue = prevKey;
		return;
	}
	
	if ( curve.IsQuaternionCurve() )
	{
		const Maths::C_Quaternion q = SampleQuaternionCurve( curve, timeInFrames );

		switch ( curve.GetKeyType() )
		{
			case E_KeyType_Quaternion32:
				CompressedCurveUtils::CompressQuaternion32( q, outKeyValue.m_u32 );
				break;			
				
			case E_KeyType_Quaternion48:
				CompressedCurveUtils::CompressQuaternion48( q, outKeyValue.m_vec3U16 );
				break;			
			
			case E_KeyType_QuaternionX16:
				CompressedCurveUtils::CompressSingleAxisQuaternion16( q, CompressedCurveUtils::E_QuaternionAxis_X, outKeyValue.m_u16 );
				break;			

			case E_KeyType_QuaternionY16:
				CompressedCurveUtils::CompressSingleAxisQuaternion16( q, CompressedCurveUtils::E_QuaternionAxis_Y, outKeyValue.m_u16 );
				break;
			case E_KeyType_QuaternionZ16:
				CompressedCurveUtils::CompressSingleAxisQuaternion16( q, CompressedCurveUtils::E_QuaternionAxis_Z, outKeyValue.m_u16 );
				break;

			case E_KeyType_QuaternionF32:
				Maths::Store< Maths::HVec4, MathsStorage::Float4 >( Maths::HVec4( q ), (MathsStorage::Float4*) outKeyValue.m_vec4F32 );
				break;
				
			default:
				BIZ_ASSERT( false );
				break;
		}
	}
	else if ( curve.IsVector3Curve() )
	{
		const Maths::HVec3 v = SampleVector3Curve( curve, timeInFrames );
		MathsStorage::Float3 vs;
		Maths::Store( v, &vs );
		switch ( curve.GetKeyType() )
		{
			case E_KeyType_Vector3F32:
				outKeyValue.m_vec3F32[ 0 ] = v.X();
				outKeyValue.m_vec3F32[ 1 ] = v.Y();
				outKeyValue.m_vec3F32[ 2 ] = v.Z();
				break;
				
			case E_KeyType_Vector3U16:
				CompressedCurveUtils::CompressVector3U16( vs.v, curve.GetOffset(), curve.GetScale(), outKeyValue.m_vec3U16 );
				break;
				
			case E_KeyType_Vector3U8:
				CompressedCurveUtils::CompressVector3U8( vs.v, curve.GetOffset(), curve.GetScale(), outKeyValue.m_vec3U8 );
				break;
				
			default:
				BIZ_ASSERT( false );
				break;
		}
	}
	else if ( curve.IsScalarCurve() )
	{
		BIZ_ASSERT( curve.GetKeyType() == E_KeyType_ScalarF32 );
		F32 s = SampleScalarCurve( curve, timeInFrames );
		outKeyValue.m_f32 = s;
	}
	
}

void ConvertKeyValue(
	E_KeyType fromKeyType,
	E_KeyType toKeyType,
	const F32* offset,
	const F32* scale,
	const KeyValue& keyValue,
	KeyValue& outKeyValue )
{
	if ( IsQuaternionKeyType( fromKeyType ) )
	{
		BIZ_ASSERT( fromKeyType == E_KeyType_QuaternionF32 );
	
		const Maths::C_Quaternion q = Maths::Load< Maths::C_Quaternion, MathsStorage::Float4 >( (const MathsStorage::Float4*) keyValue.m_vec4F32 );

		switch ( toKeyType )
		{
			case E_KeyType_Quaternion32:
				CompressedCurveUtils::CompressQuaternion32( q, outKeyValue.m_u32 );
				break;			
				
			case E_KeyType_Quaternion48:
				CompressedCurveUtils::CompressQuaternion48( q, outKeyValue.m_vec3U16 );
				break;			
			
			case E_KeyType_QuaternionX16:
				CompressedCurveUtils::CompressSingleAxisQuaternion16( q, CompressedCurveUtils::E_QuaternionAxis_X, outKeyValue.m_u16 );
				break;			

			case E_KeyType_QuaternionY16:
				CompressedCurveUtils::CompressSingleAxisQuaternion16( q, CompressedCurveUtils::E_QuaternionAxis_Y, outKeyValue.m_u16 );
				break;
			case E_KeyType_QuaternionZ16:
				CompressedCurveUtils::CompressSingleAxisQuaternion16( q, CompressedCurveUtils::E_QuaternionAxis_Z, outKeyValue.m_u16 );
				break;

			case E_KeyType_QuaternionF32:
				outKeyValue = keyValue;
				break;
				
			default:
				BIZ_ASSERT( false );
				break;
		}
	}
	else if ( IsVector3KeyType( fromKeyType ) )
	{
		BIZ_ASSERT( fromKeyType == E_KeyType_Vector3F32 );
		
		switch ( toKeyType )
		{
			case E_KeyType_Vector3F32:
				outKeyValue = keyValue;
				break;
				
			case E_KeyType_Vector3U16:
				CompressedCurveUtils::CompressVector3U16( keyValue.m_vec3F32, offset, scale, outKeyValue.m_vec3U16 );
				break;
				
			case E_KeyType_Vector3U8:
				CompressedCurveUtils::CompressVector3U8( keyValue.m_vec3F32, offset, scale, outKeyValue.m_vec3U8 );
				break;
				
			default:
				BIZ_ASSERT( false );
				break;
		}
	}
	else if ( IsScalarKeyType( fromKeyType ) )
	{
		BIZ_ASSERT( fromKeyType == E_KeyType_ScalarF32 && toKeyType == E_KeyType_ScalarF32 );
		outKeyValue = keyValue;
	}
}

} // namespace Animation

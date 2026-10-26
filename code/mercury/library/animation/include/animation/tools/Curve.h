#ifndef ANIMATION_TOOLS_CURVE_H
#define ANIMATION_TOOLS_CURVE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <animation/cycle/CurveTypes.h>

namespace AnimationTools
{

/// @addtogroup Animation_Tools
//! @{


/// Key types.
enum E_KeyType
{
	E_KeyType_Quaternion32 = 0,			///< 32-bit quantised quaternion.
	E_KeyType_Quaternion48,				///< 48-bit quantised quaternion.
	E_KeyType_QuaternionX16,			///< 16-bit quantised quaternion for rotation about X axis.
	E_KeyType_QuaternionY16,			///< 16-bit quantised quaternion for rotation about Y axis.
	E_KeyType_QuaternionZ16,			///< 16-bit quantised quaternion for rotation about Z axis.
	E_KeyType_QuaternionF32,			///< 4 x float uncompressed quaternion.
	E_KeyType_Vector3F32,				///< 3 x float uncompressed vector 3.
	E_KeyType_Vector3U16,				///< Quantised 3 x 16-bit vector within predefined bounds.
	E_KeyType_Vector3U8,				///< Quantised 3 x 8-bit vector within predefined bounds.
	E_KeyType_ScalarF32,				///< Scalar float uncompressed.
	E_KeyType_Count,
	E_KeyType_Invalid = E_KeyType_Count	///< Invalid key type.
};

/// Curve types.
enum E_CurveType
{
	E_CurveType_ConstantQuaternion48 = 0,
	E_CurveType_ConstantQuaternionF32,
	E_CurveType_ConstantVector3F32,
	E_CurveType_ConstantScalarF32,
	
	E_CurveType_VaryingQuaternion32,
	E_CurveType_VaryingQuaternion48,
	E_CurveType_VaryingQuaternionF32,
	E_CurveType_VaryingQuaternionX16,
	E_CurveType_VaryingQuaternionY16,
	E_CurveType_VaryingQuaternionZ16,
	
	E_CurveType_VaryingVector3F32,
	E_CurveType_VaryingVector3U16,
	E_CurveType_VaryingVector3U8,
	E_CurveType_VaryingScalarF32,

	E_CurveType_BasePoseRotation,
	E_CurveType_BasePoseTranslation,
	
	E_CurveType_IdentityQuaternion,
	E_CurveType_ZeroVector3,
	
	E_CurveType_Count,
	E_CurveType_Invalid = E_CurveType_Count
};


union KeyValue
{
	F32 m_vec4F32[ 4 ];
	F32 m_vec3F32[ 3 ];
	F32 m_vec2F32[ 2 ];
	F32 m_f32;
	U32 m_u32;
	U16 m_u16;
	U16 m_vec3U16[ 3 ];
	U8 m_vec3U8[ 3 ];
	U8 m_u8;
};


///
/// A linear curve with sparse key frames.
///
class C_Curve
{
public:
	C_Curve();
	~C_Curve();
	
	/// Clear the curve of all data, making it invalid.
	void Clear();
	
	/// Create a curve equivalent to a joint's base pose rotation. The curve itself contains no data.
	void CreateBasePoseRotationCurve();
	
	/// Create a curve equivalent to a joint's base pose translation. The curve itself contains no data.
	void CreateBasePoseTranslationCurve();
	
	/// Create a curve equivalent to the identity quaternion. The curve itself contains no data.
	void CreateIdentityRotationCurve();
	
	/// Create a curve equivalent to a zero 3-d vector. The curve itself contains no data.
	void CreateZeroTranslationCurve();
	
	/// Create a quaternion curve of the specified type (which must be a valid quaternion curve type).
	bool CreateQuaternionCurve(
		E_KeyType keyType, 
		U32 numKeys,
		const U32* keyTimes,
		const Maths::C_Quaternion* quatKeys
	);

	/// Create a vector3 curve of the specified type (which must be a valid vector3 curve type).
	bool CreateVector3Curve(
		E_KeyType keyType,
		U32 numKeys,
		const U32* keyTimes,
		const MathsStorage::Float3* vector3Keys,
		const F32 offset[ 3 ] = 0,
		const F32 scale[ 3 ] = 0
	);

	/// Create a scalar curve of the specified type (which must be a valid scalar curve type).
	bool CreateScalarCurve(
		E_KeyType keyType,
		U32 numKeys,
		const U32* keyTimes,
		const F32* scalarKeys
	);

	/// Get type of curve.
	E_CurveType GetType() const;
	
	/// Get type of keys.
	E_KeyType GetKeyType() const;

	/// Set the key type and derive the curve type from this and the number of keys.
	void SetKeyTypeAndDeriveCurveType( E_KeyType keyType, U32 numKeys );
	
	/// Get number of keys.
	U32 GetNumKeys() const;

	/// Get a key frame's time, in frames.
	U32 GetKeyTime( U32 keyIndex ) const;

	/// Find key index for a given time in frames.
	void FindKeyIndexForTime( F32 timeInFrames, U32& outLowerKeyIndex, F32& outInterpolationFraction ) const;

	/// Returns index of first key with time @a timeInFrames or -1 if there is no key with this time.
	S32 FindFirstKeyIndexForTime(U32 timeInFrames) const;

	/// Get quaternion key. Curve must be a valid quaternion type.
	Maths::C_Quaternion GetQuaternionKey( U32 keyIndex ) const;
	
	/// Get vector3 key. Curve must be a valid vector3 type.
	Maths::HVec3 GetVector3Key( U32 keyIndex ) const;
	
	/// Get scalar key. Curve must be a valid scalar type.
	F32 GetScalarKey( U32 keyIndex ) const;
	
	/// Returns true if curve is constant (i.e. a single key).
	bool IsConstant() const;

	/// Returns true if curve is varying (i.e. more than one key).
	bool IsVarying() const;
	
	/// Returns true if curve is invalid.
	bool IsInvalid() const;

	/// Returns true if curve is valid.
	bool IsValid() const;
	
	/// Returns true if curve is a base pose curve.
	bool IsBasePose() const;
	
	/// Returns true if curve is not a base pose curve;
	//bool IsNonBasePose() const;
	
	/// Returns true if curve has keys (i.e. it's not a base pose or identity curve).
	bool HasKeys() const;
	
	
	/// Get scale factor used to unpack curve data.
	const F32* GetScale() const;
	
	/// Get offset used to unpack curve data.
	const F32* GetOffset() const;
	
	// Direct access to compressed key data.
	
	void GetKeyDataVec4F32( U32 keyIndex, F32 outKey[ 4 ] ) const;
	void GetKeyDataVec3F32( U32 keyIndex, F32 outKey[ 3 ] ) const;
	void GetKeyDataF32( U32 keyIndex, F32& outKey ) const;
	void GetKeyDataU32( U32 keyIndex, U32& outKey ) const;
	void GetKeyDataU16( U32 keyIndex, U16& outKey ) const;
	void GetKeyDataVec3U16( U32 keyIndex, U16 outKey[ 3 ] ) const;
	void GetKeyDataVec3U8( U32 keyIndex, U8 outKey[ 3 ] ) const;
	
	void GetKeyData( U32 keyIndex, KeyValue& outKeyValue ) const;
	
	// Access to interpolated keys in compressed formats.
	
	void SampleVec4F32( F32 timeInFrames, F32 outKey[ 4 ] ) const;
	void SampleVec3F32( F32 timeInFrames, F32 outKey[ 3 ] ) const;
	void SampleF32( F32 timeInFrames, F32& outKey ) const;
	void SampleU32( F32 timeInFrames, U32& outKey ) const;
	void SampleU16( F32 timeInFrames, U16& outKey ) const;
	void SampleVec3U16( F32 timeInFrames, U16 outKey[ 3 ] ) const;
	void SampleVec3U8( F32 timeInFrames, U8 outKey[ 3 ] ) const;
		

	bool IsQuaternionCurve() const;
	bool IsVector3Curve() const;
	bool IsScalarCurve() const;
	
private:

	void CopyInKeyTimes( const U32* keyTimes, U32 numKeys );


	
	void CalculateBounds( U32 numVectors, const F32* vectors, U32 numDimensions, F32* outMins, F32* outMaxs ) const;
	void SetScaleAndOffset( const F32 minBounds[ 3 ], const F32 maxBounds[ 3 ] );
	
	MathsStorage::Float3 m_offset;
	MathsStorage::Float3 m_scale;

	U32	m_numKeys;
	Btl::Vector< U32 > m_keyTimes;
	Btl::Vector< KeyValue > m_keyValues;
	E_CurveType m_type;
	E_KeyType m_keyType;
};

//
// Curve utility functions.
// 

bool IsQuaternionKeyType( E_KeyType keyType );
bool IsVector3KeyType( E_KeyType keyType );
bool IsScalarKeyType( E_KeyType keyType );



/// Get name of curve type.
const CHAR8* ToString( E_CurveType curveType );

/// Get name of key type.
const CHAR8* ToString( E_KeyType keyType );

Maths::C_Quaternion SampleQuaternionCurve( const C_Curve& curve, F32 timeInFrames );

Maths::HVec3 SampleVector3Curve( const C_Curve& curve, F32 timeInFrames );

F32 SampleScalarCurve( const C_Curve& curve, F32 timeInFrames );

Maths::C_Quaternion GetFirstQuaternionKey( const C_Curve& curve );

Maths::C_Quaternion GetLastQuaternionKey( const C_Curve& curve );

Maths::HVec3 GetFirstVector3Key( const C_Curve& curve );

Maths::HVec3 GetLastVector3Key( const C_Curve& curve );

F32 GetFirstScalarKey( const C_Curve& curve );

F32 GetLastScalarKey( const C_Curve& curve );

U32 GetCurveSizeInBytes( const C_Curve& curve );

U32 GetKeyTypeValueSize( E_KeyType keyType );

U32 GetCurveKeyValueSize( const C_Curve& curve );

E_KeyType GetKeyTypeFromCurveType( E_CurveType curveType );

Animation::E_VaryingCurveType GetVaryingCurveType( E_CurveType curveType );

Animation::E_ConstantCurveType GetConstantCurveType( E_CurveType curveType );

void SampleCurveAsKeyValue( const C_Curve& curve, F32 timeInFrames, KeyValue& outKeyValue );

/// Convert from an uncompressed key value to another key value type.
void ConvertKeyValue(
	E_KeyType fromKeyType,
	E_KeyType toKeyType,
	const F32* offset,
	const F32* scale,
	const KeyValue& keyValue,
	KeyValue& outKeyValue
);

/// @}

}



#endif


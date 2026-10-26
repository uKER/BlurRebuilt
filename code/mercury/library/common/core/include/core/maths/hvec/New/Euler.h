#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_EULER_H
#define SOFT_MATHS_HVEC_NEW_EULER_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_EULER_H
#define MATHS_HVEC_NEW_EULER_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Euler
/// @{

/// Make EulerXYZ from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerXYZ MakeEulerXYZ( const Mat33 MATREF m );

/// Make EulerXZY from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerXZY MakeEulerXZY( const Mat33 MATREF m );

/// Make EulerYXZ from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerYXZ MakeEulerYXZ( const Mat33 MATREF m );

/// Make EulerYZX from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerYZX MakeEulerYZX( const Mat33 MATREF m );

/// Make EulerZXY from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerZXY MakeEulerZXY( const Mat33 MATREF m );

/// Make EulerZYX from Mat33: matrix MUST already be ortho-normalised!
NOALIAS const EulerZYX MakeEulerZYX( const Mat33 MATREF m );



/// Make EulerXYZ from Vector3
NOALIAS __forceinline const EulerXYZ MakeEulerXYZ( const Vector3 PCREF v );

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerXYZ MakeEulerXYZ( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// EulerXYZ get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerXYZ PCREF e );

/// EulerXYZ Get X
NOALIAS __forceinline const VF32 X( const EulerXYZ PCREF e );

/// EulerXYZ Get Y
NOALIAS __forceinline const VF32 Y( const EulerXYZ PCREF e );

/// EulerXYZ Get Z
NOALIAS __forceinline const VF32 Z( const EulerXYZ PCREF e );


/// Make EulerXZY from Vector3
NOALIAS __forceinline const EulerXZY MakeEulerXZY( const Vector3 PCREF v );

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerXZY MakeEulerXZY( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// EulerXZY get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerXZY PCREF e );

/// EulerXZY Get X
NOALIAS __forceinline const VF32 X( const EulerXZY PCREF e );

/// EulerXZY Get Y
NOALIAS __forceinline const VF32 Y( const EulerXZY PCREF e );

/// EulerXZY Get Z               
NOALIAS __forceinline const VF32 Z( const EulerXZY PCREF e );


/// Make EulerYXZ from Vector3
NOALIAS __forceinline const EulerYXZ MakeEulerYXZ( const Vector3 PCREF v );

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerYXZ MakeEulerYXZ( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// EulerYXZ get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerYXZ PCREF e );

/// EulerYXZ Get X
NOALIAS __forceinline const VF32 X( const EulerYXZ PCREF e );

/// EulerYXZ Get Y
NOALIAS __forceinline const VF32 Y( const EulerYXZ PCREF e );

/// EulerYXZ Get Z
NOALIAS __forceinline const VF32 Z( const EulerYXZ PCREF e );


/// Make EulerYZX from Vector3
NOALIAS __forceinline const EulerYZX MakeEulerYZX( const Vector3 PCREF v );

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerYZX MakeEulerYZX( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// EulerYZX get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerYZX PCREF e );

/// EulerYZX Get X
NOALIAS __forceinline const VF32 X( const EulerYZX PCREF e );

/// EulerYZX Get Y
NOALIAS __forceinline const VF32 Y( const EulerYZX PCREF e );

/// EulerYZX Get Z
NOALIAS __forceinline const VF32 Z( const EulerYZX PCREF e );


/// Make EulerZXY from Vector3
NOALIAS __forceinline const EulerZXY MakeEulerZXY( const Vector3 PCREF v );

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerZXY MakeEulerZXY( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// EulerZXY get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerZXY PCREF e );

/// EulerZXY Get X
NOALIAS __forceinline const VF32 X( const EulerZXY PCREF e );

/// EulerZXY Get Y
NOALIAS __forceinline const VF32 Y( const EulerZXY PCREF e );

/// EulerZXY Get Z
NOALIAS __forceinline const VF32 Z( const EulerZXY PCREF e );


/// Make EulerZYX from Vector3
NOALIAS __forceinline const EulerZYX MakeEulerZYX( const Vector3 PCREF v );

/// Make EulerXYZ from three VF32's
NOALIAS __forceinline const EulerZYX MakeEulerZYX( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// EulerZYX get Vector3
NOALIAS __forceinline const Vector3 GetVector3( const EulerZYX PCREF e );

/// EulerZYX Get X
NOALIAS __forceinline const VF32 X( const EulerZYX PCREF e );

/// EulerZYX Get Y
NOALIAS __forceinline const VF32 Y( const EulerZYX PCREF e );

/// EulerZYX Get Z
NOALIAS __forceinline const VF32 Z( const EulerZYX PCREF e );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



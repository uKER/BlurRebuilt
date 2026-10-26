#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_SHEARMAT3_H
#define SOFT_MATHS_HVEC_NEW_SHEARMAT3_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_SHEARMAT3_H
#define MATHS_HVEC_NEW_SHEARMAT3_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup ShearMat3
/// @{

/// Make ShearMat3 from 3 float elements [ XY, XZ, YZ ]
NOALIAS __forceinline const ShearMat3 MakeShearMat3( const F32& xy, const F32& xz, const F32& yz );

/// Make ShearMat3 from 3 VF32 elements [ XY, XZ, YZ ]
NOALIAS __forceinline const ShearMat3 MakeShearMat3( const VF32 VECREF xy, const VF32 VECREF xz, const VF32 VECREF yz );

/// Make ShearMat3 from a shear vector [ XY, XZ, YZ ]
NOALIAS __forceinline const ShearMat3 MakeShearMat3( const Vector3 PCREF shear );

/// Get shear vector [ XY, XZ, YZ ] from a ShearMat3
NOALIAS __forceinline const Vector3 GetShear( const ShearMat3 PCREF m );

/// Set ShearMat3 from a shear vector [ XY, XZ, YZ ]
NOALIAS __forceinline void SetShear( ShearMat3* pDst, const Vector3 PCREF shear );

/// Get XY from a ShearMat3
NOALIAS __forceinline const VF32 GetXY( const ShearMat3 PCREF m );

/// Get XZ from a ShearMat3
NOALIAS __forceinline const VF32 GetXZ( const ShearMat3 PCREF m );

/// Get YZ from a ShearMat3
NOALIAS __forceinline const VF32 GetYZ( const ShearMat3 PCREF m );


/// ShearMat3 Finite test
NOALIAS __forceinline const VBool Finite( const ShearMat3 PCREF m );


/// ShearMat3 * VF32 Right Multiplication
NOALIAS __forceinline const ShearMat3 operator * ( const ShearMat3 PCREF a, const VF32 VECREF b );

/// ShearMat3 * VF32 Left Multiplication
NOALIAS __forceinline const ShearMat3 operator * ( const VF32 VECREF a, const ShearMat3 PCREF b );


/// ShearMat3 * Mat33 Right Multiplication
NOALIAS __forceinline const Mat33 operator * ( const ShearMat3 PCREF a, const Mat33 MATREF b );

/// ShearMat3 * Mat33 Left Multiplication
NOALIAS __forceinline const Mat33 operator * ( const Mat33 MATREF a, const ShearMat3 PCREF b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



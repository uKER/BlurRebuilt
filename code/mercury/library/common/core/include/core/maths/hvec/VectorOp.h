#ifndef MATHS_HVEC_VECTOR_OP_H
#define MATHS_HVEC_VECTOR_OP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//
// +-------------------------------------+
// | *** HVec3 Operators & Functions *** |
// +-------------------------------------+
//

/// HVec3 Get X
NOALIAS const HF32 X( const HVec3 PCREF v );

/// HVec3 Get Y
NOALIAS const HF32 Y( const HVec3 PCREF v );

/// HVec3 Get Z
NOALIAS const HF32 Z( const HVec3 PCREF v );

/// HVec3 Set X
NOALIAS const HVec3 SetX( const HVec3 PCREF v, const HF32 PCREF x );

/// HVec3 Set Y
NOALIAS const HVec3 SetY( const HVec3 PCREF v, const HF32 PCREF y );

/// HVec3 Set Z
NOALIAS const HVec3 SetZ( const HVec3 PCREF v, const HF32 PCREF z );

/// HVec3 Zero element X
NOALIAS const HVec3 ZeroX( const HVec3 PCREF v );

/// HVec3 Zero element Y
NOALIAS const HVec3 ZeroY( const HVec3 PCREF v );

/// HVec3 Zero element Z
NOALIAS const HVec3 ZeroZ( const HVec3 PCREF v );

/// HVec3 Zero elements X & Y
NOALIAS const HVec3 ZeroXY( const HVec3 PCREF v );

/// HVec3 Zero elements X & Z
NOALIAS const HVec3 ZeroXZ( const HVec3 PCREF v );

/// HVec3 Zero elements Y & Z
NOALIAS const HVec3 ZeroYZ( const HVec3 PCREF v );

/// HVec3 Shuffle
template < unsigned IX, unsigned IY, unsigned IZ >
NOALIAS const HVec3 Shuffle( const HVec3 PCREF v );

/// HVec3 Unary Negation Operator
NOALIAS const HVec3 operator - ( const HVec3 PCREF v );

/// HVec3 Binary Addition Operator
NOALIAS const HVec3_Add operator + ( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// HVec3 Binary Subtraction Operator
NOALIAS const HVec3_Subtract operator - ( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// HVec3 Binary Multiplication Operator
NOALIAS const HVec3_Multiply operator * ( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// HVec3 Binary Division Operator
NOALIAS const HVec3_Multiply operator / ( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// HVec3 Add Update Operator
NOALIAS void operator += ( HVec3& v0, const HVec3 PCREF v1 );

/// HVec3 Subtract Update Operator
NOALIAS void operator -= ( HVec3& v0, const HVec3 PCREF v1 );

/// HVec3 Multiply Update Operator
NOALIAS void operator *= ( HVec3& v0, const HVec3 PCREF v1 );

/// HVec3 Divide Update Operator
NOALIAS void operator /= ( HVec3& v0, const HVec3 PCREF v1 );

/// Multiply HVec3 * HF32
NOALIAS const HVec3_Multiply operator * ( const HVec3 PCREF v, const HF32 PCREF f );

/// Multiply HF32 * HVec3
NOALIAS const HVec3_Multiply operator * ( const HF32 PCREF f, const HVec3 PCREF v );

/// Multiply HVec3 * F32
NOALIAS const HVec3_Multiply operator * ( const HVec3 PCREF v, const F32& f );

/// Multiply F32 * HVec3
NOALIAS const HVec3_Multiply operator * ( const F32& f, const HVec3 PCREF v );

/// Divide HVec3 / HF32 Operator
NOALIAS const HVec3_Multiply operator / ( const HVec3 PCREF v, const HF32 PCREF f );

/// Divide HF32 / HVec3 Operator
NOALIAS const HVec3_Multiply operator / ( const HF32 PCREF f, const HVec3 PCREF v );

/// Divide HVec3 / F32 Operator
NOALIAS const HVec3_Multiply operator / ( const HVec3 PCREF v, const F32& f );

/// Divide F32 / HVec3 Operator
NOALIAS const HVec3_Multiply operator / ( const F32& f, const HVec3 PCREF v );

/// Multiply HVec3 * HF32 Update Operator
NOALIAS void operator *= ( HVec3& v, const HF32 PCREF f );

/// Divide HVec3 / HF32 Update Operator
NOALIAS void operator /= ( HVec3& v, const HF32 PCREF f );

/// Multiply HVec3 * F32 Update Operator
NOALIAS void operator *= ( HVec3& v, const F32& f );

/// Divide HVec3 / F32 Update Operator
NOALIAS void operator /= ( HVec3& v, const F32& f );

/// HVec3 Abs
NOALIAS const HVec3 Abs( const HVec3 PCREF v0 );

/// HVec3 Min
NOALIAS const HVec3 Min( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// HVec3 Max
NOALIAS const HVec3 Max( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// Reciprocal of elements
NOALIAS const HVec3 Reciprocal( const HVec3 PCREF v );

/// Horizontal sum of all elements
NOALIAS const HF32 Sum( const HVec3 PCREF v );

/// Vector Length
NOALIAS const HF32 Length( const HVec3 PCREF v );

/// Vector Length Squared
NOALIAS const HF32 LengthSquared( const HVec3 PCREF v );

/// Vector Reciprocal of Length
NOALIAS const HF32 ReciprocalLength( const HVec3 PCREF v );

/// Combine with HVecW to form a HVec4
NOALIAS const HVec4 operator || ( const HVec3 PCREF xyz, const HVecW PCREF w );

/// Dot product
NOALIAS const HF32 operator | ( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// Cross Product
NOALIAS const HVec3 operator ^ ( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// Transform: Apply rotation then translation
NOALIAS const HVec3 Transform( const HVec3 PCREF vertex, const C_Quaternion PCREF rotation, const HVec3 PCREF translation );

/// Vertex Distance
NOALIAS const HF32 Distance( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// Vertex Distance Squared
NOALIAS const HF32 DistanceSquared( const HVec3 PCREF v0, const HVec3 PCREF v1 );

/// Lerp
NOALIAS const HVec3 Lerp( const HVec3 PCREF src, const HVec3 PCREF dst, const HF32 PCREF f );
NOALIAS const C_Vector Lerp( const C_Vector PCREF src, const C_Vector PCREF dst, const HF32 PCREF f );
NOALIAS const C_Vertex Lerp( const C_Vertex PCREF src, const C_Vertex PCREF dst, const HF32 PCREF f );

/// Calculates vector length, length squared and reciprocal of length simultaneously
NOALIAS void LengthSquaredAndReciprocal( const HVec3 PCREF v, HF32& length, HF32& length_squared, HF32& reciprocal_length );

/// Calculates vector normal, length, length squared and reciprocal of length simultaneously
NOALIAS const C_Normalised NormalLengthSquaredAndReciprocal( const HVec3 PCREF v );

/// Vector Normalise
NOALIAS const C_Normal Normalise( const HVec3 PCREF v );

/// Multiplication: Normal * HF32 -> Vector
//NOALIAS __forceinline const C_Vector operator * (const C_Normal PCREF n, const HF32 PCREF f) {return C_Vector(n.m_V * f);}
//NOALIAS __forceinline const C_Vector operator * (const HF32 PCREF f, const C_Normal PCREF n) {return C_Vector(n.m_V * f);}
//NOALIAS __forceinline const C_Vector operator * (const C_Normal PCREF n, const float& f) {return C_Vector(n.m_V * Load<HF32>( &f ));}

/// Multiplication: Vector * HF32 -> Vector
//NOALIAS __forceinline const C_Vector operator * (const C_Vector PCREF n, const HF32 PCREF f) {return C_Vector(n.m_V * f);}
//NOALIAS __forceinline const C_Vector operator * (const HF32 PCREF f, const C_Vector PCREF n) {return C_Vector(n.m_V * f);}
//NOALIAS __forceinline const C_Vector operator * (const C_Vector PCREF n, const float& f) {return C_Vector(n.m_V * Load<HF32>( &f ));}

//NOALIAS __forceinline const C_Vector operator / ( const C_Vector PCREF v, const HF32 PCREF f ) { return C_Vector( v.m_V * Rcp(f) ); }

/// Division: Normal / HF32 -> Vector
//NOALIAS __forceinline const C_Vector operator / (const C_Normal PCREF n, const HF32 PCREF f) {return C_Vector(n.m_V * Rcp(f));}
//NOALIAS __forceinline const C_Vector operator / (const C_Normal PCREF n, const float& f) {return C_Vector(n.m_V * Rcp(Load<HF32>( &f )));}

//NOALIAS __forceinline const C_Vertex operator * ( const HF32 PCREF f, const C_Vertex PCREF v ) { return f * v.m_V; }
//NOALIAS __forceinline const C_Vector operator * ( const HF32 PCREF f, const C_Vector PCREF v ) { return f * v.m_V; }
//NOALIAS __forceinline const C_Vector operator * ( const HF32 PCREF f, const C_Normal PCREF v ) { return f * v.m_V; }
//NOALIAS __forceinline const C_Vector operator * ( const C_Normal PCREF v, const HF32 PCREF f ) { return f * v.m_V; }

NOALIAS __forceinline const C_Vertex operator * ( const C_Vertex PCREF v, const HF32 PCREF f ) { return C_Vertex( v.m_V * f ); }
NOALIAS __forceinline const C_Vector operator * ( const C_Vector PCREF v, const HF32 PCREF f ) { return C_Vector( v.m_V * f ); }
NOALIAS __forceinline const C_Vector operator * ( const C_Normal PCREF n, const HF32 PCREF f ) { return C_Vector( n.m_V * f ); }

NOALIAS __forceinline const C_Vertex operator * ( const HF32 PCREF f, const C_Vertex PCREF v ) { return C_Vertex( v.m_V * f ); }
NOALIAS __forceinline const C_Vector operator * ( const HF32 PCREF f, const C_Vector PCREF v ) { return C_Vector( v.m_V * f ); }
NOALIAS __forceinline const C_Vector operator * ( const HF32 PCREF f, const C_Normal PCREF n ) { return C_Vector( n.m_V * f ); }

NOALIAS __forceinline const C_Vertex operator / ( const C_Vertex PCREF v, const HF32 PCREF f ) { return C_Vertex( v.m_V * Rcp(f) ); }
NOALIAS __forceinline const C_Vector operator / ( const C_Vector PCREF v, const HF32 PCREF f ) { return C_Vector( v.m_V * Rcp(f) ); }
NOALIAS __forceinline const C_Vector operator / ( const C_Normal PCREF n, const HF32 PCREF f ) { return C_Vector( n.m_V * Rcp(f) ); }

NOALIAS __forceinline const C_Vertex operator * ( const C_Vertex PCREF v, const F32& f ) { return v * Load<HF32>( &f ); }
NOALIAS __forceinline const C_Vector operator * ( const C_Vector PCREF v, const F32& f ) { return v * Load<HF32>( &f ); }
NOALIAS __forceinline const C_Vector operator * ( const C_Normal PCREF n, const F32& f ) { return n * Load<HF32>( &f ); }

NOALIAS __forceinline const C_Vertex operator * ( const float& f, const C_Vertex PCREF v ) { return C_Vertex( v.m_V * Load<HF32>( &f ) ); }
NOALIAS __forceinline const C_Vector operator * ( const float& f, const C_Vector PCREF v ) { return C_Vector( v.m_V * Load<HF32>( &f ) ); }
NOALIAS __forceinline const C_Vector operator * ( const float& f, const C_Normal PCREF n ) { return C_Vector( n.m_V * Load<HF32>( &f ) ); }

NOALIAS __forceinline const C_Vertex operator / ( const C_Vertex PCREF v, const F32& f ) { return v * Reciprocal( Load<HF32>( &f ) ); }
NOALIAS __forceinline const C_Vector operator / ( const C_Vector PCREF v, const F32& f ) { return v * Reciprocal( Load<HF32>( &f ) ); }
NOALIAS __forceinline const C_Vector operator / ( const C_Normal PCREF n, const F32& f ) { return n * Reciprocal( Load<HF32>( &f ) ); }


/// HVec3_Multiply Abs
NOALIAS const HVec3 Abs( const HVec3_Multiply PCREF v0 );

/// HVec3_Add Abs
NOALIAS const HVec3 Abs( const HVec3_Add PCREF v0 );

/// HVec3_Subtract Abs
NOALIAS const HVec3 Abs( const HVec3_Subtract PCREF v0 );








/// C_Vertex Abs
NOALIAS const C_Vertex Abs( const C_Vertex PCREF v );

/// C_Vertex Sum
NOALIAS const HF32 Sum( const C_Vertex PCREF v );


/// C_Vector Abs
NOALIAS const C_Vector Abs( const C_Vector PCREF v );

/// C_Vector Sum
NOALIAS const HF32 Sum( const C_Vector PCREF v );




/// Calculate distances from vertex to four planes - TODO: Move to GeometryOp.h
NOALIAS void DistanceToPlanes( const HVec3 PCREF v,
                               const C_Plane PCREF p0, const C_Plane PCREF p1, const C_Plane PCREF p2, const C_Plane PCREF p3, 
                               HF32&               d0, HF32&               d1, HF32&               d2, HF32&               d3 );


}; // namespace Maths

#endif


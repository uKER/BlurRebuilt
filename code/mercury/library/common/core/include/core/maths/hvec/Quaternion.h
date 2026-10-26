#ifndef QUATERNION_H
#define QUATERNION_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{
//
// ------------------------------------------------
// |-- C_Quaternion : Quaternion rotation class --|
// ------------------------------------------------
//

NOALIAS __forceinline const HVec4 InitQuaternion( const HMat33& matrix );

/// C_Quaternion : Quaternion rotation class
class C_Quaternion
{
public:
    
    /// New Maths Quaternion Constructor
    NOALIAS __forceinline /*explicit*/ C_Quaternion( const Quaternion& q );
    
    /// New Maths Quaternion Cast
    NOALIAS __forceinline operator const Quaternion() const __restrict;
    
    /// Default Constructor
    NOALIAS __forceinline C_Quaternion();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_Quaternion( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Quaternion( const C_Quaternion& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Quaternion& operator = ( const C_Quaternion& copy );
    
    /// Explicit V128 Constructor
    NOALIAS __forceinline explicit C_Quaternion( const V128 PCREF src );
    
    /// Explicit HVec4 Constructor
    NOALIAS __forceinline explicit C_Quaternion( const HVec4 PCREF src );
    
    /// HVec3 Axis + HF32 Angle Constructor
    NOALIAS __forceinline C_Quaternion( const HVec3 PCREF axis, const HF32 PCREF theta );
    
    /// HF32 Imag XYZ + Real Constructor ( NOTE: NOT axis + angle! )
    NOALIAS __forceinline C_Quaternion( const HF32 PCREF imag_x, const HF32 PCREF imag_y, const HF32 PCREF imag_z, const HF32 PCREF real );
    
    /// HVec3 Imag XYZ + HVecW Real Constructor ( NOTE: NOT axis + angle! )
    NOALIAS __forceinline C_Quaternion( const HVec3 PCREF imag, const HVecW PCREF real );
    
    /// Explicit HVec3 Imaginary part Constructor
    NOALIAS __forceinline explicit C_Quaternion( const HVec3 PCREF imag );
    
    /// Explicit Real part Constructor
    NOALIAS __forceinline explicit C_Quaternion( const HVecW PCREF real );
    
    /// Explicit HMat33 Rotation Matrix Constructor
    NOALIAS __forceinline explicit C_Quaternion( const HMat33& matrix );
    
    /// Cast to HVec4
    NOALIAS __forceinline operator const HVec4() const __restrict;
    
    /// Cast to HMat33 Rotation Matrix - REPLACE with HMat33 Constructor from Quaternion?
    NOALIAS __forceinline operator const HMat33 () const __restrict;

	template < int T_Precision >
	NOALIAS __forceinline explicit C_Quaternion( const MathsStorage::Fixed4_S16<T_Precision>* const src );
	
	template < int T_Precision >
	NOALIAS __forceinline operator const MathsStorage::Fixed4_S16<T_Precision> () const __restrict;
      
    /// Read from Stream
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;
  
    //
    // *** OLD Legacy member functions - use equivalent non-member friend functions instead! ***
    //
    
    /// Returns the Imaginary part as a HVec3
    SYS_DEPRECATED NOALIAS __forceinline const HVec3           Imag() const __restrict;
    
    /// Returns the Real part as a HVecW
    SYS_DEPRECATED NOALIAS __forceinline const HVecW           Real() const __restrict;
    
    /// Cos( theta * 0.5 ) : theta == rotation angle in radians
    SYS_DEPRECATED NOALIAS __forceinline const HF32            CosHalfTheta() const __restrict;
    
    /// Sin( theta * 0.5 ) : theta == rotation angle in radians
    SYS_DEPRECATED NOALIAS __forceinline const HF32            SinHalfTheta() const __restrict;
    
    /// 1.0 / Sin( theta * 0.5 ) : theta == rotation angle in radians
    SYS_DEPRECATED NOALIAS __forceinline const HF32            ReciprocalSinHalfTheta() const __restrict;
    
    /// Cos( theta ) : theta == rotation angle in radians
    SYS_DEPRECATED NOALIAS __forceinline const HF32            CosTheta() const __restrict;
    
    /// Length
    SYS_DEPRECATED NOALIAS __forceinline const HF32            Length() const __restrict;
    
    /// Shortest Division: num / denom = num * !denom - negates q2 if necessary
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    ShortestDivide( const C_Quaternion PCREF denom ) const __restrict;

    /// Normalise this Quaternion
    SYS_DEPRECATED NOALIAS __forceinline void                  Normalise() __restrict;
    
    /// Returns normalised (unit length) copy of this Quaternion
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    Normalised() const __restrict;
    
    /// Lerp: Linear Interpolation
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    Lerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Lerp: Linear Interpolation with normalisation
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    LerpNormalised( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    ShortestLerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary - with normalisation
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    ShortestLerpNormalised( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Slerp: Spherical Linear Interpolation
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    Slerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Shortest Slerp: Spherical Linear Interpolation - Negates dst if necessary
    SYS_DEPRECATED NOALIAS __forceinline const C_Quaternion    ShortestSlerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Finite Test
    SYS_DEPRECATED NOALIAS __forceinline const Scalar_Bool     Finite() const __restrict;
    
    #ifdef SHOOTER_VECMAT_DEBUG_PRINT
    void                            DbgPrint( char* text = "" ) const;
    #endif

    #if defined( _VMX128_ )
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) { m_V = src; }
    NOALIAS __forceinline const V128            GetV128() const                 { return m_V; }
    #else
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) { m_V.SetV128( src ); }
    NOALIAS __forceinline const V128            GetV128() const                 { return m_V.GetV128(); }
    #endif
    
//private:
    
    // The quaternion is stored with the real(scalar) part in the W component and
    // the imaginary(vector) part in the X, Y & Z components.
    
    #if defined( _VMX128_ )
    V128                        m_V;
    #else
    HVec4                       m_V;
    #endif
};


/// C_QuatTrans : Quaternion rotation & translation class
class C_QuatTrans
{
public:
    
    NOALIAS __forceinline C_QuatTrans( const QuatTrans& qt )
		: m_Quaternion( qt.m_Q )
		, m_Translation( qt.m_T )
    {
    }
    
    NOALIAS __forceinline const C_QuatTrans& operator = ( const QuatTrans& qt )
    {
		m_Quaternion = qt.m_Q;
		m_Translation = HVec3( qt.m_T );
		return *this;
    }
    
    NOALIAS __forceinline operator QuatTrans ()
    {
		QuatTrans qt;
		qt.m_Q = m_Quaternion;
		qt.m_T = m_Translation;
		return qt;
    }
    
    /// Default Constructor
    NOALIAS __forceinline C_QuatTrans();
    
    /// Copy Constructor
    NOALIAS __forceinline C_QuatTrans( const C_QuatTrans& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_QuatTrans& operator = ( const C_QuatTrans& copy );
    
    /// Quaternion & HVec3 Constructor
    NOALIAS __forceinline C_QuatTrans( const C_Quaternion PCREF q, const HVec3 PCREF t );
    
    /// HMat43 Constructor
    NOALIAS __forceinline C_QuatTrans( const HMat43& matrix );
    
    /// Accessor: Get Quaternion
    NOALIAS __forceinline const C_Quaternion    Quaternion() const __restrict;
    
    /// Accessor: Get Translation
    NOALIAS __forceinline const HVec3           Translation() const __restrict;
    
    /// Accessor: Set Quaternion
    NOALIAS __forceinline void                  Quaternion( const C_Quaternion PCREF q ) __restrict;
    
    /// Accessor: Set Translation
    NOALIAS __forceinline void                  Translation( const HVec3 PCREF t ) __restrict;
    
    /// Accessor: Convert to HMat43
    NOALIAS __forceinline const HMat43          Matrix() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    C_Quaternion            m_Quaternion;
    HVec3                   m_Translation;
};




#if 1

//
// *** NEW Non-member friend functions to replace old member functions for compatibility with UtMaths ***
//

/// Returns the Imaginary part as a HVec3
NOALIAS __forceinline const HVec3              Imag( const C_Quaternion PCREF src );

/// Returns the Real part as a HVecW
NOALIAS __forceinline const HVecW              Real( const C_Quaternion PCREF src );

/// Cos( theta * 0.5 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32               CosHalfTheta( const C_Quaternion PCREF src );

/// Sin( theta * 0.5 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32               SinHalfTheta( const C_Quaternion PCREF src );

/// 1.0 / Sin( theta * 0.5 ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32               ReciprocalSinHalfTheta( const C_Quaternion PCREF src );

/// Cos( theta ) : theta == rotation angle in radians
NOALIAS __forceinline const HF32               CosTheta( const C_Quaternion PCREF src );

/// Length
NOALIAS __forceinline const HF32               Length( const C_Quaternion PCREF src );

/// Addition
NOALIAS __forceinline const C_Quaternion       operator + ( const C_Quaternion PCREF src0, const C_Quaternion PCREF src1 );

/// Subtraction
NOALIAS __forceinline const C_Quaternion       operator - ( const C_Quaternion PCREF src0, const C_Quaternion PCREF src1 );

/// Unary Negation
NOALIAS __forceinline const C_Quaternion       operator - ( const C_Quaternion PCREF src );

/// Conjugate (Multiplicative Inverse)
NOALIAS __forceinline const C_Quaternion       operator ! ( const C_Quaternion PCREF src );

/// Scalar multiplication
NOALIAS __forceinline const C_Quaternion       operator * ( const C_Quaternion PCREF src, const HF32 PCREF f );

/// Division: num / denom = num * !denom
NOALIAS __forceinline const C_Quaternion       operator / ( const C_Quaternion PCREF num, const C_Quaternion PCREF denom );

/// Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const C_Quaternion       ShortestDivide( const C_Quaternion PCREF num, const C_Quaternion PCREF denom );


/// C_Quaternion Left Division: denom \ num = !denom * num
NOALIAS __forceinline const C_Quaternion    LeftDivide( const C_Quaternion PCREF denom, const C_Quaternion PCREF num );

/// C_Quaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const C_Quaternion    RightDivide( const C_Quaternion PCREF num, const C_Quaternion PCREF denom );

/// C_Quaternion Nearest Left Division: denom \ num = !denom * num  - negates denom if necessary
NOALIAS __forceinline const C_Quaternion    NearestLeftDivide( const C_Quaternion PCREF denom, const C_Quaternion PCREF num );

/// C_Quaternion Nearest Right Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const C_Quaternion    NearestRightDivide( const C_Quaternion PCREF num, const C_Quaternion PCREF denom );


/// C_Quaternion Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const HVec3   LeftDifferential( const C_Quaternion PCREF denom, const C_Quaternion PCREF num );

/// C_Quaternion Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const HVec3   RightDifferential( const C_Quaternion PCREF num, const C_Quaternion PCREF denom );

/// C_Quaternion Nearest Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const HVec3   NearestLeftDifferential( const C_Quaternion PCREF denom, const C_Quaternion PCREF num );

/// C_Quaternion Nearest Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const HVec3   NearestRightDifferential( const C_Quaternion PCREF num, const C_Quaternion PCREF denom );


/// Dot Product: Returns 4 element dot product == cosine( angle / 2 ) between the 2 quaternions!
NOALIAS __forceinline const HF32               operator | ( const C_Quaternion PCREF src0, const C_Quaternion PCREF src1 );

/// Returns normalised (unit length) copy of this Quaternion
NOALIAS __forceinline const C_Quaternion       Normalised( const C_Quaternion PCREF src );

/// Lerp: Linear Interpolation
NOALIAS __forceinline const C_Quaternion       Lerp( const C_Quaternion PCREF src, const C_Quaternion PCREF dst, const HF32 PCREF t );

/// Lerp: Linear Interpolation with normalisation
NOALIAS __forceinline const C_Quaternion       LerpNormalised( const C_Quaternion PCREF src, const C_Quaternion PCREF dst, const HF32 PCREF t );

/// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const C_Quaternion       ShortestLerp( const C_Quaternion PCREF src, const C_Quaternion PCREF dst, const HF32 PCREF t );

/// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary - with normalisation
NOALIAS __forceinline const C_Quaternion       ShortestLerpNormalised( const C_Quaternion PCREF src, const C_Quaternion PCREF dst, const HF32 PCREF t );

/// Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const C_Quaternion       Slerp( const C_Quaternion PCREF src, const C_Quaternion PCREF dst, const HF32 PCREF t );

/// Shortest Slerp: Spherical Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const C_Quaternion       ShortestSlerp( const C_Quaternion PCREF src, const C_Quaternion PCREF dst, const HF32 PCREF t );

/// Finite Test
NOALIAS __forceinline const Scalar_Bool        Finite( const C_Quaternion PCREF src );

#endif



/// Nearest: Returns whichever one of the two equivalent targets is nearest to the source
NOALIAS __forceinline const C_Quaternion        Nearest( const C_Quaternion PCREF source, const C_Quaternion PCREF target );

/// Nearest: Returns whichever one of the two equivalent targets is nearest to the source
NOALIAS __forceinline const C_QuatTrans         Nearest( const C_QuatTrans& source, const C_QuatTrans& target );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const C_Quaternion        Select( const Vector4_Bool PCREF selector, const C_Quaternion PCREF resultIfTrue, const C_Quaternion PCREF resultIfFalse );



/// Multiplication Operator: C_Quaternion * C_Quaternion --> C_Quaternion
NOALIAS __forceinline const C_Quaternion        operator * ( const C_Quaternion PCREF q1, const C_Quaternion PCREF q2 );

/// Multiplication Operator: HVec3 * C_Quaternion --> HVec3
NOALIAS __forceinline const HVec3               operator * ( const HVec3 PCREF v, const C_Quaternion PCREF q );

/// Multiplication Operator: Normal * C_Quaternion --> Normal
NOALIAS __forceinline const C_Normal            operator * ( const C_Normal PCREF n, const C_Quaternion PCREF q );

/// Multiplication Operator: C_Vector * C_Quaternion --> C_Vector
NOALIAS __forceinline const C_Vector            operator * ( const C_Vector PCREF v, const C_Quaternion PCREF q );

/// Multiplication Operator: C_Quaternion * F32 --> C_Quaternion
NOALIAS __forceinline const C_Quaternion        operator * ( const C_Quaternion PCREF q, const F32& f );

/// Multiplication Update Operator: C_Quaternion *= F32
NOALIAS __forceinline void                      operator *= ( C_Quaternion& q, const F32& f );

/// Division Operator: C_Quaternion / F32 --> C_Quaternion
NOALIAS __forceinline const C_Quaternion        operator / ( const C_Quaternion PCREF q, const F32& f );

/// Division Update Operator: C_Quaternion /= F32
NOALIAS __forceinline void                      operator /= ( C_Quaternion& q, const F32& f );



/// Multiplication Operator: C_QuatTrans * C_QuatTrans --> C_QuatTrans
NOALIAS __forceinline const C_QuatTrans         operator * ( const C_QuatTrans& qt1, const C_QuatTrans& qt2 );

/// Multiplication Operator: C_Vertex * C_QuatTrans --> C_Vertex
NOALIAS __forceinline const C_Vertex            operator * ( const C_Vertex PCREF v, const C_QuatTrans& qt );

/// Inverse: C_QuatTrans
NOALIAS __forceinline const C_QuatTrans         Inverse( const C_QuatTrans& qt );



}; // namespace Maths


#endif

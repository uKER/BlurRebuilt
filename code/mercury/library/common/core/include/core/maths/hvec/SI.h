#ifndef SI_H
#define SI_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

template < int kg, int m, int s >
class KgMS
{
public:
    
    static const int Kg = kg;
    static const int M  = m;
    static const int S  = s;
};

template < class LHS >
struct SI_Squared
{
    typedef KgMS< LHS::Kg * 2, LHS::M * 2, LHS::S * 2 > Result;
};

template < class LHS >
struct SI_SquareRoot
{
    static const bool ValidateKg = CompileTimeError< !( LHS::Kg & 1 ) >::Result;
    static const bool ValidateM  = CompileTimeError< !( LHS::M & 1 ) >::Result;
    static const bool ValidateS  = CompileTimeError< !( LHS::S & 1 ) >::Result;
    
    typedef KgMS< LHS::Kg / 2, LHS::M / 2, LHS::S / 2 > Result;
};

template < class LHS >
struct SI_ReciprocalSquareRoot
{
    static const bool ValidateKg = CompileTimeError< !( LHS::Kg & 1 ) >::Result;
    static const bool ValidateM  = CompileTimeError< !( LHS::M & 1 ) >::Result;
    static const bool ValidateS  = CompileTimeError< !( LHS::S & 1 ) >::Result;
    
    typedef KgMS< -LHS::Kg / 2, -LHS::M / 2, -LHS::S / 2 > Result;
};

template < class LHS >
struct SI_Inverse
{
    typedef KgMS< -LHS::Kg, -LHS::M, -LHS::S > Result;
};

template < class LHS, class RHS >
struct SI_Product
{
    typedef KgMS< LHS::Kg + RHS::Kg, LHS::M + RHS::M, LHS::S + RHS::S > Result;
};

template < class LHS, class RHS >
struct SI_Quotient
{
    typedef KgMS< LHS::Kg - RHS::Kg, LHS::M - RHS::M, LHS::S - RHS::S > Result;
};


typedef KgMS<0,0,0>     SI_None;        // Unitless scalar value ( e.g. radians )

typedef KgMS<1,0,0>     SI_Kg;          // Mass                             ( Kilograms )
typedef KgMS<-1,0,0>    SI_pKg;         // Reciprocal Mass                  ( per Kilograms )
typedef KgMS<0,1,0>     SI_M;           // Distance                         ( Metres )
typedef KgMS<0,-1,0>    SI_pM;          // Reciprocal Distance              ( per Metre )
typedef KgMS<0,0,1>     SI_S;           // Time                             ( Seconds )
typedef KgMS<0,0,-1>    SI_pS;          // Reciprocal Time                  ( per Seconds )
typedef KgMS<0,0,-2>    SI_pS2;         // Reciprocal Time Squared          ( per Seconds squared )

typedef KgMS<0,2,0>     SI_M2;          // Area                             ( Metres squared )
typedef KgMS<0,3,0>     SI_M3;          // Volume                           ( Metres cubed )
typedef KgMS<0,1,-1>    SI_MpS;         // Velocity                         ( Metres per Second )
typedef KgMS<0,1,-2>    SI_MpS2;        // Acceleration                     ( Metres per Second squared )
typedef KgMS<1,1,-1>    SI_KgMpS;       // Momentum / Impulse               ( Kilograms Metres per Second )
typedef KgMS<1,-3,0>    SI_KgpM3;       // Density                          ( Kilograms per Metre cubed )
typedef KgMS<1,2,0>     SI_KgM2;        // Inertia Tensor                   ( Kilograms Metres squared )
typedef KgMS<-1,-2,0>   SI_pKg_pM2;     // Inverse Inertia Tensor           ( per Kilograms per Metres squared )
typedef KgMS<-1,2,0>    SI_pKg_M2;      // ?                                ( Metres squared per Kilogram )

typedef KgMS<0,0,0>     SI_Rad;         // Angle in Radians                 ( Metres per Metre == unitless )
typedef KgMS<0,0,-1>    SI_RpS;         // Angular Velocity                 ( Radians per Second )
typedef KgMS<0,0,-2>    SI_RpS2;        // Angular Acceleration             ( Radians per Second squared )
typedef KgMS<1,2,-1>    SI_KgM2RpS;     // Angular Momentum                 ( Kilograms Metres squared Radians per Second )
typedef KgMS<1,2,-1>    SI_KgM2pS;      // Impulse Torque                   ( Kilograms Metres squared per Second )
typedef KgMS<1,2,-2>    SI_KgM2pS2;     // Torque                           ( Kilograms Metres squared per Second squared )
typedef KgMS<0,0,-1>    SI_Hz;          // Frequency in Hertz               ( per Second )
typedef KgMS<1,1,-2>    SI_N;           // Force in Newtons                 ( Kilograms Metres per Second squared )
typedef KgMS<-1,-1,2>   SI_pN;          // Reciprocal of Force in Newtons   ( Kilograms Metres per Second squared )
typedef KgMS<2,2,-4>    SI_N2;          // Force in Newtons squared         ( Kilograms Metres per Second squared ^ 2 )
typedef KgMS<1,0,-2>    SI_NpM;         // Spring Constant                  ( Kilograms per Second squared : Newtons per Metre )
typedef KgMS<1,0,-1>    SI_NSpM;        // Linear Spring Damping            ( Kilograms per Second : Newton Seconds per Metre )
typedef KgMS<1,1,-1>    SI_NS;          // Angular Spring Damping           ( Kilograms Metres per Second : Newton Seconds )
typedef KgMS<1,2,-2>    SI_J;           // Energy in Joules                 ( Kilograms Metres squared per Second squared )
typedef KgMS<1,2,-3>    SI_W;           // Power in Watts                   ( Kilograms Metres squared per Second cubed )
typedef KgMS<1,-1,-2>   SI_Pa;          // Pressure in Pascals              ( Kilograms per Metre per Second squared )



template < class LHS > class C_SiScalar;
template < class LHS > class C_SiVertex;
template < class LHS > class C_SiVector;
template < class LHS > class C_SiQuaternion;
template < class LHS, bool LHS_OrthoNormalised > class C_SiRotMatrix;




/// LHS == Unit type of this
///     == Unit type of LEFT HAND SIDE for binary operators
///     == Unit type of RIGHT HAND SIDE for binary operators which take two identical unit types
/// RHS == Unit type of RIGHT HAND SIDE for binary operators which take second param of other unit types
template < class LHS >
class C_SiScalar
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_SiScalar();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_SiScalar( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_SiScalar( const C_SiScalar<LHS>& copy );
    
    /// F32 Constructor
    NOALIAS __forceinline C_SiScalar( const F32& f );
    
    /// HF32 Constructor
    NOALIAS __forceinline C_SiScalar( const HF32 PCREF f );
    
    /// Copy Assignment
    NOALIAS __forceinline void                        operator = ( const C_SiScalar<LHS>& copy );
    
    /// F32 Assignment
    NOALIAS __forceinline void                        operator = ( const F32& f ) __restrict;
    
    /// HF32 Assignment
    NOALIAS __forceinline void                        operator = ( const HF32 PCREF f ) __restrict;
    
    /// Get HF32: There is no cast operator to HF32 since this would allow the implicit casting away of the unit type!
    NOALIAS __forceinline const HF32                  Get() const __restrict;
    
    /// Abs
    NOALIAS __forceinline const C_SiScalar<LHS>       Abs() const __restrict;
    
    /// Min
    NOALIAS __forceinline const C_SiScalar<LHS>       Min( const C_SiScalar<LHS> PCREF src ) const __restrict;
    
    /// Max
    NOALIAS __forceinline const C_SiScalar<LHS>       Max( const C_SiScalar<LHS> PCREF src ) const __restrict;
    
    /// Unary Minus
    NOALIAS __forceinline const C_SiScalar<LHS>       operator - () const __restrict;
    
    /// Binary Addition Operator
    NOALIAS __forceinline const C_SiScalar<LHS>       operator + ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    
    /// Binary Subtraction Operator
    NOALIAS __forceinline const C_SiScalar<LHS>       operator - ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    
    /// Binary Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiScalar< typename SI_Product< LHS, RHS >::Result >     operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Binary Division Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiScalar< typename SI_Quotient< LHS, RHS >::Result >    operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Binary Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result >     operator * ( const C_SiVector<RHS> PCREF rhs ) const __restrict;
    
    /// Reciprocal
    NOALIAS __forceinline const C_SiScalar< typename SI_Inverse< LHS >::Result > Reciprocal() const __restrict;
    
    /// Update operators
    NOALIAS __forceinline void                        operator += ( const C_SiScalar<LHS> PCREF rhs ) __restrict;
    NOALIAS __forceinline void                        operator -= ( const C_SiScalar<LHS> PCREF rhs ) __restrict;
    NOALIAS __forceinline void                        operator *= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict;
    NOALIAS __forceinline void                        operator /= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict;
    
    /// Comparison Operators
    NOALIAS __forceinline const S32                   operator <  ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator >  ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator <= ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator >= ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator == ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator != ( const C_SiScalar<LHS> PCREF rhs ) const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    static const int            Kg = LHS::Kg;
    static const int            M  = LHS::M;
    static const int            S  = LHS::S;
    
public:
    
    HF32                        m_Scalar;
};          

template < class LHS > NOALIAS __forceinline const C_SiScalar< typename SI_SquareRoot<LHS>::Result > Sqrt( const C_SiScalar<LHS> PCREF src );

template < class LHS > NOALIAS __forceinline const C_SiScalar< typename SI_ReciprocalSquareRoot<LHS>::Result > ReciprocalSqrt( const C_SiScalar<LHS> PCREF src );


/// Promote HF32's to C_SiScalar<>'s

template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator + ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator - ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator * ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator / ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );

template < class LHS > NOALIAS __forceinline const S32                operator <  ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const S32                operator >  ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const S32                operator <= ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const S32                operator >= ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const S32                operator == ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );
template < class LHS > NOALIAS __forceinline const S32                operator != ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs );

template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator + ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator - ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator * ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator / ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );

template < class RHS > NOALIAS __forceinline const S32                operator <  ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator >  ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator <= ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator >= ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator == ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator != ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs );

template < class RHS > NOALIAS __forceinline void                     operator += ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline void                     operator -= ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline void                     operator *= ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline void                     operator /= ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs );

/// Promote F32's to C_SiScalar<>'s

template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator + ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator - ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator * ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator / ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );

template < class LHS > NOALIAS __forceinline const S32                operator <  ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const S32                operator >  ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const S32                operator <= ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const S32                operator >= ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const S32                operator == ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );
template < class LHS > NOALIAS __forceinline const S32                operator != ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs );

template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator + ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator - ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator * ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator / ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );

template < class RHS > NOALIAS __forceinline const S32                operator <  ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator >  ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator <= ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator >= ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator == ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline const S32                operator != ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs );

template < class RHS > NOALIAS __forceinline void                     operator += ( F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline void                     operator -= ( F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline void                     operator *= ( F32& lhs, const C_SiScalar<RHS> PCREF rhs );
template < class RHS > NOALIAS __forceinline void                     operator /= ( F32& lhs, const C_SiScalar<RHS> PCREF rhs );

/// Promote C_Vector to C_SiVector< SI_None >

template < class LHS > NOALIAS __forceinline const C_SiVector<LHS>    operator * ( const C_SiScalar<LHS> PCREF lhs, const C_Vector PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiVector<RHS>    operator * ( const C_Vector PCREF lhs, const C_SiScalar<RHS> PCREF rhs );

template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator | ( const C_SiVector<LHS> PCREF lhs, const C_Vector PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator | ( const C_Vector PCREF lhs, const C_SiVector<RHS> PCREF rhs );

template < class LHS > NOALIAS __forceinline const C_SiVector<LHS>    operator ^ ( const C_SiVector<LHS> PCREF lhs, const C_Vector PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiVector<RHS>    operator ^ ( const C_Vector PCREF lhs, const C_SiVector<RHS> PCREF rhs );

/// Promote C_Normal to C_SiVector< SI_None >

template < class LHS > NOALIAS __forceinline const C_SiVector<LHS>    operator * ( const C_SiScalar<LHS> PCREF lhs, const C_Normal PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiVector<RHS>    operator * ( const C_Normal PCREF lhs, const C_SiScalar<RHS> PCREF rhs );

template < class LHS > NOALIAS __forceinline const C_SiScalar<LHS>    operator | ( const C_SiVector<LHS> PCREF lhs, const C_Normal PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiScalar<RHS>    operator | ( const C_Normal PCREF lhs, const C_SiVector<RHS> PCREF rhs );

template < class LHS > NOALIAS __forceinline const C_SiVector<LHS>    operator ^ ( const C_SiVector<LHS> PCREF lhs, const C_Normal PCREF rhs );
template < class RHS > NOALIAS __forceinline const C_SiVector<RHS>    operator ^ ( const C_Normal PCREF lhs, const C_SiVector<RHS> PCREF rhs );



/// LHS == Unit type of this
///     == Unit type of LEFT HAND SIDE for binary operators
///     == Unit type of RIGHT HAND SIDE for binary operators which take two identical unit types
/// RHS == Unit type of RIGHT HAND SIDE for binary operators which take second param of other unit types
template < class LHS >
class C_SiVertex
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_SiVertex();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_SiVertex( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_SiVertex( const C_SiVertex<LHS>& copy );
    
    /// HF32 XYZ Constructor
    NOALIAS __forceinline C_SiVertex( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z );
    
    /// C_SiScalar<LHS> XYZ Constructor
    NOALIAS __forceinline C_SiVertex( const C_SiScalar<LHS> PCREF x, const C_SiScalar<LHS> PCREF y, const C_SiScalar<LHS> PCREF z );
    
    /// HVec3 Constructor
    NOALIAS __forceinline C_SiVertex( const HVec3 PCREF vec );
    
    /// C_Vertex Constructor
    NOALIAS __forceinline C_SiVertex( const C_Vertex PCREF vertex );
    
    /// HVec3_Multiply Constructor
    NOALIAS __forceinline C_SiVertex( const HVec3_Multiply& src );
    
    /// HVec3_Add Constructor
    NOALIAS __forceinline C_SiVertex( const HVec3_Add& src );
    
    /// HVec3_Subtract Constructor
    NOALIAS __forceinline C_SiVertex( const HVec3_Subtract& src );
    
    /// Copy Assignment
    NOALIAS __forceinline void                        operator = ( const C_SiVertex<LHS>& copy );
    
    /// Get C_Vertex: There is no cast operator to C_Vertex since this would allow the implicit casting away of the unit type!
    NOALIAS __forceinline const C_Vertex              Get() const __restrict;
    
    /// Element Accessors
    NOALIAS __forceinline const C_SiScalar<LHS>       X() const __restrict;
    NOALIAS __forceinline const C_SiScalar<LHS>       Y() const __restrict;
    NOALIAS __forceinline const C_SiScalar<LHS>       Z() const __restrict;
    
    /// Element Zeroing Functions
    NOALIAS __forceinline void                        ZeroX() __restrict;
    NOALIAS __forceinline void                        ZeroY() __restrict;
    NOALIAS __forceinline void                        ZeroZ() __restrict;
    NOALIAS __forceinline void                        ZeroXY() __restrict;
    NOALIAS __forceinline void                        ZeroXZ() __restrict;
    NOALIAS __forceinline void                        ZeroYZ() __restrict;
    
    /// Zero the vertex if any element exceeds +/- bounds
    NOALIAS __forceinline void                        ZeroIfOutOfBounds( const Maths::HF32 PCREF bounds ) __restrict;
    
    /// Element Zeroing Functions
    NOALIAS __forceinline const C_SiVertex<LHS>       ZeroedX() const __restrict;
    NOALIAS __forceinline const C_SiVertex<LHS>       ZeroedY() const __restrict;
    NOALIAS __forceinline const C_SiVertex<LHS>       ZeroedZ() const __restrict;
    NOALIAS __forceinline const C_SiVertex<LHS>       ZeroedXY() const __restrict;
    NOALIAS __forceinline const C_SiVertex<LHS>       ZeroedXZ() const __restrict;
    NOALIAS __forceinline const C_SiVertex<LHS>       ZeroedYZ() const __restrict;
    
    /// Lerp
    NOALIAS __forceinline const C_SiVertex<LHS>       Lerp( const C_SiVertex<LHS> PCREF rhs, const HF32 PCREF f ) const __restrict;
    
    /// Distance
    NOALIAS __forceinline const C_SiScalar<LHS>       Distance( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    
    /// Vertex Addition Operator
    NOALIAS __forceinline const C_SiVertex<LHS>       operator + ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    
    /// Vertex Subtraction Operator
    NOALIAS __forceinline const C_SiVector<LHS>       operator - ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    
    /// Add Update Operator
    NOALIAS __forceinline void                        operator += ( const C_SiVertex<LHS> PCREF rhs ) __restrict;
    
    /// Subtract Update Operator
    NOALIAS __forceinline void                        operator -= ( const C_SiVertex<LHS> PCREF rhs ) __restrict;
    
    /// Vertex + Vector Addition Operator
    NOALIAS __forceinline const C_SiVertex<LHS>       operator + ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    
    /// Vertex - Vector Subtraction Operator
    NOALIAS __forceinline const C_SiVertex<LHS>       operator - ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    
    /// Vertex += Vector Addition Operator
    NOALIAS __forceinline void                        operator += ( const C_SiVector<LHS> PCREF rhs ) __restrict;
    
    /// Vertex -= Vector Subtraction Operator
    NOALIAS __forceinline void                        operator -= ( const C_SiVector<LHS> PCREF rhs ) __restrict;
    
    /// Binary Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVertex< typename SI_Product< LHS, RHS >::Result >     operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Binary Division Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVertex< typename SI_Quotient< LHS, RHS >::Result >    operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Scalar HF32 Multiplication Operator
    NOALIAS __forceinline const C_SiVertex<LHS>       operator * ( const HF32 PCREF rhs ) const __restrict;
    
    /// Scalar HF32 Division Operator
    NOALIAS __forceinline const C_SiVertex<LHS>       operator / ( const HF32 PCREF rhs ) const __restrict;
    
    /// Comparison Operators
    NOALIAS __forceinline const S32                   operator <  ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator >  ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator <= ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator >= ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator == ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator != ( const C_SiVertex<LHS> PCREF rhs ) const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    static const int            Kg = LHS::Kg;
    static const int            M  = LHS::M;
    static const int            S  = LHS::S;
    
//protected:
public:
    
    HVec3                       m_Vertex;
};


template < typename LHS > NOALIAS __forceinline C_SiVertex<LHS>   operator * ( const C_SiVertex<LHS> PCREF v, const F32& f );
template < typename LHS > NOALIAS __forceinline C_SiVertex<LHS>   operator / ( const C_SiVertex<LHS> PCREF v, const F32& f );

template < typename LHS > NOALIAS __forceinline void              operator *= ( C_SiVertex<LHS>& v, const F32& f );
template < typename LHS > NOALIAS __forceinline void              operator /= ( C_SiVertex<LHS>& v, const F32& f );






/// LHS == Unit type of this
///     == Unit type of LEFT HAND SIDE for binary operators
///     == Unit type of RIGHT HAND SIDE for binary operators which take two identical unit types
/// RHS == Unit type of RIGHT HAND SIDE for binary operators which take second param of other unit types
template < class LHS >
class C_SiVector
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_SiVector();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_SiVector( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_SiVector( const C_SiVector<LHS>& copy );
    
    /// C_SiScalar<LHS> XYZ Constructor
    NOALIAS __forceinline C_SiVector( const C_SiScalar<LHS> PCREF x, const C_SiScalar<LHS> PCREF y, const C_SiScalar<LHS> PCREF z );
    
    /// HVec3 Constructor
    NOALIAS __forceinline C_SiVector( const HVec3 PCREF vec );
    
    /// C_Vector Constructor
    NOALIAS __forceinline C_SiVector( const C_Vector PCREF vector );
    
    /// HVec3_Multiply Constructor
    NOALIAS __forceinline C_SiVector( const HVec3_Multiply& src );
    
    /// HVec3_Add Constructor
    NOALIAS __forceinline C_SiVector( const HVec3_Add& src );
    
    /// HVec3_Subtract Constructor
    NOALIAS __forceinline C_SiVector( const HVec3_Subtract& src );
    
    /// Copy Assignment
    NOALIAS __forceinline void                        operator = ( const C_SiVector<LHS>& copy );
    
    /// Get C_Vector: There is no cast operator to C_Vector since this would allow the implicit casting away of the unit type!
    NOALIAS __forceinline const C_Vector              Get() const __restrict;
    
    /// Element Accessors
    NOALIAS __forceinline const C_SiScalar<LHS>       X() const __restrict;
    NOALIAS __forceinline const C_SiScalar<LHS>       Y() const __restrict;
    NOALIAS __forceinline const C_SiScalar<LHS>       Z() const __restrict;
    
    /// Element Zeroing Functions
    NOALIAS __forceinline void                        ZeroX() __restrict;
    NOALIAS __forceinline void                        ZeroY() __restrict;
    NOALIAS __forceinline void                        ZeroZ() __restrict;
    NOALIAS __forceinline void                        ZeroXY() __restrict;
    NOALIAS __forceinline void                        ZeroXZ() __restrict;
    NOALIAS __forceinline void                        ZeroYZ() __restrict;
    
    /// Zero the vector if any element exceeds +/- bounds
    NOALIAS __forceinline void                        ZeroIfOutOfBounds( const Maths::HF32 PCREF bounds ) __restrict;
    
    /// Element Zeroing Functions
    NOALIAS __forceinline const C_SiVector<LHS>       ZeroedX() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       ZeroedY() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       ZeroedZ() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       ZeroedXY() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       ZeroedXZ() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       ZeroedYZ() const __restrict;
    
    /// Length of vector
    NOALIAS __forceinline const C_SiScalar<LHS>       Length() const __restrict;
    
    /// Length squared of vector
    NOALIAS __forceinline const C_SiScalar< typename SI_Squared< LHS >::Result > LengthSquared() const __restrict;
    
    /// Normal of vector
    NOALIAS __forceinline const C_Normalised          Normalised() const __restrict;
    
    /// Normal of vector
    NOALIAS __forceinline const C_Normalised          SafeNormalised( const C_Normal PCREF zeroLengthResult = C_Normal() ) const __restrict;
    
    /// Limit the length of vector
    NOALIAS __forceinline void                        Limit( const C_SiScalar<LHS> PCREF limit ) __restrict;
    
    /// Unary Negation
    NOALIAS __forceinline const C_SiVector<LHS>       operator - () const __restrict;
    
    /// Vector Addition Operator
    NOALIAS __forceinline const C_SiVector<LHS>       operator + ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    
    /// Vector Subtraction Operator
    NOALIAS __forceinline const C_SiVector<LHS>       operator - ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    
    /// Add Update Operator
    NOALIAS __forceinline void                        operator += ( const C_SiVector<LHS> PCREF rhs ) __restrict;
    
    /// Subtract Update Operator
    NOALIAS __forceinline void                        operator -= ( const C_SiVector<LHS> PCREF rhs ) __restrict;
    
    /// Scalar Multiplication Operator
    NOALIAS __forceinline const C_SiVector<LHS>       operator * ( const C_SiScalar<SI_None> PCREF rhs ) const __restrict;
    
    /// Scalar Multiplication Update Operator
    NOALIAS __forceinline void                        operator *= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict;
    
    /// Scalar Division Operator
    NOALIAS __forceinline const C_SiVector<LHS>       operator / ( const C_SiScalar<SI_None> PCREF rhs ) const __restrict;
    
    /// Scalar Division Update Operator
    NOALIAS __forceinline void                        operator /= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict;
    
    /// Scalar Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result >     operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Scalar Division Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVector< typename SI_Quotient< LHS, RHS >::Result >    operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Vector Dot Product Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiScalar< typename SI_Product< LHS, RHS >::Result >     operator | ( const C_SiVector<RHS> PCREF rhs ) const __restrict;
    
    /// Vector Cross Product Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result >     operator ^ ( const C_SiVector<RHS> PCREF rhs ) const __restrict;
    
    /// Vector Dot Product Operator
    //NOALIAS __forceinline const C_SiScalar<LHS>       operator | ( const C_Normal rhs ) const __restrict;
    
    /// Vector Cross Product Operator
    //NOALIAS __forceinline const C_SiVector<LHS>       operator ^ ( const C_Normal rhs ) const __restrict;
    
    /// Matrix Multiplication Operator
    template < class RHS, bool RHS_OrthoNormalised >
    NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > operator * ( const C_SiRotMatrix<RHS,RHS_OrthoNormalised> PCREF rhs ) const __restrict;
    
    /// Get component of this vector parallel to input normal
    NOALIAS __forceinline const C_SiVector<LHS>             Parallel( const C_Normal PCREF normal ) const __restrict;
    
    /// Get component of this vector perpendicular to input normal
    NOALIAS __forceinline const C_SiVector<LHS>             Perpendicular( const C_Normal PCREF normal ) const __restrict;
    
    /// Get components of this vector parallel & perpendicular to input normal
    NOALIAS __forceinline void                              ParaPerp( const C_Normal PCREF normal, C_SiVector<LHS>& para, C_SiVector<LHS>& perp ) const __restrict;
    
    /// Star function = Cross Product Matrix ( a* Matrix )
    NOALIAS __forceinline const C_SiRotMatrix<LHS,false>    Star() const __restrict;
    NOALIAS __forceinline const C_SiRotMatrix<LHS,false>    StarTransposed() const __restrict;
    
    /// Comparison Operators
    NOALIAS __forceinline const S32                   operator <  ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator >  ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator <= ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator >= ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator == ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    NOALIAS __forceinline const S32                   operator != ( const C_SiVector<LHS> PCREF rhs ) const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    static const int            Kg = LHS::Kg;
    static const int            M  = LHS::M;
    static const int            S  = LHS::S;
    
//protected:
public:
    
    HVec3                       m_Vector;
};


template < typename LHS > NOALIAS __forceinline C_SiVector<LHS>   operator * ( const C_SiVector<LHS> PCREF v, const F32& f );
template < typename LHS > NOALIAS __forceinline C_SiVector<LHS>   operator / ( const C_SiVector<LHS> PCREF v, const F32& f );

template < typename LHS > NOALIAS __forceinline void              operator *= ( C_SiVector<LHS>& v, const F32& f );
template < typename LHS > NOALIAS __forceinline void              operator /= ( C_SiVector<LHS>& v, const F32& f );






/// Quaternion rotation class with SI units
// LHS == Unit type of this
//     == Unit type of LEFT HAND SIDE for binary operators
//     == Unit type of RIGHT HAND SIDE for binary operators which take two identical unit types
// RHS == Unit type of RIGHT HAND SIDE for binary operators which take second param of other unit types
template < class LHS >
class C_SiQuaternion
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_SiQuaternion();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_SiQuaternion( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_SiQuaternion( const C_SiQuaternion<LHS>& copy );
    
    /// Quaternion Constructor
    NOALIAS __forceinline C_SiQuaternion( const C_Quaternion PCREF quaternion );
    
    /// C_SiVector Constructor: Vector direction is the axis and vector length is the angle in radians
    //NOALIAS __forceinline C_SiQuaternion( const C_SiVector<LHS> vector );
    
    /// Axis + Angle Constructor
    NOALIAS __forceinline C_SiQuaternion( const C_SiVector<SI_None> PCREF vector, const C_SiScalar<SI_Rad> PCREF theta );
    
    /// C_SiVector Constructor: Imaginary part is set directly from the input vector
    NOALIAS __forceinline C_SiQuaternion( const C_SiVector<LHS> PCREF vector );
    
    /// C_SiRotMatrix Constructor
    NOALIAS __forceinline C_SiQuaternion( const C_SiRotMatrix<LHS,true>& matrix );
    
    /// HMat33 Constructor
    NOALIAS __forceinline C_SiQuaternion( const HMat33& matrix );
    
    /// Copy Assignment
    NOALIAS __forceinline void                        operator = ( const C_SiQuaternion<LHS>& copy );
    
    /// Get Quaternion: There is no cast operator to C_Quaternion since this would allow the implicit casting away of the unit type!
    NOALIAS __forceinline const C_Quaternion          Get() const __restrict;
    
    /// Accessor: Get quaternion axis
    NOALIAS __forceinline const C_Normal              Axis() const __restrict;
    
    /// Accessor: Get quaternion angle
    NOALIAS __forceinline const C_SiScalar<LHS>       Angle() const __restrict;
    
    /// Addition Operator
    NOALIAS __forceinline const C_SiQuaternion<LHS>   operator + ( const C_SiQuaternion<LHS> PCREF src ) const __restrict;
    
    /// Add Update Operator : TODO: Not supported by C_Quaternion yet!
    //NOALIAS __forceinline void                        operator += ( const C_SiQuaternion<LHS> src ) __restrict;
    
    /// Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiQuaternion< typename SI_Product< LHS, RHS >::Result > operator * ( const C_SiQuaternion<RHS> PCREF src ) const __restrict;
    
    /// Division Operator: q1 / q2 = q1 * q2^-1
    template < class RHS >
    NOALIAS __forceinline const C_SiQuaternion< typename SI_Quotient< LHS, RHS >::Result > operator / ( const C_SiQuaternion<RHS> PCREF src ) const __restrict;
    
    /// 'Shortest' Division: q1 / q2 = q1 * q2^-1
    template < class RHS >
    NOALIAS __forceinline const C_SiQuaternion< typename SI_Quotient< LHS, RHS >::Result > ShortestDivide( const C_SiQuaternion<RHS> PCREF src ) const __restrict;
    
    /// Slerp
    NOALIAS __forceinline const C_SiQuaternion<LHS>   Slerp( const C_SiQuaternion<LHS> PCREF src, const HF32 PCREF f ) const __restrict;
    
    /// Normalise: Get normalised copy of quaternion
    NOALIAS __forceinline const C_SiQuaternion<LHS>   Normalised() const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    #if 0
    
    // TODO: Support all of these functions from C_Quaternion!
    
    /// Returns the Imaginary part as a HVec3
    NOALIAS __forceinline const HVec3                 Imag() const __restrict;
    
    /// Returns the Real part as a HVecW
    NOALIAS __forceinline const HVecW                 Real() const __restrict;
    
    /// Cos( theta / 2 ) : theta == rotation angle in radians
    NOALIAS __forceinline const HF32                  CosHalfTheta() const __restrict;
    
    /// Sin( theta / 2 ) : theta == rotation angle in radians
    NOALIAS __forceinline const HF32                  SinHalfTheta() const __restrict;
    
    /// Cos( theta ) : theta == rotation angle in radians
    NOALIAS __forceinline const HF32                  CosTheta() const __restrict;
    
    /// Addition
    //NOALIAS __forceinline const C_Quaternion          operator + ( const C_Quaternion src ) const __restrict;
    
    /// Subtraction
    NOALIAS __forceinline const C_Quaternion          operator - ( const C_Quaternion PCREF src ) const __restrict;
    
    /// Unary Negation
    NOALIAS __forceinline const C_Quaternion          operator - () const __restrict;
    
    /// Conjugate (Multiplicative Inverse) - Assumes Unit Quaternion!
    NOALIAS __forceinline const C_Quaternion          operator ! () const __restrict;
    
    /// Scalar multiplication
    NOALIAS __forceinline const C_Quaternion          operator * ( const HF32 PCREF f ) const __restrict;
    
    /// Multiplication
    //NOALIAS __forceinline const C_Quaternion          operator * ( const C_Quaternion src ) const __restrict;
    
    /// Division: q1 / q2 = q1 * q2^-1
    //NOALIAS __forceinline const C_Quaternion          operator / ( const C_Quaternion src ) const __restrict;
    
    /// Dot Product: Returns 4 element dot product == cosine( angle / 2 ) between the 2 quaternions!
    NOALIAS __forceinline const HF32                  operator | ( const C_Quaternion PCREF src ) const __restrict;
    
    /// Normalise this Quaternion
    NOALIAS __forceinline void                        Normalise() __restrict;
    
    /// Returns normalised (unit length) copy of this Quaternion
    //NOALIAS __forceinline const C_Quaternion          Normalised() const __restrict;
    
    /// Lerp: Linear Interpolation
    NOALIAS __forceinline const C_Quaternion          Lerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Shortest Lerp: Shortest Linear Interpolation - Negates dst if necessary
    NOALIAS __forceinline const C_Quaternion          ShortestLerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Slerp: Spherical Linear Interpolation
    //NOALIAS __forceinline const C_Quaternion          Slerp( const C_Quaternion dst, const HF32 t ) const __restrict;
    
    /// Shortest Slerp: Spherical Linear Interpolation - Negates dst if necessary
    NOALIAS __forceinline const C_Quaternion          ShortestSlerp( const C_Quaternion PCREF dst, const HF32 PCREF t ) const __restrict;
    
    /// Finite Test
    //NOALIAS __forceinline const S32                   Finite() const __restrict;
    
    #endif
    
    static const int            Kg = LHS::Kg;
    static const int            M  = LHS::M;
    static const int            S  = LHS::S;
    
//protected:
public:
    
    C_Quaternion                m_Quaternion;
};







/// LHS == Unit type of this
///     == Unit type of LEFT HAND SIDE for binary operators
///     == Unit type of RIGHT HAND SIDE for binary operators which take two identical unit types
/// RHS == Unit type of RIGHT HAND SIDE for binary operators which take second param of other unit types
template < class LHS, bool LHS_OrthoNormalised >
class C_SiRotMatrix
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_SiRotMatrix();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_SiRotMatrix( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_SiRotMatrix( const C_SiRotMatrix<LHS,LHS_OrthoNormalised>& copy );
    
    /// HMat33 Constructor
    NOALIAS __forceinline C_SiRotMatrix( const HMat33& matrix );
    
    /// C_SiQuaternion Constructor
    NOALIAS __forceinline C_SiRotMatrix( const C_SiQuaternion<LHS> PCREF quaternion );
    
    /// C_SiVector Axis X, Y & Z Constructor
    NOALIAS __forceinline C_SiRotMatrix( const C_SiVector<LHS> PCREF ax, const C_SiVector<LHS> PCREF ay, const C_SiVector<LHS> PCREF az );
    
    /// Get HMat33: There is no cast operator to HMat33 since this would allow the implicit casting away of the unit type!
    NOALIAS __forceinline const HMat33                Get() const __restrict;
    
    /// Copy Assignment
    template < bool RHS_OrthoNormalised >
    NOALIAS __forceinline void                        operator = ( const C_SiRotMatrix< LHS, LHS_OrthoNormalised || RHS_OrthoNormalised >& copy );
    
    /// Axis Accessors
    NOALIAS __forceinline const C_SiVector<LHS>       AX() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       AY() const __restrict;
    NOALIAS __forceinline const C_SiVector<LHS>       AZ() const __restrict;
    
    /// Matrix Multiply Operator
    template < class RHS, bool RHS_OrthoNormalised >
    NOALIAS __forceinline const C_SiRotMatrix< typename SI_Product< LHS, RHS >::Result, LHS_OrthoNormalised && RHS_OrthoNormalised > operator * ( const C_SiRotMatrix<RHS,RHS_OrthoNormalised>& src ) const __restrict;
    
    /// Matrix Multiply Update Operator
    NOALIAS __forceinline void                              operator *= ( const C_SiRotMatrix<LHS,LHS_OrthoNormalised>& src ) __restrict;
    
    /// Addition Operator: Element by element addition
    NOALIAS __forceinline const C_SiRotMatrix<LHS,false>    operator + ( const C_SiRotMatrix<LHS,LHS_OrthoNormalised>& src ) const __restrict;
    
    /// Addition Update Operator: Element by element addition
    NOALIAS __forceinline void                              operator += ( const C_SiRotMatrix<LHS,LHS_OrthoNormalised>& src ) __restrict;
    
    /// Scalar Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiRotMatrix< typename SI_Product< LHS, RHS >::Result, false > operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Vector Multiplication Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > operator * ( const C_SiVector<RHS> PCREF rhs ) const __restrict;
    
    /// Binary Division Operator
    template < class RHS >
    NOALIAS __forceinline const C_SiRotMatrix< typename SI_Quotient< LHS, RHS >::Result, false > operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict;
    
    /// Transpose
    NOALIAS __forceinline const C_SiRotMatrix< typename SI_Inverse< LHS >::Result, LHS_OrthoNormalised > Transpose() const __restrict;
    
    /// Inverse
    NOALIAS __forceinline const C_SiRotMatrix< typename SI_Inverse< LHS >::Result, LHS_OrthoNormalised > Inverse() const __restrict;
    
    /// Normalise
    NOALIAS __forceinline void                        Normalise() __restrict;
    
    /// Normalise
    NOALIAS __forceinline void                        OrthoNormalise() __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    static const int            Kg = LHS::Kg;
    static const int            M  = LHS::M;
    static const int            S  = LHS::S;
    
    static const bool           m_OrthoNormalised = LHS_OrthoNormalised;
    
//protected:
public:
    
    HMat33                      m_Matrix;
};



//
// Scalar SI Unit Types
//

typedef C_SiScalar<SI_None>         C_RawScalar;

typedef C_SiScalar<SI_Kg>           C_MassKg;
typedef C_SiScalar<SI_pKg>          C_ReciprocalMass_pKg;
typedef C_SiScalar<SI_M>            C_DistanceM;
typedef C_SiScalar<SI_pM>           C_ReciprocalDistance_pM;
typedef C_SiScalar<SI_S>            C_TimeS;
typedef C_SiScalar<SI_pS>           C_ReciprocalTime_pS;
typedef C_SiScalar<SI_pS2>          C_ReciprocalTime_pS2;
typedef C_SiScalar<SI_pS>           C_FrequencyHz;
typedef C_SiScalar<SI_pS2>          C_FrequencySquaredHz2;
typedef C_SiScalar<SI_M2>           C_AreaM2;
typedef C_SiScalar<SI_M3>           C_VolumeM3;
typedef C_SiScalar<SI_KgpM3>        C_DensityKgpM3;
typedef C_SiScalar<SI_KgM2>         C_ScalarTensorKgM2;
typedef C_SiScalar<SI_Rad>          C_AngleR;
typedef C_SiScalar<SI_NpM>          C_LinearSpringConstantNpM;
typedef C_SiScalar<SI_N>            C_AngularSpringConstantNpR;
typedef C_SiScalar<SI_NSpM>         C_LinearSpringDampingNSpM;
typedef C_SiScalar<SI_NS>           C_AngularSpringDampingNSpR;

typedef C_SiScalar<SI_N>            C_ScalarForceN;
typedef C_SiScalar<SI_pN>           C_ScalarReciprocalForce_pN;
typedef C_SiScalar<SI_N2>           C_ScalarForceN2;
typedef C_SiScalar<SI_MpS>          C_ScalarVelocityMpS;
typedef C_SiScalar<SI_RpS>          C_ScalarAngularVelocityRpS;
typedef C_SiScalar<SI_MpS2>         C_ScalarAccelerationMpS2;
typedef C_SiScalar<SI_KgMpS>        C_ScalarImpulseKgMpS;
typedef C_SiScalar<SI_KgM2pS2>      C_ScalarTorqueKgM2pS2;

//
// Vertex & Vector SI Unit Types
//

typedef C_SiVertex<SI_None>         C_RawVertex;
typedef C_SiVector<SI_None>         C_RawVector;

typedef C_SiVertex<SI_M>            C_VertexM;
typedef C_SiVector<SI_M>            C_VectorM;
typedef C_SiVector<SI_M2>           C_VectorProductM2;

typedef C_SiVector<SI_N>            C_ForceN;
typedef C_SiVector<SI_MpS>          C_VelocityMpS;
typedef C_SiVector<SI_MpS2>         C_AccelerationMpS2;
typedef C_SiVector<SI_KgMpS>        C_MomentumKgMpS;
typedef C_SiVector<SI_KgMpS>        C_ImpulseKgMpS;
typedef C_SiVector<SI_KgM2pS>       C_ImpulseTorqueKgM2pS;
typedef C_SiVector<SI_KgM2pS2>      C_TorqueKgM2pS2;
typedef C_SiVector<SI_RpS>          C_AngularVelocityRpS;
typedef C_SiVector<SI_RpS2>         C_AngularAccelerationRpS2;
typedef C_SiVector<SI_KgM2RpS>      C_AngularMomentumKgM2RpS;
typedef C_SiVector<SI_KgM2>         C_VectorTensorKgM2;

//
// Matrix
//

typedef C_SiRotMatrix<SI_Rad,true>      C_RotationMatrixR;
typedef C_SiRotMatrix<SI_KgM2,false>    C_InertiaTensorKgM2;
typedef C_SiRotMatrix<SI_pKg_pM2,false> C_InverseInertiaTensor_pKg_pM2;

//
// Quaternion
//

typedef C_SiQuaternion<SI_Rad>      C_QuaternionR;





//
// -------------------------------------------
// |-- Non-Member Friend Support Functions --|
// -------------------------------------------
//

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> Select( const Scalar_Bool PCREF selector, const C_SiScalar<LHS> PCREF resultIfTrue, const C_SiScalar<LHS> PCREF resultIfFalse );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> Select( const Vector3_Bool PCREF selector, const C_SiVertex<LHS> PCREF resultIfTrue, const C_SiVertex<LHS> PCREF resultIfFalse );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> Select( const Vector3_Bool PCREF selector, const C_SiVector<LHS> PCREF resultIfTrue, const C_SiVector<LHS> PCREF resultIfFalse );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiQuaternion<LHS> Select( const Vector4_Bool PCREF selector, const C_SiQuaternion<LHS> PCREF resultIfTrue, const C_SiQuaternion<LHS> PCREF resultIfFalse );



NOALIAS __forceinline const C_InertiaTensorKgM2 ParallelAxis( const C_InertiaTensorKgM2& it, const C_VectorM PCREF axis, const C_MassKg PCREF m );




}; // namespace Maths


#endif

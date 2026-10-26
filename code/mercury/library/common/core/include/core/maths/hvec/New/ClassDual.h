#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_DUAL_H
#define SOFT_MATHS_HVEC_NEW_CLASS_DUAL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_DUAL_H
#define MATHS_HVEC_NEW_CLASS_DUAL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup DualScalar
/// @{

///
/// Dual Numbers:
///
/// The algebra of dual numbers, R^, is similar to complex numbers,
/// being comprised of 2 values, the real part and the dual part.
/// The difference between dual numbers and complex numbers is that
/// the dual unit (e) squares to zero whereas the complex unit (i)
/// squares to minus one.
///
/// a^ = a0 + e.ae, where:
/// a^ is a dual number,
/// a0 is the real or non-dual part,
/// ae is the dual part and
/// e  is the dual unit satisfying e^2 = 0
///
/// Multiplication:
/// (a0 + e.ae)(b0 + e.be) = a0.b0 + e.(a0.be + ae.b0)
///
/// (b0 + e.be)(a0 + e.ae) = b0.a0 + e.(be.a0 + b0.ae)
///                        = a0.b0 + e.(a0.be + ae.b0)
///                        = (a0 + e.ae)(b0 + e.be)
/// Therefore dual numbers commute.
///

/// DualScalar: Vector register dual number class.
class ALIGN16 PASSINREG DARWIN64_ABI DualScalar
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    
    /// DualScalar Default Constructor: Undefined
    NOALIAS __forceinline DualScalar() {}
    
    /// DualScalar Copy Constructor
    NOALIAS __forceinline DualScalar( const DualScalar& copy ) : m_V( copy.m_V ) {}
    
    /// DualScalar Copy Assignment Operator
    NOALIAS __forceinline const DualScalar& operator = ( const DualScalar& copy ) { m_V = copy.m_V; return copy; }
    
    #endif
    
    Vec4F32 m_V; ///< m_V.x = real part, m_V.y = dual part
};


/// DualVector3: Vector register dual vector3 class.
//class ALIGN16 DARWIN64_ABI DualVector3
//{
//public:
//    
//    /// DualVector3 Default Constructor: Undefined
//    NOALIAS __forceinline DualVector3() {}
//    
//    /// DualVector3 Copy Constructor
//    NOALIAS __forceinline DualVector3( const DualVector3& copy );
//    
//    /// DualVector3 Copy Assignment Operator
//    NOALIAS __forceinline const DualVector3& operator = ( const DualVector3& copy );
//    
//    Vector3 m_V0; ///< real part
//    Vector3 m_Ve; ///< dual part
//};


/// DualQuaternion: Vector register dual quaternion class.
class ALIGN16 DARWIN64_ABI DualQuaternion
{
public:
    
    /// DualQuaternion Default Constructor: Undefined
    NOALIAS __forceinline DualQuaternion() {}
    
    /// DualQuaternion Copy Constructor
    NOALIAS __forceinline DualQuaternion( const DualQuaternion& copy );
    
    /// DualQuaternion Copy Assignment Operator
    NOALIAS __forceinline const DualQuaternion& operator = ( const DualQuaternion& copy );
    
    Vec8F32 m_V; ///< i, j, k, 1, ei, ej, ek, e
};






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



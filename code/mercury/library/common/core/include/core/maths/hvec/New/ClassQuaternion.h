#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_QUATERNION_H
#define SOFT_MATHS_HVEC_NEW_CLASS_QUATERNION_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_QUATERNION_H
#define MATHS_HVEC_NEW_CLASS_QUATERNION_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Quaternion_Classes
/// @{

/// Quaternion: Vector register quaternion class.
class ALIGN16 PASSINREG DARWIN64_ABI Quaternion
{
public:
    
    /// NOTE: The Quaternion class is NOT implicitly normalised,
    /// since non-unit-length quaternions are necessary for some operations!
    /// (e.g. transforming a vector directly by a quaternion)
    
    #if !defined( PASSINREG_ENABLED )
    
    /// Quaternion Default Constructor: Undefined
    NOALIAS __forceinline Quaternion() {}
    
    /// Quaternion Copy Constructor
    NOALIAS __forceinline Quaternion( const Quaternion& copy ) : m_Q( copy.m_Q ) {}
    
    /// Quaternion Copy Assignment Operator
    NOALIAS __forceinline const Quaternion& operator = ( const Quaternion& copy ) { m_Q = copy.m_Q; return copy; }
    
    #endif
    
    Vec4F32 m_Q; ///< m_Q.xyz = imaginary vector part, m_Q.w = real scalar part
};

/// Non-aligned, float Storage class Quaternion
typedef Storage< Quaternion, MathsStorage::Float4 > StorageQuaternion;


/// QuatTrans: Vector register quaternion transform class.
class ALIGN16 DARWIN64_ABI QuatTrans
{
public:
    
    /// QuatTrans Default Constructor: Undefined
    NOALIAS __forceinline QuatTrans() {}
    
    /// QuatTrans Copy Constructor
    NOALIAS __forceinline QuatTrans( const QuatTrans& copy );
    
    /// QuatTrans Copy Assignment Operator
    NOALIAS __forceinline const QuatTrans& operator = ( const QuatTrans& copy );
    
    Quaternion m_Q; ///< m_Q.xyz = imaginary vector part, m_Q.w = real scalar part
    Vector3    m_T; ///< m_T.xyz = translation
};


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



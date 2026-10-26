#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_JACOBIAN_H
#define SOFT_MATHS_HVEC_NEW_CLASS_JACOBIAN_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_JACOBIAN_H
#define MATHS_HVEC_NEW_CLASS_JACOBIAN_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Jacobian_Classes
/// @{

/// Mat26: Vector register 2 row x 6 column matrix class
class ALIGN16 DARWIN64_ABI Mat26
{
public:
    
    NOALIAS __forceinline Mat26() {};
    NOALIAS __forceinline Mat26( const Mat26& copy );                    ///< Mat26 Copy Constructor
    NOALIAS __forceinline const Mat26& operator = ( const Mat26& copy ); ///< Mat26 Copy Assignment
    
    static const S32 m_Rows = 2; ///< Matrix size(rows)
    static const S32 m_Cols = 6; ///< Matrix size(columns)
    
    Vector6 m_Row[m_Rows]; ///< Mat26 storage
};

/// Mat36: Vector register 3 row x 6 column matrix class
class ALIGN16 DARWIN64_ABI Mat36
{
public:
    
    NOALIAS __forceinline Mat36() {};
    NOALIAS __forceinline Mat36( const Mat36& copy );                    ///< Mat36 Copy Constructor
    NOALIAS __forceinline const Mat36& operator = ( const Mat36& copy ); ///< Mat36 Copy Assignment
    
    static const S32 m_Rows = 3; ///< Matrix size(rows)
    static const S32 m_Cols = 6; ///< Matrix size(columns)
    
    Vector6 m_Row[m_Rows]; ///< Mat36 storage
};

/// Mat46: Vector register 4 row x 6 column matrix class
class ALIGN16 DARWIN64_ABI Mat46
{
public:
    
    NOALIAS __forceinline Mat46() {};
    NOALIAS __forceinline Mat46( const Mat46& copy );                    ///< Mat46 Copy Constructor
    NOALIAS __forceinline const Mat46& operator = ( const Mat46& copy ); ///< Mat46 Copy Assignment
    
    static const S32 m_Rows = 4; ///< Matrix size(rows)
    static const S32 m_Cols = 6; ///< Matrix size(columns)
    
    Vector6 m_Row[m_Rows]; ///< Mat46 storage
};

/// Mat55: Vector register 5 row x 5 column matrix class
class ALIGN16 DARWIN64_ABI Mat55
{
public:
    
    NOALIAS __forceinline Mat55() {};
    NOALIAS __forceinline Mat55( const Mat55& copy );                    ///< Mat55 Copy Constructor
    NOALIAS __forceinline const Mat55& operator = ( const Mat55& copy ); ///< Mat55 Copy Assignment
    
    static const S32 m_Rows = 5; ///< Matrix size(rows)
    static const S32 m_Cols = 5; ///< Matrix size(columns)
    
    Vector5 m_Row[m_Rows]; ///< Mat55 storage
};

/// Mat56: Vector register 5 row x 6 column matrix class
class ALIGN16 DARWIN64_ABI Mat56
{
public:
    
    NOALIAS __forceinline Mat56() {};
    NOALIAS __forceinline Mat56( const Mat56& copy );                    ///< Mat56 Copy Constructor
    NOALIAS __forceinline const Mat56& operator = ( const Mat56& copy ); ///< Mat56 Copy Assignment
    
    static const S32 m_Rows = 5; ///< Matrix size(rows)
    static const S32 m_Cols = 6; ///< Matrix size(columns)
    
    Vector6 m_Row[m_Rows]; ///< Mat56 storage
};

/// Mat66: Vector register 6 row x 6 column matrix class
class ALIGN16 DARWIN64_ABI Mat66
{
public:
    
    NOALIAS __forceinline Mat66() {};
    NOALIAS __forceinline Mat66( const Mat66& copy );                    ///< Mat66 Copy Constructor
    NOALIAS __forceinline const Mat66& operator = ( const Mat66& copy ); ///< Mat66 Copy Assignment
    
    static const S32 m_Rows = 6; ///< Matrix size(rows)
    static const S32 m_Cols = 6; ///< Matrix size(columns)
    
    Vector6 m_Row[m_Rows]; ///< Mat66 storage
};

/// Mat76: Vector register 7 row x 6 column matrix class
class ALIGN16 DARWIN64_ABI Mat76
{
public:
    
    NOALIAS __forceinline Mat76() {};
    NOALIAS __forceinline Mat76( const Mat76& copy );                    ///< Mat76 Copy Constructor
    NOALIAS __forceinline const Mat76& operator = ( const Mat76& copy ); ///< Mat76 Copy Assignment
    
    static const S32 m_Rows = 7; ///< Matrix size(rows)
    static const S32 m_Cols = 6; ///< Matrix size(columns)
    
    Vector6 m_Row[m_Rows]; ///< Mat76 storage
};

/// Mat62: Vector register 6 row x 2 column matrix class
class ALIGN16 DARWIN64_ABI Mat62
{
public:
    
    NOALIAS __forceinline Mat62() {};
    NOALIAS __forceinline Mat62( const Mat62& copy );                    ///< Mat62 Copy Constructor
    NOALIAS __forceinline const Mat62& operator = ( const Mat62& copy ); ///< Mat62 Copy Assignment
    
    static const S32 m_Rows = 6; ///< Matrix size(rows)
    static const S32 m_Cols = 2; ///< Matrix size(columns)
    
    Vector2 m_Row[m_Rows]; ///< Mat62 storage
};

/// Mat63: Vector register 6 row x 3 column matrix class
class ALIGN16 DARWIN64_ABI Mat63
{
public:
    
    NOALIAS __forceinline Mat63() {};
    NOALIAS __forceinline Mat63( const Mat63& copy );                    ///< Mat63 Copy Constructor
    NOALIAS __forceinline const Mat63& operator = ( const Mat63& copy ); ///< Mat63 Copy Assignment
    
    static const S32 m_Rows = 6; ///< Matrix size(rows)
    static const S32 m_Cols = 3; ///< Matrix size(columns)
    
    Vector3 m_Row[m_Rows]; ///< Mat63 storage
};

/// Mat64: Vector register 6 row x 4 column matrix class
class ALIGN16 DARWIN64_ABI Mat64
{
public:
    
    NOALIAS __forceinline Mat64() {};
    NOALIAS __forceinline Mat64( const Mat64& copy );                    ///< Mat64 Copy Constructor
    NOALIAS __forceinline const Mat64& operator = ( const Mat64& copy ); ///< Mat64 Copy Assignment
    
    static const S32 m_Rows = 6; ///< Matrix size(rows)
    static const S32 m_Cols = 4; ///< Matrix size(columns)
    
    Vector4 m_Row[m_Rows]; ///< Mat64 storage
};

/// Mat65: Vector register 6 row x 5 column matrix class
class ALIGN16 DARWIN64_ABI Mat65
{
public:
    
    NOALIAS __forceinline Mat65() {};
    NOALIAS __forceinline Mat65( const Mat65& copy );                    ///< Mat65 Copy Constructor
    NOALIAS __forceinline const Mat65& operator = ( const Mat65& copy ); ///< Mat65 Copy Assignment
    
    static const S32 m_Rows = 6; ///< Matrix size(rows)
    static const S32 m_Cols = 5; ///< Matrix size(columns)
    
    Vector5 m_Row[m_Rows]; ///< Mat65 storage
};

/// Mat67: Vector register 6 row x 7 column matrix class
class ALIGN16 DARWIN64_ABI Mat67
{
public:
    
    NOALIAS __forceinline Mat67() {};
    NOALIAS __forceinline Mat67( const Mat67& copy );                    ///< Mat67 Copy Constructor
    NOALIAS __forceinline const Mat67& operator = ( const Mat67& copy ); ///< Mat67 Copy Assignment
    
    static const S32 m_Rows = 6; ///< Matrix size(rows)
    static const S32 m_Cols = 7; ///< Matrix size(columns)
    
    Vector7 m_Row[m_Rows]; ///< Mat67 storage
};


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



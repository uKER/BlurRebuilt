#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_MATRIX_H
#define SOFT_MATHS_HVEC_NEW_CLASS_MATRIX_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_MATRIX_H
#define MATHS_HVEC_NEW_CLASS_MATRIX_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Matrix_Classes
/// @{

/// Matrix row index enums
enum E_Row
{
    E_RowX = 0,
    E_RowY = 1,
    E_RowZ = 2,
    E_RowW = 3,
    
    E_Row0 = 0,
    E_Row1 = 1,
    E_Row2 = 2,
    E_Row3 = 3,
    E_Row4 = 4,
    E_Row5 = 5,
    E_Row6 = 6,
    E_Row7 = 7
};

/// Matrix column index enums
enum E_Col
{
    E_ColX = 0,
    E_ColY = 1,
    E_ColZ = 2,
    E_ColW = 3,
    
    E_Col0 = 0,
    E_Col1 = 1,
    E_Col2 = 2,
    E_Col3 = 3,
    E_Col4 = 4,
    E_Col5 = 5,
    E_Col6 = 6,
    E_Col7 = 7
};


/// Mat33: Vector register 3 row x 3 column matrix class
class ALIGN16 DARWIN64_ABI Mat33
{
public:
    
    NOALIAS __forceinline Mat33() {};                                    ///< Mat33 Default Constructor (Undefined!)
    NOALIAS __forceinline Mat33( const Mat33& copy );                    ///< Mat33 Copy Constructor
    NOALIAS __forceinline const Mat33& operator = ( const Mat33& copy ); ///< Mat33 Copy Assignment
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_RowX; ///< Mat33 storage (row 0)
    vector float    m_RowY; ///< Mat33 storage (row 1)
    vector float    m_RowZ; ///< Mat33 storage (row 2)
    #elif defined( _VMX128_ )
    __vector4       m_ColX; ///< Mat33 storage (column 0)
    __vector4       m_ColY; ///< Mat33 storage (column 1)
    __vector4       m_ColZ; ///< Mat33 storage (column 2)
    #elif defined( _SSE_ )
    __m128          m_RowX; ///< Mat33 storage (row 0)
    __m128          m_RowY; ///< Mat33 storage (row 1)
    __m128          m_RowZ; ///< Mat33 storage (row 2)
    #else
    F32             m[3][3]; ///< Mat33 storage
    #endif
};

/// @brief Mat43: Vector register 4 row x 3 column matrix class
/// @par NOTE: This is a 4x4 matrix with implicit column W (0,0,0,1)
/// @par The actual storage does NOT contain the implicit column!
class ALIGN16 DARWIN64_ABI Mat43
{
public:
    
    NOALIAS __forceinline Mat43() {};                                    ///< Mat43 Default Constructor (Undefined!)
    NOALIAS __forceinline Mat43( const Mat43& copy );                    ///< Mat43 Copy Constructor
    NOALIAS __forceinline const Mat43& operator = ( const Mat43& copy ); ///< Mat43 Copy Assignment
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_RowX; ///< Mat43 storage (row 0)
    vector float    m_RowY; ///< Mat43 storage (row 1)
    vector float    m_RowZ; ///< Mat43 storage (row 2)
    vector float    m_RowW; ///< Mat43 storage (row 3)
    #elif defined( _VMX128_ )
    __vector4       m_ColX; ///< Mat43 storage (column 0)
    __vector4       m_ColY; ///< Mat43 storage (column 1)
    __vector4       m_ColZ; ///< Mat43 storage (column 2)
    #elif defined( _SSE_ )
    __m128          m_RowX; ///< Mat43 storage (row 0)
    __m128          m_RowY; ///< Mat43 storage (row 1)
    __m128          m_RowZ; ///< Mat43 storage (row 2)
    __m128          m_RowW; ///< Mat43 storage (row 3)
    #else
    F32             m[4][3]; ///< Mat43 storage
    #endif
};

/// Mat44: Vector register 4 row x 4 column matrix class
class ALIGN16 DARWIN64_ABI Mat44
{
public:
    
    NOALIAS __forceinline Mat44() {};                                    ///< Mat44 Default Constructor (Undefined!)
    NOALIAS __forceinline Mat44( const Mat44& copy );                    ///< Mat44 Copy Constructor
    NOALIAS __forceinline const Mat44& operator = ( const Mat44& copy ); ///< Mat44 Copy Assignment
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_RowX; ///< Mat44 storage (row 0)
    vector float    m_RowY; ///< Mat44 storage (row 1)
    vector float    m_RowZ; ///< Mat44 storage (row 2)
    vector float    m_RowW; ///< Mat44 storage (row 3)
    #elif defined( _VMX128_ )
    __vector4       m_ColX; ///< Mat44 storage (column 0)
    __vector4       m_ColY; ///< Mat44 storage (column 1)
    __vector4       m_ColZ; ///< Mat44 storage (column 2)
    __vector4       m_ColW; ///< Mat44 storage (column 3)
    #elif defined( _SSE_ )
    __m128          m_RowX; ///< Mat44 storage (row 0)
    __m128          m_RowY; ///< Mat44 storage (row 1)
    __m128          m_RowZ; ///< Mat44 storage (row 2)
    __m128          m_RowW; ///< Mat44 storage (row 3)
    #else
    F32             m[4][4]; ///< Mat44 storage
    #endif
};

/// @brief ScaleMat3: Vector register 3 element diagonal scale matrix class.
/// @code
/// [ SX  0   0  ]
/// [ 0   SY  0  ]
/// [ 0   0   SZ ]
/// @endcode
class ALIGN16 PASSINREG DARWIN64_ABI ScaleMat3
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline ScaleMat3() {};                                        ///< ScaleMat3 Default Constructor (Undefined!)
    NOALIAS __forceinline ScaleMat3( const ScaleMat3& copy );                    ///< ScaleMat3 Copy Constructor
    NOALIAS __forceinline const ScaleMat3& operator = ( const ScaleMat3& copy ); ///< ScaleMat3 Copy Assignment
    #endif
    
    Vector3 m_Scale; ///< Diagonal elements m00=SX, m11=SY, m22=SZ
};

/// @brief ShearMat3: Vector register 3 element shear matrix class.
/// @code
/// [  1   0   0 ]
/// [ XY   1   0 ]
/// [ XZ  YZ   1 ]
/// @endcode
class ALIGN16 PASSINREG DARWIN64_ABI ShearMat3
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline ShearMat3() {};                                        ///< ShearMat3 Default Constructor (Undefined!)
    NOALIAS __forceinline ShearMat3( const ShearMat3& copy );                    ///< ShearMat3 Copy Constructor
    NOALIAS __forceinline const ShearMat3& operator = ( const ShearMat3& copy ); ///< ShearMat3 Copy Assignment
    #endif
    
    Vector3 m_Shear; ///< Shear elements m10=XY, m20=XZ, m21=YZ
};

/// OrthoMat33: Vector register 3 row x 3 column orthonormal rotation matrix class
class ALIGN16 DARWIN64_ABI OrthoMat33 // RotationMatrix, RotMat, RotMat33, OrthoMat, OrthoMat33 ?
{
public:
    
    NOALIAS __forceinline OrthoMat33() {};                                         ///< OrthoMat33 Default Constructor (Undefined!)
    NOALIAS __forceinline OrthoMat33( const OrthoMat33& copy );                    ///< OrthoMat33 Copy Constructor
    NOALIAS __forceinline const OrthoMat33& operator = ( const OrthoMat33& copy ); ///< OrthoMat33 Copy Assignment
    
    Mat33 m; ///< generic 3x3 matrix
};

/// OrthoMat43: Vector register 4 row x 3 column orthonormal rotation & translation matrix class
class ALIGN16 DARWIN64_ABI OrthoMat43 // RotTransMatrix, RotTransMat, RotMat43, OrthoMat43 ?
{
public:
    
    NOALIAS __forceinline OrthoMat43() {};                                         ///< OrthoMat43 Default Constructor (Undefined!)
    NOALIAS __forceinline OrthoMat43( const OrthoMat43& copy );                    ///< OrthoMat43 Copy Constructor
    NOALIAS __forceinline const OrthoMat43& operator = ( const OrthoMat43& copy ); ///< OrthoMat43 Copy Assignment
    
    Mat43 m; ///< generic 4x3 matrix
};


/// EulerXYZ: Euler angle rotations specified in order about axes XYZ
class ALIGN16 DARWIN64_ABI EulerXYZ
{
public:
    
    NOALIAS __forceinline EulerXYZ() {};                                                                        ///< EulerXYZ Default Constructor (Undefined!)
    NOALIAS __forceinline EulerXYZ( const EulerXYZ& copy ) : m_V( copy.m_V ) {};                                ///< EulerXYZ Copy Constructor
    NOALIAS __forceinline const EulerXYZ& operator = ( const EulerXYZ& copy ) { m_V = copy.m_V; return copy; }; ///< EulerXYZ Copy Assignment
    
    Vector3 m_V; ///< Euler angle rotations about axes XYZ: order of multiplication is XYZ
};

/// EulerXZY: Euler angle rotations specified in order about axes XZY
class ALIGN16 DARWIN64_ABI EulerXZY
{
public:
    
    NOALIAS __forceinline EulerXZY() {};                                                                        ///< EulerXZY Default Constructor (Undefined!)
    NOALIAS __forceinline EulerXZY( const EulerXZY& copy ) : m_V( copy.m_V ) {};                                ///< EulerXZY Copy Constructor
    NOALIAS __forceinline const EulerXZY& operator = ( const EulerXZY& copy ) { m_V = copy.m_V; return copy; }; ///< EulerXZY Copy Assignment
    
    Vector3 m_V; ///< Euler angle rotations about axes XYZ: order of multiplication is XZY
};

/// EulerYXZ: Euler angle rotations specified in order about axes YXZ
class ALIGN16 DARWIN64_ABI EulerYXZ
{
public:
    
    NOALIAS __forceinline EulerYXZ() {};                                                                        ///< EulerYXZ Default Constructor (Undefined!)
    NOALIAS __forceinline EulerYXZ( const EulerYXZ& copy ) : m_V( copy.m_V ) {};                                ///< EulerYXZ Copy Constructor
    NOALIAS __forceinline const EulerYXZ& operator = ( const EulerYXZ& copy ) { m_V = copy.m_V; return copy; }; ///< EulerYXZ Copy Assignment
    
    Vector3 m_V; ///< Euler angle rotations about axes XYZ: order of multiplication is YXZ
};

/// EulerYZX: Euler angle rotations specified in order about axes YZX
class ALIGN16 DARWIN64_ABI EulerYZX
{
public:
    
    NOALIAS __forceinline EulerYZX() {};                                                                        ///< EulerYZX Default Constructor (Undefined!)
    NOALIAS __forceinline EulerYZX( const EulerYZX& copy ) : m_V( copy.m_V ) {};                                ///< EulerYZX Copy Constructor
    NOALIAS __forceinline const EulerYZX& operator = ( const EulerYZX& copy ) { m_V = copy.m_V; return copy; }; ///< EulerYZX Copy Assignment
    
    Vector3 m_V; ///< Euler angle rotations about axes XYZ: order of multiplication is YZX
};

/// EulerZXY: Euler angle rotations specified in order about axes ZXY
class ALIGN16 DARWIN64_ABI EulerZXY
{
public:
    
    NOALIAS __forceinline EulerZXY() {};                                                                        ///< EulerZXY Default Constructor (Undefined!)
    NOALIAS __forceinline EulerZXY( const EulerZXY& copy ) : m_V( copy.m_V ) {};                                ///< EulerZXY Copy Constructor
    NOALIAS __forceinline const EulerZXY& operator = ( const EulerZXY& copy ) { m_V = copy.m_V; return copy; }; ///< EulerZXY Copy Assignment
    
    Vector3 m_V; ///< Euler angle rotations about axes XYZ: order of multiplication is ZXY
};

/// EulerZYX: Euler angle rotations specified in order about axes ZYX
class ALIGN16 DARWIN64_ABI EulerZYX
{
public:
    
    NOALIAS __forceinline EulerZYX() {};                                                                        ///< EulerZYX Default Constructor (Undefined!)
    NOALIAS __forceinline EulerZYX( const EulerZYX& copy ) : m_V( copy.m_V ) {};                                ///< EulerZYX Copy Constructor
    NOALIAS __forceinline const EulerZYX& operator = ( const EulerZYX& copy ) { m_V = copy.m_V; return copy; }; ///< EulerZYX Copy Assignment
    
    Vector3 m_V; ///< Euler angle rotations about axes XYZ: order of multiplication is ZYX
};







/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



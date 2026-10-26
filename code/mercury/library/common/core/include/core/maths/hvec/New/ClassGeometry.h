#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_GEOMETRY_H
#define SOFT_MATHS_HVEC_NEW_CLASS_GEOMETRY_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_GEOMETRY_H
#define MATHS_HVEC_NEW_CLASS_GEOMETRY_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Geometry_Classes
/// @{

/// QuadraticResult: pair of results from solving: ax^2 + bx + c = 0
class ALIGN16 DARWIN64_ABI QuadraticResult
{
public:
    
    NOALIAS __forceinline QuadraticResult() {}                                               ///< QuadraticResult Default Constructor (Undefined!)
    NOALIAS __forceinline QuadraticResult( const QuadraticResult& copy );                    ///< QuadraticResult Copy Constructor
    NOALIAS __forceinline const QuadraticResult& operator = ( const QuadraticResult& copy ); ///< QuadraticResult Copy Assignment
    
    VF32 m_X1; ///< first result
    VF32 m_X2; ///< second result
};

/// Line: pair of points
class ALIGN16 DARWIN64_ABI Line
{
public:
    
    NOALIAS __forceinline Line() {}                                    ///< Line Default Constructor (Undefined!)
    NOALIAS __forceinline Line( const Line& copy );                    ///< Line Copy Constructor
    NOALIAS __forceinline const Line& operator = ( const Line& copy ); ///< Line Copy Assignment
    
    Vec4F32 m_StartLength;  ///< start point (x,y,z) & length (w)
    Vec4F32 m_EndRcpLength; ///< end point (x,y,z) & reciprocal length (w)
};

/// Plane: stored in Hessian Normal Form, normal (x,y,z) and distance to origin (w).
class ALIGN16 PASSINREG DARWIN64_ABI Plane
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Plane() {}                                                                     ///< Plane Default Constructor (Undefined!)
    NOALIAS __forceinline Plane( const Plane& copy ) : m_V( copy.m_V ) {}                                ///< Plane Copy Constructor
    NOALIAS __forceinline const Plane& operator = ( const Plane& copy ) { m_V = copy.m_V; return copy; } ///< Plane Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Vec4F32 storage: normal (x,y,z) & distance to origin (w)
};

/// Sphere: stored as centre (x,y,z) and radius (w).
class ALIGN16 PASSINREG DARWIN64_ABI Sphere
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Sphere() {}                                                                      ///< Sphere Default Constructor (Undefined!)
    NOALIAS __forceinline Sphere( const Sphere& copy ) : m_V( copy.m_V ) {}                                ///< Sphere Copy Constructor
    NOALIAS __forceinline const Sphere& operator = ( const Sphere& copy ) { m_V = copy.m_V; return copy; } ///< Sphere Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Vec4F32 storage: centre (x,y,z) & radius (w)
};

/// Triangle: stored as point A and edge vectors AB & AC
class ALIGN16 DARWIN64_ABI Triangle
{
public:
    
    NOALIAS __forceinline Triangle() {}                                        ///< Triangle Default Constructor (Undefined!)
    NOALIAS __forceinline Triangle( const Triangle& copy );                    ///< Triangle Copy Constructor
    NOALIAS __forceinline const Triangle& operator = ( const Triangle& copy ); ///< Triangle Copy Assignment
    
    Point3  m_PointA;       ///< Point A
    Vector3 m_EdgeVectorAB; ///< Vector from A to B (U coord)
    Vector3 m_EdgeVectorAC; ///< Vector from A to C (V coord)
};

/// AxisBox: Axis Aligned Bounding Box (AABB), stored as min & max vectors
class ALIGN16 DARWIN64_ABI AxisBox
{
public:
    
    NOALIAS __forceinline AxisBox() {}                                       ///< AxisBox Default Constructor (Undefined!)
    NOALIAS __forceinline AxisBox( const AxisBox& copy );                    ///< AxisBox Copy Constructor
    NOALIAS __forceinline const AxisBox& operator = ( const AxisBox& copy ); ///< AxisBox Copy Assignment
    
    Vector3 m_Min; ///< Vector of minimum XYZ coords
    Vector3 m_Max; ///< Vector of maximum XYZ coords
};

typedef AxisBox AABB;
#define MakeAABB MakeAxisBox


/// OrientedBox: Oriented Bounding Box (OBB), stored as min & max vectors
class ALIGN16 DARWIN64_ABI OrientedBox
{
public:
    
    NOALIAS __forceinline OrientedBox() {}                                           ///< OrientedBox Default Constructor (Undefined!)
    NOALIAS __forceinline OrientedBox( const OrientedBox& copy );                    ///< OrientedBox Copy Constructor
    NOALIAS __forceinline const OrientedBox& operator = ( const OrientedBox& copy ); ///< OrientedBox Copy Assignment
    
    Vector3 m_HalfExtent; ///< Half the extent of the AxisBox, in space of m_Matrix, centre is at origin, min = -m_HalfExtent, max = m_HalfExtent
    Mat43   m_Matrix;     ///< Position & Orientation of centre of AxisBox
};

typedef OrientedBox OBB;
#define MakeOBB MakeOrientedBox


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



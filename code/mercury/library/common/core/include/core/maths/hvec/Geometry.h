#ifndef GEOMETRY_H
#define GEOMETRY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

typedef enum
{
    E_PlaneTestInvalid   = 0x0,
    E_PlaneTestFront     = 0x1,
    E_PlaneTestBack      = 0x2,
    E_PlaneTestIntersect = E_PlaneTestFront | E_PlaneTestBack
} E_PlaneTest;


class C_Plane;
class C_Line;
class C_Sphere;
class C_Cylinder;
class C_Capsule;
class C_AxisBox;



//
// -----------------------------------------------------------------------------------
// |-- Quadratic Result Class - Stores the two F32 results of a quadratic equation --|
// -----------------------------------------------------------------------------------
//

class C_QuadraticResult
{
public:
    
    /// Undefined Constructor
    NOALIAS C_QuadraticResult( const _UNDEFINED undefined );
    
    /// T0 & T1 Constructor : Ensures T0 is near & T1 is far
    NOALIAS C_QuadraticResult( const HF32 PCREF t0, const HF32 PCREF t1 );
    
    /// Accessor: Get result T0
    NOALIAS const HF32          T0() const __restrict;
    
    /// Accessor: Get result T1
    NOALIAS const HF32          T1() const __restrict;
    
    /// Returns true if T0 & T1 in valid range [ 0.0f, 1.0f ]
    NOALIAS const S32           Hit() const __restrict;
    
    /// Returns true if T0 in valid range [ 0.0f, 1.0f ]
    NOALIAS const S32           ValidT0() const __restrict;
    
    /// Returns true if T1 in valid range [ 0.0f, 1.0f ]
    NOALIAS const S32           ValidT1() const __restrict;
    
public:
    
    HF32                        m_T0;
    HF32                        m_T1;
};

/// Quadratic Equation Solver
NOALIAS __forceinline const C_QuadraticResult SolveQuadratic( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c );

/// Real Cubic Root: result = a ^ 1/3
NOALIAS __forceinline const HF32 RealCubeRoot( const HF32 PCREF a );

//
// ---------------------------------------------------------------------
// |-- C_Plane : Plane class stored as position & distance to origin --|
// ---------------------------------------------------------------------
//

class C_Plane
{
//friend const C_Plane Select( const Scalar_Bool& selector, const C_Plane& resultIfTrue, const C_Plane& resultIfFalse );
public:
    
    /// Default Constructor ( Zero normal & zero distance to origin! )
    NOALIAS __forceinline C_Plane();

	// Zero Init constructor.
	NOALIAS __forceinline C_Plane( _ZERO_INIT );

	// Undefined constructor.
	NOALIAS __forceinline C_Plane( _UNDEFINED );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Plane( const C_Plane& copy );
    
    #if defined( _SIMD_ )
    /// Explicit V128 Constructor
    NOALIAS __forceinline explicit C_Plane( const V128 src ) : m_V( src ) {}
    #endif
    
    /// Explicit HVec4 Constructor
    NOALIAS __forceinline explicit C_Plane( const HVec4 PCREF src );
    
    /// Normal & Vertex Constructor
    NOALIAS __forceinline C_Plane( const C_Normal PCREF normal, const C_Vertex PCREF position );
    
    /// Vertex Triple Constructor
    NOALIAS __forceinline C_Plane( const C_Vertex PCREF A, const C_Vertex PCREF B, const C_Vertex PCREF C );
    
    /// Cast to HVec4
    NOALIAS __forceinline operator const HVec4 () const __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Plane&            operator = ( const C_Plane& copy );
    
    /// Accessor: Get Normal
    NOALIAS __forceinline const C_Normal            Normal() const __restrict;
    
    /// Accessor: Get distance to origin
    NOALIAS __forceinline const HF32                DistanceToOrigin() const __restrict;
    
    /// Distance from the Plane to the input Vertex
    NOALIAS __forceinline const HF32                Distance( const C_Vertex PCREF vertex ) const __restrict;
    
    /// Unary Negation: Negate plane normal & distance from origin - plane still divides same space
    NOALIAS __forceinline const C_Plane             operator - () const __restrict;
    
    /// Returns copy of plane shifted along its normal by the given distance
    NOALIAS __forceinline const C_Plane             operator + ( const HF32 PCREF distance ) const __restrict;
    
    /// Returns copy of plane shifted back along its normal by the given distance
    NOALIAS __forceinline const C_Plane             operator - ( const HF32 PCREF distance ) const __restrict;
    
    /// Shifts the plane along its normal by the given distance
    NOALIAS __forceinline void                      operator += ( const HF32 PCREF distance ) __restrict;
    
    /// Shifts the plane back along its normal by the given distance
    NOALIAS __forceinline void                      operator -= ( const HF32 PCREF distance ) __restrict;
    
    /// Returns copy of plane rotated & translated by the given Matrix
    NOALIAS __forceinline const C_Plane             operator * ( const HMat43& m ) const __restrict;
    
    /// Returns copy of plane rotated & translated by the given Matrix
    NOALIAS __forceinline const C_Plane             operator * ( const HMat44& m ) const __restrict;
    
    /// Determines if the given Line intersects this Plane, returns the t value of the line intersect point
    NOALIAS __forceinline const HF32                Intersect( const C_Line& line ) const __restrict;
    
    /// Tests if the given Sphere is in front, behind or intersecting this Plane
    NOALIAS /*__forceinline*/ const E_PlaneTest     Test( const C_Sphere PCREF sphere, const HF32 PCREF tolerance = HF32( ZERO_INIT ) ) const __restrict;
    
    /// Tests if the given Box is in front, behind or intersecting this Plane
    NOALIAS /*__forceinline*/ const E_PlaneTest     Test( const C_AxisBox& box, const HF32 PCREF tolerance = HF32( ZERO_INIT ) ) const __restrict;
    
    /// Determines if the given Plane is Co-Planar with this Plane, within the given tolerances
	NOALIAS __forceinline const bool                CoPlanar( const C_Plane PCREF src, const HF32 PCREF normalTolerance = cos( Maths::Pi / 180.0f ), const HF32 PCREF offsetTolerance = 0.01f ) const __restrict;
    
    /// Comparison: Equal
    NOALIAS __forceinline const Vector4_Bool        operator == ( const C_Plane PCREF src ) const __restrict;
    
    /// Comparison: Not-Equal
    NOALIAS __forceinline const Vector4_Bool        operator != ( const C_Plane PCREF src ) const __restrict;
    
    /// Returns a copy of the given Vertex, translated along this Planes normal such that it lies on this Plane
    NOALIAS __forceinline const C_Vertex            OnPlane( const C_Vertex PCREF vertex ) const __restrict;
    
    /// Returns a copy of the given Vector, with any component in the direction of this Planes normal cancelled out
    NOALIAS __forceinline const C_Vector            ParallelToPlane( const C_Vector PCREF vector ) const __restrict;

	/// Flips the normal of the plane.
	NOALIAS __forceinline void						FlipNormal( ) __restrict;
    
    /// Read from Stream
    template < typename T_Stream > void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    HVec4       m_V;
};



//
// ------------------------------------------------------------
// |-- C_Line : Line class stored as start, normal & length --|
// ------------------------------------------------------------
//

class C_Line
{
//friend class C_Plane;
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Line();
    
    /// Constructor: Vertex Start, Normal & Length to End
    NOALIAS __forceinline C_Line( const C_Vertex PCREF start, const C_Normal PCREF normal, const HF32 PCREF length );
    
    /// Constructor: Vertex Start, Normal & Length to End
    NOALIAS __forceinline C_Line( const C_Vertex PCREF start, const C_PackedNormalLength PCREF normalLength );
    
    /// Constructor: Vertex Start & Vertex End
    NOALIAS __forceinline C_Line( const C_Vertex PCREF start, const C_Vertex PCREF end );
    
    /// Constructor: Vertex Start & Vector to End
    NOALIAS __forceinline C_Line( const C_Vertex PCREF start, const C_Vector PCREF vector );
    
    /// Accessor: Get Start
    NOALIAS __forceinline const C_Vertex              Start() const __restrict;
    
    /// Accessor: Get End
    NOALIAS __forceinline const C_Vertex              End() const __restrict;
    
    /// Accessor: Get Normal
    NOALIAS __forceinline const C_Normal              Normal() const __restrict;
    
    /// Accessor: Get Vector
    NOALIAS __forceinline const C_Vector              Vector() const __restrict;
    
    /// Accessor: Get Length
    NOALIAS __forceinline const HF32                  Length() const __restrict;
    
    /// Accessor: Get Reciprocal Length
    NOALIAS __forceinline const HF32                  ReciprocalLength() const __restrict;
    
    /// Set Length
    NOALIAS __forceinline void                        SetLength( const HF32 PCREF length ) __restrict;
    
    /// Calculate point on Line from T value
    NOALIAS __forceinline const C_Vertex              Point( const HF32 PCREF t ) const __restrict;
    
    /// Calculate point on Line from T value
    NOALIAS __forceinline const C_Vertex              operator () ( const HF32 PCREF t ) const __restrict;
    
    /// Intersection of two lines: Returns T value of intersection point on this line
    NOALIAS __forceinline const HF32                  Intersect( const C_Line& line2 ) const __restrict;
    
    /// Returns the distance between the closest points on the lines
    NOALIAS __forceinline const HF32                  Distance( const C_Line& line2 ) const __restrict;
    
    /// Returns T value of closest point on this Line to the given point
    NOALIAS __forceinline const HF32                  ClosestT( const C_Vertex PCREF point ) const __restrict;
    
    /// Returns the distance squared between the given point and the closest point to it on the line
    NOALIAS __forceinline const HF32                  DistanceSquared( const C_Vertex PCREF point ) const __restrict;
    
    /// Returns the distance between the given point and the closest point to it on the line
    NOALIAS __forceinline const HF32                  Distance( const C_Vertex PCREF point ) const __restrict;
    
    /// Returns T value of closest point on this line, closest point on the line & distance between closest point and given point
    NOALIAS __forceinline const HF32                  ClosestPoint( const C_Vertex PCREF point, C_Vertex& out_closest_point, C_Normal& out_normal, HF32& out_distance ) const __restrict;
    
    /// Read from Stream
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;
    
//private:
    
    C_VertexRcpLength           m_StartRcpLength;
    C_PackedNormalLength        m_NormalLength;
};






//
// ------------------------------------------------------------------------------
// |-- C_Sphere : Sphere class stored as a HVec4 with centre XYZ and radius W --|
// ------------------------------------------------------------------------------
//

PASSINREG class C_Sphere
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Sphere();
    
    #if defined( _SIMD_ )
    /// Explicit V128 Constructor
    NOALIAS __forceinline explicit C_Sphere( const V128 PCREF src );
    #endif
    
    /// Copy Constructor
    NOALIAS __forceinline C_Sphere( const C_Sphere& copy );
    
    /// Constructor: HF32 XYZ & Radius
    NOALIAS __forceinline C_Sphere( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z, const HF32 PCREF r );
    
    /// Constructor: Vertex & Radius
    NOALIAS __forceinline C_Sphere( const C_Vertex PCREF centre, const HF32 PCREF radius );
    
    /// Accessor: Returns the centre as a Vertex
    NOALIAS __forceinline const C_Vertex              Centre() const __restrict;
    
    /// Accessor: Returns the radius as a HF32
    NOALIAS __forceinline const HF32                  Radius() const __restrict;
    
    /// Line-Sphere Intersection
    NOALIAS __forceinline const C_QuadraticResult     Intersect( const C_Line& line ) const __restrict;
    
    /// Point inside sphere
    NOALIAS __forceinline const S32                   Inside( const C_Vertex PCREF point ) const __restrict;
    
    /// Sphere-Point Distance
    NOALIAS __forceinline const HF32                  Distance( const C_Vertex PCREF point ) const __restrict;
    
    /// Sphere-Sphere Distance
    NOALIAS __forceinline const HF32                  Distance( const C_Sphere PCREF sphere ) const __restrict;
    
    /// Sphere-Sphere Intersection
    NOALIAS __forceinline const Scalar_Bool           Intersect( const C_Sphere PCREF src ) const __restrict;
    
    /// Sphere * Rotation + Translation Matrix (Unit Axes assumed!)
    NOALIAS __forceinline const C_Sphere              operator * ( const HMat43& m ) const __restrict;

	/// Equality check for spheres.
	NOALIAS __forceinline const bool				  operator == ( const C_Sphere& rkSphere) const __restrict;

	/// In-equality check for spheres.
	NOALIAS __forceinline const bool				  operator != ( const C_Sphere& rkSphere) const __restrict;

	NOALIAS __forceinline const C_Sphere              Transformed( const C_Quaternion PCREF rotation, const C_Vector PCREF translation ) const __restrict;
    
    /// Finite check
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;

//private:
    
    // The sphere is stored with the centre in the XYZ components and the radius in the W component
    
    HVec4                               m_V;
};





//
// -----------------------------------
// |-- C_Cylinder : Cylinder class --|
// -----------------------------------
//

class C_Cylinder // TODO: Update this to use HF32 parameters & results instead of F32's
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Cylinder();
    
    /// Start + End + Radius Constructor
    NOALIAS __forceinline C_Cylinder( const C_Vertex PCREF start, const C_Vertex PCREF end, const HF32 PCREF radius );
    
    /// Start + Vector + Radius Constructor
    NOALIAS __forceinline C_Cylinder( const C_Vertex PCREF start, const C_Vector PCREF vector, const HF32 PCREF radius );
    
    /// Start + Normal + Length + Radius Constructor
    NOALIAS __forceinline C_Cylinder( const C_Vertex PCREF start, const C_Normal PCREF normal, const HF32 PCREF length, const HF32 PCREF radius );
    
    /// Accessor: Get Start
    NOALIAS __forceinline const C_Vertex            Start() const __restrict;
    
    /// Accessor: Get End
    NOALIAS __forceinline const C_Vertex            End() const __restrict;
    
    /// Accessor: Get Vector
    NOALIAS __forceinline const C_Vector            Vector() const __restrict;
    
    /// Accessor: Get Normal
    NOALIAS __forceinline const C_Normal            Normal() const __restrict;
    
    /// Accessor: Get Length
    NOALIAS __forceinline const HF32                Length() const __restrict;
    
    /// Accessor: Get Radius
    NOALIAS __forceinline const HF32                Radius() const __restrict;
    
    /// Returns copy of cylinder Rotated & Translated by given Matrix
    NOALIAS __forceinline const C_Cylinder          operator * ( const HMat43& rotTransMatrix ) const __restrict;
    
    /// Cylinder-Vertex Distance
    NOALIAS __forceinline const HF32                Distance( const C_Vertex PCREF vertex ) const __restrict;
    
    /// Cylinder-Line Intersection - Walls only - no hits on end caps
    NOALIAS /*__forceinline*/ const HF32            IntersectWalls( const C_Line& line, C_Normal& outputNormal ) const __restrict;
    
    /// Cylinder-Line Intersection - End caps only - no hits on walls
    NOALIAS __forceinline const HF32                IntersectEndCaps( const C_Line& line, C_Normal& outputNormal ) const __restrict;
    
    /// Cylinder-Line Intersection - Full: Combination of Walls + End Caps
    NOALIAS __forceinline const HF32                Intersect( const C_Line& line, C_Normal& outputNormal ) const __restrict;
    
    /// Read from Stream
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;

//private:
    
    C_Sphere                    m_StartRadius;
    C_PackedNormalLength        m_NormalLength;
};








//
// ---------------------------------
// |-- C_Capsule : Capsule class --|
// ---------------------------------
//

class C_Capsule
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Capsule();
    
    /// Start + End + Radius Constructor
    NOALIAS __forceinline C_Capsule( const C_Vertex PCREF start, const C_Vertex PCREF end, const HF32 PCREF radius );
    
    /// Start + Vector + Radius Constructor
    NOALIAS __forceinline C_Capsule( const C_Vertex PCREF start, const C_Vector PCREF vector, const HF32 PCREF radius );
    
    /// Start + Normal + Length + Radius Constructor
    NOALIAS __forceinline C_Capsule( const C_Vertex PCREF start, const C_Normal PCREF normal, const HF32 PCREF length, const HF32 PCREF radius );
    
    /// Accessor: Get Start
    NOALIAS __forceinline const C_Vertex              Start() const __restrict;
    
    /// Accessor: Get End
    NOALIAS __forceinline const C_Vertex              End() const __restrict;
    
    /// Accessor: Get Vector
    NOALIAS __forceinline const C_Vector              Vector() const __restrict;
    
    /// Accessor: Get Normal
    NOALIAS __forceinline const C_Normal              Normal() const __restrict;
    
    /// Accessor: Get Length
    NOALIAS __forceinline const HF32                  Length() const __restrict;
    
    /// Accessor: Get Radius
    NOALIAS __forceinline const HF32                  Radius() const __restrict;
    
    /// Matrix Multiplication
    NOALIAS __forceinline const C_Capsule             operator * ( const HMat43& rotTransMatrix ) const __restrict;
    
    /// Read from Stream
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;
    
//private:
    
    C_Sphere                    m_StartRadius;
    C_PackedNormalLength        m_NormalLength;
};







//
// ---------------------------------------------------
// |-- C_AxisBox : Axis Aligned Bounding Box class --|
// ---------------------------------------------------
//

class C_AxisBox
{
public:
    
    /// Typedef for the vertex array for all 8 vertices of the box
    typedef HVec3 C_VertexArray[8];
    
    /// Struct for the face indices into the vertex array of the box
    struct C_Face
    {
        NOALIAS __forceinline C_Face( const U8 v0, const U8 v1, const U8 v2, const U8 v3 );
        
        U8 m_V0;
        U8 m_V1;
        U8 m_V2;
        U8 m_V3;
    };
    
    /// Default Constructor: Sets invalid extents
	NOALIAS __forceinline C_AxisBox();
    
    /// Copy Constructor
    NOALIAS __forceinline C_AxisBox( const C_AxisBox& copy );
    
    /// HVec3 Min & Max Constructor
	NOALIAS __forceinline C_AxisBox( const HVec3 PCREF min, const HVec3 PCREF max );
    
    /// Vertex Pair Constructor
    NOALIAS __forceinline C_AxisBox( const C_Vertex PCREF A, const C_Vertex PCREF B );
    
    /// Vertex Triple Constructor
    NOALIAS __forceinline C_AxisBox( const C_Vertex PCREF A, const C_Vertex PCREF B, const C_Vertex PCREF C );

	 /// As above, but using HVec3 directly
    NOALIAS __forceinline C_AxisBox( const HVec3 PCREF A, const HVec3 PCREF B, const HVec3 PCREF C );
    
    /// Sphere Constructor
    NOALIAS __forceinline C_AxisBox( const C_Sphere PCREF sphere );
    
    /// Sphere Array Constructor
    NOALIAS __forceinline C_AxisBox( C_Sphere const * sphere, const U32 spheres );
    
    /// Vertex Array Constructor
    NOALIAS __forceinline C_AxisBox( C_Vertex const * vertex, const U32 vertices );
    
    /// Transformed Vertex Array Constructor
    NOALIAS __forceinline C_AxisBox( C_Vertex const * vertex, const U32 vertices, const HMat43& matrix );
    
    /// Capsule Constructor
    NOALIAS __forceinline C_AxisBox( const C_Capsule& capsule );
    
    /// Construct from Box3 storage
	NOALIAS __forceinline C_AxisBox( MathsStorage::Box3 const * const pStorage );

    /// Cast to Box3
	NOALIAS __forceinline operator const MathsStorage::Box3 () const __restrict;

    /// Accessor: Get Min
    NOALIAS __forceinline const HVec3               Min() const __restrict;
    
    /// Accessor: Get Max
    NOALIAS __forceinline const HVec3               Max() const __restrict;
    
    /// Accessor: Set Min
    NOALIAS __forceinline void                      Min( const HVec3 PCREF min ) __restrict;
    
    /// Accessor: Set Max
    NOALIAS __forceinline void                      Max( const HVec3 PCREF max ) __restrict;
    
    /// Get Centre of box
    NOALIAS __forceinline const HVec3               Centre() const __restrict;
    
    /// Get Diagonal of box ( min -> max )
    NOALIAS __forceinline const HVec3               Diagonal() const __restrict;
    
    /// Get Bounding Sphere
    NOALIAS __forceinline const C_Sphere            BoundingSphere() const __restrict;
    
    /// Set Zero extents
    NOALIAS __forceinline void                      SetZeroExtents() __restrict;
    
    /// Set invalid extents
    NOALIAS __forceinline void                      SetInvalidExtents() __restrict;
    
    /// Update extents with vertex
    NOALIAS __forceinline void                      UpdateExtents( const C_Vertex PCREF v ) __restrict;
    
    /// Update extents with another bounding box
    NOALIAS __forceinline void                      UpdateExtents( const C_AxisBox& box ) __restrict;
    
    /// Update extents with a sphere
    NOALIAS __forceinline void                      UpdateExtents( const C_Sphere PCREF sphere ) __restrict;   
    
    /// Update extents with a capsule
    NOALIAS __forceinline void                      UpdateExtents( const C_Capsule& capsule ) __restrict;   
    
    /// Determine if box inside another box
    NOALIAS __forceinline const U32                 Inside( const C_AxisBox& other ) const __restrict;
    
    /// Determine if point inside box
    NOALIAS __forceinline const U32                 Inside( const C_Vertex PCREF v ) const __restrict;
    
    /// Determine if sphere intersects this box : NOTE: not yet 100% accurate - may return true when sphere not actually overlapping corners of box!
    NOALIAS __forceinline const Scalar_Bool         Intersects( const C_Sphere PCREF sphere ) const __restrict;
    
    /// Determine if this box intersects another box
    NOALIAS __forceinline const Scalar_Bool         Intersects( const C_AxisBox& other ) const __restrict;

	/// Determine if this box intersects a triangle formed by the three points.
	//NOALIAS __forceinline const Scalar_Bool			Intersects( const C_Vertex& rkP1, const C_Vertex& rkP2, const C_Vertex& rkP3 ) const __restrict;
    
	/// Determine if this box intersects with a position (same as Inside function)
    NOALIAS __forceinline const Scalar_Bool         Intersects( const HVec3 PCREF other ) const __restrict;

    /// Split this box into 2 other boxes, in the position specified in the X axis
    NOALIAS __forceinline const bool                SplitAxisX( const HVecX PCREF xPos, C_AxisBox& front, C_AxisBox& back ) __restrict;
    
    /// Split this box into 2 other boxes, in the position specified in the Y axis
    NOALIAS __forceinline const bool                SplitAxisY( const HVecY PCREF yPos, C_AxisBox& front, C_AxisBox& back ) __restrict;
    
    /// Split this box into 2 other boxes, in the position specified in the Z axis
    NOALIAS __forceinline const bool                SplitAxisZ( const HVecZ PCREF zPos, C_AxisBox& front, C_AxisBox& back ) __restrict;
    
    /// Calculate half the surface area of the box
    NOALIAS __forceinline const HF32                HalfSurfaceArea() const __restrict;
    
    /// Calculate the surface area of the box
    NOALIAS __forceinline const HF32                SurfaceArea() const __restrict;
    
    /// Calculate the volume of the box
    NOALIAS __forceinline const HF32                Volume() const __restrict;
    
	//const HF32                DistanceTo( const C_Vertex point ) const __restrict; // clamp point to rectangle bounds, then do distance check

    /// Get Array of all 8 vertices of the box
    NOALIAS __forceinline void                      GetVertexArray( C_VertexArray& vertexArray ) const __restrict;
    
    /// Get the face vertex indices for the face with exterior normal pointing in the positive X direction
    NOALIAS __forceinline const C_Face              GetFacePosX() const __restrict;
    
    /// Get the face vertex indices for the face with exterior normal pointing in the negative X direction
    NOALIAS __forceinline const C_Face              GetFaceNegX() const __restrict;
    
    /// Get the face vertex indices for the face with exterior normal pointing in the positive Y direction
    NOALIAS __forceinline const C_Face              GetFacePosY() const __restrict;
    
    /// Get the face vertex indices for the face with exterior normal pointing in the negative Y direction
    NOALIAS __forceinline const C_Face              GetFaceNegY() const __restrict;
    
    /// Get the face vertex indices for the face with exterior normal pointing in the positive Z direction
    NOALIAS __forceinline const C_Face              GetFacePosZ() const __restrict;
    
    /// Get the face vertex indices for the face with exterior normal pointing in the negative Z direction
    NOALIAS __forceinline const C_Face              GetFaceNegZ() const __restrict;   

    /// Split this box into 2 other boxes, in arbitrary axis specified in axisBitMask (position must be zero in other axes!)
    NOALIAS __forceinline const bool                SplitAxis( const HVec3 PCREF pos, const Vector3_Bool PCREF axisSelector, C_AxisBox& resultFront, C_AxisBox& resultBack ) __restrict;
    
	template< typename Archive > __forceinline void Deserialise( Archive& ar ) __restrict;
	
	template< typename Archive > __forceinline void Serialise( Archive& ar ) __restrict;
    
//private:
    
	HVec3		                m_Min;
	HVec3		                m_Max;
};

//
// Non-Member, Non-Friend Support Functions
//

NOALIAS __forceinline const C_Plane Select( const Scalar_Bool PCREF selector, const C_Plane PCREF resultIfTrue, const C_Plane PCREF resultIfFalse );

NOALIAS const Scalar_Bool Intersect( const C_Line& line, const C_AxisBox& box, const HVec3 PCREF rcpLineVector, HF32& resultEnterT, HF32& resultExitT );

NOALIAS const Scalar_Bool Intersect( const C_Line& line, const C_AxisBox& box, HF32& resultEnterT, HF32& resultExitT );

/// Calculates the intersection point between 2 lines defined by A0->A1 & B0->B1. Returns 'true' if results are valid
NOALIAS __forceinline const Scalar_Bool IntersectLines(
    const C_Vertex PCREF    in_vertex_A0,           // Line A0->A1
    const C_Vertex PCREF    in_vertex_A1,           
    const C_Vertex PCREF    in_vertex_B0,           // Line B0->B1
    const C_Vertex PCREF    in_vertex_B1,           
    C_Vertex&               out_intersect_AB );     // Intersection point


/// Calculates the distance, t-values, closest points & intersection point between 2 lines defined by A0->A1 & B0->B1. Returns 'true' if results are valid
NOALIAS __forceinline const Scalar_Bool IntersectLines(
    const C_Vertex PCREF    in_vertex_A0,           // Line A0->A1
    const C_Vertex PCREF    in_vertex_A1,
    const C_Vertex PCREF    in_vertex_B0,           // Line B0->B1
    const C_Vertex PCREF    in_vertex_B1,
    HF32&                   out_distance,           // Distance between closest points on lines
    HF32&                   out_t_A,                // T-value of closest point on A0->A1
    HF32&                   out_t_B,                // T-value of closest point on B0->B1
    C_Vertex&               out_closest_A,          // Closest point on A0->A1
    C_Vertex&               out_closest_B,          // Closest point on B0->B1
    C_Vertex&               out_intersect_AB,       // Intersection point
    C_Normal&               out_normal );           // Normal: cross product of both lines


/// Calculate the cloest point on the line to the input point, returning the t-value for the closest point on the line
NOALIAS __forceinline const HF32 LineClosestPoint(
    const C_Vertex PCREF    x1,                 // In: Line x1->x2
    const C_Vertex PCREF    x2,
    const C_Vertex PCREF    p,                  // In: Point p
    C_Vertex&               out_closest_point,  // Out: Closest point on line x1->x2 to input point p
    C_Normal&               out_normal,         // Out: Normal from closest point on line x1->x2 to point p
    HF32&                   out_distance );     // Out: Distance between point p and closest point on line x1->x2


/// Calculate the line in point-normal form which is the intersection of the two planes
NOALIAS __forceinline const Maths::Scalar_Bool IntersectPlanes(
    const C_Plane PCREF     in_plane0,
    const C_Plane PCREF     in_plane1,
    C_Vertex&               out_point,
    C_Normal&               out_normal );

// Calculate the point at which three planes intersect
NOALIAS __forceinline const Maths::Scalar_Bool IntersectPlanes(
	const C_Plane PCREF     in_plane0,
	const C_Plane PCREF     in_plane1,
	const C_Plane PCREF     in_plane2,
	C_Vertex&			    out_point );

/// Determine if plane intersects sphere
NOALIAS __forceinline const Maths::Scalar_Bool IntersectPlaneSphere(
    const C_Plane PCREF     plane,
    const C_Sphere PCREF    sphere );

/// Determine if a sphere is above the plane
NOALIAS __forceinline const Maths::Scalar_Bool SphereAbovePlane(
    const C_Sphere PCREF    sphere,
    const C_Plane PCREF     plane );

/// Determine if a sphere is below the plane
NOALIAS __forceinline const Maths::Scalar_Bool SphereBelowPlane(
    const C_Sphere PCREF    sphere,
    const C_Plane PCREF     plane );

// Axis box support functions, these assume box min <= box max

// returns the longest axis of the box, also sets the direction of the longest axis and the length of the longest axis
NOALIAS const S32         GetLongestAxis( const C_AxisBox& box, C_Normal& longestAxis, HF32& longestAxisLength );

// returns a box which is the intersection of 'boxA' and 'boxB'
NOALIAS const C_AxisBox   Intersection( const C_AxisBox& boxA, const C_AxisBox& boxB );

// returns the result of clipping one box by another
NOALIAS const C_AxisBox   Clip( const C_AxisBox& box, const C_AxisBox& limit );

// returns the closest point to 'point' that is inside the box
NOALIAS const C_Vertex    Clip( const C_Vertex PCREF point, const C_AxisBox& box );

// returns the distance between 'box' and 'point'
NOALIAS const HF32        Distance( const C_Vertex PCREF point, const C_AxisBox& box );

// returns the distance between 'boxA' and 'boxB'
NOALIAS const HF32        Distance( const C_AxisBox& boxA, const C_AxisBox& boxB );

}; // namespace Maths



#endif

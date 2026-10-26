#ifndef UIPOLYGONCLASSES_H
#define UIPOLYGONCLASSES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

/// @addtogroup PolygonClasses
/// @{

/// default number of polygons to reserve
#define	MAX_NUM_POLYGONS		8		// -- default reserve amount - still dynamic
/// default number of points to reserve
#define	MAX_NUM_POINTS			512		// -- default reserve amount - still dynamic

/// @}

namespace UI
{

class C_UIResultPolygon;
class C_UIClipPolygon;
class C_AdobePolygon;

/// @addtogroup PolygonClasses
/// @{

// ---------------
// C_UIResultPoint
// ---------------
/// Simple holder class for a point, a vector and it's number of control points if it's a q-spline.
class C_UIResultPoint
{
public:
	/// default - zero.
	C_UIResultPoint()
		: m_Point( 0.0f, 0.0f, 0.0f ),
		m_Vector( 0.0f, 0.0f, 0.0f, 0.0f ),
		m_uNumCurvePoints( 0 )
	{
		//
	}

	/// set point and curve
	C_UIResultPoint( const F32 fX, const F32 fY, const U8 uNumCurvePoints )
		: m_Point( fX, fY, 0.0f ),
		m_Vector( 0.0f, 0.0f, 0.0f, 0.0f ),
		m_uNumCurvePoints( uNumCurvePoints )
	{
		//
	}
	
	/// set point
	C_UIResultPoint( const Maths::HVec2& p )
		: m_Point( p.X(), p.Y(), 0.0f ),
		m_Vector( 0.0f, 0.0f, 0.0f, 0.0f ),
		m_uNumCurvePoints( 0 )
	{
		//
	}

	/// set point and vector
	C_UIResultPoint( const Maths::HVec2& p, const Maths::HVec2& v )
		: m_Point( p.X(), p.Y(), 0.0f ),
		m_Vector( v.X(), v.Y(), 0.0f, 0.0f ),
		m_uNumCurvePoints( 0 )
	{
		//
	}

	/// set point and vector
	C_UIResultPoint( const Maths::HVec2& p, const Maths::HVec3& v )
		: m_Point( p.X(), p.Y(), 0.0f ),
		m_Vector( v.X(), v.Y(), v.Z(), 0.0f ),
		m_uNumCurvePoints( 0 )
	{
		//
	}

	/// set point and vector
	C_UIResultPoint( const Maths::HVec2& p, const Maths::HVec4& v )
		: m_Point( p.X(), p.Y(), 0.0f ),
		m_Vector( v.X(), v.Y(), v.Z(), v.W() ),
		m_uNumCurvePoints( 0 )
	{
		//
	}

	~C_UIResultPoint()					{  }

public:
	Maths::HVec2	GetPoint()			{ return Maths::HVec2( m_Point.X(), m_Point.Y() ); }	///< @a
	Maths::HVec3&	GetPoint3()			{ return m_Point; }										///< @a

	Maths::HVec2	GetVector()			{ return Maths::HVec2( m_Vector.X(), m_Vector.Y() ); }					///< @a
	Maths::HVec3	GetVector3()		{ return Maths::HVec3( m_Vector.X(), m_Vector.Y(), m_Vector.Z() ); }	///< @a
	Maths::HVec4&	GetVector4()		{ return m_Vector; }													///< @a


	F32				X()					{ return m_Point.X(); }									///< @a
	void			X( const F32 x )	{ m_Point.X( x ); }										///< @a

	F32				Y()					{ return m_Point.Y(); }									///< @a
	void			Y( const F32 y )	{ m_Point.Y( y ); }										///< @a

	F32				Z()					{ return m_Point.Z(); }									///< @a
	void			Z( const F32 z )	{ m_Point.Z( z ); }										///< @a

	F32				VX()				{ return m_Vector.X(); }								///< @a
	void			VX( const F32 x )	{ m_Vector.X( x ); }									///< @a

	F32				VY()				{ return m_Vector.Y(); }								///< @a
	void			VY( const F32 y )	{ m_Vector.Y( y ); }									///< @a

	F32				VZ()				{ return m_Vector.Z(); }								///< @a
	void			VZ( const F32 z )	{ m_Vector.Z( z ); }									///< @a

	F32				VW()				{ return m_Vector.W(); }								///< @a
	void			VW( const F32 w )	{ m_Vector.W( w ); }									///< @a

	U8				NumCurvePoints()	{ return m_uNumCurvePoints; }							///< @a

	void			NumCurvePoints( const U8 uNumCurvePoints )	{ m_uNumCurvePoints = uNumCurvePoints; }		///< @a

	/// set from another C_UIResultPoint
	void			Set( C_UIResultPoint& o )
					{
						m_Point.X( o.X() );
						m_Point.Y( o.Y() );
						m_Point.Z( o.Z() );
						m_Vector.X( o.VX() );
						m_Vector.Y( o.VY() );
						m_Vector.Z( o.VZ() );
						m_Vector.W( o.VW() );
						m_uNumCurvePoints = o.NumCurvePoints();
					}

	/// point-only comparision
	bool			operator == ( C_UIResultPoint& o )
					{
						return ( ( m_Point.X() == o.X() ) && ( m_Point.Y() == o.Y() ) && ( m_Point.Z() == o.Z() ) );
					}

	/// point-only comparision
	bool			operator != ( C_UIResultPoint& o )
					{
						return ( ( m_Point.X() != o.X() ) || ( m_Point.Y() != o.Y() ) || ( m_Point.Z() != o.Z() ) );
					}

private:
	Maths::HVec3		m_Point;
	Maths::HVec4		m_Vector;
	U8					m_uNumCurvePoints;
};

// -----------------
// C_UIResultPolygon
// -----------------
/// dynamic vector of C_UIResultPoint
typedef Btl::Vector< C_UIResultPoint >		ResultPointList;
typedef Btl::Vector< U16 >					ResultIndexList;


/// holds a list of C_UIResultPoint - assumes no self-intersections.
class C_UIResultPolygon
{
public:
	/// default - reserves the default amount of points.
	C_UIResultPolygon()
		: m_bHoleProcessed( false )
	{
		m_PointList.Reserve( MAX_NUM_POINTS );
	}

	/// construct from a C_UIClipPolygon
	C_UIResultPolygon( C_UIClipPolygon& p );

	/// free up the memory used by the list of points.
	~C_UIResultPolygon()
	{
		m_PointList.Destroy();
	}

public:
	bool				HoleProcessed()	const									{ return m_bHoleProcessed; }				///< @a
	void				HoleProcessed( const bool bHoleProcessed )				{ m_bHoleProcessed = bHoleProcessed; }		///< @a

	void				AddPoint( const Maths::HVec2& p, const Maths::HVec2& v ){ m_PointList.PushBack( C_UIResultPoint( p, v ) ); }	///< @a
	void				AddPoint( const Maths::HVec2& p, const Maths::HVec3& v ){ m_PointList.PushBack( C_UIResultPoint( p, v ) ); }	///< @a
	void				AddPoint( const Maths::HVec2& p, const Maths::HVec4& v ){ m_PointList.PushBack( C_UIResultPoint( p, v ) ); }	///< @a
	void				AddPoint( const Maths::HVec2& p )						{ m_PointList.PushBack( C_UIResultPoint( p ) ); }		///< @a
	void				AddPoint( const C_UIResultPoint& p )					{ m_PointList.PushBack( p ); }							///< @a

	void				SetPoint( const U32 uIndex, C_UIResultPoint& p )		{ m_PointList[ uIndex ] = p; }	///< @a

	U32					NumPoints() const										{ return m_PointList.Size(); }	///< @a

	bool				CW();	///< winding order of this polygon
	void				ReverseWindingOrder();	///< CW => CCW, CCW => CW

	ResultPointList&	GetPointList()											{ return m_PointList; }	///< get the point list.

	C_UIResultPoint&	GetPoint( U32 uIndex )									{ return m_PointList[ uIndex ]; }	///< element access.

	C_UIResultPoint&	operator [] ( U32 uIndex )								{ return m_PointList[ uIndex ]; }	///< element access.

	Maths::HVec2		GetOutlinePoint( S32 sIndex );

private:
	bool				m_bHoleProcessed;
	ResultPointList		m_PointList;
};

/// dynamic vector of C_UIResultPolygon
typedef Btl::Vector< C_UIResultPolygon >		UIPolygonList;

// -----------------
// C_ResultIndexList
// -----------------
/// Holds a list of indices that reference a C_UIResultPolygon
class C_ResultIndexList
{
public:
	/// default - reserve the index count
	C_ResultIndexList()	
		: m_bOk( false )
	{
		m_IndexList.Reserve( MAX_NUM_POINTS * 3 );
	}

	/// free up the list
	~C_ResultIndexList()
	{
		m_IndexList.Clear();
	}

public:
	bool				Ok()									{ return m_bOk; }	///< @a
	void				Ok( const bool bOk )					{ m_bOk = bOk; }	///< @a

	void				AddIndex( const U16 sIndex )			{ m_IndexList.PushBack( sIndex ); }	///< @a
	S32					NumIndices()							{ return m_IndexList.Size(); }	///< @a

	ResultIndexList&	GetIndicies()							{ return m_IndexList; }	///< get the point list.

	S32					operator [] ( const U16 sIndex )		{ return m_IndexList[ sIndex ]; }	///< element access

private:
	bool						m_bOk;
	ResultIndexList				m_IndexList;
};

/// dynamic vector of C_ResultIndexList
typedef Btl::Vector< C_ResultIndexList >		UIPolygonIndexList;

// ---------------
// Clip Operations
// ---------------
/// Clip operations we support
enum E_UIPolyClip
{
	E_UIPolyClip_Intersection = 0,	///< @a
	E_UIPolyClip_AMinusB,			///< @a
	E_UIPolyClip_BMinusA,			///< @a
	E_UIPolyClip_Union,				///< @a
	E_UIPolyClip_Exclude,			///< @a

	E_UIPolyClip_Count,
};

// --------------
// C_UIClipVertex
// --------------
/// A vertex class used internally by the ClipOperation algorithm.
class C_UIClipVertex
{
public:
	C_UIClipVertex();	///< default - zero everything.
	C_UIClipVertex( const Maths::HVec2& pos );	///< point construct.
	C_UIClipVertex( const F32 x, const F32 y );	///< point construct.
	~C_UIClipVertex();

public:
	void				Reset();	///< zero everything apart from the point.
	bool				operator == ( const C_UIClipVertex& o );	///< point-only comparision
	bool				operator != ( const C_UIClipVertex& o );	///< point-only comparision

public:
	Maths::HVec2		m_Pos;
	C_UIClipVertex*		m_pNext;
	C_UIClipVertex*		m_pPrev;
	bool				m_bIntersect;
	C_UIClipVertex*		m_pNeighbour;
	F32					m_fAlpha;
	bool				m_bEntryStatus;
	bool				m_bProcessed;
};

// ---------------
// C_UIClipPolygon
// ---------------
class C_UIClipPolygon
{
public:
	C_UIClipPolygon( const bool bCleanup = true );	///< default = @e bCleanup states whether we're new-ing things in this instance
	C_UIClipPolygon( C_UIResultPolygon& p );	///< construct from a C_UIResultPolygon
	~C_UIClipPolygon();	///< cleanup if required

public:
	void				Reset();	///< remove all of the intersect vertices so we're just left with the original polygon.
	void				ResetProcessed();	///< reset all of the m_bProcessed flags

	void				AddVertex( const F32 x, const F32 y );	///< @a
	void				AddVertex( const Maths::HVec2& pos );	///< @a
	void				AddVertex( C_UIClipVertex* pVertex );	///< @a

	void				CreateInitialNextAndBackLinks();	///< link it all together
	void				AddAndSortInto( C_UIClipVertex* pAddVertex, C_UIClipVertex* pStart, C_UIClipVertex* pEnd );	///< @a add and sort in an intersection vertex based on it's alpha value

//	void				DebugRender( const Maths::RGBA c1, const Maths::HVec2 offset = Maths::HVec2( 0.0f, 0.0f ), const bool bGradient = false );	///< @a

	C_UIClipVertex*		operator [] ( S32 sIndex );	///< element access

	C_UIClipVertex*		GetVertex( S32 sIndex );	///< @a

	Btl::Vector< C_UIClipVertex* >&	GetVertices()			{ return m_Vertices; }	///< get the vertex list

	S32					NumVertices()						{ return m_Vertices.Size(); }	///< @a

private:
	bool								m_bCleanup;
	S32									m_sNumOriginalVertices;		// intersect vertices are always added to the end
	Btl::Vector< C_UIClipVertex* >		m_Vertices;
};

/// @}

// --------------------------------------------------------------------------
// meh - adding these in here as it's used in conjuction with the above class
// --------------------------------------------------------------------------

/// @addtogroup Maths
/// @{

/// 2D line intersection
/// @param a - 0-1 value modifed by function - portion of p2-p1 after p1 where the intersection occurs.
/// @param b - 0-1 value modifed by function - portion of q2-q1 after q1 where the intersection occurs.
/// @param bSegmentTest - if true, only allowing intersections between the two vectors - else allow intersect range outside of 0-1.
/// @return false if lines are parallel or intersect outside of boundaries if it's a segment test - true otherwise.
/// @remarks - only use the returned @e a and @e b values if this returns <b>true</b>
bool	LineIntersect( const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& q1, const Maths::HVec2& q2, F32& a, F32& b, const bool bSegmentTest = true );

/// Determines if a 2D point is inside a 2D polygon.
/// @remarks - assumes polygon is not self-intersecting.
bool	IsVertexInsidePoly( const Maths::HVec2& p1, C_UIResultPolygon& poly );

/// Determines if a 2D point is inside a 2D polygon.
/// @remarks - assumes polygon is not self-intersecting.
bool	IsVertexInsidePoly( const C_UIClipVertex* pVertex, C_UIClipPolygon& poly );

/// @}

};	// namespace UI

#endif	/* UIPOLYGONCLASSES_H */

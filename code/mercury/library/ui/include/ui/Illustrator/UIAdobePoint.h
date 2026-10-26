
#ifndef UI_UIADOBEPOINT_H
#define UI_UIADOBEPOINT_H

namespace UI
{

/// @addtogroup Illustrator
/// @{

class C_AdobePoint;

/// UtVector to store a list of C_AdobePoints.
typedef Btl::Vector< C_AdobePoint > AdobePointList;

//-------------------------------------------------------------------
// Class : C_AdobePoint
//-------------------------------------------------------------------
/// Point class for polygons read in from .ipf files.
class C_AdobePoint
{
public:
	// Constructors
	C_AdobePoint();									///< @a
	C_AdobePoint( const Maths::HVec2& pos );				///< @a
	C_AdobePoint( const F32 fX, const F32 fY );					///< @a
	C_AdobePoint( const Maths::HVec2& vPos, const Maths::HVec2& vIn, const Maths::HVec2& vOut, const bool bCorner );		///< @a

	C_AdobePoint( const Maths::HVec2& vPos, Maths::HVec2& vUV ); ///< @a

	// Destructor
	~C_AdobePoint();								///< @a

	bool operator == ( const C_AdobePoint& o );		///< @a

	bool operator != ( const C_AdobePoint& o );		///< @a

public:
	Maths::HVec2		m_Pos;					///< @a
	Maths::HVec2		m_Normal;				///< @a
	Maths::HVec2		m_UV;					///< @a

	Maths::HVec2		m_In;					///< @a
	Maths::HVec2		m_Out;					///< @a
	bool				m_Corner;				///< @a

	/// Used for spline data.
	F32					m_Length;				///< Length of arc.
	F32					m_DistFromStart;		///< Distance from spline start.
	F32					m_Time;					///< Distance from spline start 0 - 1.
};

/// @}

} // namespace UI


#endif // UI_UIADOBEPOINT_H

#ifndef GEOMETRY_2D_H
#define GEOMETRY_2D_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

class C_Rect;
class C_Transform2d;

//
// ---------------------------------------------
// |-- C_Rect : Axis Aligned Rectangle class --|
// ---------------------------------------------
//

class C_Rect
{
public:
        
    /// Default Constructor: Sets invalid extents
	C_Rect();
    
    /// Copy Constructor
    C_Rect( const C_Rect& copy );
    
    /// HVec2 Min & Max Constructor
	C_Rect( const HVec2 PCREF min, const HVec2 PCREF max );

	/// SetDimensions sets up C_Rect from min x,y position and with/height
	void SetDimensions( const F32 x, const F32 y, const F32 width, const F32 height );

	/// SetBounds sets up C_Rect from min and max extents
	void SetBounds( const F32 xmin, const F32 ymin, const F32 xmax, const F32 ymax );

	/// SetDimensions sets up C_Rect from min x,y position and with/height
	void SetDimensions( const HVec2 PCREF min, const HVec2 PCREF size );

	/// SetBounds sets up C_Rect from min and max extents
	void SetBounds( const HVec2 PCREF min, const HVec2 PCREF max );

	/// Accessor: Get Min
    const HVec2                 Min() const;
    
    /// Accessor: Get Max
    const HVec2                 Max() const;
    
    /// Accessor: Set Min
    void                        Min( const HVec2 PCREF min );
    
    /// Accessor: Set Max
    void                        Max( const HVec2 PCREF max );
    
    /// Get Centre of box
    const HVec2                 Centre() const;
    
    /// Get Diagonal of box ( min -> max )
    const HVec2                 Size() const;
    
    /// Set Zero extents
    void                        SetZeroExtents();
    
    /// Set invalid extents
    void                        SetInvalidExtents();
    
    ///// Update extents with vertex
    //void                        UpdateExtents( const Maths::HVec2& v );
    //
    ///// Update extents with another rectangle
    //void                        UpdateExtents( const C_Rect& box );
    //    
    ///// Determine if rect inside another rect
    //NOALIAS const U32           Inside( const C_Rect& other ) const __restrict;
    //
    ///// Determine if point inside rect
    //const U32                   Inside( const Maths::HVec2& v ) const;

    ///// Calculate the area of the rect
    //const HF32                  Area() const;
    
	//const HF32                  DistanceTo( const Maths::HVec2& point ) const; // clamp point to rectangle bounds, then do distance check

	template< typename Archive > 
	void						Deserialise( Archive& ar );
	
	template< typename Archive > 
	void						Serialise( Archive& ar );
    
private:
    
	HVec2		                m_Min;
	HVec2		                m_Max;
};

const Maths::Scalar_Bool Intersect(const C_Rect& r1, const C_Rect& r2);

//
// ---------------------------------------------
// |-- C_Transform2d : Axis Aligned Rectangle class --|
// ---------------------------------------------
//

/// a scale factor and offset to go from one co-ordinate system to another.
/// Allows multiplication (put one frame in another) and inverse operations.
class C_Transform2d
{
public:
							/// Constructor. Transform2d will be identity. (scale = 1, offset = 0).
							C_Transform2d();

							/// Construct from C_Rect (transforms from 0->1 to the C_Rect dimensions).
							C_Transform2d(const Maths::C_Rect& frame);

							/// Concatentate the frame transforms
	C_Transform2d			operator *(const C_Transform2d& input );

							/// Set the identity transform
	void					Identity();

							/// Invert the transform
	void					Invert();

							/// Apply the transform to a 2d vector
	const Maths::HVec2		Apply( const HVec2 PCREF src) const;

							/// Apply the inverse transform to a 2d vector
	const Maths::HVec2		ApplyInverse( const HVec2 PCREF src) const;

    const Maths::HVec2&     Scale() const;

    void					Scale( const HVec2 PCREF scale );

    const Maths::HVec2&     Offset() const;

    void					Offset( const HVec2 PCREF offset);

private:
	Maths::HVec2			m_Scale;
	Maths::HVec2			m_Offset;
};

}; // namespace Maths



#endif

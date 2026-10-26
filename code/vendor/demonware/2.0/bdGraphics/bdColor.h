// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provides a platform independent class to represent a color

#ifndef BD_COLOR_H
#define BD_COLOR_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdVector4.h>

/// A color data structure. Provides the 4 components of color
class bdColor
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Empty Constructor.
	inline bdColor();

	/// Construct a color with red, green and blue components.
	/// Sets the alpha value to opaque.
	/// \param red[in] The value for the red component of the color.
	/// \param green[in] The value for the green component of the color.
	/// \param blue[in] The value for the blue component of the color.
	inline bdColor(	bdUByte8 red,
					bdUByte8 green,
					bdUByte8 blue);

	/// Construct a color with alpha (transparency), red, green and blue components.
	/// \param alpha[in] The value for the transparency of the color. Higher is more opaque.
	/// \param red[in] The value for the red component of the color.
	/// \param green[in] The value for the green component of the color.
	/// \param blue[in] The value for the blue component of the color.
	inline bdColor(	bdUByte8 alpha,
					bdUByte8 red,
					bdUByte8 green,
					bdUByte8 blue);

	/// Construct a color from another color object.
	/// \param other[in] The color to create this color from.
	inline bdColor(const bdColor &other);

	/// Construct a color from a four-element vector object.
	/// \param other[in] The color to create this color from.
	/// - X is red.
	/// - Y is green.
	/// - Z is blue.
	/// - W is alpha.
	inline bdColor(const bdVector4 &other);

	/// Constructor from a bdUInt32.
	/// \param other[in] The color to create this color from.
	inline bdColor(bdUInt32 other);

	/// \return The value of the alpha (transparency) component of the color.
	inline bdUByte8 getAlpha() const;

	/// \return The value of the red component of the color.
	inline bdUByte8 getRed() const;

	/// \return The value of the green component of the color.
	inline bdUByte8 getGreen() const;

	/// \return The value of the blue component of the color.
	inline bdUByte8 getBlue() const;

	/// Change the transparency of the color.
	/// \param alpha[in] The new value for the alpha component.
	inline void setAlpha(const bdUByte8 alpha);

	/// Change the red component of the color.
	/// \param red[in] The new value for the red component.
	inline void setRed(const bdUByte8 red);

	/// Change the green component of the color.
	/// \param green[in] The new value for the green component.
	inline void setGreen(const bdUByte8 green);

	/// Change the blue component of the color.
	/// \param blue[in] The new value for the blue component.
	inline void setBlue(const bdUByte8 blue);

	/// bdUInt32 conversion operator. Used for packing the color into a bdVertex.
	/// \return The components of the color packed into a bdUInt32.
	inline operator bdUInt32() const;

	/// Convert the color to an array of bdFloat32s. The values are normalized so that each component is between
	/// 0.0 and 1.0.
	/// \return The components of the color packed into an array of bdFloat32s.
	/// # color[0] is the red component
	/// # color[1] is the green component
	/// # color[2] is the blue component
	/// # color[3] is the alpha component
	inline bdFloat32 *asFloat(bdFloat32 color[4]) const;

	/// \param color[in] The color parameter.
	/// \return The red component of a color packed into a bdUInt32.
	static inline bdUByte8 getRed(bdUInt32 color);

	/// \param color[in] The color parameter.
	/// \return The green component of a color packed into a bdUInt32.
	static inline bdUByte8 getGreen(bdUInt32 color);

	/// \param color[in] The color parameter.
	/// \return The blue component of a color packed into a bdUInt32.
	static inline bdUByte8 getBlue(bdUInt32 color);

	/// \param color[in] The color parameter.
	/// \return The alpha component of a color packed into a bdUInt32.
	static inline bdUByte8 getAlpha(bdUInt32 color);

protected:
	/// The transparency of the color. Zero is transparent. Bigger values are more opaque.
	bdUByte8 m_alpha;

	/// The amount of red in the color.
	bdUByte8 m_red;

	/// The amount of green in the color.
	bdUByte8 m_green;

	/// The amount of blue in the color.
	bdUByte8 m_blue;

	/// Specifies how many bytes to shift a color component by to place it in its correct position within a bdUInt32
#ifdef BD_BIG_ENDIAN
	enum bdColorShift
	{
		BD_COLOR_ALPHA	= 0,
		BD_COLOR_RED	= 8,
		BD_COLOR_GREEN	= 16,
		BD_COLOR_BLUE	= 24
	};
#else
	enum bdColorShift
	{
		BD_COLOR_ALPHA	= 24,
		BD_COLOR_RED	= 16,
		BD_COLOR_GREEN	= 8,
		BD_COLOR_BLUE	= 0
	};
#endif

	static inline bdUByte8 extractColor(bdUInt32 color, bdColorShift component);
};

#include <bdGraphics/bdColor.inl>

#endif //BD_COLOR_H

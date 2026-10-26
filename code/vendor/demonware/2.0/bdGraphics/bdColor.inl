// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifdef BD_PLATFORM_WIN32
#include <bdGraphics/bdGfxLib.h>
#endif //BD_PLATFORM_WIN32

inline
bdColor::bdColor()
{
	m_alpha = 255;
	m_red = 0;
	m_green = 0;
	m_blue = 0;
}

inline
bdColor::bdColor(bdUByte8 r, bdUByte8 g, bdUByte8 b)
{
	m_alpha = 255;
	m_red = r;
	m_green = g;
	m_blue = b;
}

inline
bdColor::bdColor(bdUByte8 a, bdUByte8 r, bdUByte8 g, bdUByte8 b)
{
	m_alpha = a;
	m_red = r;
	m_green = g;
	m_blue = b;
}

inline
bdColor::bdColor(const bdColor &other)
{
	m_alpha = other.m_alpha;
	m_red = other.m_red;
	m_green = other.m_green;
	m_blue = other.m_blue;
}

inline
bdColor::bdColor(const bdVector4 &other)
{
	m_alpha = static_cast<bdUByte8>(other.getW() * 255.0f);
	m_red = static_cast<bdUByte8>(other.getX() * 255.0f);
	m_green = static_cast<bdUByte8>(other.getY() * 255.0f);
	m_blue = static_cast<bdUByte8>(other.getZ() * 255.0f);
}

inline
bdColor::bdColor(bdUInt32 other)
{
	m_alpha = getAlpha(other);
	m_red = getRed(other);
	m_green = getGreen(other);
	m_blue = getBlue(other);
}

inline
bdUByte8 bdColor::getAlpha() const
{
	return m_alpha;
}

inline
bdUByte8 bdColor::getRed() const
{
	return m_red;
}

inline
bdUByte8 bdColor::getGreen() const
{
	return m_green;
}

inline
bdUByte8 bdColor::getBlue() const
{
	return m_blue;
}

inline
void bdColor::setAlpha(const bdUByte8 alpha)
{
	m_alpha = alpha;
}

inline
void bdColor::setRed(const bdUByte8 red)
{
	m_red = red;
}

inline
void bdColor::setGreen(const bdUByte8 green)
{
	m_green = green;
}

inline
void bdColor::setBlue(const bdUByte8 blue)
{
	m_blue = blue;
}

inline
bdColor::operator bdUInt32() const
{
	bdUInt32 color;

	// The color components are in different orders depending on whether we are using D3D or OpenGL
#if defined BD_PLATFORM_XENON || defined BD_PLATFORM_XBOX || defined BD_WIN32_DIRECTX
	color = m_alpha;
	color <<= 8;
	color |= m_red;
	color <<= 8;
	color |= m_green;
	color <<= 8;
	color |= m_blue;
#elif defined BD_WIN32_GL || defined BD_PLATFORM_PSP || defined BD_PLATFORM_PS2
	color = m_alpha;
	color <<= 8;
	color |= m_blue;
	color <<= 8;
	color |= m_green;
	color <<= 8;
	color |= m_red;
#else

#endif // BD_PLATFORM_*

	return color;
}

inline
bdFloat32 *bdColor::asFloat(bdFloat32 color[4]) const
{
	if (color)
	{
		color[0] = static_cast<bdFloat32>(m_red) / 255.0f;
		color[1] = static_cast<bdFloat32>(m_green) / 255.0f;
		color[2] = static_cast<bdFloat32>(m_blue) / 255.0f;
		color[3] = static_cast<bdFloat32>(m_alpha) / 255.0f;
	}

	return color;
}

inline
bdUByte8 bdColor::extractColor(bdUInt32 color, bdColorShift component)
{
	return static_cast<bdUByte8>((color >> component) & 0xff);
}

inline
bdUByte8 bdColor::getRed(bdUInt32 color)
{
	return extractColor(color, BD_COLOR_RED);
}

inline
bdUByte8 bdColor::getGreen(bdUInt32 color)
{
	return extractColor(color, BD_COLOR_GREEN);
}

inline
bdUByte8 bdColor::getBlue(bdUInt32 color)
{
	return extractColor(color, BD_COLOR_BLUE);
}

inline
bdUByte8 bdColor::getAlpha(bdUInt32 color)
{
	return extractColor(color, BD_COLOR_ALPHA);
}

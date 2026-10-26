#ifndef RENDER_FONT_H
#define RENDER_FONT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"
#include "Display/texture/Texture2D.h"
#include "Display/Buffer/VertexDeclaration.h"
#include <btl/containers/Vector.h>
#include <btl/containers/List.h>

namespace Render
{
namespace Font
{

class C_FontResource;
class C_FontRendered;

enum E_Justification
{
	E_Justification_Left = 0,
	E_Justification_Centre,
	E_Justification_Right,
	E_Justification_Justify,

	E_Justification_Count
};

enum E_Version
{
	E_Version_Invalid = 0,
	E_Version_First,
	E_Version_Latest = E_Version_First
};

enum E_Format
{
	E_Format_PC = 0,
	E_Format_XB2,
	E_Format_PS3,

    E_Format_Count
};

struct C_CharacterRect
{ 
	F32 m_UMin;
	F32 m_VMax;
	F32 m_UMax;
	F32 m_VMin;
};

class C_CharInfo
{
public:

	const U16				Unicode() const						{ return m_Unicode; }
	const U16				TexturePage() const					{ return m_TexturePage; }
	const F32				Offset() const						{ return m_Offset; }
	const F32				Height() const						{ return m_Height; }
	const F32				Width() const						{ return m_Width; }
	const C_CharacterRect	Rect() const						{ return m_Rect; }

    void	                Unicode(const U16 src)				{ m_Unicode = src; }
	void	                TexturePage( const U16 src )		{ m_TexturePage = src; }
	void	                Offset( const F32 src )		        { m_Offset = src; }
	void	                Height( const F32 src )				{ m_Height = src; }
	void	                Width( const F32 src )				{ m_Width = src; }
	void	                Rect( const C_CharacterRect src )	{ m_Rect = src; }

	
	template < typename T_Stream >
    void Deserialise( T_Stream& stream )
	{
		stream >> m_Unicode;
		stream >> m_TexturePage;
		stream >> m_Offset;
		stream >> m_Height;
		stream >> m_Width;
		stream >> m_Rect.m_UMin;
		stream >> m_Rect.m_VMax;
		stream >> m_Rect.m_UMax;
		stream >> m_Rect.m_VMin;
	}

	template < typename T_Stream >
    void Serialise( T_Stream& stream )
	{
		stream << m_Unicode;
		stream << m_TexturePage;
		stream << m_Offset;
		stream << m_Height;
		stream << m_Width;
		stream << m_Rect.m_UMin;
		stream << m_Rect.m_VMax;
		stream << m_Rect.m_UMax;
		stream << m_Rect.m_VMin;
	}
	
private:

	U16				m_Unicode;		///< Unicode value of the Character
    U16				m_TexturePage;	///< Which Texture Page the Character is on
    F32				m_Offset;		///< (0.0->1.0) - Dist from Top of Row down to Top of Character
	F32				m_Height;		///< (0.0->1.0) - Dist from Base of Char up to Top of Character
    F32				m_Width;		///< (0.0->1.0) - Relative Width of Character, 1.0 = Widest
    C_CharacterRect m_Rect;			///< Coord Extents of the Character Rectangle within the Texture Page
};


//Object to render a font
class C_FontRenderer
{
	//Font style
	F32							m_SpaceWidth;		///< The Width of a "space", ' ', Character
	F32	 						m_SpacingH;			///< Spacing Distance between subsequent Characters
	F32							m_SpacingV;			///< Spacing Distance between subsequent Rows of Characters
	F32 						m_ScaleX;			///< Width Scale Factor
	F32 						m_ScaleY;			///< Height Scale Factor
	F32							m_Feather;			///< Feather

	bool						m_ShadowEnable;		///< Shadow
	float						m_ShadowOffset;		///< Font shadow offset, 1.f is one character
	
	Maths::RGBA					m_ColourTop;		///< Colour to Shade from Top of Character
	Maths::RGBA					m_ColourBottom;		///< Colour to Shade from Bottom of Character
	
	Maths::RGBA					m_ColourOutline;	///< Colour to Shade border of Character
	float						m_OutlineSize;		///< Outline size

	//Font Resource
	C_FontResource*				m_pFontResource;
	
	friend class C_FontSystem;

	C_FontRenderer();
public:
	//Style
	F32 GetSpaceWidth() const { return m_SpaceWidth;};
	F32 GetSpacingH() const { return m_SpacingH;};
	F32 GetSpacingV() const { return m_SpacingV;};
	F32 GetScaleX() const { return m_ScaleX;};
	F32 GetScaleY() const { return m_ScaleY;};
	F32 GetFeather() const { return m_Feather;};
	bool GetShadowEnable() const { return m_ShadowEnable;};
	F32 GetShadowOffset() const { return m_ShadowOffset;};
	Maths::RGBA GetColourTop() const { return m_ColourTop;};
	Maths::RGBA GetColourBottom() const { return m_ColourBottom;};
	Maths::RGBA GetColourOutline() const { return m_ColourOutline;};
	F32 GetOutlineSize() const { return m_OutlineSize;};

	void SetToDefaults();
	void SetSpaceWidth(F32 spaceWidth = 0.4f) { m_SpaceWidth = spaceWidth;};
	void SetSpacingH(F32 spacingH = 0.0f) { m_SpacingH = spacingH;};
	void SetSpacingV(F32 spacingV = 0.9f) { m_SpacingV = spacingV;};
	void SetScaleX(F32 scaleX = 24.0f) { m_ScaleX = scaleX;};
	void SetScaleY(F32 scaleY = 24.0f) { m_ScaleY = scaleY;};
	void SetFeather(F32 feather = 2.0f) { m_Feather = feather;};
	void SetShadowEnable(bool shadowEnable = false) { m_ShadowEnable = shadowEnable;};
	void SetShadowOffset(F32 shadowOffset = 0.07f) { m_ShadowOffset = shadowOffset;};
	void SetColourTop(Maths::RGBA colourTop = Maths::RGBA(0xff, 0xff, 0xff, 0xff)) { m_ColourTop = colourTop;};
	void SetColourBottom(Maths::RGBA colourBottom = Maths::RGBA(0xc8, 0xc8, 0xff, 0xff)) { m_ColourBottom = colourBottom;};
	void SetColourOutline(Maths::RGBA colourOutline = Maths::RGBA(0x0, 0x0, 0x0, 0xff)) { m_ColourOutline = colourOutline;};
	void SetOutlineSize(F32 outlineSize = 0.f) { m_OutlineSize = outlineSize;};
	
	//Get Height of the font
	F32				GetHeight() {return (m_ScaleY * (1.0f + m_SpacingH));}
	//Get total dimension for a text
	Maths::HVec2	GetDimensions(U32 nWrapWidth, const char *text);

	//Print text
	void Print(const Maths::HVec2 &vPos, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0);
	
	//Print 3D
	void Print3D(const Maths::HVec3 &vPos, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0);
	void Print3D(const Maths::HVec3 &vPos, const F32 size, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0);
};

struct VertexFont
{
	F32 px, py;
    F32 cx, cy, cz, cw;
	F32 u, v;

	void Pos(const F32 x, const F32 y) { px = x; py = y; }
	void Pos(const Maths::HVec2& p) { px = p.X(); py = p.Y(); }
	void Col(const F32 x, const F32 y, const F32 z, const F32 w) { cx = x; cy = y; cz = z; cw = w; }
	void Col(const Maths::RGBA c) { cx = c.R() / 255.f; cy = c.G() / 255.f; cz = c.B() / 255.f; cw = c.A() / 255.f; }
	void UV(const F32 U, const F32 V) { u = U; v = V; }

};

class C_FontResource: public Btl::ListElementDouble<C_FontResource>
{
	Util::C_String			m_FontFile;

	Display::C_Texture2D*	m_pTexturePages;
	U16						m_pCharLookup[65536]; //Fast access (maybe to much)
	C_CharInfo*				m_pCharInfo;

	E_Format			    m_Format;
	U16					    m_CharacterCount;
	U16						m_TexturePageCount;


	C_FontResource();
	virtual ~C_FontResource();

	bool					Load();
	F32		                GetCharWidth(U16 character) const;
	
	//Reuse vertex memory used to group the characters
	Btl::Vector<VertexFont> m_FontVertexMemory[256];

	void DrawLine(const Maths::HVec2 &vPos, E_Justification justification, U32 nWrapWidth, const U16 *text, F32 spaceWidth, F32 spacingH, F32 spacingV, F32 scaleX, F32 scaleY, F32 startFontBorder, F32 feather, Maths::RGBA colourTop, Maths::RGBA colourBottom, F32 fLineLength, S32 nSpacesCount);
	void DrawText(const Maths::HVec2 &vPos, E_Justification justification, U32 nWrapWidth, const char *text, F32 spaceWidth, F32 spacingH, F32 spacingV, F32 scaleX, F32 scaleY, F32 startFontBorder, F32 feather, Maths::RGBA colourTop, Maths::RGBA colourBottom, Display::E_BlendMode blendMode = Display::E_BlendMode_Alpha );

	friend class C_FontSystem;
	friend class C_FontRenderer;

public:
	void Destroy();
};

class C_FontSystem : public Display::I_Renderer
{
public:

	C_FontSystem();
	~C_FontSystem();

	bool OnCreate();
	void OnDestroy();
	
	//Get FontRenderer
	C_FontRenderer* GetFontRenderer(const char* font);
	
	friend class C_FontResource;
private:
	 
	void SetRenderState(float startFontBorder, float alphaGradientSize, Display::E_BlendMode blendMode = Display::E_BlendMode_Alpha);
	void UnsetRenderState();
	void* BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount);
	void EndVertices(void* pVertex = NULL);

	//Render data
	Display::C_VertexDeclaration*			m_pVertexDeclaration;
	
	Display::C_VertexShader					m_VertexShader;
	Display::C_PixelShader					m_PixelShader;

	Display::C_PixelShaderParameter			m_psFontShaderParams;

	//Font resource cache
	Btl::List<C_FontResource> m_FontResourceCache;
};

extern C_FontSystem* g_pFontSystem;
INLINE void FontSystem(C_FontSystem* pFontSystem)
{
	g_pFontSystem = pFontSystem;
}
INLINE C_FontSystem* FontSystem()
{
	return g_pFontSystem;
}
INLINE bool FontSystemExists()
{
	return g_pFontSystem != NULL;
}

}; // namespace Font
}; // namespace Render

#endif // RENDER_FONT_H

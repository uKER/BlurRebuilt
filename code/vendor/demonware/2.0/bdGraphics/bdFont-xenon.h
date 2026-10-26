// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_TEXT_XENON_H
#define BD_TEXT_XENON_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdColor.h>

#include <fxl.h>

// The file name of the arial font (excluding extension)
//static const bdChar8 *BD_ARIAL_FONT_FILE = "DEVKIT:\\bitdemon-XENON\\Arial_12";
static const bdNChar8 *BD_ARIAL_FONT_FILE = "D:\\fonts\\Arial_12";

enum bdFontSavedStates
{
    BD_SAVEDSTATE_D3DRS_ALPHABLENDENABLE,
    BD_SAVEDSTATE_D3DRS_SRCBLEND,
    BD_SAVEDSTATE_D3DRS_DESTBLEND,
    BD_SAVEDSTATE_D3DRS_ALPHATESTENABLE,
    BD_SAVEDSTATE_D3DRS_ALPHAREF,
    BD_SAVEDSTATE_D3DRS_ALPHAFUNC,
    BD_SAVEDSTATE_D3DRS_FILLMODE,
    BD_SAVEDSTATE_D3DRS_CULLMODE,
    BD_SAVEDSTATE_D3DRS_ZENABLE,
    BD_SAVEDSTATE_D3DRS_STENCILENABLE,
    BD_SAVEDSTATE_D3DRS_VIEWPORTENABLE,
    BD_SAVEDSTATE_D3DSAMP_MINFILTER,
    BD_SAVEDSTATE_D3DSAMP_MAGFILTER,
    BD_SAVEDSTATE_D3DSAMP_ADDRESSU,
    BD_SAVEDSTATE_D3DSAMP_ADDRESSV,

    BD_SAVEDSTATE_COUNT,
};

// Specifies left-justified text
#define BD_FONT_LEFT       0x00000000

// Specifies right-justified text
#define BD_FONT_RIGHT      0x00000001

// Specifies horizontally centered text
#define BD_FONT_CENTER_X   0x00000002

// Specifies vertically centered text
#define BD_FONT_CENTER_Y   0x00000004

// Truncates text to the maxPixelWidth
#define BD_FONT_TRUNCATED  0x00000008

struct bdGlyphAttr;

// Represents a font used to draw text on to the screen.
class bdFont
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS
	
	/// Default Constructor. Initializes member variables.
	bdFont();

	/// Destructor.
	/// Frees allocated memory.
	~bdFont();

	/// Initialization of the font.
	/// Initializes the font using the data files given by \a filename. bdFont uses two files to create a font. A .abc
	/// file and .tga texture file can be created with the Fontmaker tool available with the XBox SDK. These files
	/// should have the same filename and be in the same directory. They are differentiated by their filename
	/// extensions. \a filename should not include the extension and period.
	/// \param filename[in] The path and filename of the font file (without the extension).
    bool init(const bdNChar8 *const filename);

	/// Get a pointer to the font's texture.
	IDirect3DTexture9* getTexture() const;

	/// Get the height of the font.
	bdFloat32 getFontHeight() const;

	/// Get the width and height of the bounding rectangle of the string \a text.
	/// \param text[in] The text to be measured.
	/// \param width[out] The width of the text's bounding rectangle.
	/// \param height[out] The height of the text's bounding rectangle.
	/// \param firstLineOnly[in]
	/// - True, only the first line is processed
	/// - False, the entire message is processed
	void getTextExtent(	const bdUInt16 *text,
						bdFloat32 *const width,
						bdFloat32 *const height,
						bdBool firstLineOnly = false) const;

	/// Gets the width of the string \a text.
	/// \param text[in] The string to find the width of.
	bdFloat32 getTextWidth(const bdUInt16 *text) const;

	/// Set the position for the text to appear on screen.
	/// \param x[in] The x-coordinate of the text position.
	/// \param y[in] The y-coordinate of the text position.
	void setCursorPosition(	bdFloat32 x,
							bdFloat32 y);

	/// Set the scale factors for the font.
	/// \param scaleFactorX[in] The amount to scale the font by along the x-axis.
	/// \param scaleFactorY[in] The amount to scale the font by along the y-axis.
	void setScaleFactors(	bdFloat32 scaleFactorX,
							bdFloat32 scaleFactorY);

	/// Set the amount of slant to use in the font.
	/// \param slant[in] The new value for the slant of the font.
	void setSlantFactors(bdFloat32 slant);

	/// Specifies whether we should use the screen width or the render target width when drawing right-justifying text.
	/// \param useScreenSize[in]
	/// - True, Use the screen size for the width
	/// - False, Use the render target size for the width
	void useScreenSize(bdBool useScreenSize);

	/// Create a texture of the string \a text.
	/// \param text[in] The message to make a texture of.
	/// \param bgColor[in] The background color of the texture.
	/// \param textColor[in] The color of text in the texture.
	/// \param d3dFormat[in] The format of the texture to create. Pick one of the D3DFMT_* enum members.
	LPDIRECT3DTEXTURE9 createTexture(	const bdUInt16 *const text,
										D3DCOLOR bgColor = 0,
										D3DCOLOR textColor = 0xffffffff,
										D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8);

	/// Draws some text to the current render target at a specific location.
	/// \param originX[in] The x-coordinate of the point to begin drawing text at.
	/// \param originY[in] The y-coordinate of the point to begin drawing text at.
	/// \param color[in] The color of the text.
	/// \param text[in] The message to print on the screen.
	/// \param flags[in] Specify flags for the text to left- or right-justify, truncate or center it.
	/// The following are valid flags:
	/// - \b BD_FONT_LEFT Default. Left-justifies text.
	/// - \b BD_FONT_RIGHT Right-justifies text.
	/// - \b BD_FONT_CENTER_X Centers the text horizontally.
	/// - \b BD_FONT_CENTER_Y Centers the text vertically.
	/// - \b BD_FONT_TRUNCATED Use with \a maxPixelWidth to cause text to be wrapped to a maximum width.
	/// \param maxPixelWidth[in] Used with BD_FONT_TRUNCATED to specify a maximum width for the text.
	void drawText(	bdFloat32 originX,
		bdFloat32 originY,
		bdULong color,
		const bdUInt16 *text,
		bdULong flags = 0L,
		bdFloat32 maxPixelWidth= 0.0f);

	/// Draws some text to the current render target, starting at the current cursor location.
	/// \param color[in] The color of the text.
	/// \param text[in] The message to print on the screen.
	/// \param flags[in] Specify flags for the text to left- or right-justify, truncate or center it.
	/// The following are valid flags:
	/// - \b BD_FONT_LEFT Default. Left-justifies text.
	/// - \b BD_FONT_RIGHT Right-justifies text.
	/// - \b BD_FONT_CENTER_X Centers the text horizontally.
	/// - \b BD_FONT_CENTER_Y Centers the text vertically.
	/// - \b BD_FONT_TRUNCATED Use with \a maxPixelWidth to cause text to be wrapped to a maximum width.
	/// \param maxPixelWidth[in] Used with BD_FONT_TRUNCATED to specify a maximum width for the text.
	void drawText(	bdULong color,
					const bdUInt16 *text,
                    bdULong flags = 0L,
					bdFloat32 maxPixelWidth = 0.0f);

	/// Called before drawing the text to save and set states.
	void begin();

	/// Called after drawing text to restore the saved states.
	void end();

protected:
	///
	bdFloat32		m_fontHeight;
	bdFloat32		m_topPadding;
	bdFloat32		m_bottomPadding;
	bdFloat32		m_yAdvance;
	bdFloat32		m_xScaleFactor;
	bdFloat32		m_yScaleFactor;
	bdFloat32		m_slantFactor;
	bdFloat32		m_cursorX;
	bdFloat32		m_cursorY;
	bdBool			m_useScreenSize;
	bdUInt16		m_maxGlyph;
	bdInt16			*m_translatorTable;
	bdULong			m_numGlyphs;
	bdGlyphAttr		*m_glyphs;
	bdBool			m_saveState;
	bdULong			m_savedState[BD_SAVEDSTATE_COUNT];
	bdULong			m_nestedBeginCount;
	FXLHANDLE		m_hTechFontShader;
	FXLHANDLE		m_hPass0;
	FXLHANDLE		m_hPosScale;
	FXLHANDLE		m_hColor;
	FXLHANDLE		m_hTexScale;
	FXLHANDLE		m_hChannelSelector;
	FXLHANDLE		m_hMask;
	FXLHANDLE		m_hFontTexture;

protected:
	// A pointer to the vertex declaration for drawing text.
	LPDIRECT3DVERTEXDECLARATION9	m_vertexDecl;

	// A pointer to the texture containing all the characters of the font
	LPDIRECT3DTEXTURE9				m_fontTexture;

	// A pointer to the effect used to draw the font.
	LPFXLEFFECT						m_effect;
};
/*
/// Represents text to be drawn to the screen. Can draw text with two fonts, arial and courier.
class bdText
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default constructor.
	bdText();

	/// Default destructor.
	~bdText();

	/// Initializes the courier and arial fonts.
	bdBool init();

	/// Set the message to use when drawing to the screen.
	bdBool setMessage(const bdChar8 *const message);

	/// Set the position to draw the text from.
	/// \param x[in] The x-coordinate to place the text at.
	/// \param y[in] The y-coordinate to place the text at.
	void setCursor(bdFloat32 x, bdFloat32 y);

	/// Set the color to use when drawing the text.
	/// \param color[in] The color to draw the text in.
	void setColor(const bdColor &color);

	// Draw the text to the render target.
	void draw() const;

protected:
	/// The x-coordinate to place the text at.
	bdFloat32 m_cursorX;

	/// The y-coordinate to place the text at.
	bdFloat32 m_cursorY;

	/// The text to draw.
	bdWChar16 *m_message;

	/// The arial font.
	mutable bdFont m_arial;

	/// The courier font.
	mutable bdFont m_courier;

	/// The color to draw the text.
	bdColor m_textColor;
};
*/
#endif // BD_TEXT_H

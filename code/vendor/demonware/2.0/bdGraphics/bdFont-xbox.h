// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_TEXT_XBOX_H
#define BD_TEXT_XBOX_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdColor.h>

#include <xtl.h>

// The file name of the arial font (excluding extension)
static const bdNChar8 *BD_ARIAL_FONT_FILE = "D:\\fonts\\Arial_12";

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

class bdFont
{
public:
	BD_DECLARE_NEW_AND_DELETE_OPERATORS

	/// Default Constructor. Initializes member variables.
	bdFont();

	/// Destructor.
	/// Frees allocated memory.
	~bdFont();

	bdBool destroy();

	/// Initialization of the font.
	/// Initializes the font using the data files given by \a filename. bdFont uses two files to create a font. A .abc
	/// file and .tga texture file can be created with the Fontmaker tool available with the XBox SDK. These files
	/// should have the same filename and be in the same directory. They are differentiated by their filename
	/// extensions. \a filename should not include the extension and period.
	/// \param filename[in] The path and filename of the font file (without the extension).
	bool init(const bdNChar8 *const filename);

	// Returns the dimensions of a text string
	/// Get the width and height of the bounding rectangle of the string \a text.
	/// \param text[in] The text to be measured.
	/// \param width[out] The width of the text's bounding rectangle.
	/// \param height[out] The height of the text's bounding rectangle.
	/// \param firstLineOnly[in]
	/// - True, only the first line is processed
	/// - False, the entire message is processed
	bdBool getTextExtent(	const bdUInt16 *text,
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
	void setSlantFactor(bdFloat32 slant);

	/// Create a texture of the string \a text.
	/// \param text[in] The message to make a texture of.
	/// \param bgColor[in] The background color of the texture.
	/// \param textColor[in] The color of text in the texture.
	/// \param d3dFormat[in] The format of the texture to create. Pick one of the D3DFMT_* enum members.
	/// \return A pointer to the texture of the text.
	D3DTexture *createTexture(	const bdUInt16 *const text,
								D3DCOLOR bgColor = 0,
								D3DCOLOR textColor = 0xffffffff,
								D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8);

	// Public calls to render text. Callers can simply call DrawText(), but for
	// performance, they should batch multiple calls together, bracketed by 
	// calls to Begin() and End().
	void begin();

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
	bdBool drawText(D3DCOLOR color,
					const bdUInt16 *text,
					bdULong flags = 0L,
					bdFloat32 maxPixelWidth = 0.0f);

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
	bdBool drawText(bdFloat32 originX,
					bdFloat32 originY,
					D3DCOLOR color,
					const bdUInt16 *text,
					bdULong flags = 0L,
					bdFloat32 maxPixelWidth= 0.0f);

	/// Called after drawing text to restore the saved states.
	bdBool end();

	// Internal creation calls
	bdBool createShaders();

	/// Get a pointer to the font's texture.
	D3DTexture* getTexture() const;

	/// Get the height of the font.
	bdFloat32 getFontHeight() const;

protected:
	// Font vertical dimensions
	bdFloat32 m_fontHeight;
	bdFloat32 m_topPadding;
	bdFloat32 m_bottomPadding;
	bdFloat32 m_yAdvance;

	bdFloat32 m_xScaleFactor;
	bdFloat32 m_yScaleFactor;
	bdFloat32 m_slantFactor;

	bdFloat32 m_cursorX;
	bdFloat32 m_cursorY;

	// Translator table for supporting Unicode ranges
	bdUInt16 m_maxGlyph;
	bdInt16 *m_translatorTable;
	bdBool m_translatorTableWasAllocated;

	// Glyph data for the font
	DWORD m_numGlyphs;
	bdGlyphAttr *m_glyphs;

	// Saved state for rendering (if not using a pure device)
	bdBool m_saveState;
	DWORD m_savedState[16];

	// D3D rendering objects
	D3DTexture *m_fontTexture;
	static DWORD m_fontVertexShader;
	static DWORD m_fontPixelShader;
	bdULong m_nestedBeginCount;
};

#endif

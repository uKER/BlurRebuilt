// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#include <xtl.h>
#include <xgraphics.h>

#include <bdGraphics/bdD3D.h>
#include <bdGraphics/bdFont-xenon.h>

// The extension for font data files
static const bdNChar8 *BD_FONT_EXT = ".abc";

// The extension for font texture files
static const bdNChar8 *BD_TEX_EXT = ".tga";

struct bdGlyphAttr
{
	bdUInt16 m_tu1;		// Texture coordinates for the image
	bdUInt16 m_tv1;
	bdUInt16 m_tu2;
	bdUInt16 m_tv2;
	bdInt16 m_offset;	// Pixel offset for glyph start
	bdInt16 m_width;	// Pixel width of the glyph
	bdInt16 m_advance;	// Pixels to advance after the glyph
	bdUInt16  m_mask;	// Channel mask
};

// The HLSL program for the font shaders
static const bdNChar8 shaderProgramSource[] =
"struct VS_IN												\n"
"{															\n"
"   float2   Pos		: POSITION;							\n"
"   float2   Tex		: TEXCOORD0;						\n"
"};															\n"
"															\n"
"struct VS_OUT												\n"
"{															\n"
"   float4 Position		: POSITION;							\n"
"   float4 Diffuse		: COLOR0;							\n"
"   float2 TexCoord0	: TEXCOORD0;						\n"
"};															\n"
"															\n"
"uniform float2   PosScale : register(c0);					\n"
"uniform float4   Color    : register(c1);					\n"
"uniform float2   TexScale : register(c2);					\n"
"															\n"
"VS_OUT FontVertexShader( VS_IN In )						\n"
"{															\n"
"   VS_OUT Out;												\n"
"   Out.Position.x  = ( (In.Pos.x-0.5)*PosScale.x - 1.0 );	\n"
"   Out.Position.y  =-( (In.Pos.y-0.5)*PosScale.y - 1.0 );	\n"
"   Out.Position.z  = ( 0.0 );								\n"
"   Out.Position.w  = ( 1.0 );								\n"
"   Out.Diffuse     = Color;								\n"
"   Out.TexCoord0.x = In.Tex.x * TexScale.x;				\n"
"   Out.TexCoord0.y = In.Tex.y * TexScale.y;				\n"
"   return Out;												\n"
"}															\n"
"															\n"
"uniform float4 ChannelSelector	: register(c0);				\n"
"uniform float4 Mask			: register(c1);				\n"
"															\n"
//"sampler			FontTexture     : register(s0);			\n"
"texture	FontTexture;									\n"
"sampler	FontSampler = sampler_state						\n"
"{															\n"
"    Texture = <FontTexture>;								\n"
"    MipFilter = LINEAR;									\n"
"    MinFilter = LINEAR;									\n"
"    MagFilter = LINEAR;									\n"
"};															\n"
"															\n"
"float4 FontPixelShader( VS_OUT In ) : COLOR0				\n"
"{															\n"
"	// Fetch a texel from the font texture					\n"
"	float4 FontTexel = tex2D( FontSampler, In.TexCoord0 );	\n"
"															\n"
"	// Select the color from the channel					\n"
"	float value = dot( FontTexel, ChannelSelector );		\n"
"															\n"
"	// For white pixels, the high bit is 1 and the low		\n"
"	// bits are luminance, so r0.a will be > 0.5. For the	\n"
"	// RGB channel, we want to lop off the msb and shift	\n"
"	// the lower bits up one bit. This is simple to do		\n"
"	// with the _bx2 modifier. Since these pixels are		\n"
"	// opaque, we emit a 1 for the alpha channel (which		\n"
"	// is 0.5 x2 ).											\n"
"															\n"
"	// For black pixels, the high bit is 0 and the low		\n"
"	// bits are alpha, so r0.a will be < 0.5. For the RGB	\n"
"	// channel, we emit zero. For the alpha channel, we		\n"
"	// just use the x2 modifier to scale up the low bits	\n"
"	// of the alpha.										\n"
"	float4 Color;											\n"
"	Color.rgb = ( value > 0.5f ? 2*value-1 : 0.0f );		\n"
"	Color.a   = 2 * ( value > 0.5f ? 1.0f : value );		\n"
"															\n"
"	// Apply a mask value, which let's ignore the above		\n"
"	// and use the original texel with all it's channels	\n"
"	Color = lerp( Color, FontTexel, Mask );					\n"
"															\n"
"	// Return the texture color modulated with the vertex	\n"
"	// color												\n"
"	return Color * In.Diffuse;								\n"
"}															\n"
"															\n"
"technique FontShader										\n"
"{															\n"
"	pass P0													\n"
"	{														\n"
"		AlphaBlendEnable = True;							\n"
"		DestBlend = INVSRCALPHA;							\n"
"		SrcBlend = SRCALPHA;								\n"
"		AlphaTestEnable = True;								\n"
"		AlphaRef = 0x08;									\n"
"		AlphaFunc = GREATEREQUAL;							\n"
"		FillMode = SOLID;									\n"
"		CullMode = CCW;										\n"
"		ZEnable = FALSE;									\n"
"		VertexShader = compile vs_3_0 FontVertexShader();	\n"
"		PixelShader = compile ps_3_0 FontPixelShader();		\n"
"	}														\n"
"}															\n";

bdFont::bdFont()
{
	// Initialize the member variables to default values
	m_fontTexture = BD_NULL;

	m_fontHeight = 36.0f;
	m_topPadding = 0.0f;
	m_bottomPadding = 0.0f;
	m_yAdvance = 36.0f;
	
	m_numGlyphs = 0L;
	m_glyphs = BD_NULL;

	m_cursorX = 0.0f;
	m_cursorY = 0.0f;
	
	m_xScaleFactor = 1.0f;
	m_yScaleFactor = 1.0f;
	m_slantFactor = 0.0f;
	
	m_maxGlyph = 0;
	m_translatorTable = BD_NULL;
	
	m_nestedBeginCount = 0L;
	m_useScreenSize = false;
}

bdFont::~bdFont()
{
	// Free any allocated memory
	if (m_translatorTable)
	{
		delete [] m_translatorTable;
		m_translatorTable = NULL;
	}

	if (m_glyphs)
	{
		delete [] m_glyphs;
		m_glyphs = NULL;
	}
}

bool bdFont::init(const bdNChar8 * const filename)
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();
	LPD3DXBUFFER compiledEffect = NULL;
	LPD3DXBUFFER errorBuffer = NULL;
	HANDLE fontFile;
	bdULong numBytesRead;
	bdNChar8 *fontFilename;
	bdNChar8 *fontTexFilename;
	bdUInt fileVersion;

	if (FAILED(FXLCompileEffect(shaderProgramSource, sizeof(shaderProgramSource),
		NULL, NULL, D3DXSHADER_FXLPARAMETERS_AS_VARIABLE_NAMES,	&compiledEffect, &errorBuffer)))
	{
		// There was an error. Output the error message
		OutputDebugString("Error compiling shader");
		OutputDebugString(reinterpret_cast<bdNChar8*>(errorBuffer->GetBufferPointer()));

		// Free the error buffer
		if (errorBuffer)
		{
			errorBuffer->Release();
			errorBuffer = NULL;
		}
		return false;
	}

	if (FAILED(FXLCreateEffect(device, compiledEffect->GetBufferPointer(), NULL, &m_effect)))
	{
		return false;
	}

	m_hTechFontShader	= m_effect->GetTechniqueHandle("FontShader");
	m_hPass0			= m_effect->GetPassHandleFromIndex(m_hTechFontShader, 0);
	m_hPosScale			= m_effect->GetParameterHandle("PosScale");
	m_hColor			= m_effect->GetParameterHandle("Color");
	m_hTexScale			= m_effect->GetParameterHandle("TexScale");
	m_hChannelSelector	= m_effect->GetParameterHandle("ChannelSelector");
	m_hMask				= m_effect->GetParameterHandle("Mask");
	m_hFontTexture		= m_effect->GetParameterHandle("FontSampler");

	fontFilename = new bdNChar8[strlen(filename) + strlen(BD_FONT_EXT) + 1];
	fontTexFilename = new bdNChar8[strlen(filename) + strlen(BD_TEX_EXT) + 1];

	strcpy(fontFilename, filename);
	strcat(fontFilename, BD_FONT_EXT);

	strcpy(fontTexFilename, filename);
	strcat(fontTexFilename, BD_TEX_EXT);

	if (FAILED(D3DXCreateTextureFromFile(device, fontTexFilename, &m_fontTexture)))
	{
		return false;
	}

	m_effect->SetSampler(m_hFontTexture, m_fontTexture);

	fontFile = CreateFile(fontFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_READONLY, NULL);

	if (fontFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	ReadFile(fontFile, &fileVersion, sizeof(fileVersion), &numBytesRead, NULL);

	if (fileVersion == 0x00000005)
	{
		ReadFile(fontFile, &m_fontHeight, sizeof(float), &numBytesRead, NULL);
		ReadFile(fontFile, &m_topPadding, sizeof(float), &numBytesRead, NULL);
		ReadFile(fontFile, &m_bottomPadding, sizeof(float), &numBytesRead, NULL);
		ReadFile(fontFile, &m_yAdvance, sizeof(float), &numBytesRead, NULL);
		ReadFile(fontFile, &m_maxGlyph, sizeof(m_maxGlyph), &numBytesRead, NULL);
		m_translatorTable = new short[m_maxGlyph + 1];
		ReadFile(fontFile, m_translatorTable, (m_maxGlyph + 1) * sizeof(bdUInt16), &numBytesRead, NULL);
		ReadFile(fontFile, &m_numGlyphs, sizeof(unsigned long), &numBytesRead, NULL);
		m_glyphs = new bdGlyphAttr[m_numGlyphs];
		ReadFile(fontFile, m_glyphs, m_numGlyphs * sizeof(bdGlyphAttr), &numBytesRead, NULL);
	}
	else
	{
		return false;
	}

	D3DVERTEXELEMENT9 vertexEle[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};

	device->CreateVertexDeclaration(vertexEle, &m_vertexDecl);

	return true;
}


LPDIRECT3DTEXTURE9 bdFont::getTexture() const
{
	return m_fontTexture;
}

bdFloat32 bdFont::getFontHeight() const
{
	return m_yAdvance;
}

void bdFont::getTextExtent(const bdUInt16 *text,
						   bdFloat32 *const width,
						   bdFloat32 *const height,
						   bdBool firstLineOnly) const
{
	BD_ASSERT(text != BD_NULL, "text cannot be NULL");
	BD_ASSERT(width != BD_NULL, "width cannot be NULL");
	BD_ASSERT(height != BD_NULL, "height cannot be NULL");

	*width = 0.0f;
	*height = 0.0f;

	bdFloat32 sx = 0.0f;
	bdFloat32 sy = m_fontHeight;

	while (*text)
	{
		bdUInt16 letter = *text++;

		if (letter == L'\n')
		{
			if (firstLineOnly)
				break;
			sx = 0.0f;
			sy += m_yAdvance;
		}

		if (letter == L'\r')
			continue;

		bdGlyphAttr* icon;
		if (letter > m_maxGlyph || m_translatorTable[letter] == 0)
		{
			icon = &m_glyphs[0];
		}
		else
		{
			icon = &m_glyphs[m_translatorTable[letter]];
		}

		sx += icon->m_offset;
		sx += icon->m_advance;

		if (sx > (*width))
		{
			*width = sx;
		}

		if (sy > (*height))
		{
			*height = sy;
		}
	}

	*width *= m_xScaleFactor;
	*height *= m_yScaleFactor;
}

bdFloat32 bdFont::getTextWidth(const bdUInt16 *text) const
{
	bdFloat32 width = 0.0f;
	bdFloat32 height = 0.0f;

	getTextExtent(text, &width, &height);
	return width;
}

void bdFont::setCursorPosition(bdFloat32 x, bdFloat32 y)
{
	m_cursorX = floorf(x);
	m_cursorY = floorf(y);
}

void bdFont::setScaleFactors(bdFloat32 scaleFactorX, bdFloat32 scaleFactorY)
{
	m_xScaleFactor = scaleFactorX;
	m_yScaleFactor = scaleFactorY;
}

void bdFont::setSlantFactors(bdFloat32 slant)
{
	m_slantFactor = slant;
}

void bdFont::useScreenSize(bdBool useScreenSize)
{
	m_useScreenSize = useScreenSize;
}

IDirect3DTexture9* bdFont::createTexture(const bdUInt16 *const text,
										 D3DCOLOR bgColor,
                                         D3DCOLOR textColor,
										 D3DFORMAT d3dFormat)
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();
	bdFloat32 texWidth;
	bdFloat32 texHeight;
	bdULong width;
	bdULong height;
	LPDIRECT3DSURFACE9 oldRenderTarget;
	LPDIRECT3DSURFACE9 newRenderTarget;
	LPDIRECT3DTEXTURE9 newTexture;

	// Make sure the texture format is tiled
	if (FALSE == XGIsTiledFormat(d3dFormat))
	{
		return NULL;
	}

	// Get the width and height for the texture
	getTextExtent(text, &texWidth, &texHeight);
	width = static_cast<bdULong>(texWidth);
	height = static_cast<bdULong>(texHeight);
	
	// Create a new render target
	if (FAILED(device->CreateRenderTarget(width, height, d3dFormat, D3DMULTISAMPLE_NONE, 0L, FALSE,
											&newRenderTarget, NULL)))
	{
		return NULL;
	}

	// Create a new texture
	if (FAILED(device->CreateTexture(width, height, 1, 0L, d3dFormat, D3DPOOL_DEFAULT, &newTexture, NULL)))
	{
		newRenderTarget->Release();
		return NULL;
	}

	// Store the old render target
	device->GetRenderTarget(0, &oldRenderTarget);

	// Use the new render target
	device->SetRenderTarget(0, newRenderTarget);

	// Clear the render target to the background color
	device->Clear(0L, NULL, D3DCLEAR_TARGET, bgColor, 1.0f, 0L);

	// Draw the text to the render target
	drawText(0, 0, textColor, text, 0L);

	// Resolve the render target to the texture
	device->Resolve(0L, NULL, newTexture, NULL, 0, 0, NULL, 0, 0, NULL);

	// Set the render target back to the old one
	device->SetRenderTarget(0, oldRenderTarget);

	// Free up memory by releasing the render targets
	oldRenderTarget->Release();
	newRenderTarget->Release();

	// Return the texture
	return newTexture;
}

void bdFont::drawText(bdULong color, const bdUInt16 *text, bdULong flags, bdFloat32 maxPixelWidth)
{
	drawText(m_cursorX, m_cursorY, color, text, flags, maxPixelWidth);
}

void bdFont::drawText(bdFloat32 originX, bdFloat32 originY, bdULong color, const bdUInt16 *text,
						 bdULong flags, bdFloat32 maxPixelWidth)
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();
	LPDIRECT3DVERTEXDECLARATION9 oldVertexDecl;
	LPDIRECT3DVERTEXSHADER9 oldVertexShader;
	LPDIRECT3DPIXELSHADER9 oldPixelShader;

	device->GetVertexDeclaration(&oldVertexDecl);
	device->GetVertexShader(&oldVertexShader);
	device->GetPixelShader(&oldPixelShader);

	// Set up the state of the shader/device
	begin();

	m_effect->BeginTechnique(m_hTechFontShader, FXL_RESTORE_DEFAULT_RENDER_STATE);


	// Set the color to draw the text.
	bdFloat32 vColor[4];
	vColor[0] = ((color & 0x00ff0000) >> 16L) / 255.0f;
	vColor[1] = ((color & 0x0000ff00) >>  8L) / 255.0f;
	vColor[2] = ((color & 0x000000ff) >>  0L) / 255.0f;
	vColor[3] = ((color & 0xff000000) >> 24L) / 255.0f;
	m_effect->SetVectorF(m_hColor, vColor);

    // Check for right-justification
	if ((flags & BD_FONT_RIGHT) && (originX < 0.0f))
	{
		D3DSURFACE_DESC desc;
		if (m_useScreenSize)
		{
			D3DDISPLAYMODE mode;
			device->GetDisplayMode(0, &mode);
			desc.Width = mode.Width;
			desc.Height = mode.Height;
		}
		else
		{
			LPDIRECT3DSURFACE9 renderTarget;
			device->GetRenderTarget(0, &renderTarget);
			renderTarget->GetDesc(&desc);
			renderTarget->Release();
		}
		
		// Move the origin over to the right
		originX += desc.Width;
	}

	// Place the cursor
	m_cursorX = floorf(originX);
	m_cursorY = floorf(originY);

	// Move the origin to make room for the top padding
	originY -= m_topPadding;

	// If we truncate the text, we add three ellipses to the end of the original line to denote this. We need to find
	// the width of those ellipses, so that they can be included in the measurement.
	bdFloat32 ellipsesPixelWidth = m_xScaleFactor * 3.0f *
		(m_glyphs[m_translatorTable[L'.']].m_offset + m_glyphs[m_translatorTable[L'.']].m_advance);

	// Check for truncation of text
	if (flags & BD_FONT_TRUNCATED)
	{
		// Don't truncate unless maxPixelWidth is a positive value
		if (maxPixelWidth <= 0.0f)
		{
			flags &= ~BD_FONT_TRUNCATED;
		}
		else
		{
			bdFloat32 w, h;
			getTextExtent(text, &w, &h, true);

			// There is no need to truncate if the maxPixelWidth is larger than the text width
			if (w <= maxPixelWidth)
			{
				flags &= ~BD_FONT_TRUNCATED;
			}
		}
	}

	// Move the cursor if the text is to be vertically centered.
	if (flags & BD_FONT_CENTER_Y)
	{
		float w, h;
		getTextExtent(text, &w, &h);

		m_cursorY = floorf(m_cursorY - (h / 2));
	}

	bool startingNewLine = true;

	// Loop through the text to draw it character by character
	while(*text)
	{
		if (startingNewLine)
		{
			// If starting a new line, we need to put the origin in the correct position, horizontally, if
			// BD_FONT_RIGHT or BD_FONT_CENTER_X were specified.
			if (flags & (BD_FONT_RIGHT|BD_FONT_CENTER_X))
			{
				float w, h;
				getTextExtent(text, &w, &h, TRUE);

				if (flags & BD_FONT_RIGHT)
					m_cursorX = floorf(originX - w);
				if (flags & BD_FONT_CENTER_X)
					m_cursorX = floorf(originX - w/2);
			}
			startingNewLine = false;
		}

		// Save the current letter and move text onto the next one.
		bdUInt16 letter = *text++;

		// Handle a new line character
		if (letter == L'\n')
		{
			m_cursorX = originX;
			m_cursorY += m_yAdvance * m_yScaleFactor;
			startingNewLine = true;
			continue;
		}

		// Ignore carriage return characters
		if (letter == L'\r')
		{
			continue;
		}

		// Find the glyph for this character.
		bdGlyphAttr* glyph = &m_glyphs[(letter <= m_maxGlyph) ? m_translatorTable[letter] : 0];

		// Store the data for the glyph. Use the scale factor to make sure its drawn at the desired scale.
		bdFloat32 offset	= m_xScaleFactor * static_cast<float>(glyph->m_offset);
		bdFloat32 advance	= m_xScaleFactor * static_cast<float>(glyph->m_advance);
		bdFloat32 width		= m_xScaleFactor * static_cast<float>(glyph->m_width);
		bdFloat32 height	= m_yScaleFactor * m_fontHeight;

		// If truncation is required, we should draw in the ellipses.
		if (flags & BD_FONT_TRUNCATED)
		{
			if (m_cursorX + offset + width + ellipsesPixelWidth + m_slantFactor > originX + maxPixelWidth)
			{
				drawText(m_cursorX, m_cursorY, color, reinterpret_cast<bdUInt16*>(L"..."));
				break;
			}
		}

		// Calculate the masking constants for the shader
		bdFloat32 shaderConsts0[4] =
		{
			glyph->m_mask & 0x0f00 ? 1.0f : 0.0f,
			glyph->m_mask & 0x00f0 ? 1.0f : 0.0f,
			glyph->m_mask & 0x000f ? 1.0f : 0.0f,
			glyph->m_mask & 0xf000 ? 1.0f : 0.0f
		};
        bdFloat32 shaderConsts1[4] =
		{
			glyph->m_mask == 0x0000 ? 1.0f : 0.0f,
			glyph->m_mask == 0x0000 ? 1.0f : 0.0f,
			glyph->m_mask == 0x0000 ? 1.0f : 0.0f,
			glyph->m_mask == 0x0000 ? 1.0f : 0.0f
		};

		// Set the shader constants
		//device->SetPixelShaderConstantF(0, shaderConsts0, 1);
		//device->SetPixelShaderConstantF(1, shaderConsts1, 1);
		m_effect->SetVectorF(m_hChannelSelector, shaderConsts0);
		m_effect->SetVectorF(m_hMask, shaderConsts1);

		// Find the extents of the character
		m_cursorX	+= offset;
		bdFloat32 left1	 = m_cursorX;
		bdFloat32 left2	 = left1 + m_slantFactor;
		bdFloat32 right1 = left1 + width;
		bdFloat32 right2 = left2 + width;
		bdFloat32 top	 = m_cursorY;
		bdFloat32 bottom = top + height;
		m_cursorX	+= advance;

		// Define a polygon to fit the character on to
		float v[4][4] =
		{
			{left2,		top,	glyph->m_tu1, glyph->m_tv1},
			{right2,	top,	glyph->m_tu2, glyph->m_tv1},
			{right1,	bottom,	glyph->m_tu2, glyph->m_tv2},
			{left1,		bottom, glyph->m_tu1, glyph->m_tv2},
		};

		// Run the effect
		m_effect->BeginPass(m_hPass0);

		device->SetVertexDeclaration(m_vertexDecl);

		m_effect->Commit();
		
		// Draw the polygon
		device->DrawPrimitiveUP(D3DPT_QUADLIST, 1, v, sizeof(v[0]));

		m_effect->EndPass();

	}

	m_effect->EndTechnique();

	// Restore the saved states.
	end();

	device->SetPixelShader(oldPixelShader);
	device->SetVertexShader(oldVertexShader);
	device->SetVertexDeclaration(oldVertexDecl);

	oldVertexDecl->Release();
	oldVertexShader->Release();
	oldPixelShader->Release();
}

void bdFont::begin()
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();

	// Set state on the first call
	if( 0 == m_nestedBeginCount )
	{
		// Save state
		if( m_saveState )
		{
			// Note, we are not saving the texture, vertex, or pixel shader,
			//       since it's not worth the performance. We're more interested
			//       in saving state that would cause hard to find problems.
			device->GetRenderState( D3DRS_ALPHABLENDENABLE, &m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHABLENDENABLE ] );
			device->GetRenderState( D3DRS_SRCBLEND,         &m_savedState[ BD_SAVEDSTATE_D3DRS_SRCBLEND ] );
			device->GetRenderState( D3DRS_DESTBLEND,        &m_savedState[ BD_SAVEDSTATE_D3DRS_DESTBLEND ] );
			device->GetRenderState( D3DRS_ALPHATESTENABLE,  &m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHATESTENABLE ] );
			device->GetRenderState( D3DRS_ALPHAREF,         &m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHAREF ] );
			device->GetRenderState( D3DRS_ALPHAFUNC,        &m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHAFUNC ] );
			device->GetRenderState( D3DRS_FILLMODE,         &m_savedState[ BD_SAVEDSTATE_D3DRS_FILLMODE ] );
			device->GetRenderState( D3DRS_CULLMODE,         &m_savedState[ BD_SAVEDSTATE_D3DRS_CULLMODE ] );
			device->GetRenderState( D3DRS_ZENABLE,          &m_savedState[ BD_SAVEDSTATE_D3DRS_ZENABLE ] );
			device->GetRenderState( D3DRS_STENCILENABLE,    &m_savedState[ BD_SAVEDSTATE_D3DRS_STENCILENABLE ] );
			device->GetRenderState( D3DRS_VIEWPORTENABLE,   &m_savedState[ BD_SAVEDSTATE_D3DRS_VIEWPORTENABLE ] );
			device->GetSamplerState( 0, D3DSAMP_MINFILTER,  &m_savedState[ BD_SAVEDSTATE_D3DSAMP_MINFILTER ] );
			device->GetSamplerState( 0, D3DSAMP_MAGFILTER,  &m_savedState[ BD_SAVEDSTATE_D3DSAMP_MAGFILTER ] );
			device->GetSamplerState( 0, D3DSAMP_ADDRESSU,   &m_savedState[ BD_SAVEDSTATE_D3DSAMP_ADDRESSU ] );
			device->GetSamplerState( 0, D3DSAMP_ADDRESSV,   &m_savedState[ BD_SAVEDSTATE_D3DSAMP_ADDRESSV ] );
		}
        
		// Set render state
		//        device->SetTexture( 0, fontTexture );
		//		m_effect->SetSampler(hFontTexture, fontTexture);
		/*        device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		device->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA );
		device->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA );
		device->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
		device->SetRenderState( D3DRS_ALPHAREF,         0x08 );
		device->SetRenderState( D3DRS_ALPHAFUNC,        D3DCMP_GREATEREQUAL );
		device->SetRenderState( D3DRS_FILLMODE,         D3DFILL_SOLID );
		device->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
		device->SetRenderState( D3DRS_ZENABLE,          FALSE );
		device->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
		device->SetRenderState( D3DRS_VIEWPORTENABLE,   TRUE );
		device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		device->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		device->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		device->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

		//        device->SetVertexDeclaration( g_pFontVertexDecl );
		//        device->SetVertexShader( g_pFontVertexShader );
		//        device->SetPixelShader( g_pFontPixelShader );
		*/
		D3DSURFACE_DESC desc;
		if( m_useScreenSize )
		{
			D3DDISPLAYMODE DisplayMode;
			device->GetDisplayMode( 0, &DisplayMode );
			desc.Width = DisplayMode.Width;
			desc.Height = DisplayMode.Height;
		}
		else
		{
			LPDIRECT3DSURFACE9 pRenderTarget;
			device->GetRenderTarget( 0, &pRenderTarget );
			pRenderTarget->GetDesc( &desc );
			pRenderTarget->Release();
		}

		// Set the position scale factor as a vertex shader constant
		bdFloat32 vPosScale[4];
		vPosScale[0] = 2.0f/desc.Width;
		vPosScale[1] = 2.0f/desc.Height;
		vPosScale[2] = 0.0f;
		vPosScale[3] = 0.0f;
		//device->SetVertexShaderConstantF( 0, vPosScale, 1 );
		m_effect->SetVectorF(m_hPosScale, vPosScale);

		// Set the texture scaling factor as a vertex shader constant
		D3DSURFACE_DESC TextureDesc;
		m_fontTexture->GetLevelDesc( 0, &TextureDesc );
		bdFloat32 vTexScale[4];
		vTexScale[0] = 1.0f/TextureDesc.Width;
		vTexScale[1] = 1.0f/TextureDesc.Height;
		vTexScale[2] = 0.0f;
		vTexScale[3] = 0.0f;
		//device->SetVertexShaderConstantF( 2, vTexScale, 1 );
		m_effect->SetVectorF(m_hTexScale, vTexScale);
	}

	// Keep track of the nested begin/end calls.
	m_nestedBeginCount++;
}

void bdFont::end()
{
	LPDIRECT3DDEVICE9 device = bdD3D::getInstance()->getDevice();

//	assert( nestedBeginCount > 0 );
    if( --m_nestedBeginCount > 0 )
        return;

    // Restore state
    if( m_saveState )
    {
        device->SetTexture( 0, NULL );
        device->SetVertexDeclaration( NULL );
        device->SetVertexShader( NULL );
        device->SetPixelShader( NULL );
        device->SetRenderState( D3DRS_ALPHABLENDENABLE, m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHABLENDENABLE ] );
        device->SetRenderState( D3DRS_SRCBLEND,         m_savedState[ BD_SAVEDSTATE_D3DRS_SRCBLEND ] );
        device->SetRenderState( D3DRS_DESTBLEND,        m_savedState[ BD_SAVEDSTATE_D3DRS_DESTBLEND ] );
        device->SetRenderState( D3DRS_ALPHATESTENABLE,  m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHATESTENABLE ] );
        device->SetRenderState( D3DRS_ALPHAREF,         m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHAREF ] );
        device->SetRenderState( D3DRS_ALPHAFUNC,        m_savedState[ BD_SAVEDSTATE_D3DRS_ALPHAFUNC ] );
        device->SetRenderState( D3DRS_FILLMODE,         m_savedState[ BD_SAVEDSTATE_D3DRS_FILLMODE ] );
        device->SetRenderState( D3DRS_CULLMODE,         m_savedState[ BD_SAVEDSTATE_D3DRS_CULLMODE ] );
        device->SetRenderState( D3DRS_ZENABLE,          m_savedState[ BD_SAVEDSTATE_D3DRS_ZENABLE ] );
        device->SetRenderState( D3DRS_STENCILENABLE,    m_savedState[ BD_SAVEDSTATE_D3DRS_STENCILENABLE ] );
        device->SetRenderState( D3DRS_VIEWPORTENABLE,   m_savedState[ BD_SAVEDSTATE_D3DRS_VIEWPORTENABLE ] );
        device->SetSamplerState( 0, D3DSAMP_MINFILTER,  m_savedState[ BD_SAVEDSTATE_D3DSAMP_MINFILTER ] );
        device->SetSamplerState( 0, D3DSAMP_MAGFILTER,  m_savedState[ BD_SAVEDSTATE_D3DSAMP_MAGFILTER ] );
        device->SetSamplerState( 0, D3DSAMP_ADDRESSU,   m_savedState[ BD_SAVEDSTATE_D3DSAMP_ADDRESSU ] );
        device->SetSamplerState( 0, D3DSAMP_ADDRESSV,   m_savedState[ BD_SAVEDSTATE_D3DSAMP_ADDRESSV ] );
    }
}
/*
bdText::bdText()
:	m_textColor(255, 255, 255) // White text
{
	m_message = BD_NULL;
	m_cursorX = 5;
	m_cursorY = 5;
}

bdText::~bdText()
{
	if (m_message)
	{
		delete [] m_message;
	}
}

bool bdText::init()
{
	if (!m_arial.init(BD_ARIAL_FONT_FILE))
	{
		return false;
	}

	if (!m_courier.init(BD_COURIER_FONT_FILE))
	{
		return false;
	}

	return true;
}

bool bdText::setMessage(const bdChar8 *const message)
{
	// Store the text locally
	if (message)
	{
		bdUInt length = bdStrlen(message);

		m_message = new bdUInt16[length + 1];

		if (!m_message)
		{
			return false;
		}

		// Convert the string to a wide-character string
		mbstowcs(m_message, message, length + 1);
	}

	return true;
}

void bdText::setCursor(bdFloat32 x, bdFloat32 y)
{
	m_cursorX = x;
	m_cursorY = y;
}

void bdText::setColor(const bdColor &color)
{
	m_textColor = color;
}

void bdText::draw() const
{
	if (m_message)
	{
		// Use arial to draw the text
		m_arial.drawText(m_cursorX, m_cursorY, m_textColor, m_message);
	}
}
*/
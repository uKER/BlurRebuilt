// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XBOX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdGraphics/bdD3D.h>
#include <bdGraphics/bdFont-xbox.h>

#include <xtl.h>
#include <xgraphics.h>

// The extension for font data files
static const bdNChar8 *BD_FONT_EXT = ".abc";

// The extension for font texture files
static const bdNChar8 *BD_TEX_EXT = ".tga";

struct bdGlyphAttr
{
	bdFloat32 m_tu1;	// Texture coordinates for the image
	bdFloat32 m_tv1;
	bdFloat32 m_tu2;
	bdFloat32 m_tv2;
	bdInt16 m_offset;	// Pixel offset for glyph start
	bdInt16 m_width;	// Pixel width of the glyph
	bdInt16 m_advance;	// Pixels to advance after the glyph
};

DWORD bdFont::m_fontVertexShader = 0L;
DWORD bdFont::m_fontPixelShader = 0L;

bdFont::bdFont()
{
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
    m_translatorTableWasAllocated = false;
    
    m_nestedBeginCount = 0L;
}

bdFont::~bdFont()
{
	destroy();
}

bdBool bdFont::createShaders()
{
    // Create the vertex shader
    if (m_fontVertexShader == 0L)
    {
        // Specify the vertex declaration, used here to manually specify a constant.
        bdULong fontVertexDecl[] =
        {
            D3DVSD_STREAM(0),
            D3DVSD_REG( 0, D3DVSDT_FLOAT4 ),        // Vertex/tex coord combo
            D3DVSD_REG( 3, D3DVSDT_D3DCOLOR ),      // Color
            D3DVSD_END()
        };

        // Microcode for the vertex shader, where input is defined as:
        //    v0.xy = screen space position
        //    v0.zw = tex coords
        //    v3    = diffuse color
        static bdULong fontVertexShaderInstructions[] = 
        {
            0x00032078,
            0x00000000, 0x00200015, 0x0836106c, 0x2070c800, // mov oPos.xy, v0.xy
            0x00000000, 0x002000bf, 0x0836106c, 0x2070c848, // mov oT0.xy, v0.zw
            0x00000000, 0x0020061b, 0x0836106c, 0x2070f819  // mov oD0, v3
        };

        // Create the vertex shader
        if(FAILED(D3DDevice::CreateVertexShader(fontVertexDecl, 
												fontVertexShaderInstructions, 
												&m_fontVertexShader,
												D3DUSAGE_PERSISTENTDIFFUSE)))
		{
			return false;
		}

    }

    // Create the pixel shader
    if(m_fontPixelShader == 0L)
    {
        D3DPIXELSHADERDEF psd;
        ZeroMemory(&psd, sizeof(psd));
        psd.PSCombinerCount = PS_COMBINERCOUNT(1, 0);
        psd.PSTextureModes  = PS_TEXTUREMODES(PS_TEXTUREMODES_PROJECT2D, 0, 0, 0);

        //------------- Stage 0 -------------
        psd.PSRGBInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0|PS_CHANNEL_RGB, PS_REGISTER_V0|PS_CHANNEL_RGB,   0, 0);
        psd.PSAlphaInputs[0] = PS_COMBINERINPUTS(PS_REGISTER_T0|PS_CHANNEL_ALPHA,
												PS_REGISTER_V0|PS_CHANNEL_ALPHA, 0, 0);
        psd.PSRGBOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, 0, 0, PS_COMBINEROUTPUT_AB_MULTIPLY);
        psd.PSAlphaOutputs[0] = PS_COMBINEROUTPUTS(PS_REGISTER_R0, 0, 0, PS_COMBINEROUTPUT_AB_MULTIPLY);

        //------------- Final combiner -------------
        psd.PSFinalCombinerInputsABCD = PS_COMBINERINPUTS(PS_REGISTER_C0, PS_REGISTER_R0, PS_REGISTER_ZERO,
															PS_REGISTER_C1);
        psd.PSFinalCombinerInputsEFG  = PS_COMBINERINPUTS(0, 0, PS_REGISTER_R0|PS_CHANNEL_ALPHA, 0);

        // Set constants to scale output to an NTSC-safe range
        // NTSC_MAX = 0.8549f => 0x00dadada
        // NTSC_MIN = 0.0625f => 0x00101010
        psd.PSFinalCombinerConstant0 = 0x00dadada;
        psd.PSFinalCombinerConstant1 = 0x00101010;
        psd.PSFinalCombinerConstants = PS_FINALCOMBINERCONSTANTS(0,1,PS_GLOBALFLAGS_NO_TEXMODE_ADJUST);

        // Create the pixel shader, as defined above.
        if(FAILED(D3DDevice::CreatePixelShader(&psd, &m_fontPixelShader)))
		{
            return false;
		}
    }

    return true;
}

template <typename T1>
void EndianSwap(T1 &val)
{
	bdByte8 *src;
	bdByte8 *dst;
	T1 res;

	src = reinterpret_cast<bdByte8*>(&val);
	dst = reinterpret_cast<bdByte8*>(&res);

	dst += sizeof(T1) - 1;

	for (bdUInt i = 0; i < sizeof(T1); i++, src++, dst--)
	{
		*dst = *src;
	}

	val = res;
}

bdBool bdFont::init(const bdNChar8 *const filename)
{
	LPDIRECT3DDEVICE8 device = bdD3D::getInstance()->getDevice();
	HANDLE fontFile;
	bdULong numBytesRead;
	bdNChar8 *fontFilename;
	bdNChar8 *fontTexFilename;
	bdUInt fileVersion;

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

	fontFile = CreateFile(fontFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
							FILE_ATTRIBUTE_READONLY, NULL);

	if (fontFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	delete [] fontFilename;
	delete [] fontTexFilename;
	fontFilename = BD_NULL;
	fontTexFilename = BD_NULL;

	// Read the file version number
	ReadFile(fontFile, &fileVersion, sizeof(bdUInt), &numBytesRead, NULL);

	if (fileVersion == 0x00000005)
	{
		ReadFile(fontFile, &m_fontHeight, sizeof(bdFloat32), &numBytesRead, NULL);
		ReadFile(fontFile, &m_topPadding, sizeof(bdFloat32), &numBytesRead, NULL);
		ReadFile(fontFile, &m_bottomPadding, sizeof(bdFloat32), &numBytesRead, NULL);
		ReadFile(fontFile, &m_yAdvance, sizeof(bdFloat32), &numBytesRead, NULL);
		ReadFile(fontFile, &m_maxGlyph, sizeof(m_maxGlyph), &numBytesRead, NULL);

		m_translatorTable = new bdInt16[m_maxGlyph + 1];
		ReadFile(fontFile, m_translatorTable, (m_maxGlyph + 1) * sizeof(bdUInt16), &numBytesRead, NULL);

		ReadFile(fontFile, &m_numGlyphs, sizeof(unsigned long), &numBytesRead, NULL);
		m_glyphs = new bdGlyphAttr[m_numGlyphs];
		ReadFile(fontFile, m_glyphs, m_numGlyphs * sizeof(bdGlyphAttr), &numBytesRead, NULL);
	}
	else
	{
		OutputDebugString("XBFont: Incorrect version number on font file!\n");

		// Make sure we don't leak memory.
		if (m_translatorTableWasAllocated)
		{
			delete [] m_translatorTable;
		}

		m_translatorTable = BD_NULL;
		m_translatorTableWasAllocated = false;
		return false;
	}

	// Creates the vertex shader for rendering fonts, and a pixel shader for
	// font rendering to ensure NTSC safe colors
	if(!createShaders())
	{
		return false;
	}

	// Determine whether we should save/restore state
	D3DDEVICE_CREATION_PARAMETERS d3dcp;
	D3DDevice::GetCreationParameters( &d3dcp );
	m_saveState = (d3dcp.BehaviorFlags & D3DCREATE_PUREDEVICE) ? false : true;

	return true;
}

bdBool bdFont::destroy()
{
//	m_xprResource.Destroy();

	m_fontTexture = BD_NULL;

	m_fontHeight = 0.0f;
	m_topPadding = 0.0f;
	m_bottomPadding = 0.0f;
	m_yAdvance = 0.0f;

	m_numGlyphs = 0L;

	if (m_glyphs)
	{
		delete [] m_glyphs;
		m_glyphs = BD_NULL;
	}

	m_maxGlyph = 0;

	if( m_translatorTableWasAllocated )
	{
		delete [] m_translatorTable;
		m_translatorTable = BD_NULL;
	}

	m_translatorTable = BD_NULL;
	m_translatorTableWasAllocated = false;

	m_nestedBeginCount = 0L;

	return true;
}

void bdFont::setCursorPosition( bdFloat32 fCursorX, bdFloat32 fCursorY )
{
    m_cursorX = floorf( fCursorX );
    m_cursorY = floorf( fCursorY );
}

void bdFont::setScaleFactors( bdFloat32 fXScaleFactor, bdFloat32 fYScaleFactor )
{
    m_xScaleFactor = fXScaleFactor;
    m_yScaleFactor = fYScaleFactor;
}

void bdFont::setSlantFactor(bdFloat32 fSlantFactor)
{
    m_slantFactor = fSlantFactor;
}

bdBool bdFont::getTextExtent(const bdUInt16 *strText, bdFloat32 *const pWidth, 
							 bdFloat32 *const pHeight, bdBool bFirstLineOnly ) const
{
    // Check parameters
    if(NULL==strText || NULL==pWidth || NULL==pHeight)
	{
        return false;
	}

    // Set default text extent in output parameters
    (*pWidth)   = 0.0f;
    (*pHeight)  = 0.0f;

    // Initialize counters that keep track of text extent
    bdFloat32 sx = 0.0f;
    bdFloat32 sy = m_fontHeight;

    // Loop through each character and update text extent
    while(*strText)
    {
        bdUInt16 letter = *strText++;
    
        // Handle newline character
        if(letter == L'\n')
        {
            if( bFirstLineOnly )
                break;
            sx  = 0.0f;
            sy += m_yAdvance;
        }

        // Translate unprintable characters
        bdGlyphAttr* pGlyph;
        if (letter > m_maxGlyph || m_translatorTable[letter] == 0)
            pGlyph = &m_glyphs[0];
        else
            pGlyph = &m_glyphs[m_translatorTable[letter]];

        // Get text extent for this character's glyph
        sx += pGlyph->m_offset;
        sx += pGlyph->m_advance;

        // Store text extent of string in output parameters
        if(sx > (*pWidth))
		{
			(*pWidth)  = sx;
		}

        if(sy > (*pHeight))
		{
			(*pHeight) = sy;
		}
     }

    // Apply the scale factor to the result
    (*pWidth)  *= m_xScaleFactor;
    (*pHeight) *= m_yScaleFactor;

    return true;
}

bdFloat32 bdFont::getTextWidth( const bdUInt16* strText ) const
{
    bdFloat32 fTextWidth  = 0.0f;
    bdFloat32 fTextHeight = 0.0f;

    getTextExtent(strText, &fTextWidth, &fTextHeight);
    return fTextWidth;
}

void bdFont::begin()
{
    // Set state on the first call
    if( 0 == m_nestedBeginCount )
    {
        // Save state
        if( m_saveState )
        {
            // Note, we are not saving the texture, vertex, or pixel shader,
            //       since it's not worth the performance. We're more interested
            //       in saving state that would cause hard to find problems.
            D3DDevice::GetRenderState(D3DRS_ALPHABLENDENABLE, &m_savedState[0]);
            D3DDevice::GetRenderState(D3DRS_SRCBLEND,         &m_savedState[1]);
            D3DDevice::GetRenderState(D3DRS_DESTBLEND,        &m_savedState[2]);
            D3DDevice::GetRenderState(D3DRS_ALPHATESTENABLE,  &m_savedState[3]);
            D3DDevice::GetRenderState(D3DRS_ALPHAREF,         &m_savedState[4]);
            D3DDevice::GetRenderState(D3DRS_ALPHAFUNC,        &m_savedState[5]);
            D3DDevice::GetRenderState(D3DRS_FILLMODE,         &m_savedState[6]);
            D3DDevice::GetRenderState(D3DRS_CULLMODE,         &m_savedState[7]);
            D3DDevice::GetRenderState(D3DRS_ZENABLE,          &m_savedState[8]);
            D3DDevice::GetRenderState(D3DRS_STENCILENABLE,    &m_savedState[9]);
            D3DDevice::GetRenderState(D3DRS_EDGEANTIALIAS,    &m_savedState[10]);
            D3DDevice::GetTextureStageState(0, D3DTSS_MINFILTER, &m_savedState[11]);
            D3DDevice::GetTextureStageState(0, D3DTSS_MAGFILTER, &m_savedState[12]);
			D3DDevice::GetVertexShader(&m_savedState[13]);
			D3DDevice::GetPixelShader(&m_savedState[14]);
        }

        // Set the necessary render states
        D3DDevice::SetTexture(0, m_fontTexture);
        D3DDevice::SetVertexShader(m_fontVertexShader);
        D3DDevice::SetPixelShader(m_fontPixelShader);
        D3DDevice::SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        D3DDevice::SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_SRCALPHA);
        D3DDevice::SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_INVSRCALPHA);
        D3DDevice::SetRenderState(D3DRS_ALPHATESTENABLE,  TRUE);
        D3DDevice::SetRenderState(D3DRS_ALPHAREF,         0x08);
        D3DDevice::SetRenderState(D3DRS_ALPHAFUNC,        D3DCMP_GREATEREQUAL);
        D3DDevice::SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
        D3DDevice::SetRenderState(D3DRS_CULLMODE,         D3DCULL_NONE);
        D3DDevice::SetRenderState(D3DRS_ZENABLE,          FALSE);
        D3DDevice::SetRenderState(D3DRS_STENCILENABLE,    FALSE);
        D3DDevice::SetRenderState(D3DRS_EDGEANTIALIAS,    FALSE);
        D3DDevice::SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
        D3DDevice::SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

        // Begin the drawing of vertices
        D3DDevice::Begin(D3DPT_QUADLIST);
    }

    // Keep track of the nested begin/end calls.
    m_nestedBeginCount++;
}

bdBool bdFont::drawText(D3DCOLOR dwColor, const bdUInt16* strText, bdULong dwFlags, bdFloat32 fMaxPixelWidth)
{
    return drawText(m_cursorX, m_cursorY, dwColor, strText, dwFlags, fMaxPixelWidth);
}

bdBool bdFont::drawText(bdFloat32 fOriginX, bdFloat32 fOriginY, D3DCOLOR dwColor,
						const bdUInt16* strText, bdULong dwFlags, bdFloat32 fMaxPixelWidth)
{
    // Set up stuff (i.e. lock the vertex buffer) to prepare for drawing text
    begin();

    // Set the common color for all the vertices to follow
    D3DDevice::SetVertexDataColor( D3DVSDE_DIFFUSE, dwColor );

    // Set the starting screen position
    m_cursorX = floorf( fOriginX );
    m_cursorY = floorf( fOriginY );

    // Adjust for padding
    fOriginY -= m_topPadding;

    bdFloat32 fEllipsesPixelWidth = m_xScaleFactor * 3.0f * (m_glyphs[m_translatorTable[L'.']].m_offset + m_glyphs[m_translatorTable[L'.']].m_advance);

    if( dwFlags & BD_FONT_TRUNCATED )
    {
        // Check if we will really need to truncate the string
        if( fMaxPixelWidth <= 0.0f )
        {
            dwFlags &= (~BD_FONT_TRUNCATED);
        }
        else
        {
            bdFloat32 w, h;
            getTextExtent(strText, &w, &h, true);
    
            // If not, then clear the flag
            if( w <= fMaxPixelWidth )
                dwFlags &= (~BD_FONT_TRUNCATED);
        }
    }

    // If vertically centered, offset the starting m_cursorY value
    if( dwFlags & BD_FONT_CENTER_Y )
    {
        bdFloat32 w, h;
        getTextExtent( strText, &w, &h );
        m_cursorY = floorf( m_cursorY - h / 2 );
    }

    // Set a flag so we can determine initial justification effects
    bdBool bStartingNewLine = true;

    while(*strText)
    {
        // If starting text on a new line, determine justification effects
        if(bStartingNewLine)
        {
            if( dwFlags & (BD_FONT_RIGHT|BD_FONT_CENTER_X) )
            {
                // Get the extent of this line
                bdFloat32 w, h;
                getTextExtent(strText, &w, &h, true);

                // Offset this line's starting m_cursorX value
                if( dwFlags & BD_FONT_RIGHT )
                    m_cursorX = floorf( fOriginX - w );
                if( dwFlags & BD_FONT_CENTER_X )
                    m_cursorX = floorf( fOriginX - w / 2 );
            }
            bStartingNewLine = false;
        }

        // Get the current letter in the string
        bdUInt16 letter = *strText++;

        // Handle the newline character
        if( letter == L'\n' )
        {
            m_cursorX  = fOriginX;
            m_cursorY += m_yAdvance;
            bStartingNewLine = true;
            continue;
        }

        // Translate unprintable characters
        bdGlyphAttr* pGlyph = &m_glyphs[ (letter <= m_maxGlyph) ? m_translatorTable[letter] : 0 ];

        bdFloat32 offset  = m_xScaleFactor * static_cast<bdFloat32>(pGlyph->m_offset);
        bdFloat32 advance = m_xScaleFactor * static_cast<bdFloat32>(pGlyph->m_advance);
        bdFloat32 width   = m_xScaleFactor * static_cast<bdFloat32>(pGlyph->m_width);
        bdFloat32 height  = m_yScaleFactor * m_fontHeight;

        if( dwFlags & BD_FONT_TRUNCATED )
        {
            // Check if we will be exceeded the max allowed width
            if( m_cursorX + offset + width + fEllipsesPixelWidth + m_slantFactor > fOriginX + fMaxPixelWidth )
            {
                // Yup. Let's draw the ellipses, then bail
                drawText(m_cursorX, m_cursorY, dwColor, L"...");
                end();
                return true;
            }
        }

        // Setup the screen coordinates
        m_cursorX += offset;
        bdFloat32 left1  = m_cursorX;
        bdFloat32 left2  = left1 + m_slantFactor;
        bdFloat32 right1 = left1 + width;
        bdFloat32 right2 = left2 + width;
        bdFloat32 top    = m_cursorY;
        bdFloat32 bottom = top + height;
        m_cursorX += advance;

        // Draw the quad using SetVertexData
        D3DDevice::SetVertexData4f(D3DVSDE_VERTEX, left1, bottom, pGlyph->m_tu1, pGlyph->m_tv2);
        D3DDevice::SetVertexData4f(D3DVSDE_VERTEX, left2, top, pGlyph->m_tu1, pGlyph->m_tv1);
        D3DDevice::SetVertexData4f(D3DVSDE_VERTEX, right2, top, pGlyph->m_tu2, pGlyph->m_tv1);
        D3DDevice::SetVertexData4f(D3DVSDE_VERTEX, right1, bottom, pGlyph->m_tu2, pGlyph->m_tv2);
    }

    // Call End() to complete the begin/end pair for drawing text
    end();

    return true;
}

bdBool bdFont::end()
{
    // Keep track of nested calls to begin/end.
    if(m_nestedBeginCount == 0L)
	{
        return false;
	}

    if(--m_nestedBeginCount > 0)
	{
        return true;
	}

    // End the drawing of vertex data
    D3DDevice::End();

    // Restore state
    if(m_saveState)
    {
        D3DDevice::SetTexture(0, NULL);
        D3DDevice::SetPixelShader(m_savedState[14]);
		D3DDevice::SetVertexShader(m_savedState[13]);
        D3DDevice::SetRenderState(D3DRS_ALPHABLENDENABLE, m_savedState[0]);
        D3DDevice::SetRenderState(D3DRS_SRCBLEND,         m_savedState[1]);
        D3DDevice::SetRenderState(D3DRS_DESTBLEND,        m_savedState[2]);
        D3DDevice::SetRenderState(D3DRS_ALPHATESTENABLE,  m_savedState[3]);
        D3DDevice::SetRenderState(D3DRS_ALPHAREF,         m_savedState[4]);
        D3DDevice::SetRenderState(D3DRS_ALPHAFUNC,        m_savedState[5]);
        D3DDevice::SetRenderState(D3DRS_FILLMODE,         m_savedState[6]);
        D3DDevice::SetRenderState(D3DRS_CULLMODE,         m_savedState[7]);
        D3DDevice::SetRenderState(D3DRS_ZENABLE,          m_savedState[8]);
        D3DDevice::SetRenderState(D3DRS_STENCILENABLE,    m_savedState[9]);
        D3DDevice::SetRenderState(D3DRS_EDGEANTIALIAS,    m_savedState[10]);
        D3DDevice::SetTextureStageState(0, D3DTSS_MINFILTER, m_savedState[11]);
        D3DDevice::SetTextureStageState(0, D3DTSS_MAGFILTER, m_savedState[12]);
    }

    return true;
}

LPDIRECT3DTEXTURE8 bdFont::createTexture(	const bdUInt16* strText, 
											D3DCOLOR dwBackgroundColor, 
											D3DCOLOR dwTextColor, 
											D3DFORMAT d3dFormat)
{
    // Calculate texture dimensions
    bdFloat32 fTexWidth;
    bdFloat32 fTexHeight;
    getTextExtent(strText, &fTexWidth, &fTexHeight);
    bdUInt dwWidth  = (bdUInt)fTexWidth;
    bdUInt dwHeight = (bdUInt)fTexHeight;

    switch( d3dFormat )
    {
        case D3DFMT_A8R8G8B8:
        case D3DFMT_X8R8G8B8:
        case D3DFMT_R5G6B5:
        case D3DFMT_X1R5G5B5:
            // For swizzled textures, make sure the dimensions are a power of two
            for( bdUInt wmask=1; dwWidth&(dwWidth-1); wmask = (wmask<<1)+1 )
                dwWidth  = ( dwWidth + wmask ) & ~wmask;
            for( bdUInt hmask=1; dwHeight&(dwHeight-1); hmask = (hmask<<1)+1 )
                dwHeight = ( dwHeight + hmask ) & ~hmask;

            // Also, enforce a minimum pitch of 64 bytes
            dwWidth = max( 64/XGBytesPerPixelFromFormat(d3dFormat), dwWidth );

            break;

        case D3DFMT_LIN_A8R8G8B8:
        case D3DFMT_LIN_X8R8G8B8:
        case D3DFMT_LIN_R5G6B5:
        case D3DFMT_LIN_X1R5G5B5:
            // For linear textures, make sure the stride is a multiple of 64 bytes
            dwWidth  = ( dwWidth + 0x1f ) & ~0x1f;
            break;

        default:
            // All other formats are unsupported as render targets
            return NULL;
    }

    // Create the texture
    LPDIRECT3DTEXTURE8 pTexture;
    if( FAILED( D3DDevice::CreateTexture( dwWidth, dwHeight, 1, 0L, d3dFormat, 
                                          D3DPOOL_DEFAULT, &pTexture ) ) )
        return NULL;
    
    // Get the current backbuffer and zbuffer
    LPDIRECT3DSURFACE8 pBackBuffer, pZBuffer;
    D3DDevice::GetRenderTarget( &pBackBuffer );
    D3DDevice::GetDepthStencilSurface( &pZBuffer );

    // Set the new texture as the render target
    LPDIRECT3DSURFACE8 pTextureSurface;
    pTexture->GetSurfaceLevel( 0, &pTextureSurface );
    D3DVIEWPORT8 vp = { 0, 0, dwWidth, dwHeight, 0.0f, 1.0f };
    D3DDevice::SetRenderTarget( pTextureSurface, NULL );
    D3DDevice::SetViewport( &vp );
    D3DDevice::Clear( 0L, NULL, D3DCLEAR_TARGET, dwBackgroundColor, 1.0f, 0L );

    // Render the text
    drawText( 0, 0, dwTextColor, strText, 0L );

    // Restore the render target
    D3DVIEWPORT8 vpBackBuffer = { 0, 0, 640, 480, 0.0f, 1.0f };
    D3DDevice::SetRenderTarget( pBackBuffer, pZBuffer );
    D3DDevice::SetViewport( &vpBackBuffer );

	if (pBackBuffer)
	{
		pBackBuffer->Release();
	}

    if (pZBuffer)
	{
		pZBuffer->Release();
	}

    if (pTextureSurface)
	{
		pTextureSurface->Release();
	}

    // Return the new texture
    return pTexture;
}

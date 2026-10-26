#include "stdafx.h"
#include "XMLParser.h"
#include "Fontmaker.h"
#include "SystemCommands.h"
#include "commandline.h"
#include "Config.h"
#include <math.h>

FontMaker gFontMaker;

/*
WCHAR uni_characters[EUROPEAN_TOTAL] = 
	{
	// NULL Character and Space
	0x02c8,																			//1
	//1-0(Basic)
	0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039,0x0030,			//10
	//a-z (Basic)
	0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,0x0068,0x0069,0x006a,			//26
	0x006b,0x006c,0x006d,0x006e,0x006f,0x0070,0x0071,0x0072,0x0073,0x0074,
	0x0075,0x0076,0x0077,0x0078,0x0079,0x007a,
	//A-Z (Basic)
	0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,0x0048,0x0049,0x004a,			//26
	0x004b,0x004c,0x004d,0x004e,0x004f,0x0050,0x0051,0x0052,0x0053,0x0054,
	0x0055,0x0056,0x0057,0x0058,0x0059,0x005a,
	//Symbols (Basic)
	0x0028,0x0029,0x0026,0x005f,0x005e,0x0025,0x005c,0x002F,0x0040,0x0023,			//39
	0x005b,0x005d,0x0024,0x00a3,0x00a5,0x20ac,0x003b,0x003a,0x0027,0x0022,
	0x003c,0x003e,0x003f,0x0021,0x00bf,0x00a1,0x002d,0x002a,0x002b,0x003d,
	0x007b,0x007d,0x00ab,0x00bb,0x002c,0x002e,0x0060,0x007c,0x007e,					
	
	//à-ÿ Accents (Extended)
	0x00e0,0x00e1,0x00e2,0x00e4,0x00e6,0x00e7,0x00e8,0x00e9,0x00ea,0x00eb,			//26
	0x00ec,0x00ed,0x00ee,0x00ef,0x00f1,0x00f2,0x00f3,0x00f4,0x00f5,0x00df,
	0x00f9,0x00fa,0x00fb,0x00fc,0x00fd,0x00ff,
	//À-Ÿ Accents (Extended)
	0x00c0,0x00c1,0x00c2,0x00c4,0x00c6,0x00c7,0x00c8,0x00c9,0x00ca,0x00cb,			//26
	0x00cc,0x00cd,0x00ce,0x00cf,0x00d1,0x00d2,0x00d3,0x00d4,0x00d5,0x00df,
	0x00d9,0x00da,0x00db,0x00dc,0x00dd,0x0178,										// (154)

	//additional chars for MS
	0x00e3,0x00c3,0x00f6,0x00d6,0x00b0,0x00ba,0x00b9,0x00b2,0x00b3,0x00aa,				//11
	0x0152,0x0153,

	//copyright, trade mark, registered													//3
	0x00a9,0x2122,0x00ae,
	};
*/

void FontMaker::StartFontMaker(char *filename)
{
	strcpy(mFileName,filename);

	mFontWeight=400;
	mItalicized=false;
	mLanguage=UIFONT_LANGUAGE_EUROPEAN;
	mType=FM_TYPE_UI;
	mVersion=0x00000004;
	mTexWidth=64;
	mTexHeight=64;       
	mTexBPP=16;
	mDropShadow=false;
	mGlow = false;
	mGlowGamma = 2.0f;
	mOutline=0;
	mExtrude=0;
	mAntiAliased=false;
	mBlobby=false;
	mIntensityOutline=false;
	mBlackAndWhite=false,
	mVrtSpacing=0;
	mHrzSpacing=0;

	mSizeOveride=1.f;
	mTopOffset=0.f;

	memset(&mFontSets,0,sizeof(FontAttributes)*MAX_FONT_SETS);

	mNumFontSets=0;
	mTotalBundles=0;
	mEaBundles=0;
}

HRESULT FontMaker::CalcTextureSize( HDC hFontDC,int* pdwTexWidth, int* pdwTexHeight,int total )
{
    int dwTexWidth  = 64; // Initial (minimum) texture dimensions
    int dwTexHeight = 64;
    int x           =  mHrzSpacing; // Initial cursor position is (0,0)
    int y           =  mVrtSpacing; 

	int a,c;

	a=total;

	for(c = 0; c < a; c++ )
	{
		WCHAR str[2] = L"x";
		SIZE  size;
		ABC   abc;

		//if(mLanguage == UIFONT_LANGUAGE_EUROPEAN) str[0] = uni_characters[c];
		//else if(mLanguage == UIFONT_LANGUAGE_JAPANESE) str[0] = jap_characters[c];
		//else if(mLanguage == UIFONT_LANGUAGE_KOREAN) str[0] = korean_characters[c];
		//else if(mLanguage == UIFONT_LANGUAGE_CHINESE) str[0] = chinese_characters[c];

		str[0]=mCurrentGlyphs[c];
		if(str[0] == 32)
			continue;

		// Get the size needed to render the glyph
		GetTextExtentPoint32W( hFontDC, str, 1, &size );

		// Check if the glpyh would exceed the right edge of the texture
//		if( (x+2*size.cx+1+mExtrude+mHrzSpacing) > dwTexWidth )
		if( x + size.cx + (mOutline*2) + (mDropShadow*2) + mExtrude + mHrzSpacing > dwTexWidth )
		{
			// If so, advance the cursor down a line
			x  = mHrzSpacing;
			y += size.cy+mExtrude;
			y += (mVrtSpacing*2);

			// Now check if we go past the bottom edge of the texture
			if( (y+size.cy+1) > dwTexHeight )
			{
				// If so, grow the texture size by a power of two
				if( dwTexWidth < dwTexHeight )
					dwTexWidth *= 2;
				else
					dwTexHeight *= 2;

				// And restart the loop at the first glyph
				x = 0;
				y = 0;
				c = 0;

				//if(mLanguage == UIFONT_LANGUAGE_EUROPEAN) str[0] = uni_characters[c];
				//else if(mLanguage == UIFONT_LANGUAGE_JAPANESE) str[0] = jap_characters[c];
				//else if(mLanguage == UIFONT_LANGUAGE_KOREAN) str[0] = korean_characters[c];
				//else if(mLanguage == UIFONT_LANGUAGE_CHINESE) str[0] = chinese_characters[c];

				str[0]=mCurrentGlyphs[c];
				
				GetTextExtentPoint32W( hFontDC, str, 1, &size );
			}
		}
		// Update the cursor from the ABC widths for the letter
		if( FALSE == GetCharABCWidthsW( hFontDC, str[0], str[0], &abc ) )
			x += size.cx + (mOutline*2) + (mDropShadow*2) + mExtrude;
		else
			x += abc.abcB + (mOutline*2) + (mDropShadow*2) + mExtrude;
		x+=mHrzSpacing*2;
	}

	// Return the result
    (*pdwTexWidth)  = dwTexWidth;
    (*pdwTexHeight) = dwTexHeight;

    return S_OK;
}

HRESULT FontMaker::WriteTargaFile( char* strFileName, int dwWidth, int dwHeight, int* pRGBAData )
{
	gCommandLine.AddLineArg(TC_BLACK,"    Creating TGA %s (%d %d)",strFileName,dwWidth,dwHeight);

	struct TargaHeader
    {
        BYTE IDLength;
        BYTE ColormapType;
        BYTE ImageType;
        BYTE ColormapSpecification[5];
        WORD XOrigin;
        WORD YOrigin;
        WORD ImageWidth;
        WORD ImageHeight;
        BYTE PixelDepth;
        BYTE ImageDescriptor;
    } tga;

	HRESULT ret;

    // Create the file
	char filename[256];
	sprintf(filename,"%s\\%s",mTextureDirectory,strFileName);
	FILE* file = fopen( filename, "wb" );
	if(file)
	{
		// Write the TGA header
		ZeroMemory( &tga, sizeof(tga) );
		tga.IDLength        = 0;
		tga.ImageType       = 2;
		tga.ImageWidth      = (WORD)dwWidth;
		tga.ImageHeight     = (WORD)dwHeight;
		tga.PixelDepth      = 32;
		tga.ImageDescriptor = 0x28;
		fwrite( &tga, sizeof(TargaHeader), 1, file );

		// Write the pixels
		fwrite( pRGBAData, sizeof(int), dwHeight*dwWidth, file );

		// Close the file and return okay
		fclose( file );

		ret = S_OK;
	}
	else
	{
		ret = E_FAIL;
	}

	// If it failed to open it *may* be because read only files exist but we still may bundle them later
	strcpy(mBundledTextures[mNumBundledTextures],strFileName);
	mNumBundledTextures++;

    return ret;
}

HRESULT FontMaker::WriteFontInfoFile(char *name)
{
    // Create the info file
	char datname[128];
	char filename[256];

	sprintf(datname,"%s.dat",name);
	sprintf(filename,"%s\\%s",gConfig.mPCExport,datname);

	FILE* file = fopen( filename, "wb+" );
    if( NULL == file ) return E_FAIL;

	// Version number (increment if the structure format changes)
	fwrite(&mVersionNumber,sizeof(int),1,file);

    // Write out the texture values
	fwrite(&mNumFontSets,sizeof(int),1,file);

	for(int a=0;a<mNumFontSets;a++)
	{
		fwrite(&mFontSets[a].Name,sizeof(char),128,file);

		fwrite(&mFontSets[a].Language,sizeof(int),1,file);

		fwrite(&mFontSets[a].TextureName,sizeof(char),128,file);
		fwrite(&mFontSets[a].TextureWidth,sizeof(int),1,file);
		fwrite(&mFontSets[a].TextureHeight,sizeof(int),1,file);
		fwrite(&mFontSets[a].Number,sizeof(int),1,file);
		fwrite(&mFontSets[a].FontHeight,sizeof(int),1,file);
		fwrite(&mFontSets[a].ExtrudeSize,sizeof(int),1,file);
		fwrite(&mFontSets[a].SpacingSize,sizeof(int),1,file);

		fwrite(&mFontSets[a].TopOffset,sizeof(float),1,file);
		fwrite(&mFontSets[a].SizeOveride,sizeof(float),1,file);

		fwrite(mFontSets[a].Glyphs,sizeof(struct GLYPH_ATTR),mFontSets[a].Number,file);

		fwrite(&mFontSets[a].NumKernPairs,sizeof(int),1,file);
		fwrite(mFontSets[a].Kerning,sizeof(struct KERN_ATTR),mFontSets[a].NumKernPairs,file);
	}

    // Close the file and return okay
    fclose( file );

	char command[1024];
	sprintf(command,"xbcp /y \"%s\\%s\" %s\\data\\%s",mTextureDirectory,datname,"Target",datname);
	gSystemCommands.AddCommand(command);

	//if(gUI.Config.CopytoServer)
	//{
	//	sprintf(command,"copy /y \"%s\\%s\" %s\\data\\%s",mTextureDirectory,datname,"Server",datname);
	//	gSystemCommands.AddCommand(command);
	//}

	//if(gUI.Config.Export == UIEXPORT_FULL)
	//{
	//	sprintf(command,"copy /y \"%s\\UIFont.lua\" %s\\UIFont.lua",gUI.Config.UIMakerTextureDir,gUI.Config.LuaDir);
	//	gSystemCommands.AddCommand(command);

	//	sprintf(command,"xbcp /y \"%s\\UIFont.lua\" %s\\scripts\\UIFont.lua",gUI.Config.UIMakerTextureDir,gUI.Config.TargetDir);
	//	gSystemCommands.AddCommand(command);

	//	if(gUI.Config.CopytoServer)
	//	{
	//		sprintf(command,"copy /y \"%s\\UIFont.lua\" %s\\scripts\\UIFont.lua",gUI.Config.UIMakerTextureDir,gUI.Config.ServerDir);
	//		gSystemCommands.AddCommand(command);
	//	}
	//}

	return S_OK;
}

int FontMaker::GetFont(char *name)
{
	for(int a=0;a<mNumFontSets;a++)
	{
		if(!strcmp(name,mFontSets[a].Name))
		{
			return a;
		}
	}

	return 0;
}

void FontMaker::WriteFontStore(HDC hDC,char *name,int number_of_fonts,char *texture_name)
{
	gCommandLine.AddLineArg(TC_BLACK,"    Creating Font Data %s with %d glyphs",name,number_of_fonts);

	KERNINGPAIR p[4096];
	int num_kern_pairs=GetKerningPairs(hDC,4096,&p[0]);

	strcpy(mFontSets[mNumFontSets].Name,name);

	mFontSets[mNumFontSets].Language=mLanguage;

	strtok(texture_name,".");
	strcpy_s(mFontSets[mNumFontSets].TextureName, sizeof(mFontSets[mNumFontSets].TextureName), texture_name);
	_strlwr(mFontSets[mNumFontSets].TextureName);

	mFontSets[mNumFontSets].TextureHeight=mTexHeight;
	mFontSets[mNumFontSets].TextureWidth=mTexWidth;
	mFontSets[mNumFontSets].Number=number_of_fonts;
	mFontSets[mNumFontSets].FontHeight=mFontEffectiveHeight;
	mFontSets[mNumFontSets].ExtrudeSize=mExtrude;
	mFontSets[mNumFontSets].SpacingSize=mHrzSpacing;
	mFontSets[mNumFontSets].TopOffset=mTopOffset;
	mFontSets[mNumFontSets].SizeOveride=mSizeOveride;

	if(mFontSets[mNumFontSets].Glyphs)
	{
		delete mFontSets[mNumFontSets].Glyphs;
		mFontSets[mNumFontSets].Glyphs=NULL;
	}

	mFontSets[mNumFontSets].Glyphs = new GLYPH_ATTR[number_of_fonts];

	memset(mFontSets[mNumFontSets].Glyphs,0,sizeof(GLYPH_ATTR)*number_of_fonts);

	memcpy(mFontSets[mNumFontSets].Glyphs,mGlyphs,sizeof(GLYPH_ATTR)*number_of_fonts);

	mFontSets[mNumFontSets].NumKernPairs=0;
	for(int a=0;a<num_kern_pairs;a++)
	{
		if(p[a].iKernAmount)
		{
			mFontSets[mNumFontSets].NumKernPairs++;
		}
	}

	mFontSets[mNumFontSets].Kerning = new KERN_ATTR[mFontSets[mNumFontSets].NumKernPairs];

	int b=0;
	for(int a=0;a<num_kern_pairs;a++)
	{
		if(p[a].iKernAmount)
		{
			mFontSets[mNumFontSets].Kerning[b].amount=p[a].iKernAmount;
			mFontSets[mNumFontSets].Kerning[b].first=p[a].wFirst;
			mFontSets[mNumFontSets].Kerning[b].second=p[a].wSecond;
			b++;
		}
	}

	mNumFontSets++;
}

void FontMaker::FillGlyphTable(char *name,int start,UIFONT_GLYPH_FILE_TYPE type)
{
	char filename[256];
	int foreign_chars[0xffff];
	
	memset(&foreign_chars,0,sizeof(int)*0xffff);

	mNumForeignChars=0;

	FILE *fp;
	
	WCHAR wc;
	sprintf(filename,"%s\\%s",mTextDirectory,name);

	fp=fopen(filename,"rb");

	bool read_glpyhs=false;

	if(fp)
	{
		wc=fgetwc(fp);
		while(wc != 0xffff)
		{
			if(read_glpyhs)
			{
				foreign_chars[wc]++;
			}			

			if(wc == 0x003d)	//we have an equals 
			{
				read_glpyhs=true;
			}
			else if(wc == 0x000d)	//we have an eol
			{
				read_glpyhs=false;
			}
			wc=fgetwc(fp);
		}

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Opening Foreign Language Glyph File",MB_OK|MB_ICONINFORMATION);
	}


	foreign_chars[0] = 1;
	for(int a=start;a<0xffff;a++)
	{
		if(foreign_chars[a])
		{
			mCurrentGlyphs[mNumForeignChars]=a;
			mNumForeignChars++;
		}
	}
}

int SampleColour(int left, int top, int right, int bottom, int* bits, int bitWidth, int x, int y)
{
	if ((x < left) || (y < top) || (x >= (right - 1)) || (y >= (bottom - 1)))
		return 0;

	return bits[(y * bitWidth) + x];
}

template<typename T>
T Clamp(T value, T low, T high)
{
	return (value < low) ? low : ((value > high) ? high : value);
}

template<typename T>
T Square(T v)
{
	return v * v;
}

double GaussianDistribution(double x, double y, double rho)
{
	return exp(-((x * x) + (y * y)) / (2.0 * rho * rho)) / sqrt(2.0 * 3.14159 * rho * rho);
}

HRESULT FontMaker::CreateFontFiles(char *name,char *texture_name, char* overide_name, char *compression, int levels)
{
	int tempy;
	int aa=NONANTIALIASED_QUALITY;
	if(mAntiAliased) aa=ANTIALIASED_QUALITY;
	int texture=0;

	char *mBackupFontName="Arial Unicode MS";

	memset(mGlyphs,0,sizeof(GLYPH_ATTR)*MAX_GLYPHS);

	gCommandLine.AddLineArg(TC_BLACK,"    Creating font %s with texture %s...",name,texture_name);

    // Create a DC for rendering text
    HDC hDC = CreateCompatibleDC( NULL );
    HDC hDC1 = CreateCompatibleDC( NULL );

#ifdef _BACKUP_FONT
    // Create a font. Note that the the font height is converted to what a form
    // that CreateFont expects. Also note that by specifying ANTIALIASED_QUALITY,
    // we might get an antialiased font, but this is not guaranteed.
    HFONT hBackupFont = CreateFont( -MulDiv( mFontHeight, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ), 
													  0, 0, 0, mFontWeight, mItalicized,
													  FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
													  CLIP_DEFAULT_PRECIS, aa,
													  VARIABLE_PITCH, mBackupFontName );

    if( NULL==hBackupFont )
        return E_FAIL;
#endif

	HFONT hFont = CreateFont( -MulDiv( mFontHeight, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ), 
                              0, 0, 0, mFontWeight, mItalicized,
                              FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, aa,
                              VARIABLE_PITCH, mFontName );
    if( NULL==hFont )
        return E_FAIL;

    // Setup the DC for the font
    SetTextColor( hDC, RGB(255,255,255) );
    SetBkColor( hDC, 0x00000000 );
    SetBkMode( hDC, TRANSPARENT );
    SelectObject( hDC, hFont );
    SetTextAlign( hDC, TA_TOP );
    SetMapMode( hDC, MM_TEXT );

    SetTextColor( hDC1, RGB(255,255,255) );
    SetBkColor( hDC1, 0xff00f000 );
    SetBkMode( hDC1, TRANSPARENT );
    SelectObject( hDC1, hFont );
    SetTextAlign( hDC1, TA_TOP );
    SetMapMode( hDC1, MM_TEXT );

    // Determine the texture size
    CalcTextureSize( hDC, &mTexWidth, &mTexHeight, mNumForeignChars);

#ifdef _BACKUP_FONT
    // Recreate the font. Okay, this is weird, but, apparently, calling text
    // functions such as GetTextExtentPoint32() or GetCharABCWidths() before
    // creating a DIB section (the bitmap below) breaks antialiasing of the
    // font. The CalcTextureSize() function made some of these calls, so we
    // now get around the problem by re-creating the font.
    DeleteObject( hBackupFont );

    hBackupFont = CreateFont( -MulDiv( mFontHeight, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ), 
													  0, 0, 0, mFontWeight, mItalicized,
													  FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
													  CLIP_DEFAULT_PRECIS, aa,
													  VARIABLE_PITCH, mBackupFontName );
    if( NULL==hBackupFont )
        return E_FAIL;

	DeleteObject( hFont );
#endif

    hFont = CreateFont( -MulDiv( mFontHeight, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ), 
                    0, 0, 0, mFontWeight, mItalicized,
                    FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, aa,
                    VARIABLE_PITCH, mFontName );
    if( NULL==hFont )
        return E_FAIL;

    SelectObject( hDC, hFont );
    SelectObject( hDC1, hFont );

    // Prepare to create a bitmap
    int*     pBitmapBits;
    int*     pBitmapBits1;
    BITMAPINFO bmi;
    ZeroMemory( &bmi.bmiHeader, sizeof(BITMAPINFOHEADER) );
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       =  (int)mTexWidth;
    bmi.bmiHeader.biHeight      = -(int)mTexHeight;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biBitCount    = 32;

    BITMAPINFO bmi1;
    ZeroMemory( &bmi1.bmiHeader, sizeof(BITMAPINFOHEADER) );
    bmi1.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi1.bmiHeader.biWidth       =  (int)mTexWidth;
    bmi1.bmiHeader.biHeight      = -(int)mTexHeight;
    bmi1.bmiHeader.biPlanes      = 1;
    bmi1.bmiHeader.biCompression = BI_RGB;
    bmi1.bmiHeader.biBitCount    = 32;

    // Create a bitmap for the font
    HBITMAP hbmBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits, NULL, 0 );

    HBITMAP hbmBitmap1 = CreateDIBSection( hDC1, &bmi1, DIB_RGB_COLORS,
                                          (VOID**)&pBitmapBits1, NULL, 0 );
	SelectObject( hDC, hbmBitmap );
	SelectObject( hDC1, hbmBitmap1 );

    // Get the effective font height
    WCHAR str[2] = L"A";
    SIZE  size;
    GetTextExtentPoint32W( hDC, str, 1, &size );
    mFontEffectiveHeight = size.cy + 2 + (mOutline*2) + (mDropShadow*2) + mExtrude;

	// gaus

	const int drawRadius = 15;

	float gau[(drawRadius + 1 + drawRadius) * (drawRadius + 1 + drawRadius)];

	float gauCentre = (float)GaussianDistribution(0, 0, 10.0f);

	for(int fy = -drawRadius; fy <= drawRadius; fy++)
	{
		for(int fx = -drawRadius; fx <= drawRadius; fx++)
		{
			float d = 0.0f;

			for(int g = 0; g < 5; g++)
			{
				float s = powf(2.0f, (float)g);

				float v = (float)GaussianDistribution(fx * s, fy * s, 10.0f) / (5.0f * gauCentre);

				d += v;
			}

			gau[((fy + drawRadius) * (drawRadius + 1 + drawRadius)) + fx + drawRadius] = powf(d, 1.5f) / (24.0f * 2.0f);
		}
	}

    // Loop through all printable character and output them to the bitmap..
    // Meanwhile, keep track of the corresponding tex coords for each character.
    int index = 0;
    int x     = mHrzSpacing+mOutline;
    int y     = mVrtSpacing+mOutline;
	int a,c;

	a=mNumForeignChars;

	for(c = 0; c < a; c++ )
	{
		//if(mLanguage == UIFONT_LANGUAGE_EUROPEAN) str[0] = uni_characters[c];
		//else if(mLanguage == UIFONT_LANGUAGE_JAPANESE) str[0] = jap_characters[c];
		//else if(mLanguage == UIFONT_LANGUAGE_KOREAN) str[0] = korean_characters[c];
		//else if(mLanguage == UIFONT_LANGUAGE_CHINESE) str[0] = chinese_characters[c];

		SelectObject( hDC, hFont );
		SelectObject( hDC1, hFont );

		str[0]=mCurrentGlyphs[c];
		if(str[0] == 32)
			continue;

		GetTextExtentPoint32W( hDC, str, 1, &size );

		WORD gi;
		GetGlyphIndicesW(hDC,str,1,&gi,GGI_MARK_NONEXISTING_GLYPHS);

#ifdef _BACKUP_FONT
		if(gi == 0xffff)
		{
			SelectObject( hDC, hBackupFont );
			SelectObject( hDC1, hBackupFont );

			GetTextExtentPoint32W( hDC, str, 1, &size );
		}
#else
		if(gi == 0xffff && c != 0 ) // Inclide the NULL first
			continue;
#endif

//		if( (x+2*size.cx+1+mExtrude) > mTexWidth )

		if(x + size.cx + (mOutline*2) + (mDropShadow*2) + mExtrude + mHrzSpacing > mTexWidth )
		{
			x  = mHrzSpacing;
			y += size.cy + mExtrude;
			y += (mVrtSpacing*2);
		}

		// Get the ABC widths for the letter
		ABC abc;
		if( FALSE == GetCharABCWidthsW( hDC, str[0], str[0], &abc ) )
		{
			abc.abcA = 0;
			abc.abcB = size.cx;
			abc.abcC = 0;
		}

		if(mOutline>0 || mDropShadow || mExtrude)
		{
			FLOAT w = (FLOAT)abc.abcB + (mHrzSpacing*2) + (mOutline*2) + mExtrude;
			FLOAT h = (FLOAT)size.cy + (mVrtSpacing*2) + (mOutline*2) + mExtrude;
			FLOAT px = (FLOAT)x-mHrzSpacing-mOutline;
			FLOAT py = (FLOAT)y-mVrtSpacing-mOutline;

			mGlyphs[index].wOffset  = (short)(abc.abcA);
			mGlyphs[index].wWidth   = (short)(w);
			mGlyphs[index].wAdvance = (short)(abc.abcB+mOutline+abc.abcC);
			mGlyphs[index].left     = (FLOAT)(px+2)		/ (FLOAT)(mTexWidth-1);
			mGlyphs[index].top      = (FLOAT)(py+2)		/ (FLOAT)(mTexHeight-1);
			mGlyphs[index].right    = (FLOAT)(px+w+2)	/ (FLOAT)(mTexWidth-1);
			mGlyphs[index].bottom   = (FLOAT)(py+h+2)	/ (FLOAT)(mTexHeight-1);
			mGlyphs[index].character= str[0];
			index++;
		}
		else
		{
			FLOAT w = (FLOAT)abc.abcB + (mHrzSpacing*2);
			FLOAT h = (FLOAT)size.cy + (mVrtSpacing*2);
			FLOAT px = (FLOAT)x-mHrzSpacing;
			FLOAT py = (FLOAT)y-mVrtSpacing;

			if(!mHrzSpacing) w++;
			if(!mVrtSpacing) h++;

			mGlyphs[index].wOffset  = (short)(abc.abcA);
			mGlyphs[index].wWidth   = (short)(w);
			mGlyphs[index].wAdvance = (short)(abc.abcB + abc.abcC);
			mGlyphs[index].left     = (FLOAT) px	/ (FLOAT)(mTexWidth-1);
			mGlyphs[index].top      = (FLOAT) py	/ (FLOAT)(mTexHeight-1);
			mGlyphs[index].right    = (FLOAT)(px+w)	/ (FLOAT)(mTexWidth-1);
			mGlyphs[index].bottom   = (FLOAT)(py+h)	/ (FLOAT)(mTexHeight-1);
			mGlyphs[index].character= str[0];
			index++;
		}

		if(mExtrude>0)
		{
			tempy=y;

			for(int k = 0; k <= mExtrude; k++)
				for(int i = 0; i <= mOutline*2; i++)
					for(int j = 0; j <= mOutline*2; j++)
					{
						SetTextColor(hDC, RGB(255,255,255) );
 						ExtTextOutW( hDC, x-abc.abcA+k+i, tempy+mExtrude-k+j, ETO_OPAQUE, NULL, str, 1, NULL );

						SetTextColor(hDC1, RGB(0,0,0) );
						ExtTextOutW( hDC1, x-abc.abcA+k+i, tempy+mExtrude-k+j, ETO_OPAQUE, NULL, str, 1, NULL );
					}
		}

		if(mOutline>0)
		{
			tempy=y;

			// Draw the outline
			for(int i = 0; i <= mOutline*2; i++)
				for(int j = 0; j <= mOutline*2; j++)
				{
//					SetTextColor(hDC, RGB(255,255,255) );
//					ExtTextOutW( hDC, x-abc.abcA+i+mExtrude, tempy+j, ETO_OPAQUE, NULL, str, 1, NULL );

					if(mBlobby)
					{
						SetTextColor(hDC1, RGB(255,0,255) );
						ExtTextOutW( hDC1, x-abc.abcA+i+mExtrude, tempy+j, ETO_OPAQUE, NULL, str, 1, NULL );
					}
					else
					if(mIntensityOutline)
					{
						SetTextColor(hDC, RGB(255,255,255) );
						ExtTextOutW( hDC, x-abc.abcA+i+mExtrude, tempy+j, ETO_OPAQUE, NULL, str, 1, NULL );
					}
					else
					{
						SetTextColor(hDC, RGB(255,255,255) );
						ExtTextOutW( hDC, x-abc.abcA+i+mExtrude, tempy+j, ETO_OPAQUE, NULL, str, 1, NULL );
						SetTextColor(hDC1, RGB(0,0,0) );
						ExtTextOutW( hDC1, x-abc.abcA+i+mExtrude, tempy+j, ETO_OPAQUE, NULL, str, 1, NULL );
					}
					
				}

			if(mBlobby)	
				SetTextColor(hDC, RGB(0,255,255) );
			else
			if(mIntensityOutline)
				SetTextColor(hDC, RGB(255,255,255) );
			else
				SetTextColor(hDC, RGB(255,255,255) );

			ExtTextOutW( hDC, x-abc.abcA+mOutline+mExtrude, y+mOutline, ETO_OPAQUE, NULL, str, 1, NULL );

			if(mBlobby)	
				SetTextColor(hDC1, RGB(255,0,255) );
			else
			if(mIntensityOutline)
				SetTextColor(hDC1, RGB(255,255,255) );
			else		
				SetTextColor(hDC1, RGB(255,255,255) );

			ExtTextOutW( hDC1, x-abc.abcA+mOutline+mExtrude, y+mOutline, ETO_OPAQUE, NULL, str, 1, NULL );

		}
		else if(mDropShadow)
		{
			SetTextColor(hDC, RGB(255,255,255) );
			ExtTextOutW( hDC, x-abc.abcA+1, y+1, ETO_OPAQUE, NULL, str, 1, NULL );

			SetTextColor(hDC1, RGB(0,0,0) );
			ExtTextOutW( hDC1, x-abc.abcA+1, y+1, ETO_OPAQUE, NULL, str, 1, NULL );

			SetTextColor(hDC, RGB(255,255,255) );
			ExtTextOutW( hDC, x-abc.abcA, y, ETO_OPAQUE, NULL, str, 1, NULL );

			SetTextColor(hDC1, RGB(255,255,255) );
			ExtTextOutW( hDC1, x-abc.abcA, y, ETO_OPAQUE, NULL, str, 1, NULL );
		}
		else
		{
			SetTextColor(hDC, RGB(255,255,255) );
			ExtTextOutW( hDC, x-abc.abcA, y, ETO_OPAQUE, NULL, str, 1, NULL );

			SetTextColor(hDC1, RGB(255,255,255) );
			ExtTextOutW( hDC1, x-abc.abcA, y, ETO_OPAQUE, NULL, str, 1, NULL );
		}

		// Glow
		if(mGlow)
		{
			int left	= x - mHrzSpacing;
			int top		= y - mVrtSpacing;
			int right	= left + abc.abcB + (mHrzSpacing * 2) - 1;
			int bottom	= top + size.cy + (mVrtSpacing * 2) - 1;

			left = Clamp(left, 0, mTexWidth - 1);
			right = Clamp(right, 0, mTexWidth - 1);

			top = Clamp(top, 0, mTexHeight - 1);
			bottom = Clamp(bottom, 0, mTexHeight - 1);

			int cWidth = right - left;
			int cHeight = bottom - top;

			int* cBuffer = new int[cWidth * cHeight];

			for(int gy = top; gy < bottom; gy++)
			{
				for(int gx = left; gx < right; gx++)
				{
					cBuffer[((gy - top) * cWidth) + (gx - left)] = SampleColour(left, top, right, bottom, pBitmapBits, mTexWidth, gx, gy);
				}
			}
			
			float gamma = mGlowGamma;
			float invGamma = 1.0f / gamma;

			for(int gy = top; gy < bottom; gy++)
			{
				for(int gx = left; gx < right; gx++)
				{
					float red	= 0.0f;
					float total	= 0.0f;

					for(int fy = -drawRadius; fy <= drawRadius; fy++)
					{
						for(int fx = -drawRadius; fx <= drawRadius; fx++)
						{
							int c = SampleColour(0, 0, cWidth, cHeight, cBuffer, cWidth, gx + fx - left, gy + fy - top);

							float d = gau[((fy + drawRadius) * (drawRadius + 1 + drawRadius)) + fx + drawRadius];

							red += pow(GetRValue(c) / 255.0f, gamma) * d;
						}
					}

					red = powf(red, invGamma);

					int ir = Clamp((int)(red * 256.0f), 0, 255);

					pBitmapBits[mTexWidth*gy + gx] = RGB(ir, ir, ir);
				}
			}

			delete [] cBuffer;
		}

		// Advance the cursor to the next position
		x += abc.abcB + (mOutline*2) + (mDropShadow*2) + mExtrude;
		x += (mHrzSpacing*2);
	}

	// Add alpha channel
	if(mBlobby)
	{
		for( y=0; y < mTexHeight; y++ )
			for( x=0; x < mTexWidth; x++ )
			{
				pBitmapBits[mTexWidth*y + x] |= pBitmapBits1[mTexWidth*y + x] | 0xff000000;
			}
	}
	else
	if(mIntensityOutline)
	{
		for( y=0; y < mTexHeight; y++ )
			for( x=0; x < mTexWidth; x++ )
			{
				pBitmapBits[mTexWidth*y + x] |= pBitmapBits1[mTexWidth*y + x] << 24;
			}
	}
	else
	{
		const int bg_col = (mExtrude>0 || mOutline>0) ? 0 : 0x00ffffff;

		for( y=0; y < mTexHeight; y++ )
		{
			for( x=0; x < mTexWidth; x++ )
			{
				int colour;

				if(mBlackAndWhite)
				{
					pBitmapBits[mTexWidth*y + x] |= 0xff000000;
				}
				else
				{
					if(mGlow)
					{
						pBitmapBits[mTexWidth*y + x] |= pBitmapBits1[mTexWidth*y + x] << 24;
					}
					else
					{
						if(mDropShadow || mOutline>0 || mExtrude) 
						{
							colour = ( pBitmapBits1[mTexWidth*y + x] & 0x00ffffff );
						}
						else
						{
							colour=0x00ffffff;
						}

						int color = bg_col;
						int alpha = ( pBitmapBits[mTexWidth*y + x] & 0x000000ff );

						if( alpha > 0 ) color = (alpha<<24)| colour;
						pBitmapBits[mTexWidth*y + x] = color;
					}
				}
			}
		}
	}

	strcpy(mBundledNames[mNumBundledTextures],name);
	strcpy(mBundledTextureCompression[mNumBundledTextures],compression);
	mBundledTextureLevels[mNumBundledTextures]=levels;

    // Write out the files
	WriteTargaFile(texture_name,mTexWidth,mTexHeight,pBitmapBits);

	//Write out data files
	WriteFontStore(hDC, (overide_name[0] ? overide_name : name), mNumForeignChars, texture_name);

    // Clean up used objects and return
    DeleteObject( hbmBitmap );
    DeleteDC( hDC );

	DeleteObject( hbmBitmap1 );
    DeleteDC( hDC1 );

	DeleteObject( hFont );

    return S_OK;
}

void FontMaker::SetBundle(char *name)
{
	gCommandLine.AddLineArg(TC_BLACK,"");
	gCommandLine.AddLineArg(TC_BLACK,"Setting Bundle %s",name);

	strcpy(mBundleName,name);
	mNumBundledTextures=0;

	mNumFontSets=0;
}

void FontMaker::SetBundleExportPath(char *ep)
{
	gCommandLine.AddLineArg(TC_BLACK,"");
	gCommandLine.AddLineArg(TC_BLACK,"Setting Bundle Export Path %s",ep);

	strcpy(mBundleExportPath,ep);
}

void FontMaker::EndBundle()
{
	char command[1024];

	gCommandLine.AddLineArg(TC_BLACK,"Ending Bundle %s with %d textures",mBundleName,mNumBundledTextures);
	
	for(int a=0;a<mNumBundledTextures;a++)
	{
		gCommandLine.AddLineArg(TC_BLACK,"    %s",mBundledTextures[a]);
	}
	
	gCommandLine.AddLineArg(TC_BLACK,"Creating .xpr file ready for bundling");
	gCommandLine.AddLineArg(TC_BLACK,"");


	FILE *fp;

	sprintf(command,"%s\\%s.xml",gConfig.mTextureExport,mBundleName);
	fp=fopen(command,"w+");

	if(fp)
	{
		fprintf(fp,"<?xml version=\"1.0\" encoding=\"utf-8\" ?> \n\n");
		fprintf(fp,"<RDF Version=\"XPR2\">\n\n");

		for(int a=0;a<mNumBundledTextures;a++)
		{
			fprintf(fp,"<Texture\n");
			fprintf(fp,"\tName\t=\t\"%s\"\n",mBundledNames[a]);
			fprintf(fp,"\tSource\t=\t\"%s\"\n",mBundledTextures[a]);
			fprintf(fp,"\tCompressor\t=\t\"X360\"\n"); // TODO compression types 
			fprintf(fp,"\tFormat\t=\t\"%s\"\n",mBundledTextureCompression[a]);
			fprintf(fp,"\tLevels\t=\t\"%d\"\n",mBundledTextureLevels[a]);
			
			//if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED1) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT1\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED2) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT2\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED3) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT3\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED4) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT4\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_COMPRESSED5) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_DXT5\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_32BIT) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_A8R8G8B8\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_24BIT) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_R5G6B5\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_16BIT) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_A4R4G4B4\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_8BIT) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_A8\"\n");
			//else if(textureformat[a] == UI_TEXTURE_FORMAT_LIN_32BIT) 
			//	fprintf(fp,"\tFormat\t=\t\"D3DFMT_LIN_A8R8G8B8\"\n");
			//fprintf(fp,"\tLevels\t=\t\"1\"\n");
			fprintf(fp,"/>\n\n");
		}

		fprintf(fp,"</RDF>\n");

		fclose(fp);
	}
	else
	{
		AfxMessageBox("Error Opening XPR file",MB_OK|MB_ICONINFORMATION);
	}

	strcpy(mAllBundleNames[mTotalBundles],mBundleName);
	mTotalBundles++;

	if(mLanguage!=UIFONT_LANGUAGE_EUROPEAN)
	{
		strcpy(mEaBundleNames[mEaBundles],mBundleName);
		mEaBundles++;
	}

	WriteFontInfoFile(mBundleName);
}

void FontMaker::WriteOutTotalBundles()
{
	FILE *fp, *ea_fp;
	char command[1024];

	sprintf(command,"%s\\bundles.temp",gConfig.mTextureExport);
	fp=fopen(command,"w+");

	sprintf(command,"%s\\ea_bundles.temp",gConfig.mTextureExport);
	ea_fp=fopen(command,"w+");

	if(fp)
	{
		for(int a=0;a<mTotalBundles;a++)
		{
			fprintf(fp,"%s\n",mAllBundleNames[a]);
		}

		fclose(fp);
	}
	if(ea_fp)
	{
		for(int a=0;a<mEaBundles;a++)
		{
			fprintf(ea_fp,"%s\n",mEaBundleNames[a]);
		}

		fclose(ea_fp);
	}
}

void FontMaker::MakeBundles(const bool ea_only)
{
	FILE *fp;
	char command[1024];

	gCommandLine.AddLine(TC_BLACK,"Making Bundles...");

	if(ea_only)
	{
		sprintf(command,"%s\\ea_bundles.temp",gConfig.mTextureExport);
		fp=fopen(command,"r");
	}
	else
	{
		sprintf(command,"%s\\bundles.temp",gConfig.mTextureExport);
		fp=fopen(command,"r");
	}

	if(fp)
	{
		int a=0;
		unsigned char c;
		char bundle_name[128];

		c=fgetc(fp);
		while(c != 0xff)
		{
			bundle_name[a++]=c;
			if(c == 0x0a)	//end of line ---- lets bundle !!!!!
			{
				bundle_name[a-1]=NULL;

				char command[1024];

				sprintf(command,"%s \"%s\\%s.xml\" -c -f x360 -o \"%s\\%s.bpr\"",gConfig.mBundlerExe,gConfig.mTextureExport,bundle_name,gConfig.mPCExport,bundle_name);

				gSystemCommands.Flush();
				gSystemCommands.AddCommand(command);
				gSystemCommands.Execute();

				gCommandLine.AddLineArg(TC_BLACK,"    Bundling %s.xml",bundle_name);

				a=0;
			}

			c=fgetc(fp);
		}

		fclose(fp);

		gCommandLine.AddLine(TC_BLACK,"...Made All Bundles");
	}
	else
	{
		AfxMessageBox("Error Opening Temporary Bundle File - Please Build first",MB_OK|MB_ICONINFORMATION);
	}
}

void FontMaker::ShowFonts()
{
	gCommandLine.AddLine(TC_BLACK,"FontList ...");

	for(int a=0;a<mNumFontSets;a++)
	{
		gCommandLine.AddLine(TC_BLACK,mFontSets[a].Name);
	}

	gCommandLine.AddLine(TC_BLACK,"");
}

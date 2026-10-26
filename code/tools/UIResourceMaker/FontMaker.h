#pragma once

#define EUROPEAN_TOTAL  169
#define MAX_FOREIGN 1024
#define MAX_FONT_SETS 12
#define MAX_GLYPHS 65535

enum UIFONT_GLYPH_FILE_TYPE
{
	UIFONT_GLYPH_FILE_TYPE_INI,
	UIFONT_GLYPH_FILE_TYPE_XML,

	UIFONT_GLYPH_FILE_TYPE_MAX,
};

enum UIFONT_LANGUAGE
{
	UIFONT_LANGUAGE_EUROPEAN,
	UIFONT_LANGUAGE_JAPANESE,
	UIFONT_LANGUAGE_KOREAN,
	UIFONT_LANGUAGE_CHINESE,

	UIFONT_LANGUAGE_MAX,
};

enum FM_TYPE
{
	FM_TYPE_UI,
	FM_TYPE_GAME,

	FM_TYPE_MAX,
};

struct GLYPH_ATTR
{
	WCHAR character;
    FLOAT left, top, right, bottom;
    SHORT wOffset;
    SHORT wWidth;
    SHORT wAdvance;
};

struct KERN_ATTR
{
	unsigned short first;
	unsigned short second;
	char amount;
};

typedef struct FontAttributes
{
	char Name[128];

	int Language;
	
	char TextureName[128];
	int TextureWidth;
	int TextureHeight;
	int Number;
	int FontHeight;
	int ExtrudeSize;
	int	SpacingSize; 

	float TopOffset;
	float SizeOveride;

	GLYPH_ATTR *Glyphs;
	
	int NumKernPairs;
	KERN_ATTR  *Kerning;
}FontAttributes;

class FontMaker
{
public:
	FontMaker()
	{
		mFontWeight=400;
		mItalicized=false;
		mLanguage=UIFONT_LANGUAGE_EUROPEAN;
		mType=FM_TYPE_UI;
		mVersion=0x00000004;
		mTexWidth=64;
		mTexHeight=64;       
		mTexBPP=16;
		mDropShadow=false;
		mOutline=false;
		mAntiAliased=false;
		mVrtSpacing=0;
		mHrzSpacing=0;
		mNumFontSets=0;
	};

	void SetTextDir(char *name){strcpy(mTextDirectory,name);};
	void SetTextureDir(char *name){strcpy(mTextureDirectory,name);};

	char *GetFileName(){return mFileName;};

	void StartFontMaker(char *filename);
	void SetFontName(char *name){strcpy(mFontName,name);};
	void SetSize(int size){mFontHeight=size;};
	void SetFontWeight(int weight){mFontWeight=weight;};
	void SetItalics(bool italic){mItalicized=italic;};
	void SetLanguage(UIFONT_LANGUAGE lang){mLanguage=lang;};
	void SetType(FM_TYPE type){mType=type;};
	void SetDropShadow(bool ds){mDropShadow=ds;};
	void SetGlow(bool g){mGlow=g;}
	void SetGlowGamma(float gg){mGlowGamma=gg;}
	void SetOutline(int o){mOutline=o;};
	void SetExtrude(int e){mExtrude=e;};
	void SetAntiAliased(bool aa){mAntiAliased=aa;};
	void SetBlobby(bool aa){mBlobby=aa;};
	void SetBlackAndWhite(bool aa){mBlackAndWhite=aa;};
	void SetIntensityOutline(bool aa){mIntensityOutline=aa;}
	
	void SetHorizontalSpacing(int x){mHrzSpacing=x;};
	void SetVerticalSpacing(int y){mVrtSpacing=y;};

	void SetTopOffset(float t){mTopOffset=t;};
	void SetSizeOveride(float s){mSizeOveride=s;};

	void SetBundle(char *name);
	void SetBundleExportPath(char *ep);
	void EndBundle();
	void FillGlyphTable(char *name,int start,UIFONT_GLYPH_FILE_TYPE type=UIFONT_GLYPH_FILE_TYPE_INI);

	char *GetBundleExportPath(){return mBundleExportPath;};

	int GetFont(char *name);

	void ShowFonts();

	HRESULT CreateFontFiles(char *name,char *texture_name, char* overide_name, char *compression, int levels);
	HRESULT WriteFontInfoFile(char *name);

	void WriteOutTotalBundles();
	void MakeBundles(const bool ea_only);

private:
	char mFileName[128];

private:

	HRESULT CalcTextureSize( HDC hFontDC,int* pdwTexWidth, int* pdwTexHeight,int total );
	HRESULT WriteTargaFile( char* strFileName, int dwWidth, int dwHeight, int* pRGBAData );
	void WriteFontStore(HDC hDC,char *name,int number_of_fonts,char *texture_name);

    GLYPH_ATTR mGlyphs[MAX_GLYPHS];

	char mTextDirectory[128];
	char mTextureDirectory[128];

	char mFontName[512];
	int mFontHeight;
	int mFontEffectiveHeight;
	int mFontWeight;
	bool mItalicized;

	UIFONT_LANGUAGE mLanguage;
	FM_TYPE mType;

	int mVersion;
	int mTexWidth;
	int mTexHeight;
	int mTexBPP;

	bool mDropShadow;
	int mOutline;
	int mExtrude;
	bool mAntiAliased;
	bool mBlackAndWhite;
	bool mBlobby;
	bool mIntensityOutline;

	bool mGlow;
	float mGlowGamma;

	int mVrtSpacing;
	int mHrzSpacing;

	float mTopOffset;
	float mSizeOveride;

	int mNumFontSets;

	static const int mVersionNumber = -1; // Must be neg (if positive v.old vesion)

	int mNumForeignChars;
	WCHAR mCurrentGlyphs[MAX_GLYPHS];

	FontAttributes mFontSets[MAX_FONT_SETS];

	char mBundleName[128];
	int mNumBundledTextures;

	char mBundleExportPath[128];
	
	char mBundledNames[32][128];
	char mBundledTextures[32][128];
	char mBundledTextureCompression[32][128];
	int mBundledTextureLevels[32];

	char mAllBundleNames[32][128];
	char mEaBundleNames[32][128];
	int mTotalBundles;
	int mEaBundles;
};

extern FontMaker gFontMaker;
#include "stdafx.h"

#include <Xt/XtGame.h>

#include "ui/UiInterface.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/UITextureManager.h"
#include "ui/Graphics/Localisation/Loc.h"

#include "ui/Graphics/UIGraphicalInterface.h"

#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/Font/UIBitmapFont.h"
#include "ui/Graphics/Font/UIPolygonFont.h"

#include "ui/Game/UILocaliser.h"

#include "ui/Illustrator/UIAdobeSceneManager.h"

#ifndef PLATFORM_PS3
#include "tchar.h"
#else
//#define PS3_SPECIAL_PROCESS return
#include "wchar.h"
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

/*
#if 1 // WORDWRAP_FIX: mfuse : 2005.8.24 04:18
#define WORDWRAP_FIX 1
#include "ui/graphics/wordwrap.h"
#endif
*/

XT_TYPE( UI::E_FixedSpaceType, "UIFixedSpaceType" )
	XT_OPTION( UI::E_FixedSpaceType_None, "None" )
	XT_OPTION( UI::E_FixedSpaceType_Numbers, "Numbers" )
	XT_OPTION( UI::E_FixedSpaceType_Letters, "Letters" )
	XT_OPTION( UI::E_FixedSpaceType_All, "All" )
XT_END_TYPE

#define IsWhiteSpace(c) ((c) == L' ' || (c) == L'\t' || (c) == L'\r') 

namespace UI
{

// ----------------
// C_FontSpriteItem
// ----------------
C_FontSpriteItem::C_FontSpriteItem(const C_FontSpriteItem& copy)
{
	*this = (copy);
}

C_FontSpriteItem& C_FontSpriteItem::operator = (const C_FontSpriteItem& copy)
{
	// [Oscar] added copy semantics as compiler-generated code appeared to be provoking an optimisation bug in the 360 compiler
	m_Character = copy.m_Character;
	m_eFontType = copy.m_eFontType;
	m_Texture = copy.m_Texture;
	m_InputGlyph = copy.m_InputGlyph; // [Blur2PC]
	m_UserData = copy.m_UserData;
	m_Col = copy.m_Col;
	m_eItemType = copy.m_eItemType;
	m_Layer = copy.m_Layer;
	m_Pos = copy.m_Pos;
	m_Sx = copy.m_Sx;
	m_Sy = copy.m_Sy;
	m_fActualWidth = copy.m_fActualWidth;
	m_T0_px = copy.m_T0_px;
	m_T0_py = copy.m_T0_py;
	m_T0_sx = copy.m_T0_sx;
	m_T0_sy = copy.m_T0_sy;
	m_T1_px = copy.m_T1_px;
	m_T1_sx = copy.m_T1_sx;
	m_T1_py = copy.m_T1_py;
	m_T1_sy = copy.m_T1_sy;
	m_T2_px = copy.m_T2_px;
	m_T2_sx = copy.m_T2_sx;
	m_DropShadow = copy.m_DropShadow;
	m_Italics = copy.m_Italics;
	m_bLineBreak = copy.m_bLineBreak;
	return *this;
}

// use to set a bitmap font item
__forceinline void C_FontSpriteItem::Set( const I_GlyphAttributes::E_GlyphType eFontType, const C_TextureHandleIndex texture, const bool bLineBreak, 
										  const E_Type eItemType, const Maths::HVec3& pos, const F32 sx, const F32 sy, 
										  const C_UISpriteColours& col, 
										  const F32 tx1, const F32 ty1, const F32 txS, const F32 tyS, 
										  const bool bDropShadow, const bool bItalics, 
										  const LayerIndex layer, WCHAR character,
										  const F32 fActualWidth )
{
	m_Character = character;
	m_eFontType = eFontType;
	m_Texture = texture;
	m_InputGlyph = false; // [Blur2PC] only explicitly marked font sequences switch

	m_Col = col;

	m_eItemType = eItemType;
	m_Layer	= layer;
	
	m_Pos = pos;
	m_Sx = sx;
	m_Sy = sy;

	m_fActualWidth = fActualWidth;
	
	m_T0_px = tx1;
	m_T0_py = ty1;
	m_T0_sx = txS;
	m_T0_sy = tyS;

	m_DropShadow = bDropShadow;
	m_Italics = bItalics;
	m_bLineBreak = bLineBreak;
}

// use to set a polygon font item
__forceinline void C_FontSpriteItem::Set( const I_GlyphAttributes::E_GlyphType eFontType, const INTPTR uUserData, const bool bLineBreak, 
										  const E_Type eItemType, const Maths::HVec3& pos, const F32 sx,const F32 sy, 
										  const C_UISpriteColours& col, 
										  const bool bDropShadow, const bool bItalics, 
										  const LayerIndex layer, WCHAR character,
										  const F32 fActualWidth )
{
	m_Character = character;
	m_eFontType = eFontType;
	m_UserData = uUserData;
	m_InputGlyph = false; // [Blur2PC]
	
	m_Col = col;

	m_eItemType = eItemType;
	m_Layer	= layer;
	
	m_Pos = pos;
	m_Sx = sx;
	m_Sy = sy;

	m_fActualWidth = fActualWidth;

	m_Texture.Active( false );

	m_DropShadow = bDropShadow;
	m_Italics = bItalics;
	m_bLineBreak = bLineBreak;
}

// ------
// C_Font
// ------
C_Font gFont;

const F32 C_Font::s_fUseDefaultHeightPercentage = -1.0f;

#ifndef CFG_GOLD
Debug::Bool		g_debugUIBoundingBoxes("UI.Localisation.ShowFontBoundingBoxes", false);
Debug::Float	g_debugUIFontTracking("UI.Tracking", 0.0f, Debug::Range(-2.0f, 2.0f));
#endif

C_Font::C_Font() 
	: m_pFontSequences( NULL ), 
	m_bInitialised( false ),
	m_bDoNotClearVars( false )
{
	MEMORY_TRACKER_PUSH("ui.font");

	ClearVars();

	m_bUseReverseTextureLook=true;

	m_SpriteStack.Resize( m_MaxFontSpriteItems );
//	m_pSpriteItems = new C_FontSpriteItem[ m_MaxFontSpriteItems ];
//	assert(m_pSpriteItems);

	// Clear font pointers
	for(U32 a = 0; a < m_uMaxFonts; a++)
		m_pFonts[ a ] = NULL;

	MEMORY_TRACKER_POP("ui.font");

	mClipStartX = gSimple2D.ScreenLeft();
	mClipEndX = gSimple2D.ScreenRight();
	mClipStartY = 0.0f;
	mClipEndY = UI_VSCREEN_SIZEY;

	m_eFixedSpaceType = E_FixedSpaceType_None;
}

C_Font::~C_Font()
{
	Destroy( );

	// Things created by the constructor
	SAFE_DELETE_ARY(m_pFontSequences);

	m_SpriteStack.Resize( 0 );
}

void C_Font::Destroy()
{
	for(U32 a=0; a < m_uNumFonts; a++)
	{
		SAFE_DELETE_PTR( m_pFonts[ a ] );
	}

	SAFE_DELETE_PTR( m_pUIAdditionalFont );
	SAFE_DELETE_PTR( m_pCommonAdditionalFont );

	m_uNumFonts = 0;

	ClearLists( );
}

void C_Font::ClearLists(void)
{
	m_SpriteStack.Reset();
}

void C_Font::InitialiseUIAdditionalFont( const char* pFilename )
{
	S32 dataVersion;

	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("ui.font.bitmap");

	m_pUIAdditionalFont = new C_BitmapFontAttributes();
	assert(m_pUIAdditionalFont);

	Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
	Sys::I_File* fp = fs->Open(pFilename);	
	Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*fp);

	ar >> dataVersion;

	// Before version control - the first S32 was number of fonts
	S32 num;
	if(dataVersion>=0)
	{
		num = dataVersion;
		dataVersion = 0;
	}
	else
	{
		dataVersion = -dataVersion;
		ar >> num;
	}

	BIZ_ASSERT_MSG(num == 1, "There is more than one East Asian font");
	BIZ_ASSERT_MSG(dataVersion > 0, "Old data not supported");

	m_pUIAdditionalFont->Read( ar, dataVersion );
	delete fp;

	m_fDefaultHeightPercentage = ( C_UILocaliser::Instance().Language() == E_UILanguage_Korean ) ? 0.8f : 1.0f;
	
#if WORDWRAP_FIX // WORDWRAP_FIX: mfuse : 2005.8.24 05:29
	// set default WordWrap setting
	WordWrap_SetOption( WW_PROHIBITION | WW_NOHANGULWRAP );
#endif

	//MEMORY_TRACKER_POP("ui.font.bitmap");
}

void C_Font::InitialiseCommonAdditionalFont( const char* pFilename )
{
	S32 dataVersion;

	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("ui.font.bitmap");

	m_pCommonAdditionalFont = new C_BitmapFontAttributes();
	assert(m_pCommonAdditionalFont);

	Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
	Sys::I_File* fp = fs->Open(pFilename);	

	if(fp == NULL)
		return;

	Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*fp);

	ar >> dataVersion;

	// Before version control - the first S32 was number of fonts
	S32 num;
	if(dataVersion>=0)
	{
		num = dataVersion;
		dataVersion = 0;
	}
	else
	{
		dataVersion = -dataVersion;
		ar >> num;
	}

	BIZ_ASSERT_MSG(num == 1, "There is more than one East Asian font");
	BIZ_ASSERT_MSG(dataVersion > 0, "Old data not supported");

	m_pCommonAdditionalFont->Read( ar, dataVersion );
	delete fp;

	m_fDefaultHeightPercentage = ( C_UILocaliser::Instance().Language() == E_UILanguage_Korean ) ? 0.8f : 1.0f;
	
#if WORDWRAP_FIX // WORDWRAP_FIX: mfuse : 2005.8.24 05:29
	// set default WordWrap setting
	WordWrap_SetOption( WW_PROHIBITION | WW_NOHANGULWRAP );
#endif
	//MEMORY_TRACKER_POP("ui.font.bitmap");
}

void C_Font::Initialise( const char* pFilename )
{
	m_fDefaultHeightPercentage = 0.8f;

	m_bInitialised = true;

	if( pFilename != NULL )
	{
		//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("ui.font.bitmap");

		S32 dataVersion;

		Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
		Sys::I_File* fp = fs->Open(pFilename);	

		if(fp)
		{
			Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*fp);

			ar >> dataVersion;

			// Before version control - the first S32 was number of fonts
			if(dataVersion>=0)
			{
				m_uNumFonts = dataVersion;
				dataVersion = 0;
			}
			else
			{
				dataVersion = -dataVersion;
				ar >> m_uNumFonts;
			}
			
			// Read in each font
			for(U32 a=0; a < m_uNumFonts; a++)
			{
				m_pFonts[ a ] = new C_BitmapFontAttributes();
				m_pFonts[ a ]->Read( ar, dataVersion );
			}

			delete fp;
		}
		//MEMORY_TRACKER_POP("ui.font.bitmap");
	}
}

void C_Font::CreatePolygonFont( const char* pFilename, const bool b2D, const bool bOutline2D, const bool b3D, const bool bOutline3D, const bool bBevel3D, const bool bFinned2D )
{
	// create new font
	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly" );	
	
	if ( b2D )
	{
		C_UIPolygonFont* pFont = new C_UIPolygonFont;
		assert( pFont );
		pFont->Build( pFilename );
		AddPolygonFont( pFont );
	}

	if ( bOutline2D )
	{
		C_UIPolygonFont* pFont = new C_UIPolygonFontOutline;
		assert( pFont );
		pFont->Build( pFilename );
		AddPolygonFont( pFont );
	}

	if ( b3D )
	{
		C_UIPolygonFont* pFont = new C_UIPolygonFont3D;
		assert( pFont );
		pFont->Build( pFilename );
		AddPolygonFont( pFont );
	}

	if ( bOutline3D )
	{
		C_UIPolygonFont* pFont = new C_UIPolygonFontOutline3D;
		assert( pFont );
		pFont->Build( pFilename );
		AddPolygonFont( pFont );
	}

	if ( bBevel3D )
	{
		C_UIPolygonFont* pFont = new C_UIPolygonFontBevel3D;
		assert( pFont );
		pFont->Build( pFilename );
		AddPolygonFont( pFont );
	}

	if ( bFinned2D )
	{
		C_UIPolygonFont* pFont = new C_UIPolygonFontFinned;
		assert( pFont );
		pFont->Build( pFilename );
		AddPolygonFont( pFont );
	}

	//MEMORY_TRACKER_POP( "ui.font.poly" );
}

void C_Font::AddPolygonFont( C_UIPolygonFont* pPolygonFont )
{
	// add to the list
	assert( pPolygonFont );
	assert( m_uNumFonts+1 < m_uMaxFonts );

	m_pFonts[ m_uNumFonts++ ] = pPolygonFont;
}

void C_Font::CreateLuaEnums( lua_State *pState )
{
	// Push new table over old one
	lua_newtable(pState);
		if(lua_type(pState, -1) != LUA_TTABLE)
		{
			LOGFATAL( g_Log, DBSTR " : failed to create new %s table : %x - stack top %d", LUA_FONT_TABLE_NAME, (U32)(INTPTR)pState, lua_gettop(pState) );	// [Blur2PC][x64] log only
			return;
		}
		for(U32 i = 0; i < m_uNumFonts; i++)
		{
			lua_pushstring(pState, Util::C_String(m_pFonts[i]->GetName()).Lower().Chars()); 
			lua_pushnumber(pState, i); 
			lua_rawset(pState,-3);
		}
	lua_setglobal(pState,LUA_FONT_TABLE_NAME);
}

void C_Font::InitialiseSequences( lua_State* pState )
{
	MEMORY_TRACKER_PUSH("ui.font");

	lua_getglobal(pState,LUA_FONTSEQ_TABLE_NAME);
	if(lua_type(pState,-1) != LUA_TTABLE)
	{
		LOGDEBUG( g_Log, DBSTR "%s table not found", LUA_FONTSEQ_TABLE_NAME );
		lua_pop(pState,1);
		MEMORY_TRACKER_POP("ui.font");
		return;
	}
	const S32 size = luaL_getn(pState,-1);
	if(size <= 0)
	{
		LOGDEBUG( g_Log, DBSTR "%s table has %d entries, which isn't right", LUA_FONTSEQ_TABLE_NAME );
		lua_pop(pState,1);
		MEMORY_TRACKER_POP("ui.font");
		return;
	}

	m_pFontSequences = new C_FontSequence[size];
	if(!m_pFontSequences)
	{
		LOGFATAL( g_Log, DBSTR "Failed to allocate space for %d sequences", size );
		lua_pop(pState,1);
		MEMORY_TRACKER_POP("ui.font");
		return;
	}
	m_uNumFontSequences = size;

	C_FontSequence *p = m_pFontSequences;
	for(S32 i = 1; i <= size; i++, p++)
	{
		lua_rawgeti(pState,-1,i); // Push on index [i]

		// [Blur2PC] Non-input symbols and explicit split-screen join icons stay fixed.
		p->m_InputGlyph = false;
		if (Lua::DataMemberType(pState, "input_glyph") == LUA_TBOOLEAN)
			Lua::GetDataMember(pState, "input_glyph", p->m_InputGlyph);

		const char *string;
		Lua::GetDataMember( pState, "name", string );

		p->m_Sequence.Format( L"%S", string );

		Lua::GetDataMember( pState, "colour", string );

		C_UISpriteColours c;
		c.SetByName( string );		
		p->m_Colour = c[ 0 ];

		if ( Lua::DataMemberType( pState, "texture" ) == LUA_TNUMBER )
		{
			S32 index;
			Lua::GetDataMember( pState, "u", p->m_U	);
			Lua::GetDataMember( pState, "v", p->m_V	);
			Lua::GetDataMember( pState, "usize", p->m_SizeU	);
			Lua::GetDataMember( pState, "vsize", p->m_SizeV	);
			Lua::GetDataMember( pState, "texture", index );

			p->m_Texture = index;
			
			// Make these invalid.
			p->m_SceneIndex		= 0xffffffff;
			p->m_GroupName[0]	= '\0';
		}
		/*
		else // Grab shape data.
		if ( Lua::DataMemberType( pState, "scene_name") == LUA_TSTRING )
		{
			Lua::GetDataMember( pState, "scene_name", string );
			p->m_SceneIndex = C_AdobeSceneManager::Instance().GetSceneIndexByName( string );

			Lua::GetDataMember( pState, "group_name", string );
			sprintf_s( p->m_GroupName, 16, "%s", string );
		}
		*/

		// Grab other data.
		p->m_LeadingX		= 0.0f;
		if( Lua::DataMemberType( pState, "leadingx") == LUA_TNUMBER )
			Lua::GetDataMember( pState, "leadingx", p->m_LeadingX );

		p->m_TrailingX		= 0.0f;
		if( Lua::DataMemberType( pState, "trailingx") == LUA_TNUMBER )
			Lua::GetDataMember( pState, "trailingx", p->m_TrailingX );

		p->m_ScaleX			= 1.0f;
		if( Lua::DataMemberType( pState, "scalex") == LUA_TNUMBER )
			Lua::GetDataMember( pState, "scalex", p->m_ScaleX );

		p->m_ScaleY			= 1.0f;
		if( Lua::DataMemberType( pState, "scaley") == LUA_TNUMBER )
			Lua::GetDataMember( pState, "scaley", p->m_ScaleY );

		p->m_OffsetY		= 0.0f;
		if( Lua::DataMemberType( pState, "offsety") == LUA_TNUMBER )
			Lua::GetDataMember( pState, "offsety", p->m_OffsetY );

		p->m_Aspect			= 1.0f;
		if( Lua::DataMemberType( pState, "aspect") == LUA_TNUMBER )
			Lua::GetDataMember( pState, "aspect", p->m_Aspect );
		
		p->m_DropShadow		= false;
		if( Lua::DataMemberType( pState, "dropshadow")==LUA_TBOOLEAN )
		{
			bool ds;
			Lua::GetDataMember( pState, "dropshadow", ds );
			
			p->m_DropShadow = ds;
		}

		p->m_AllowItalics	= false;
		if( Lua::DataMemberType( pState, "allow_italic")==LUA_TBOOLEAN )
		{
			bool ai;
			Lua::GetDataMember( pState, "allow_italic", ai );

			p->m_AllowItalics = ai;
		}

		lua_pop(pState,1); // Pop index [i] off
	}
	lua_pop(pState,1); // Pop the LUA_FONTSEQ_TABLE_NAME

	// Shear the matrix in the x[y]
	Maths::HMat43 italics_matrix;
	italics_matrix.SetIdentity();
	italics_matrix.AY( Maths::HVec3(-sinf(Maths::DegToRad(13.0f)),1.0f,0.0f) ); // Apparently italics are 13 degrees according to Gary

	// Reserved a pre transform matrix for italicizing the font
	gSimple2D.GetPreTransformMatrix( m_ItalicsMatrixIndex, true) = italics_matrix;

	MEMORY_TRACKER_POP("ui.font");
}

void C_Font::DestroyUIAdditionalFont()
{
	SAFE_DELETE_PTR( m_pUIAdditionalFont );
}

void C_Font::DestroyCommonAdditionalFont()
{
	SAFE_DELETE_PTR( m_pCommonAdditionalFont );
}

const WCHAR C_Font::jpn_wordwrap[] = { 0x0021,0x0029,0x002C,0x002E,0x003F,0xFF1F,0x3001,0x3002,0x30FC,0xFF01,0xFF09,0xFF70,0x0000 };	//NULL terminator

const I_FontAttributes * C_Font::GetFontAttributes(WCHAR glyph, bool *ea_glyph)
{
	const I_GlyphAttributes *g_attr;
	
#ifdef GAME_RACERNOTAMAX
	if(glyph > 0x3000 && m_bUseReverseTextureLook)
	{
		g_attr=LookupGlyph(&m_pCommonAdditionalFont,glyph,true);
		if(g_attr)
		{
			if(ea_glyph)
				*ea_glyph=true;
			return(&m_pCommonAdditionalFont);
		}

		g_attr=LookupGlyph(&m_pUIAdditionalFont,glyph,true);
		if(g_attr)
		{
			if(ea_glyph)
				*ea_glyph=true;
			return(&m_pUIAdditionalFont);
		}

		g_attr=LookupGlyph(m_pCurrentFont,glyph,true);
		if(g_attr || glyph==32)
		{
			if(ea_glyph)
				*ea_glyph=false;
			return(m_pCurrentFont);
		}
	}
	else
#endif
	{
		g_attr = m_pCurrentFont->LookupGlyph(glyph,true);
		if(g_attr || glyph==32)
		{
			if(ea_glyph)
				*ea_glyph=false;

			return(m_pCurrentFont);
		}

		if(m_pCommonAdditionalFont)
		{
			g_attr = m_pCommonAdditionalFont->LookupGlyph(glyph,true);
			if(g_attr)
			{
				if(ea_glyph)
					*ea_glyph=true;

				return(m_pCommonAdditionalFont);
			}
		}

		if(m_pUIAdditionalFont)
		{
			g_attr=m_pUIAdditionalFont->LookupGlyph(glyph,true);
			if(g_attr)
			{
				if(ea_glyph)
					*ea_glyph=true;

				return(m_pUIAdditionalFont);
			}
		}
	}

	if(ea_glyph)
		*ea_glyph=false;

	return(m_pCurrentFont);	//return the current as that will have the null glyph in it
}

bool C_Font::ShouldWrap(F32 x,const WCHAR *strtext,F32 size,F32 height, C_FontStyle& style)
{
	if ( size <= 0.0f )
		return false;

#if WORDWRAP_FIX // WORDWRAP_FIX: mfuse : 2005.8.24 05:31
	WCHAR temp_str[2]={'\0','\0'};
	F32 letter_len=0;
	S32 idx=0;
	S32 idx_head;
	while(IsWhiteSpace(*(strtext+idx)))
	{
		temp_str[0]=*(strtext+idx);
		letter_len+=StrLen(temp_str,m_pCurrentFont,height,height_percentage);
		idx++;
	}
	idx_head = idx;
	while(*(strtext+idx) && !WordWrap_CanBreakLineAt(strtext+idx,strtext+idx_head))
	{
		if(*(strtext+idx) == L'~')
		{
			bool twice=false;
			idx++;
			if(*(strtext+idx) && *(strtext+idx) == L'~')
			{
				idx++;
				twice = true;
			}
			while(*(strtext+idx) && *(strtext+idx) != L'~')
			{
				idx++;
			}

			if(*(strtext+idx) && *(strtext+idx) == L'~') 
				idx++;

			if( twice )
			{
				if(*(strtext+idx) && *(strtext+idx) == L'~') idx++;
				letter_len+=height;
			}
		}
		else
		{
			const I_FontAttributes *pCurrentFont=GetFontAttributes(*(strtext+idx),NULL);
			temp_str[0]=*(strtext+idx);
			letter_len+=StrLen(temp_str,m_pCurrentFont,height,height_percentage);
			idx++;
		}
	}
	if(x+letter_len >= size)
	{
		return true;
	}
#else
	WCHAR letter=*strtext;
	WCHAR next_letter=*(strtext+1);
	WCHAR temp_str[2];
	F32 letter_len=0;
	bool is_next_char_valid_wrap=false;
	U32 pntr;

	if(letter == 0x0020)
	{
		temp_str[0]=letter;
		temp_str[1]='\0';

		letter_len = StringLength(temp_str, m_fCurrentHeight, style, true);

		pntr=1;
		letter=*(strtext+pntr);
		pntr++;
		next_letter=*(strtext+pntr);

		while(letter != L' ' && letter != L'\n' && letter != L'\0')
		{
			//check for special characters - colour
			if(letter == L'~' && next_letter == L'~')
			{
				pntr++;
				letter=*(strtext+pntr);

				//have we got /n - reset count
				if(letter == 0x002f && *(strtext+pntr+1) == 0x006e)
				{
					x = m_StartPos.X();
				}

				while(letter != L'~' && letter != L'\0' )
				{
					letter=*(strtext+pntr);
					pntr++;
				}

				pntr++;
				letter=*(strtext+pntr);

				letter_len+=m_fCurrentHeight;
			}
			else if(letter == L'~')
			{
				pntr++;
				letter=*(strtext+pntr);

				//have we got /n
//				if(letter == 0x002f && *(strtext+pntr+1) == 0x006e) return true;

				while(letter != L'~' && letter != L'\0' )
				{
					letter=*(strtext+pntr);
					pntr++;
				}

				letter=*(strtext+pntr);

				if(letter == L'~') pntr++;
				
				letter=*(strtext+pntr);
			}
			else
			{
				temp_str[0]=letter;
				temp_str[1]='\0';

				letter_len+=StringLength(temp_str,m_fCurrentHeight,style, true);
		
				letter=*(strtext+pntr);
				pntr++;
			}

		}

		if((x+letter_len) >= (size)) 
		{
			return true;
		}
	}
	else if(letter > 0x3000)
	{
		pntr=0;
		WCHAR temp_c=jpn_wordwrap[pntr];
		while(temp_c)
		{
			if(temp_c == next_letter) is_next_char_valid_wrap=true;
			temp_c=jpn_wordwrap[++pntr];
		}

		temp_str[0]=letter;
		temp_str[1]='\0';

		letter_len=StringLength(temp_str,m_fCurrentHeight,style, true);

		if(is_next_char_valid_wrap)
		{
			temp_str[0]=next_letter;
			temp_str[1]='\0';

			letter_len+=StringLength(temp_str,m_fCurrentHeight,style, true);
		}

		if((x+letter_len) >= (size)) 
		{
			return true;
		}
	}
#endif

	return false;
}

S32 C_Font::ExtractSequenceEscapeNumber( const WCHAR* pSeq )
{
	if ((pSeq[0] == '\0') || (pSeq[1] == '\0'))
		return 0;

	return _wtoi( pSeq + 2 );
}

F32 C_Font::ExtractSequenceEscapeNumberF( const WCHAR* pSeq )
{
	if ((pSeq[0] == '\0') || (pSeq[1] == '\0'))
		return 0;

	return (F32)_wtof( pSeq + 2 );
}


F32 C_Font::GetHeightPercentage( C_FontStyle& style )
{
	return ( style.HeightPercentage() == s_fUseDefaultHeightPercentage ) ?  m_fDefaultHeightPercentage : style.HeightPercentage();
}

F32 C_Font::ProcessText( const WCHAR* pStringText, const I_FontAttributes* pFont, F32 fHeight, const F32 fWrapSize, C_FontStyle& style, const E_FontFlag eFontRenderFlag )
{
	if ( !pStringText )
		return 0.0f;

	assert( pFont );

	bool bShouldDraw = ( eFontRenderFlag == E_FontFlag_Normal ) || ( eFontRenderFlag == E_FontFlag_NormalNoWrap );
	if ( bShouldDraw )
	{
		m_SpriteStack.Reset();
	}

	const I_FontAttributes* pCurrentFont = pFont;
	const I_GlyphAttributes* pFixedGlyph = 0;
	bool fixedGlyphRequiresLookup = true;

	const F32 hrzT = pCurrentFont->GetHrzOffset( true );
	const F32 hrzF = pCurrentFont->GetHrzOffset( false );
	const F32 hrz = hrzT + hrzF;
	F32 actual = (pCurrentFont->GetFontHeight() - hrzT - hrzF);
	if( actual <= 0.0f )
		actual = 1.0f;
	const F32 x_start = -(hrzF * m_fDrawHeight / actual);

	F32 x = x_start;
	F32 y = m_StartPos.Y();

	S32 sDrawCount = m_sDrawCount;

	m_pCurrentFont = pFont;
	C_UISpriteColours startColour = m_CurrentColour;

	y -= ( pCurrentFont->GetVrtOffset( true ) * m_fDrawHeight / actual );

	C_TextureParam null_texture;
	null_texture.m_Position.u	= 0.0f;
	null_texture.m_Position.v	= 0.0f;
	null_texture.m_Size.u		= 1.0f;
	null_texture.m_Size.v		= 1.0f;
	null_texture.m_TextureID	= 0;

	m_ItalicIndex = 0;

	bool bItalics = style.Italics();

	if( bItalics )
	{
		if( !m_PreTransformIndex )
		{
			m_ItalicIndex = m_ItalicsMatrixIndex;
			if(m_eCurrentJustify == E_Justify_TopLeft || m_eCurrentJustify == E_Justify_MiddleLeft || m_eCurrentJustify == E_Justify_BottomLeft)
				x += m_fCurrentHeight * sinf(Maths::DegToRad(13.0f));
			else
			if(m_eCurrentJustify == E_Justify_TopCentre || m_eCurrentJustify == E_Justify_MiddleCentre || m_eCurrentJustify == E_Justify_BottomCentre)
				x += m_fCurrentHeight * sinf(Maths::DegToRad(13.0f)) * 0.5f;
		}
		else
		{
			// We've a transform AND we're asking for italics - bloody hell, you want the moon on a stick.
			const Maths::HMat43& input_matrix	= gSimple2D.GetPreTransformMatrix( m_PreTransformIndex );
			const Maths::HMat43& italic_matrix	= gSimple2D.GetPreTransformMatrix( m_ItalicsMatrixIndex );

			// Create a new combined matrix
			m_ItalicIndex = gSimple2D.PopPreTransformMatrix();
			gSimple2D.GetPreTransformMatrix( m_ItalicIndex ) = input_matrix * italic_matrix;
		}
	}

	F32 fHeightPercentage = GetHeightPercentage( style );
	S32 sCurrentLayer = m_CurrentLayer;

//	const S32 dropShadowFontID = (stricmp(font->Name,"Arial")==0) ? gUITextureManager.GetTextureByName("FontDropShadow") : pCurrentFont->Texture;
	bool lb=true;
	bool first_word = true;

	while ( *pStringText && (sDrawCount==-1 || sDrawCount>0))
	{
		if( ( eFontRenderFlag == E_FontFlag_Normal ) || ( eFontRenderFlag == E_FontFlag_CheckingLines ) )
		{
			bool cr = false;

			// Found space?
			if( CouldWrap(*pStringText) )
			{
				first_word = false;
				if( ShouldWrap(x, pStringText, fWrapSize, m_fCurrentHeight, style))
				{
					// clear the trailing spaces
					while( IsWhiteSpace(*pStringText) )
						pStringText++;	//clear the trailing space

					// POBs three trailing spaces bug leaves us at null
					if( *pStringText == L'\0' )
						break;

					cr = true;
				}
			}

			// Break long words if they're the first word
			if( first_word && fWrapSize > 0.0f && x > fWrapSize )
				cr = true;

			// do carriage return?
			if( cr )
			{
				x = x_start;
				y += GetWrapShiftY( m_fDrawHeight, fHeightPercentage, m_CurrentPanel );
				m_uNumLinesDrawn++;
				first_word = true;
				lb = true;
			}
		}

		WCHAR letter		= *pStringText++;
		WCHAR next_letter	= *pStringText;

		m_pCurrentFont = pFont;
		pCurrentFont = pFont;

		//Single ~ is a colour
		//Double ~ is a graphic or escape sequence i.e. /t100 tabs 100 pixels
		if(letter == L'~')
		{
			bool please_continue=true;

			if(next_letter == L'~')
			{
				//Must be a font sequence
				WCHAR seq[32];
				S32 b=0;
				pStringText++; // Skip second ~

				while(*pStringText != L'~' && *pStringText)
				{
					seq[b++]=*pStringText++;
				}
				seq[b]=NULL;

				if(*pStringText) pStringText++;	//Clear remaining ~

				F32 sx1 = x;
				F32 sx2 = m_fCurrentHeight;
				F32 sy1 = y;
				F32 sy2 = m_fCurrentHeight;

				if(seq[0] == 0x002f)
				{
					if(seq[1] == L'l' || seq[1] == L'L')
					{						
						m_CurrentLayer += ExtractSequenceEscapeNumber( seq );

						if ( m_CurrentLayer > gSimple2D.TopLayer() )
							m_CurrentLayer = gSimple2D.TopLayer();
					}
					else if(seq[1] == L't' || seq[1] == L'T')
					{
						x = ExtractSequenceEscapeNumberF( seq );
					}
					//Resize by percentage, syntax ~~/s##~~ should be in the range 999 to 1
					else if(seq[1] == L's' || seq[1] == L'S')
					{
						//CharlieA - Changed so that /s resizes the text as percentage of the requested size
						F32 sizePercentage = ExtractSequenceEscapeNumberF( seq ) / 100.0f;
						F32 newHeight = sizePercentage * m_fCurrentHeight;

						assert( "/s percentage is outside 999% to 1%" && sizePercentage > 0.01f && sizePercentage < 10.f);

						//Because we're resizing the font here we need to move its position up and down
						//so that it lines up with the rest of the font. Unfortunately there's an empty
						//border around the outside of the character which messes the maths up a little 
						//so we have to use a bodge value to move it down by the extra amount 
						//Say the border is 20% of the height of the font
						F32 fontBorderPercentage = 0.2f;
						F32 y_up = gSimple2D.IsPanel3D( m_CurrentPanel ) ? -1.0f : 1.0f;
						F32 newY = y + ( m_fCurrentHeight - (m_fCurrentHeight * fontBorderPercentage) + (newHeight * fontBorderPercentage) - newHeight ) * y_up;
						y = newY;

						m_fCurrentHeight = newHeight;
					}
					else if(seq[1] == L'x')
					{
						x += ExtractSequenceEscapeNumberF( seq );
					}
					else if(seq[1] == L'X')
					{
						x += ExtractSequenceEscapeNumberF( seq ) * m_fCurrentHeight;
					}
					else if(seq[1] == L'y')
					{
						y += ExtractSequenceEscapeNumberF( seq );
					}
					else if(seq[1] == L'Y')
					{
						y += ExtractSequenceEscapeNumberF( seq ) * m_fCurrentHeight;
					}						
					else if(seq[1] == L'p' || seq[1] == L'P')
					{
						letter=L'%';
						please_continue = false;
					}
					else if(seq[1] == L'n' || seq[1] == L'N')
					{
						// clear the trailing spaces
						while(IsWhiteSpace(*pStringText))
							pStringText++;	//clear the trailing space

						if ( ( eFontRenderFlag == E_FontFlag_Normal ) || ( eFontRenderFlag == E_FontFlag_CheckingLines ) || bShouldDraw )
						{
							x = x_start;
							y += GetWrapShiftY( m_fDrawHeight, fHeightPercentage, m_CurrentPanel );
							m_uNumLinesDrawn++;
							lb=true;
						}
					}
				}
				else if(seq[0] == L'n' && seq[1] == L'n')
				{
					// clear the trailing spaces
					while(IsWhiteSpace(*pStringText))
						pStringText++;	//clear the trailing space

					if ( ( eFontRenderFlag == E_FontFlag_Normal ) || ( eFontRenderFlag == E_FontFlag_CheckingLines ) || bShouldDraw )
					{
						x = x_start;
						y += GetWrapShiftY( m_fDrawHeight, fHeightPercentage, m_CurrentPanel );
						m_uNumLinesDrawn++;
						lb=true;
					}

				}
				else
				{
					C_FontSequence* pSeq = NULL;

					for(U32 a=0;a<m_uNumFontSequences;a++)
					{
						if(wcscmp(m_pFontSequences[a].m_Sequence,seq) == 0) 
						{
							pSeq = &m_pFontSequences[a]; 
							break;
						}
					}
					
					if(pSeq == NULL)
					{
						pSeq = &m_pFontSequences[0];
					} 

					F32 fCurrentHeight = m_fCurrentHeight * fHeightPercentage;

					if ( bShouldDraw )
					{
						C_UIColour col;
						col = pSeq->m_Colour;

						col.A( m_CurrentColour[0].A() );

						F32 fY = sy1;
/*
						if ( gSimple2D.IsPanel3D( m_CurrentPanel ) )
						{
							if ( ( m_eCurrentJustify == E_Justify_TopLeft ) || ( m_eCurrentJustify == E_Justify_TopCentre ) || ( m_eCurrentJustify == E_Justify_TopRight ) )
								fY -= sy2;
							else if ( ( m_eCurrentJustify == E_Justify_BottomLeft ) || ( m_eCurrentJustify == E_Justify_BottomCentre ) || ( m_eCurrentJustify == E_Justify_BottomRight ) )
								fY += sy2;
						}
*/
						C_FontSpriteItem*p = m_SpriteStack.PopNext();
						if( p )
						{
							F32 up = gSimple2D.IsPanel3D( m_CurrentPanel ) ? 1.0f : -1.0f;
							F32 fSx = ( sx2 * pSeq->m_Aspect ) * pSeq->m_ScaleX;
							F32 fSy = sy2 * pSeq->m_ScaleY;
							Maths::HVec3 pos( sx1 + (fCurrentHeight * pSeq->m_LeadingX), fY + ( ( fCurrentHeight * pSeq->m_OffsetY ) + ( fSy - sy2 ) * 0.5f ) * up, m_StartPos.Z() );
							F32 fActualWidth = ( sx2 * pSeq->m_Aspect ) * pSeq->m_ScaleX;

							// Use a bitmap or shape for seq
							if ( pSeq->m_Texture.Active() )
							{
								p->Set( I_GlyphAttributes::E_GlyphType_Bitmap, pSeq->m_Texture, lb, C_FontSpriteItem::E_Type_Seq, 
									pos, fSx, fSy, col,
									pSeq->m_U, pSeq->m_V,
									pSeq->m_SizeU, pSeq->m_SizeV,
									pSeq->m_DropShadow,
									pSeq->m_AllowItalics ? bItalics : false, m_CurrentLayer,
									letter,
									fActualWidth );
								p->m_InputGlyph = pSeq->m_InputGlyph; // [Blur2PC]
							}
							else
							if ( pSeq->m_SceneIndex != 0xffffffff )
							{
								p->Set( I_GlyphAttributes::E_GlyphType_Shape, (INTPTR)pSeq, lb, C_FontSpriteItem::E_Type_Seq, 
									pos, fSx, fSy, col,
									pSeq->m_DropShadow,
									pSeq->m_AllowItalics ? bItalics : false, m_CurrentLayer,
									letter,
									fActualWidth );
							}
						}

						// Move aspect difference
						x += ( fCurrentHeight * ( pSeq->m_Aspect - 1.0f ) ) * pSeq->m_ScaleX;
						lb = false;
					}

					x += ( (pSeq->m_LeadingX + pSeq->m_TrailingX) * fCurrentHeight ) + ( fCurrentHeight * pSeq->m_ScaleX );
					lb = false;
				}
			}
			else
			{
				//Must be a colour
				if(pStringText[0] == 'r')
				{
					// Restore old colour
					m_CurrentColour = startColour;
				}
				else
				{
					S32 a;
					WCHAR col[9];
					for(a=0;a<8;a++)
					{
						if(*pStringText == '\0') break;
						col[a]=*pStringText++;
					}
					col[a]=0;

					S32 icol=0;
					S32 np=swscanf(col,L"%x",&icol);

					if(np == 0)
					{
						LOGWARN( g_Log,  "There is a problem with getting the colour out");
						return 0;
					}

					if ( bShouldDraw && !m_bIgnoreEscapeColours )
					{
						U8 a = m_CurrentColour[ 0 ].A();
						m_CurrentColour = icol;
						m_CurrentColour.SetAlpha( a );
					}
				}
			}

			if(*pStringText) 
				pStringText++;	//clear remaining ~

			if( please_continue )
				continue;
		}

		bool ea_glyph=false;
		pCurrentFont = GetFontAttributes(letter,&ea_glyph);

		if(letter == 0x0020 || letter == 0x00a0)
		{
			x+=(m_fDrawHeight/7);
			if(sDrawCount > 0)
				sDrawCount--;
			continue;
		}

		const I_GlyphAttributes* found_glyph;

		if (pCurrentFont->GetNumGlyphs() == 0)		//must be trying to draw an East Asian font in a European language
		{
			pCurrentFont = m_pCurrentFont;
			found_glyph = pCurrentFont->GetDefaultGlyph( );
			ea_glyph = false;
		}
		else
		{
			found_glyph = pCurrentFont->LookupGlyph(letter, false, m_fCurrentHeight );
		}
		F32 width_mod = m_fCurrentHeight / pCurrentFont->GetFontHeight();

		if(!found_glyph)
		{
			if(ea_glyph)	//must be in the UI pack
			{
				found_glyph = m_pUIAdditionalFont->LookupGlyph(letter, false, true );
				if (found_glyph)
				{
					pCurrentFont = m_pUIAdditionalFont;
				}
				else
				{
					pCurrentFont = m_pCurrentFont;
					found_glyph = pCurrentFont->GetDefaultGlyph( );
				}
			}
			else
			{
				return 0.0f;
			}
		}

		bool fixedSpace = false;

		if ( m_eFixedSpaceType != E_FixedSpaceType_None )
		{
			if ( m_eFixedSpaceType == E_FixedSpaceType_All )
				fixedSpace = true;
			else
				fixedSpace = ( m_eFixedSpaceType == E_FixedSpaceType_Letters) ^ ( letter >= L'0' && letter <= L'9' );

			if (fixedSpace)
			{
				if (fixedGlyphRequiresLookup)
				{
					pFixedGlyph					= pCurrentFont->LookupGlyph( L'8', false, false );
					fixedGlyphRequiresLookup	= false;
				}

				if (!pFixedGlyph)
					fixedSpace = false;
			}
		}

		if( !lb )
			x += (fixedSpace ? pFixedGlyph->GetOffset() : found_glyph->GetOffset()) * width_mod;

		if ( bShouldDraw )
		{
			if(found_glyph->GetType() == I_GlyphAttributes::E_GlyphType_Bitmap) // Todo : better
			{
				C_BitmapGlyphAttribute* pGlyph = (C_BitmapGlyphAttribute*)found_glyph;
				C_BitmapFontAttributes* pFont = pGlyph->GetFont();
				if(!pFont)
					pFont = (C_BitmapFontAttributes*)pCurrentFont;

				F32 width =	(pGlyph->GetWidth() + hrz) * width_mod;
				F32 sx1 = x;
				F32 sx2 = width;
				F32 sy1 = y;
				F32 sy2 = m_fCurrentHeight;
				F32 fActualWidth = sx2;

				if(ea_glyph)
				{
					sx2 *= pFont->GetSizeOveride();
					sy1 += (m_fCurrentHeight * pFont->m_TopOffset);
					sy2 = m_fCurrentHeight * pFont->GetSizeOveride();
				}
				const F32 fPixelShaveBodge = 0.0;

				F32 tx1, tx2, ty1, ty2;
#if 0
				// New exciting format
				tx1 = ((F32)pGlyph->GetLeft()	+ fPixelShaveBodge) / (F32)pFont->m_TextureWidth;
				tx2 = ((F32)pGlyph->GetRight()	- fPixelShaveBodge) / (F32)pFont->m_TextureWidth;
				ty1 = ((F32)pGlyph->GetTop()	+ fPixelShaveBodge) / (F32)pFont->m_TextureHeight;
				ty2 = ((F32)pGlyph->GetBottom()	- fPixelShaveBodge) / (F32)pFont->m_TextureHeight;
#else
				tx1 = (F32)pGlyph->GetLeft();
				tx2 = (F32)pGlyph->GetRight();
				ty1 = (F32)pGlyph->GetTop();
				ty2 = (F32)pGlyph->GetBottom();
#endif
				// Centre letters if fixed space
				if(fixedSpace)
				{
					F32 fixed_space_size = (pFixedGlyph->GetWidth() + pFixedGlyph->GetAdvance()) * width_mod;
					sx1 += (fixed_space_size-width)*0.5f;
					fActualWidth += (fixed_space_size-width)*0.5f;
				}

				C_FontSpriteItem*p = m_SpriteStack.PopNext();
				if( p )
				{
					p->Set( 
						found_glyph->GetType(),
						pFont->m_Texture, 
						lb, 
						ea_glyph ? C_FontSpriteItem::E_Type_EAsian : C_FontSpriteItem::E_Type_Normal, 
						Maths::HVec3( sx1, sy1, m_StartPos.Z() ),
						sx2,
						sy2,
						m_CurrentColour,
						tx1,
						ty1,
						tx2-tx1,
						ty2-ty1,
						true,
						bItalics, 
						m_CurrentLayer,
						letter,
						fActualWidth );
				}
			}
			else
			if(found_glyph->GetType() == I_GlyphAttributes::E_GlyphType_Polygon)
			{
				C_UIPolygonFont* pFont = (C_UIPolygonFont*)pCurrentFont;
				C_UICharacter *pChar = (C_UICharacter *)found_glyph;
				INTPTR uUserData = (INTPTR)&pChar->GetRenderInfo();	// [Blur2PC][x64]
				F32 width =	(F32)(pChar->GetWidth() + pChar->GetAdvance()) * width_mod;//found_glyph->GetWidth() * width_mod;
				F32 sx1 = x;
				F32 sx2 = width;
				F32 sy1 = y;
				F32 sy2 = m_fCurrentHeight;
				F32 fActualWidth = sx2;

				// Centre letters if fixed space
				if(fixedSpace)
				{
					F32 fixed_space_size = (pFixedGlyph->GetWidth() + pFixedGlyph->GetAdvance()) * width_mod;
					sx1 += (fixed_space_size-width)*0.5f;
					fActualWidth += (fixed_space_size-width)*0.5f;
				}

				/*
				if(ea_glyph)
				{
					sx2 *= pFont->GetSizeOveride();
					sy1 += (height * pFont->m_TopOffset);
					sy2 = height * pFont->GetSizeOveride();
				}
				*/
				C_FontSpriteItem*p = m_SpriteStack.PopNext();
				if( p )
				{
					p->Set( 
						found_glyph->GetType(),
						uUserData, 
						lb, 
						ea_glyph ? C_FontSpriteItem::E_Type_EAsian : C_FontSpriteItem::E_Type_Normal,
						Maths::HVec3( sx1, sy1, m_StartPos.Z() ),
						sx2,
						sy2,
						m_CurrentColour,
						true,
						bItalics, 
						m_CurrentLayer,
						letter,
						fActualWidth );
				}
			}
		}

		if(ea_glyph)
		{
			x += (found_glyph->GetAdvance() + found_glyph->GetWidth() + 2) * width_mod * pCurrentFont->GetSizeOveride();
		}
		else
		{
			if(fixedSpace)
			{
				x +=( (pFixedGlyph->GetAdvance() + pFixedGlyph->GetWidth()) * width_mod);
			}
			else
			{
				x+= ( (found_glyph->GetAdvance() + found_glyph->GetWidth()) * width_mod);
			}
		}

		if(*pStringText)
		{
			x += style.Tracking() * m_fCurrentHeight;

#ifndef CFG_GOLD
			x += g_debugUIFontTracking * m_fCurrentHeight;
#endif
		}

		lb = false;
		if ( eFontRenderFlag != E_FontFlag_CheckingLength )
		{
			if ( fWrapSize && CouldWrap( *pStringText ) && ShouldWrap( x, pStringText, fWrapSize, m_fCurrentHeight, style ) )
			{
				//clear the trailing space
				if(*pStringText == 0x0020)
					pStringText++;
				x = x_start;
				y += GetWrapShiftY( m_fDrawHeight, fHeightPercentage, m_CurrentPanel );
				m_uNumLinesDrawn++;
				lb=true;
			}
		}
				
		if(!fixedSpace)
		{
			const F32 kerning = pCurrentFont->LookupKerning(letter, next_letter);
			x += kerning*width_mod;
		}

		if(sDrawCount > 0)
			sDrawCount--;
	}

	// set the layer back to the original (for the drop shadows)
	m_CurrentLayer = sCurrentLayer;

	// Do the actual drawing here (so we can draw the string backwards)
	if( bShouldDraw )
	{
		const bool is3d = gSimple2D.IsPanel3D( m_CurrentPanel );
		// Justify the list
		if( m_eCurrentJustify != E_Justify_TopLeft )
		{
			U32 i = m_SpriteStack.DrawStart();
			U32 j = m_SpriteStack.DrawStart();
			U32 end = m_SpriteStack.DrawEnd();
			F32 fLineHeight = ( i == m_SpriteStack.DrawEnd() ) ? 0.0f : ( (F32)m_SpriteStack[ end-1 ].m_Pos.Y() + (is3d ? -m_SpriteStack[ end-1 ].m_Sy : m_SpriteStack[ end-1 ].m_Sy) ) - (F32)m_StartPos.Y();
			fLineHeight -= ( pCurrentFont->GetVrtOffset( false ) * m_fDrawHeight / actual );

			while( i < end )
			{				
				F32 fOffsetX = 0.0f;
				F32 fOffsetY = 0.0f;
				F32 ex = 0.0f;

				// skip to the next line (or end)
				do
				{
					ex = (m_SpriteStack[ i ].m_Pos.X() + m_SpriteStack[ i ].m_fActualWidth);
					i++;
					if( i == end || m_SpriteStack[ i ].m_bLineBreak )
						break;

				} while( i < end );
	
				// calculate line width
				F32 fLineWidth = ex - m_SpriteStack[ j ].m_Pos.X();
				fLineWidth -= ( pCurrentFont->GetVrtOffset( false ) * m_fDrawHeight / actual * 2.0f );

				/*
				switch (m_eCurrentJustify)
				{
					case E_Justify_TopLeft:
						break;	// no need to do anything
					case E_Justify_TopCentre:
						fOffsetX = fLineWidth / 2;
						break;
					case E_Justify_TopRight:
						fOffsetX = fLineWidth;
						break;
					case E_Justify_MiddleLeft:
						fOffsetY = fLineHeight / 2;
						break;
					case E_Justify_MiddleCentre:
						fOffsetX = fLineWidth / 2;
						fOffsetY = fLineHeight / 2;
						break;
					case E_Justify_MiddleRight:
						fOffsetX = fLineWidth;
						fOffsetY = fLineHeight / 2;
						break;
					case E_Justify_BottomLeft:
						fOffsetY = fLineHeight;
						break;
					case E_Justify_BottomCentre:
						fOffsetX = fLineWidth / 2;
						fOffsetY = fLineHeight;
						break;
					case E_Justify_BottomRight:
						fOffsetX = fLineWidth;
						fOffsetY = fLineHeight;
						break;
					default:
						break;
				}

				// apply the offsets
				for( ; j < i; j++ )
				{
					m_SpriteStack[ j ].m_Pos -= Maths::HVec3( fOffsetX, fOffsetY, 0.0f);
				}
				*/

				Maths::HVec3 offset( fLineWidth, fLineHeight, 0.0f );
				offset *= gSimple2D.GetJustificationOffset( m_eCurrentJustify, 0 );

				for( ; j < i; j++ )
					m_SpriteStack[ j ].m_Pos -= offset;
			}
		}

		for(U32 i = m_SpriteStack.DrawStart(); i < m_SpriteStack.DrawEnd(); i++)
			m_SpriteStack[ i ].m_Pos += Maths::HVec3(m_StartPos.X(), 0.0f, 0.0f);

		// Call back for altering list before it's rendered
		if ( m_pPreRender )
			m_pPreRender->ProcessStack( m_SpriteStack );

		// do the new rendering here - all in one function
		style.Render( m_SpriteStack, m_fCurrentHeight, m_CurrentPanel );
	}

	return x + (pCurrentFont->GetHrzOffset( true ) * m_fDrawHeight / actual);
}

void C_Font::StartFrame()
{
	//
}

void C_Font::DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStyle& style, const F32 fWrapSize, const PanelIndex panel )
{
	if ( !id.IsValid() || !id.IsLocString() )
	{
		LOGWARN( g_Log, DBSTR " : Tried to draw an invalid string" );
		ClearVars();
		return;
	}
	DrawText( layer, pos, eJustify, id.String(), colours, fHeight, style, fWrapSize, panel );
}

// [MarkC] I did this to get PS3 version compiling
void C_Font::DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStyle& style, const F32 fWrapSize, const PanelIndex panel )
{
	if ( !id.IsValid() || !id.IsLocString() )
	{
		LOGWARN( g_Log, DBSTR " : Tried to draw an invalid string" );
		ClearVars();
		return;
	}

	C_TextStringID tempid = id;
	DrawText( layer, pos, eJustify, tempid.String(), colours, fHeight, style, fWrapSize, panel );
}

void C_Font::DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const WCHAR* pString, const C_UISpriteColours& colours, const F32 fHeight, C_FontStyle& style, const F32 fWrapSize, const PanelIndex panel )
{
	if ( !m_bInitialised || !colours.HasAlpha() )
	{
		ClearVars();
		return;
	}

	const WCHAR* pStringText = pString;

	if ( !pString )
	{
		LOGWARN( g_Log, DBSTR " : Tried to draw an null string" );
		ClearVars();
		return;
	}

	E_FontFlag eFontRenderFlag = E_FontFlag_NormalNoWrap;

	U32 uFont = style.Font();

	if ( uFont > m_uNumFonts )
	{
		LOGERROR( g_Log, DBSTR "Invalid font %d passed", uFont );
		uFont = 0;
	}

	I_FontAttributes* pCurrentFont = m_pFonts[ uFont ];

	if ( !pCurrentFont )
	{
		LOGWARN( g_Log, DBSTR " : Tried to draw with a font, %d, that doesn't exist", uFont );
		ClearVars();
		return;
	}


	m_CurrentPanel = panel;
	m_CurrentLayer = layer;

	m_eCurrentJustify = eJustify;
	m_uNumLinesDrawn = 0;

	m_CurrentColour = colours;

#if WORDWRAP_FIX // WORDWRAP_FIX: mfuse : 2005.8.24 05:29
	// for Korean word wrap
	if(flags)
	{
		WordWrap_SetOption( WW_PROHIBITION | WW_NOHANGULWRAP );
	}
	else
	{
		WordWrap_SetOption( WW_PROHIBITION );
	}
#endif
	StoreVars( pCurrentFont, fHeight );

	// Store
	m_CurrentColour = colours;
	m_StartPos = pos;

	if ( fWrapSize > 0.0f )
	{
		F32 fWidth = StringLength( pString, fHeight, style, true );
		eFontRenderFlag = ( fWidth > fWrapSize ) ? E_FontFlag_Normal : E_FontFlag_NormalNoWrap;

#ifndef CFG_GOLD
		if ( g_debugUIBoundingBoxes )
		{
			gSimple2D.AddSprite( layer, Maths::HVec3( m_StartPos.X(), m_StartPos.Y(), 0.0f ), fWrapSize, fHeight, 0x80ffffff, panel );
			gSimple2D.AddJustification( eJustify );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );

			if( gFont.m_PostTransformIndex )
				gSimple2D.AddPostTransformIndex( gFont.m_PostTransformIndex );
		}
#endif
	}

	/*
	// FIX ME!
	if( gSimple2D.IsPanel3D( panel ) )
		m_StartPos.Y( m_StartPos.Y() + fHeight );
	*/

	// Process
	ProcessText( pStringText, pCurrentFont, fHeight, fWrapSize, style, eFontRenderFlag );

	// clean up
	ClearVars();
}

void C_Font::StoreVars( I_FontAttributes* pCurrentFont, const F32 fHeight )
{
	F32 actual = (pCurrentFont->GetFontHeight() - pCurrentFont->GetHrzOffset( true ) - pCurrentFont->GetHrzOffset( false ));
	if( actual <= 0.0f )
		actual = 1.0f;
	F32 mul = ( actual > 0.0f ) ? pCurrentFont->GetFontHeight() / actual : 1.0f;

	m_fDrawHeight = fHeight;
	m_fCurrentHeight = fHeight * mul;
}

void C_Font::DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const WCHAR* pString, const C_UISpriteColours& colours, const F32 fHeight, C_FontStylePasses& styles, const F32 fWrapSize, const PanelIndex panel )
{
	// we flag not to clear the vars here so transforms, fixedspace etc are preserved through render passes
	m_bDoNotClearVars = true;

	// process each style
	for ( U32 i = 0; i < styles.NumFontStyles(); i++ )
	{
		if ( i == ( styles.NumFontStyles() - 1 ) )
			m_bDoNotClearVars = false;

		DrawText( layer, pos, eJustify, pString, colours, fHeight, styles.GetFontStyle( i ), fWrapSize, panel );
	}
}

void C_Font::DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStylePasses& styles, const F32 fWrapSize, const PanelIndex panel )
{
	if ( !id.IsValid() || !id.IsLocString() )
	{
		LOGWARN( g_Log, DBSTR " : Tried to draw an invalid string" );
		ClearVars();
		return;
	}

	DrawText( layer, pos, eJustify, id.String(), colours, fHeight, styles, fWrapSize, panel );
}

// [MarkC] I did this to get PS3 version compiling
void C_Font::DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStylePasses& styles, const F32 fWrapSize, const PanelIndex panel )
{
	if ( !id.IsValid() || !id.IsLocString() )
	{
		LOGWARN( g_Log, DBSTR " : Tried to draw an invalid string" );
		ClearVars();
		return;
	}
	C_TextStringID tempid = id;
	DrawText( layer, pos, eJustify, tempid.String(), colours, fHeight, styles, fWrapSize, panel );
}
#ifndef CFG_GOLD
void C_Font::DrawDebugText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const WCHAR* pString, const C_UIColour& colour, const F32 fHeight, const F32 fWrapSize, const PanelIndex panel )
{
	DrawText( layer, pos, eJustify, pString, colour, fHeight, C_FontStyle::Instance(), fWrapSize, panel );
}
#endif

void C_Font::ClearVars( )
{
	if ( m_bDoNotClearVars )
		return;

	m_bIgnoreEscapeColours	= false;
	m_bUseReverseTextureLook = true;

	m_sDrawCount = -1;	

	m_pPreRender				= NULL;

	m_PreTransformIndex			= 0;
	m_PostTransformIndex		= 0;

	m_eFixedSpaceType = E_FixedSpaceType_None;
}

F32	C_Font::StringLength( const WCHAR* pString, const F32 fHeight, C_FontStyle& style, const bool bInternal )
{
	U32 uFont = style.Font();

	if ( uFont > m_uNumFonts )
	{
		LOGERROR( g_Log, DBSTR "invalid font, %d", uFont );
		uFont = 0;
	}

	I_FontAttributes* pFont = m_pFonts[ uFont ];

	if ( !pFont )
	{
		LOGWARN( g_Log, DBSTR "tried to draw with a font, %d, that doesn't exist", uFont );
		return 0.0f;
	}

	if ( !bInternal )
		StoreVars( pFont, fHeight );

	F32 fLength = ProcessText( pString, pFont, fHeight, 0.0f, style, E_FontFlag_CheckingLength );

	// clean up
	if ( !bInternal )
		ClearVars();

	return fLength;
}

F32	C_Font::StringLength( const WCHAR* pString, const F32 fHeight, const bool bInternal )
{
	return StringLength( pString, fHeight, C_FontStyle::Instance() );
}

F32	C_Font::StringLength( C_TextStringID& id, const F32 fHeight, C_FontStyle& style, const bool bInternal )
{
	if ( !id.IsValid() )
	{
		LOGWARN( g_Log, DBSTR "invalid string" );
		return 0.0f;
	}

	const WCHAR* pString = NULL;

	if ( id.IsLocString() )
	{
		pString = id.String();
	}
	else
	{
		LOGWARN( g_Log, DBSTR "invalid string" );
		return 0.0f;
	}

	return StringLength( pString, fHeight, style );
}

U32	C_Font::StringNumLines( const WCHAR* pString, const F32 fHeight, const F32 fWrapSize, C_FontStyle& style )
{
	if ( wcslen( pString ) == 0 )
		return 0;

	U32 uFont = style.Font();

	if ( uFont > m_uNumFonts )
	{
		LOGERROR( g_Log, DBSTR "invalid font, %d", uFont );
		uFont = 0;
	}

	I_FontAttributes* pFont = m_pFonts[ uFont ];

	if ( !pFont )
	{
		LOGWARN( g_Log, DBSTR "tried to draw with a font, %d, that doesn't exist", uFont );
		return 0;
	}

	m_uNumLinesDrawn = 0;

	StoreVars( pFont, fHeight );
	ProcessText( pString, pFont, fHeight, fWrapSize, style, E_FontFlag_CheckingLines );

	U32 uNumLines = GetNumLinesDrawn();

	// clean up
	ClearVars();

	return uNumLines;
}

U32	C_Font::StringNumLines( const WCHAR* pString, const F32 fHeight, const F32 fWrapSize )
{
	return StringNumLines( pString, fHeight, fWrapSize, C_FontStyle::Instance() );
}

U32	C_Font::StringNumLines( C_TextStringID& id, const F32 fHeight, const F32 fWrapSize, C_FontStyle& style )
{
	if ( !id.IsValid() )
	{
		LOGWARN( g_Log, DBSTR "invalid string" );
		return 0;
	}

	const WCHAR* pString = NULL;

	if ( id.IsLocString() )
	{
		pString = id.String();
	}
	else
	{
		LOGWARN( g_Log, DBSTR "invalid string" );
		return 0;
	}

	return StringNumLines( pString, fHeight, fWrapSize, style );
}

S32 C_Font::FindFontByName( const char* pFontName, const bool bZeroIfNotFound )
{
	for ( U32 i = 0; i < m_uNumFonts; i++ )
	{
		if ( !stricmp( m_pFonts[ i ]->GetName().Chars(), pFontName ) )
			return i;
	}
	
	if ( bZeroIfNotFound )
		return 0;

	return -1;
}

// C_FontSpriteItem
void C_FontSpriteStack::DrawStart( const U32 start )
{
	if( start >= m_DrawEnd )
		return;

	m_DrawStart = (U16)start;
}

void C_FontSpriteStack::DrawEnd( const U32 end )
{
	if( end <= (U16)m_DrawStart )
		return;

	if( end <= (U16)m_Array.Size() )
		m_DrawEnd = (U16)end;
}

void C_FontSpriteStack::Reset( )
{
	m_Array.Clear();
	m_DrawStart = 0;
	m_DrawEnd = 0;
}

void C_FontSpriteStack::Resize( const U32 capacity )
{
	m_Array.Reserve( capacity );

	Reset();
}

const U32 C_FontSpriteStack::Size( ) const
{
	return m_Array.Size();
}

C_FontSpriteItem* C_FontSpriteStack::PopNext( )
{
	if( m_Array.Size() == m_Array.Capacity() )
	{
		LOGWARN( g_Log, DBSTR " too many items " );
		return NULL;
	}

	m_Array.PushBack();
	C_FontSpriteItem *p = &m_Array.Back();

	m_DrawEnd = m_Array.Size(); 

	return p;
}

C_FontSpriteStack& C_FontSpriteStack::operator=( C_FontSpriteStack& _in )
{
	m_DrawStart	= _in.m_DrawStart;
	m_DrawEnd	= _in.m_DrawEnd;

	m_Array = _in.m_Array;

	return *this;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif

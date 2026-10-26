// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdDefaultMemory.h>
#include <bdGraphics/bdFont-wii.h>

#include <revolution/gx.h>
#include <revolution.h>
#include <math.h>

#define FONT_MAX	(10+26+2)	// "0-9" "A-Z" "." "-"
#define FONT_WD	24		// SDK fixed
#define FONT_HT	24		// SDK fixed
#define FONT_SHEET_WD	(FONT_WD * FONT_MAX)	// Support up to 1024.
#define FONT_SHEET_HT   FONT_HT
#define FONT_TEX_SIZE	(FONT_SHEET_WD * FONT_SHEET_HT * 4 / 8)

static f32	canvas_wd,canvas_ht ;	// Virtual canvas size
static f32	cursor_wd,cursor_ht ;	// Cursor size
static s32	cursor_nx,cursor_ny ;	// Number of cursors arranged on canvas
static f32	cursor_x1,cursor_y1 ;	// Coordinate of upper leftmost cursor
static f32	letter_ox,letter_oy ;	// Offset from the cursor position to the character position
static f32	letter_wd,letter_ht ;	// Character size

static char	*font_str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.-" ;
static void	*font_tex ;
static GXTexObj	font_tobj ;

static f32	s1_ofs ;	// Depending on whether the main unit is the Japanese or USA version:
static f32	s2_ofs ;	//  Adjustments are made to display different fonts
static f32	t1_ofs ;	// 

bdFont::bdFont()
{
}

bdFont::~bdFont()
{
}

bool bdFont::init()
{
	init_font_texture();
	return true;
}

/*******************************************************************************
	Prepare a texture from a built-in font
*******************************************************************************/
void bdFont::init_font_texture( void )
{
	OSFontHeader	*font ;
	void		*tmp ;
	void		*tex_p ;
	char		*str_p ;
	u32		*ptr ;
	s32		wd, ofs ;

  //----- Allocate texture buffer
	font_tex = bdMemory::alignedAllocate( FONT_TEX_SIZE , 32) ;

	//----- Clear texture buffer
	ptr = (u32*)( (u32)font_tex + FONT_TEX_SIZE ) - 1 ;
	do {
		*ptr = 0x00000000 ;
	} while ( --ptr != (u32*)font_tex ) ;

	//----- Load font data
	if ( OSGetFontEncode() == OS_FONT_ENCODE_SJIS ) {
		font = (OSFontHeader*)bdMemory::alignedAllocate( OS_FONT_DATA_SIZE_SJIS , 32) ;
		tmp  = bdMemory::alignedAllocate( OS_FONT_ROM_SIZE_SJIS , 32) ;

		s1_ofs =  0.15f * (f32)FONT_WD / (f32)FONT_SHEET_WD ;
		s2_ofs = -0.30f * (f32)FONT_WD / (f32)FONT_SHEET_WD ;
		t1_ofs =  0.00f ;
	} else {
		font = (OSFontHeader*)bdMemory::alignedAllocate( OS_FONT_DATA_SIZE_ANSI, 32) ;
		tmp  = bdMemory::alignedAllocate( OS_FONT_ROM_SIZE_ANSI, 32 ) ;

		s1_ofs =  0.05f * (f32)FONT_WD / (f32)FONT_SHEET_WD ;
		s2_ofs = -0.10f * (f32)FONT_WD / (f32)FONT_SHEET_WD ;
		t1_ofs = 0.13f ;
	}
	(void)OSLoadFont( font, tmp ) ;

	//----- Convert font data to a texture
	tex_p = font_tex ;
	str_p = font_str ;
	do {
		(void)OSGetFontWidth( str_p, &wd ) ;
		ofs = ( FONT_WD - wd ) / 2 ;
		(void)OSGetFontTexel( str_p, tex_p, ofs+1, FONT_SHEET_WD/4, &wd ) ;
		str_p = OSGetFontTexel( str_p, tex_p, ofs, FONT_SHEET_WD/4, &wd ) ;
		tex_p = (void*)( (u32)tex_p + FONT_WD / 8 * 32 ) ;
	} while ( *str_p != '\0' ) ;

	DCStoreRange( font_tex, FONT_TEX_SIZE ) ;
	GXInitTexObj( &font_tobj, font_tex, FONT_SHEET_WD,FONT_SHEET_HT, GX_TF_I4, GX_CLAMP,GX_CLAMP, GX_FALSE ) ;
	GXInitTexObjLOD( &font_tobj, GX_LINEAR,GX_LINEAR, 0.0f,0.0f, 0.0f, GX_DISABLE, GX_FALSE, GX_ANISO_1 ) ;

	//----- Free font data
	bdMemory::alignedDeallocate( font ) ;
	bdMemory::alignedDeallocate( tmp ) ;
}

void bdFont::init_draw_font( u16 fb_width, u16 fb_height, f32 lt_width, f32 lt_height )
{
	//----- Bless the issuance of this command
	GXInvalidateVtxCache() ;
	GXInvalidateTexAll() ;

	//----- Calculate variables
	canvas_wd = fb_width * 0.91346f ;
	canvas_ht = fb_height ;
	letter_wd =  lt_width ;
	letter_ht =  lt_height ;
	cursor_wd = letter_wd * 0.90f ;		// Reduce separation between letters a bit
	cursor_ht = letter_ht * 1.05f ;		// Make a little extra room in the vertical direction
	letter_ox = ( cursor_wd - letter_wd ) * 0.5f ;
	letter_oy = ( cursor_ht - letter_ht ) * 0.5f ;
	cursor_nx = (s32)( canvas_wd / cursor_wd  ) ;
	cursor_ny = (s32)( canvas_ht / cursor_ht ) ;
	cursor_x1 = (f32)cursor_nx * cursor_wd * -0.5f ;
	cursor_y1 = (f32)cursor_ny * cursor_ht * -0.5f ;

	//----- VERTEX
   GXClearVtxDesc() ;
	GXSetVtxDesc( GX_VA_POS , GX_DIRECT ) ;
	GXSetVtxDesc( GX_VA_TEX0, GX_DIRECT ) ;
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS , GX_POS_XY, GX_F32, 0 ) ;
	GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 ) ;

	//----- TEXTURE
	GXSetNumTexGens( 1 ) ;
	GXSetTexCoordGen( GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY ) ;
	GXLoadTexObj( &font_tobj, GX_TEXMAP0 ) ;

	//----- TEV
	GXSetNumTevStages( 1 ) ;
 	GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL ) ;
	GXSetTevColorIn( GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO ) ;
	GXSetTevColorOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV ) ;
	GXSetTevAlphaIn( GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO ) ;
	GXSetTevAlphaOp( GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_ENABLE, GX_TEVPREV ) ;
	GXSetAlphaCompare( GX_ALWAYS,0, GX_AOP_OR, GX_ALWAYS,0 ) ;

	//----- SCREEN
	GXSetBlendMode( GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP ) ;
	GXSetAlphaUpdate( GX_DISABLE ) ;

	GXSetZMode( GX_DISABLE, GX_ALWAYS, GX_DISABLE ) ;
	GXSetCullMode( GX_CULL_BACK ) ;
}
	
void bdFont::draw_font_one( f32 cx, f32 cy, char c )
{
	f32		x1,x2, y1,y2, s1,s2 ;
	s32		i ;


	if ( c >= 'A' && c <= 'Z' ) {
		i = c - 'A' + 10 ;
	} else if ( c >= '0' && c <= '9' ) {
		i = c - '0' ;
	} else if ( c == '.' ) {
		i = 10 + 26 ;
	} else if ( c == '-' ) {
		i = 10 + 26 + 1 ;
	} else {
		return ;
	}

	s1 = (f32)( i * FONT_WD ) / (f32)FONT_SHEET_WD + s1_ofs ;
	s2 = s1 + (f32)FONT_WD / (f32)FONT_SHEET_WD + s2_ofs ;

	x1 = cx * cursor_wd + cursor_x1 + letter_ox ;
	x2 = x1 + letter_wd ;

	y1 = cy * cursor_ht + cursor_y1 + letter_oy ;
	y2 = y1 + letter_ht ;

	GXBegin( GX_QUADS, GX_VTXFMT0, 4 ) ;
	  GXPosition2f32( x1,y1 ) ; GXTexCoord2f32( s1,t1_ofs ) ;
      GXPosition2f32( x2,y1 ) ; GXTexCoord2f32( s2,t1_ofs ) ;
	  GXPosition2f32( x2,y2 ) ; GXTexCoord2f32( s2,1.0f   ) ;
	  GXPosition2f32( x1,y2 ) ;	GXTexCoord2f32( s1,1.0f   ) ;
	GXEnd() ;
}

void bdFont::draw_font_letter( f32 cx, f32 cy, GXColor clr, char *str )
{
	f32		x = cx ;
	char		*cp = str ;


	GXSetTevColor( GX_TEVREG0, clr ) ;

	do {
		draw_font_one( x, cy, *cp ) ;
		x += 1.0f ;
	} while ( *++cp != '\0' ) ;
}

void bdFont::drawString(const bdNChar8* str, float cx, float cy, GXColor text_clr)  
{
	char c;
    const char* cp;
	f32		x;

  	cp = str;
	x = cx ;

	GXSetTevColor( GX_TEVREG0, text_clr ) ;

	do {
		c =(char)*cp;
		if(c == '\n')
		{
			cy += 1.0f;
			x = cx;
		}
		c = bdToUpper(c); // No lowercase font texture available, so make all CAPS
		bdFont::draw_font_one( x, cy, c);
		x += 1.0f ;
	} while ( *++cp != '\0' ) ;
}


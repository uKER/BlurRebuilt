// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_FONT_WII_H
#define BD_FONT_WII_H

#include <bdCore/bdCore.h>
#include <bdGraphics/bdColor.h>

#include <revolution/gx.h>
#include <revolution.h>
#include <math.h>

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
	static bool init();
	static void beforeFontDraw( u16 fb_width, u16 fb_height );
	static void init_draw_font( u16 fb_width, u16 fb_height, f32 lt_width, f32 lt_height );
	static void draw_font_letter( f32 cx, f32 cy, GXColor clr, char *str );
	static void draw_font_one( f32 cx, f32 cy, char c );
	static void drawString(const bdNChar8* str, float cx, float cy, GXColor text_clr) ;

protected:

	static void init_font_texture( void );
   
 
	static s32 draw_font_s32( f32 cx, f32 cy, GXColor clr, s32 val );
	static s32 draw_font_f32( f32 cx, f32 cy, GXColor clr, f32 val, s32 keta );
	static void draw_font_bit( f32 cx, f32 cy, GXColor clr, u32 val, s32 keta );
	
	static f32 calc_font_x1( f32 cx );
	static f32 calc_font_y1( f32 cy );
	static f32 calc_font_x2( f32 cx );
	static f32 calc_font_y2( f32 cy );
	
protected:


};
#endif // BD_FONT_WII_H

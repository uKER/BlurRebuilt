// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#import <Cocoa/Cocoa.h>

#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>

#import "GLString.h"

class bdDemo;

@interface bdOpenGLView : NSOpenGLView
{
	NSTimer* timer;
	
	bool fAnimate;
	CFAbsoluteTime mtime;	
}

- (void) drawRect: (NSRect) bounds;

- (void) prepareOpenGL;
- (void) awakeFromNib;

- (BOOL) acceptsFirstResponder;
- (void) keyDown:(NSEvent*)event;
- (void) keyUp:(NSEvent*)event;

- (void) createHelpString;
- (void) drawInfo;

+ (void)storeDemoReference:(bdDemo*)demo;
+ (void)updateDisplayString:(const char *const)str;

@end

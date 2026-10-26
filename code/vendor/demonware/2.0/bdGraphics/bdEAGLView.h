// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE 
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

class bdDemo;

@interface bdEAGLView : UIView {

//NSString *displayString;
@private
    GLint backingWidth;
    GLint backingHeight;
    
    EAGLContext *context;
    
    GLuint viewRenderbuffer, viewFramebuffer;
    GLuint depthRenderbuffer;
    
    NSTimer *animationTimer;
    NSTimeInterval animationInterval;
	
	CGPoint startTouchPosition;//= [touch locationInView:self];
}

@property NSTimeInterval animationInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;
- (void)setupView;

+ (void)storeDemoReference:(bdDemo*)demo;
+ (void)updateDisplayString:(const char *const)str;
//- (void) render:(CGRect) bounds;

- (void) drawString:(const char *const) str withBounds:(CGRect) bounds;
//- (void) updateDisplayString:(const char *const) str;
@end

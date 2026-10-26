// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE 
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "bdEAGLView.h"
#import "Texture2D.h"

#include <bdGraphics/bdDemo.h>
#include <bdGraphics/bdGfx.h>

#include <math.h>


#define USE_DEPTH_BUFFER 0

@interface bdEAGLView ()

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) NSTimer *animationTimer;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation bdEAGLView


// Unfortunately objective-C has no concept of a static member function, so these live here semi-global
static bdDemo *sDemo = NULL;
Texture2D	*testTex = nil;
//static char *const displayString = NULL;
static NSString *displayString; 

@synthesize context;
@synthesize animationTimer;
@synthesize animationInterval;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}


//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
    
    if ((self = [super initWithCoder:coder])) {
		
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
        
        animationInterval = 1.0 / 60.0;
		[self setupView];
		
		displayString =  [[NSString stringWithFormat:@""] retain];
		bdGfx::getInstance()->init(NULL,NULL);

    }
    return self;
}


// Touch interface
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	UITouch *touch = [[touches allObjects] objectAtIndex:0];
	
	if([touches count] == 1)
		sDemo->markKeyAsPressed(BD_BUTTON1, true);
	else if([touches count] == 3)
		
		sDemo->markKeyAsPressed(BD_BUTTON2, true);

	startTouchPosition = [touch locationInView:self];

}

#define HORIZ_SWIPE_DRAG_MIN  12
#define VERT_SWIPE_DRAG_MAX    4
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {

    UITouch *touch = [touches anyObject];
    CGPoint currentTouchPosition = [touch locationInView:self];
	if([touches count] == 2)
	{		
		// To be a swipe, direction of touch must be horizontal and long enough.
		if (fabsf(startTouchPosition.x - currentTouchPosition.x) >= HORIZ_SWIPE_DRAG_MIN &&
			fabsf(startTouchPosition.y - currentTouchPosition.y) <= VERT_SWIPE_DRAG_MAX)
		{
			// It appears to be a swipe.
			if (startTouchPosition.x < currentTouchPosition.x)
				sDemo->markKeyAsPressed(BD_MOVE_RIGHT, true);
			else
				sDemo->markKeyAsPressed(BD_MOVE_LEFT, true);
					
		}
		// To be a swipe, direction of touch must be horizontal and long enough.
		if (fabsf(startTouchPosition.x - currentTouchPosition.x) <= HORIZ_SWIPE_DRAG_MIN &&
			fabsf(startTouchPosition.y - currentTouchPosition.y) >= VERT_SWIPE_DRAG_MAX)
		{
			if (startTouchPosition.y < currentTouchPosition.y)
				sDemo->markKeyAsPressed(BD_MOVE_DOWN, true);
			else
				sDemo->markKeyAsPressed(BD_MOVE_UP, true);
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	if ([touches count] == [[event touchesForView:self] count]) {
		for(int i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
		{
			sDemo->markKeyAsPressed((BD_CONTROLS)i, false);
		}
	}
}


- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	//NSLog(@"touchesCancelled"); 
}

// </Touch interface>

- (void)setupView {  // new method for intialisation of variables and states
	
	// Enable Multi Touch of the view
	self.multipleTouchEnabled = YES;
	
	// setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Setup Orthographic Projection for the 320 x 480 of the iPhone screen
	glOrthof(0.0f, 320.0f, 480.0f, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	
}

- (void)drawView {
    sDemo->step(0.1f); //FIXME eman -timing
	
    [EAGLContext setCurrentContext:context];
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glViewport(0, 0, backingWidth, backingHeight);
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	
	bdGfx::getInstance()->runFrame();
	
	[self drawString:"" withBounds:[self bounds] ];
	
	
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (void)layoutSubviews {
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
    [self drawView];
}


- (BOOL)createFramebuffer {
    
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if (USE_DEPTH_BUFFER) {
        glGenRenderbuffersOES(1, &depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}


- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}


- (void)startAnimation {
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation {
    self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer {
    [animationTimer invalidate];
    animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval {
    
    animationInterval = interval;
    if (animationTimer) {
        [self stopAnimation];
        [self startAnimation];
    }
}


- (void)dealloc {
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

+ (void)storeDemoReference:(bdDemo*)demo
{
	sDemo = demo;
}


+ (void)updateDisplayString:(const char *const)string
{
	NSString *str = [NSString stringWithCString:string length:strlen(string)];
	
	if (![displayString isEqualToString:str])
	{
		[displayString release];
		displayString =  [[NSString stringWithString:str] retain];
		
		if (testTex)
		{
			[testTex dealloc];
			testTex = [[Texture2D alloc] initWithString:str dimensions:CGSizeMake(256, 256) alignment:UITextAlignmentCenter fontName:@"Helvetica" fontSize:14];
		}	
		else 
		{
			testTex = [[Texture2D alloc] initWithString:str dimensions:CGSizeMake(256, 256) alignment:UITextAlignmentCenter fontName:@"Helvetica" fontSize:14];
		}
	}	
	
}

- (void) drawString:(const char *const) str withBounds:(CGRect) bounds 
{	
	
	// setup viewport and projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, bounds.size.width, 0, bounds.size.height, -1, 1);
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// texturing will need these
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_TEXTURE_2D);
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	if (testTex == nil)
		testTex = [[Texture2D alloc] initWithString:[NSString stringWithFormat:@""] dimensions:CGSizeMake(256, 256) alignment:UITextAlignmentCenter fontName:@"Helvetica" fontSize:14];
	
	// text will need blending
	glEnable(GL_BLEND);
	
	// text from Texture2D uses A8 tex format, so needs GL_SRC_ALPHA
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	[testTex drawAtPoint:CGPointMake(bounds.size.width * 0.5f, bounds.size.height * 0.5f)];//* 0.1f, bounds.size.height * 0.875f)];
	
	// switch it back to GL_ONE for other types of images, rather than text because Texture2D uses CG to load, which premultiplies alpha
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

@end

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#import "bdOpenGLView.h"

#include <OpenGL/gl.h>

#include <bdGraphics/bdGfx.h>
#include <bdGraphics/bdDemo.h>

@implementation bdOpenGLView

static CFAbsoluteTime gStartTime = 0.0f;

// Unfortunately objective-C has no concept of a static member function, so these live here semi-global
static bdDemo *sDemo;
static GLString * helpStringTex;
static GLfloat sHeight, sWidth;

// string attributes
NSMutableDictionary * stanStringAttrib;
NSString * dispString;

// set app start time
static void setStartTime (void)
{	
	gStartTime = CFAbsoluteTimeGetCurrent ();
}

float ff = 0.0;

-(void) drawRect: (NSRect) bounds
{
	bdGfx::getInstance()->runFrame();
	[self drawInfo];
	glFlush();
}

-(void) prepareOpenGL
{
	// init fonts for use with strings
	NSFont * font =[NSFont fontWithName:@"Helvetica" size:12.0];
	stanStringAttrib = [[NSMutableDictionary dictionary] retain];
	[stanStringAttrib setObject:font forKey:NSFontAttributeName];
	[stanStringAttrib setObject:[NSColor whiteColor] forKey:NSForegroundColorAttributeName];
	[font release];
	
	// ensure strings are created
	[self createHelpString];
	
	bdGfx::getInstance()->init(NULL,NULL);
}

- (void)animationTimer:(NSTimer *)timer
{
	BOOL shouldDraw = NO;
	if (fAnimate) {
		CFTimeInterval deltaTime = CFAbsoluteTimeGetCurrent () - mtime;
		
		if (deltaTime > 10.0) // skip pauses
			return;
		else {
			shouldDraw = YES; // force redraw
			sDemo->step(deltaTime);
		}
	}
	
	mtime = CFAbsoluteTimeGetCurrent (); //reset time in all cases
	if (YES == shouldDraw) 
		[self drawRect:[self bounds]]; // redraw now instead dirty to enable updates during live resize
}


- (void) awakeFromNib
{
	setStartTime (); // get app start time
	fAnimate = 1;
	mtime = CFAbsoluteTimeGetCurrent ();  // set animation time start time

	//sDemo->initOSX(); //TODO: Do we need this... it seemed to do nothing.
	
	// start animation timer
	timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(animationTimer:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode]; // ensure timer fires during resize
}

- (BOOL)acceptsFirstResponder
{
	[[self window] makeFirstResponder:self];
	return YES;
}


- (void)keyDown:(NSEvent*)event
{
	int key = (int)[[event characters] characterAtIndex:0];
	int i = 0;
	
	NSLog([event characters]);
	
	for(i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		if(key == (int)bdInput::BD_CHAR_MAP_OSX[i])
		{
			sDemo->markKeyAsPressed((BD_CONTROLS)i, true);
		}
	}
}

- (void)keyUp:(NSEvent*)event
{
	int key = (int)[[event characters] characterAtIndex:0];
	int i = 0;
		
	for(i = 0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		if(key == 'q')
			[NSApp stop:self];
		
		if(key == (int)bdInput::BD_CHAR_MAP_OSX[i])
		{
			sDemo->markKeyAsPressed((BD_CONTROLS)i, false);
		}
	}
}

- (void) drawInfo
{	
	GLint matrixMode;
	GLboolean depthTest = glIsEnabled (GL_DEPTH_TEST);
	
	sHeight = 600;// camera.viewHeight;
	sWidth = 800;//camera.viewWidth;
	
	// set orthograhic 1:1  pixel transform in local view coords
	glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity ();
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity ();
	glScalef (2.0f / sWidth, -2.0f /  sHeight, 1.0f);
	glTranslatef (-sWidth / 2.0f, -sHeight / 2.0f, 0.0f);
	
	glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	[helpStringTex drawAtPoint:NSMakePoint (floor ((sWidth - [helpStringTex frameSize].width) / 2.0f), 10)];//floor ((sHeight - [helpStringTex frameSize].sHeight) / 1.0f))];
	
	// reset orginal martices
	glPopMatrix();
	glMatrixMode (GL_PROJECTION);
	glPopMatrix();
	glMatrixMode (matrixMode);
	
	glDisable (GL_TEXTURE_RECTANGLE_EXT);
	glDisable (GL_BLEND);
	if (depthTest)
		glEnable (GL_DEPTH_TEST);
}

- (void) createHelpString
{
	NSString * string = [NSString stringWithFormat:@""];
	helpStringTex = [[GLString alloc] initWithString:string withAttributes:stanStringAttrib withTextColor:[NSColor colorWithDeviceRed:1.0f green:1.0f blue:1.0f alpha:1.0f] withBoxColor:[NSColor colorWithDeviceRed:0.0f green:0.5f blue:0.0f alpha:0.5f] withBorderColor:[NSColor colorWithDeviceRed:0.3f green:0.8f blue:0.3f alpha:0.8f]];
}

+ (void)storeDemoReference:(bdDemo*)demo
{
	sDemo = demo;
}

+ (void)updateDisplayString:(const char *const)str
{
	NSString * string = [NSString stringWithCString:str length:strlen(str)];
	if (![dispString isEqualToString:string])
	{
		dispString =  [NSString stringWithString:string];
	
		if (helpStringTex)
			[helpStringTex setString:string withAttributes:stanStringAttrib];
		else {
			helpStringTex = [[GLString alloc] initWithString:string withAttributes:stanStringAttrib withTextColor:[NSColor colorWithDeviceRed:1.0f green:1.0f blue:1.0f alpha:1.0f] withBoxColor:[NSColor colorWithDeviceRed:0.5f green:0.5f blue:0.5f alpha:0.5f] withBorderColor:[NSColor colorWithDeviceRed:0.8f green:0.8f blue:0.8f alpha:0.8f]];
		}
	}	
}

@end

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#import <Cocoa/Cocoa.h>
#import <bdGraphics/bdCocoaBridge-osx.h>
#import <bdGraphics/bdOpenGLView.h>

int bdInitCocoa(int argc, char *argv[], bdDemo *demo )
{
	[bdOpenGLView storeDemoReference:demo];
	return NSApplicationMain(argc,  (const char **) argv);
}

void bdCocoaUpdateDisplayString(const char *const str)
{
	[bdOpenGLView updateDisplayString:str];
}
// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#import <UIKit/UIKit.h>
#import <bdGraphics/bdUIKitBridge-iphone.h>
#import <bdGraphics/bdEAGLView.h>
#import <bdGraphics/bdAppDelegate.h>

int bdInitUIKit(int argc, char *argv[], bdDemo *demo )
{
	[bdEAGLView storeDemoReference:demo];
	
	//FIXME eman - find out how to prevent dead code stripping and remove hack below
	bdAppDelegate*	a = [bdAppDelegate alloc];
	[a release];
	bdEAGLView*	b = [bdEAGLView alloc];
	[b release];

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, nil, nil);
	[pool release];
	return retVal;
}

void bdUIKitUpdateDisplayString(const char *const str)
{
	[bdEAGLView updateDisplayString:str];
}

// returns the size of the device token.
unsigned long bdUIKitGetDeviceToken(char *buffer)
{
	const void *deviceToken = [bdAppDelegate getDeviceToken];
	unsigned long deviceTokenLength = (unsigned long)[bdAppDelegate getDeviceTokenLength];
	
	memcpy(buffer, deviceToken, deviceTokenLength);
	
	return deviceTokenLength;
}

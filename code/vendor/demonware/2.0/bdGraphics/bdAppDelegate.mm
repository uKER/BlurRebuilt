// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#import "bdAppDelegate.h"
#import "bdEAGLView.h"

@implementation bdAppDelegate

@synthesize window;
@synthesize glView;

static NSData *deviceToken;

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	[glView startAnimation];
	
	NSLog(@"Registering for push notifications");
	[[UIApplication sharedApplication] registerForRemoteNotificationTypes: (UIRemoteNotificationTypeAlert)];
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	[glView startAnimation];
}

- (void) applicationWillTerminate:(UIApplication *)application
{
	[glView stopAnimation];
}

- (void) application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)newDeviceToken
{
	deviceToken = [[NSData alloc] initWithData:newDeviceToken];
	NSLog(@"Got the deviceToken! token: %@", deviceToken);
}

- (void) application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)err
{
	if ([err code] == 3010)
	{
		// it's the iPhone simulator
		NSLog(@"iPhone simulator can't register for push notifications...");
	}
	else
	{
		NSLog(@"Error in push notification registration. Error: %@", err);
	}
}

- (void) application:(UIApplication *)application didReceiveRemoteNotification:(NSDictionary *)userInfo
{
	NSLog(@"Got a remote notification: %@", userInfo);
	
	UIAlertView *messageBox = [[UIAlertView alloc]
							   initWithTitle:@"Got a remote notification!"
							   message:[[userInfo objectForKey:@"aps"] objectForKey:@"alert"]
							   delegate:self
							   cancelButtonTitle:@"OK"
							   otherButtonTitles:nil];
	[messageBox show];
	[messageBox release];
}

+ (const void*) getDeviceToken
{
	return [deviceToken bytes];
}

+ (unsigned int) getDeviceTokenLength
{
	return [deviceToken length];
}

- (void) dealloc
{
	[window release];
	[glView release];
	
	[super dealloc];
}

@end

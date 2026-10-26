// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#import <UIKit/UIKit.h>

@class bdEAGLView;

@interface bdAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    bdEAGLView *glView;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet bdEAGLView *glView;

@end


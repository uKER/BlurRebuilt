// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// This 'UIkit bridge' exists so that it is not necessary to call Objective-C calls from within bitdemon C++ code

class bdDemo;

int bdInitUIKit(int argc, char *argv[], bdDemo *demo);
void bdUIKitUpdateDisplayString(const char *const str);
unsigned long bdUIKitGetDeviceToken(char *buffer);
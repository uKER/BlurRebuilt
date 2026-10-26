// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// This 'Cocoa bridge' exists so that it is not necessary to call Objective-C calls from within bitdemon C++ code

class bdDemo;

int bdInitCocoa(int argc, char *argv[],bdDemo *demo);
void bdCocoaUpdateDisplayString(const char *const str);

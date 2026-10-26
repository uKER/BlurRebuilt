// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: OSX IPHONE
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdDemo.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>
#include <bdGraphics/bdGrid.h>

#if defined(BD_PLATFORM_OSX)
#include <bdGraphics/bdCocoaBridge-osx.h>
#elif defined(BD_PLATFORM_IPHONE)
#include <bdGraphics/bdUIKitBridge-iphone.h>
#else
#error Platform is Apple but not OSX or iPhone.
#endif

// Set a frame rate limit of 60 frames per second
#define BD_DEMO_FRAME_TIME ( (1.0f)/(60.0f) )

bdDemo::bdDemo()
{
	// Create the default grid
	m_grid = new bdGrid();
}

bdDemo::~bdDemo()
{
}

void bdDemo::start(bdInt32 argc, bdNChar8* argv[])
{
	if (init(argc, argv))
	{
#if defined(BD_PLATFORM_OSX)
		bdInitCocoa(argc, argv, this);
#else
		bdInitUIKit(argc, argv, this);
#endif

		// TODO: Eman thinks the above call never returns, in which case we need to figure out how to do a proper exit.
		quit();
	}
}

bdBool bdDemo::init(bdInt32 argc, bdNChar8* argv[])
{
	return true;
}

bdBool bdDemo::step(const bdFloat32)
{
	return true;
}

void bdDemo::quit()
{
}

void bdDemo::markKeyAsPressed(const BD_CONTROLS button, bdBool down)
{
	m_input.markKeyAsPressed(button, down);
}

bdInput& bdDemo::getScreen()
{
	return m_input;
}

unsigned long bdDemo::getDeviceToken(char *buffer)
{
	return bdUIKitGetDeviceToken(buffer);
}
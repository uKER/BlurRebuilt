// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: UNIX
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdGraphics/bdDemo.h>
#include <bdPlatform/bdPlatformTiming/bdPlatformTiming.h>

// Set a frame rate limit of 60 frames per second
#define BD_DEMO_FRAME_TIME ( (1.0f)/(60.0f) )

bdDemo::bdDemo()
{
}

bdDemo::~bdDemo()
{
}

void bdDemo::start(bdInt32 argc, bdNChar8* argv[])
{
	// Initialize the demo
	if (init(argc, argv))
	{

		static const bdFloat32 frameTime = BD_DEMO_FRAME_TIME;
		bdBool keepRunning = true;

		// Start the frame rate timer
		m_frameTimer.start();

		// This is the main program loop. It calls the game logic, the render code and calculates the frame rate
		while (keepRunning)
		{
			bdFloat32 elapsedTime = m_frameTimer.getElapsedTimeInSeconds();

			// To limit the frame rate, only run the frame if we have exceeded the time allocated for it
			if (elapsedTime >= frameTime)
			{
				// Reset the timer for the new frame
				m_frameTimer.start();

				// Tell the input object that a new frame has started
				m_input.startFrame();

				// Run the game logic for this frame
				if (!step(elapsedTime))
				{
					// Got a quit request
					keepRunning = false;
				}
			}
			else
			{
				// If we need to wait for the frame to complete, then we should sleep in that
				// time to use less processor power
				bdPlatformTiming::sleep(static_cast<bdUInt>((frameTime - elapsedTime) * 1000.0f));
			}

		}

		// Shutting down
		quit();

	}
}

bdBool bdDemo::init(bdInt32, bdNChar8*[])
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

bdInput& bdDemo::getScreen()
{
	return m_input;
}

// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: A demo class to provide an application framework for all demonware examples

#ifndef BD_DEMO_H
#define BD_DEMO_H

#include <bdCore/bdCore.h>

#include <bdExample/bdExampleCommon/bdInput/bdInput.h>

class bdGrid;

/// Demo class provides the DemonWare Example Application Framework.
class bdDemo
{
	public:
		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor creates the grid object.
		bdDemo();

		/// Destructor.
		virtual ~bdDemo();

		/// The main function for the application. Contains the application's main loop.
		void start(bdInt32 argc = BD_NULL, 
					bdNChar8* argv[] = BD_NULL);

		/// Step is overridden to provide the game logic of an example.
		virtual bdBool step(const bdFloat32 time);

		/// Returns an interface to the user. Allows the reading of input.
		bdInput& getScreen();

		bdUInt getFPS() const;

#ifdef BD_PLATFORM_APPLE
		/// Mark a key as having been pressed.
		void markKeyAsPressed(const BD_CONTROLS button, bdBool down);
		
		/// Copy the APNs deviceToken into the given buffer.
		/// Returns the length of the token (in bytes).
		unsigned long getDeviceToken(char *buffer);
#endif

	protected:

		/// Initialize the game.
		virtual bdBool init(bdInt32 argc, 
							bdNChar8* argv[]);		

		/// Game shutdown code.
		virtual void quit();

	protected:
		/// Input object.
		bdInput m_input;

		/// Timer object to allow the limiting and display of the frame rate.
		bdStopwatch m_frameTimer;

		bdStopwatch m_fpsTimer;

		bdUInt m_fps;

		/// Pointer to the grid object.
		bdGrid *m_grid;
};

#endif // BD_DEMO_H

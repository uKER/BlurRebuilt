// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: A platform independent class to read input from the control device

#ifndef BD_INPUT_H
#define BD_INPUT_H

#include <bdCore/bdCore.h>

/// Define the controls that can be used by bitdemon demos.
enum BD_CONTROLS
{
	BD_MOVE_UP,
	BD_MOVE_DOWN,
	BD_MOVE_LEFT,
	BD_MOVE_RIGHT,
	BD_BUTTON1,
	BD_BUTTON2,
	BD_BUTTON3,
	BD_BUTTON4,
	BD_BUTTON5,
	BD_BUTTON6,
	BD_QUIT,

	BD_NUM_DEFINED_CONTROLS,	// This member should always be last, to give the correct number of defined controls.
};

/// Provide a text description of the button mappings for each platform.
#if defined BD_PLATFORM_PS2 || defined BD_PLATFORM_PS3
#	define BD_MOVE_UP_DESC		"Up"
#	define BD_MOVE_DOWN_DESC	"Down"
#	define BD_MOVE_LEFT_DESC	"Left"
#	define BD_MOVE_RIGHT_DESC	"Right"
#	define BD_BUTTON1_DESC		"Triangle"
#	define BD_BUTTON2_DESC		"Circle"
#	define BD_BUTTON3_DESC		"X"
#	define BD_BUTTON4_DESC		"Rectangle"
#	define BD_BUTTON5_DESC		"L1"
#	define BD_BUTTON6_DESC		"R1"
#	define BD_QUIT_DESC			"Select"
#elif defined BD_PLATFORM_PSP
#	define BD_MOVE_UP_DESC		"DPad Up"
#	define BD_MOVE_DOWN_DESC	"DPad Down"
#	define BD_MOVE_LEFT_DESC	"DPad Left"
#	define BD_MOVE_RIGHT_DESC	"DPad Right"
#	define BD_BUTTON1_DESC		"Triangle"
#	define BD_BUTTON2_DESC		"Circle"
#	define BD_BUTTON3_DESC		"X"
#	define BD_BUTTON4_DESC		"Rectangle"
#	define BD_BUTTON5_DESC		"L"
#	define BD_BUTTON6_DESC		"R"
#	define BD_QUIT_DESC			"Select"
#elif defined BD_PLATFORM_XENON || defined BD_PLATFORM_XBOX
#	define BD_MOVE_UP_DESC		"Up"
#	define BD_MOVE_DOWN_DESC	"Down"
#	define BD_MOVE_LEFT_DESC	"Left"
#	define BD_MOVE_RIGHT_DESC	"Right"
#	define BD_BUTTON1_DESC		"Y"
#	define BD_BUTTON2_DESC		"B"
#	define BD_BUTTON3_DESC		"A"
#	define BD_BUTTON4_DESC		"X"
#	define BD_BUTTON5_DESC		"Left Trigger"
#	define BD_BUTTON6_DESC		"Right Trigger"
#	define BD_QUIT_DESC			"Back"
#elif defined BD_PLATFORM_IPHONE
#	define BD_MOVE_UP_DESC		"W"
#	define BD_MOVE_DOWN_DESC	"S"
#	define BD_MOVE_LEFT_DESC	"A"
#	define BD_MOVE_RIGHT_DESC	"D"
#	define BD_BUTTON1_DESC		"Tap"
#	define BD_BUTTON2_DESC		"Two-finger tap"
#	define BD_BUTTON3_DESC		"B"
#	define BD_BUTTON4_DESC		"N"
#	define BD_BUTTON5_DESC		"K"
#	define BD_BUTTON6_DESC		"L"
#	define BD_QUIT_DESC			"Q"
#elif defined BD_PLATFORM_WIN32 || defined BD_PLATFORM_OSX
#	define BD_MOVE_UP_DESC		"W"
#	define BD_MOVE_DOWN_DESC	"S"
#	define BD_MOVE_LEFT_DESC	"A"
#	define BD_MOVE_RIGHT_DESC	"D"
#	define BD_BUTTON1_DESC		"P"
#	define BD_BUTTON2_DESC		"O"
#	define BD_BUTTON3_DESC		"B"
#	define BD_BUTTON4_DESC		"N"
#	define BD_BUTTON5_DESC		"K"
#	define BD_BUTTON6_DESC		"L"
#	define BD_QUIT_DESC			"Q"
#elif defined BD_PLATFORM_UNIX
#	define BD_MOVE_UP_DESC		""
#	define BD_MOVE_DOWN_DESC	""
#	define BD_MOVE_LEFT_DESC	""
#	define BD_MOVE_RIGHT_DESC	""
#	define BD_BUTTON1_DESC		""
#	define BD_BUTTON2_DESC		""
#	define BD_BUTTON3_DESC		""
#	define BD_BUTTON4_DESC		""
#	define BD_BUTTON5_DESC		""
#	define BD_BUTTON6_DESC		""
#	define BD_QUIT_DESC			""
#elif defined BD_PLATFORM_WII
#	define BD_MOVE_UP_DESC		"Up"
#	define BD_MOVE_DOWN_DESC	"Down"
#	define BD_MOVE_LEFT_DESC	"Left"
#	define BD_MOVE_RIGHT_DESC	"Right"
#	define BD_BUTTON1_DESC		"A"
#	define BD_BUTTON2_DESC		"B"
#	define BD_BUTTON3_DESC		"Plus"
#	define BD_BUTTON4_DESC		"Minus"
#	define BD_BUTTON5_DESC		"1"
#	define BD_BUTTON6_DESC		"2"
#	define BD_QUIT_DESC			"Home"
#else

#endif

/// bdInput allows applications to receive input from controllers and keyboards.
class bdInput
{
protected:
	/// Stores the old state of each button, so we can determine if the button was pressed on this frame or not.
	bdBool m_lastButtons[BD_NUM_DEFINED_CONTROLS];
	bdBool m_pressedButtons[BD_NUM_DEFINED_CONTROLS];
public:
	/// Default Constructor.
	/// Initializes m_buttons.
	bdInput();

	/// Called by bdDemo at the start of each frame, to read the state of the controls.
	void startFrame();

	/// Determines the current state of button \a c, which is one of the BD_CONTROLS enum values.
	/// isKeyDown() returns:
	/// - True, if the button is currently pressed.
	/// - False, if the button is not currently pressed.
	static bdBool BD_CALL isKeyDown(const BD_CONTROLS button);

	/// Determines whether button \a c, which is one of the BD_CONTROLS enum values, has been pressed during the
	/// current frame. This stops button presses from being caught in successive frames, e.g. if Add Player is pressed,
	/// it will only add one player. To add another, the button must be released and pressed again.
	/// wasKeyPressed() returns:
	/// - True, if the button was pressed in the current frame.
	/// - False, if the button is not pressed or if it is pressed and this was caught by a previous frame.
	bdBool wasKeyPressed(const BD_CONTROLS button);

	/// Prints the string \a str to the screen.
	void printToScreen(const bdNChar8 *const str);

#ifdef BD_PLATFORM_APPLE
	/// The map of the BD_CONTROL values to the actual keyboard key values.
	static const int BD_CHAR_MAP_OSX[BD_NUM_DEFINED_CONTROLS];

	/// Mark a key as having been pressed.
	void markKeyAsPressed(const BD_CONTROLS button, bdBool down);
#endif

};
#endif // BD_INPUT_H

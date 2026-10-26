// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdExample/bdExampleCommon/bdInput/bdInput.h>
#include <bdGraphics/bdGfx.h>
#include <bdCore/bdMemory/bdDefaultMemory.h>


#include <revolution/pad.h>
#include <revolution/kpad.h>

const bdInt BD_CHAR_MAP_WII[BD_NUM_DEFINED_CONTROLS] =
{
	WPAD_BUTTON_UP, 	//	BD_MOVE_UP,
	WPAD_BUTTON_DOWN, 	//	BD_MOVE_DOWN,
	WPAD_BUTTON_LEFT, 	//	BD_MOVE_LEFT,
	WPAD_BUTTON_RIGHT, 	//	BD_MOVE_RIGHT,
	WPAD_BUTTON_A, 		//	BD_BUTTON1, 
	WPAD_BUTTON_B, 		//	BD_BUTTON2, 
	WPAD_BUTTON_PLUS, 	//	BD_BUTTON3, 
	WPAD_BUTTON_MINUS, 	//	BD_BUTTON4, 
	WPAD_BUTTON_1, 		//	BD_BUTTON5, 
	WPAD_BUTTON_2, 		//	BD_BUTTON6, 
	WPAD_BUTTON_HOME  	//	BD_QUIT, 
};
/*---------------------------------------------------------------------------*
   Local Variables
 *---------------------------------------------------------------------------*/
static WPADStatus    WPad[PAD_MAX_CONTROLLERS]; // internal use only



static void* exampleAlloc( u32 size )
{
	void* ptr = BD_NULL;
	if( size > 0)
	{
		ptr = bdMemory::alignedAllocate2(size,32);
	}
	
	return ptr;
}

static u8 exampleFree(void *ptr)
{
	bdMemory::alignedDeallocate2(ptr);
	return 1;
}

bdInput::bdInput()
{
    WPADRegisterAllocator(exampleAlloc, exampleFree);
	WPADInit() ;
}

void bdInput::startFrame()
{
	WPADRead( 0, WPad);
	for (bdUInt i=0; i < BD_NUM_DEFINED_CONTROLS; i++)
	{
		bdBool keyDown = isKeyDown(static_cast<BD_CONTROLS>(i));

		m_pressedButtons[i] = !m_lastButtons[i] && keyDown;
		m_lastButtons[i] = keyDown;
	}
}

bdBool BD_CALL bdInput::isKeyDown(const BD_CONTROLS button)
{
	bdBool pressed = false;

	// Make sure that button is a valid value
	BD_ASSERT(button < BD_NUM_DEFINED_CONTROLS, "button must be one of the values from the BD_CONTROLS enum");

	pressed = WPad[0].button & BD_CHAR_MAP_WII[button];

	return pressed;
}

bdBool bdInput::wasKeyPressed(const BD_CONTROLS button)
{
	return m_pressedButtons[button];
}

void bdInput::printToScreen(const bdNChar8 *const str)
{
	bdGfx::getInstance()->setText(str);
}

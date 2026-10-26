#ifndef INPUT_GAMEINPUT_H
#define INPUT_GAMEINPUT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>

namespace Input
{

//! @addtogroup Input
//! @{

class C_GameInputManager;

// Used internally to garbage collect C_ButtonInput and C_AxisInput classes.
class C_RefCount
{
public:
	C_RefCount() : m_RefCount(0) {}
	virtual ~C_RefCount() { assert(m_RefCount == 0); }

	void IncRefCount() { m_RefCount++; }
	const bool DecRefCount() { return(--m_RefCount == 0); }

private:
	S32 m_RefCount;
};

//! Base class used to map from physical device controls to game input controls.
class C_ButtonInput : public C_RefCount
{
public:
	C_ButtonInput() {}
	virtual ~C_ButtonInput() {}
	
	//! Returns the button's pressed value between 0.0 and 1.0f
	virtual const F32 GetValue() const = 0;

	// [Blur2PC] Retail Blur PC's button GetValue takes an out parameter for the type of the device the value
	// came from (Blur.exe 1.2: C_ButtonInputFromDevice 0x00843c90, combiner 0x00844090); inputs that are not
	// device-backed leave it untouched (C_ButtonInputFromAxis 0x00844170).
	virtual const F32 GetValueAndDevice( S32* pDevice ) const { (void)pDevice; return GetValue(); }
};

//! Base class used to map from physical device controls to game input controls.
class C_AxisInput : public C_RefCount
{
public:
	C_AxisInput() {}
	virtual ~C_AxisInput() {}
	
	//! Returns the axis value between -1.0f and 1.0f
	virtual const F32 GetValue() const = 0;
};

// used internally by the C_GameInput class
class C_GameInputButton
{
friend class C_GameInput;

public:
	C_GameInputButton();
	~C_GameInputButton();


private:
	void SetButtonInput( C_ButtonInput* pInput );

	void Update( F32 buttonPressedThreshold );

private:
	F32 m_Value;
    bool m_DebouncePressedState;
	bool m_DebounceReleasedState;
    U16 m_TimeHeld; // Can we make this real time?
#if PLATFORM_PC
	// [Blur2PC] m_TimeHeld counts 1/60 s ticks of real held time on PC (it counted rendered frames,
	// so menus auto-repeated fps/60 times too fast above 60 fps). m_PrevTimeHeld is the value before
	// this Update, so GetButtonAutoRepeat can fire for every tick crossed during the frame.
	U16 m_PrevTimeHeld;
	F32 m_HeldSeconds;
#endif
	C_ButtonInput* m_pButtonInput;
	// [Blur2PC] retail Blur PC keeps the type of the device that produced the value (Input::E_Device, -1 none
	// yet) after the input pointer (button record +0xc, written by 0x008431b0, read by 0x008433a0)
	S32 m_Device;
};

// used internally by the C_GameInput class
class C_GameInputAxis
{
friend class C_GameInput;

public:
	C_GameInputAxis();
	~C_GameInputAxis();

private:
	void SetAxisInput( C_AxisInput* pInput );

	void Update();
private:
    F32 m_Value;
	C_AxisInput* m_pAxisInput;
};

//! This provides a game oriented input device. 
//! The axis and buttons in this device represent actual ingame controls rather than physical buttons on a physical device. 
//! This class is generally what the game should be querying for controller state rather than a phsyical input device.
class C_GameInput
{
friend class C_GameInputManager;

public:
	C_GameInput();
	
	~C_GameInput();

	static C_GameInput* GetInstance( const S32 userIndex );

	//! Index of the signed-in player associated with this C_GameInput.
	const S32 GetUserIndex() const;

	//! Returns the current number of buttons controls available for this device.
	const S32 GetButtonCount() const;

	//! Gets the current C_ButtonInput that is providing the state for button 'buttonId'.
	C_ButtonInput* GetButtonInput( const U32 buttonId ) const;
	
	//! Sets the C_ButtonInput objects that is providing the state for button 'buttonId'. This provides the mapping to the physical devices.
	//! When a button input is assigned using this function it will be reference counted by this system and deleted when C_GameInput is destroyed.
	void SetButtonInput( const U32 buttonId, C_ButtonInput* pInput);

	//! Returns true if the button is pressed
	const bool GetButtonPressed( const U32 buttonId ) const;

	//! Returns current button value ranged 0.0f to 1.0f
	const F32 GetButtonValue( const U32 buttonId ) const;

	//! [Blur2PC] Type of the device (Input::E_Device) that last produced the button's value, -1 if none has
	//! (retail Blur PC 0x008433a0; the UI passes it to Lua with menu actions).
	const S32 GetButtonDevice( const U32 buttonId ) const;

	//! Returns true if this is the first Update cycle that a button has been pressed
	const bool GetButtonPressedDebounced( const U32 buttonId ) const;

	//! Returns true if this is the first Update cycle that a button has not been pressed after the button has been pressed
	const bool GetButtonReleasedDebounced( const U32 buttonId ) const;

	//! Returns the number of Update cycles button has been held for
	const U16 GetButtonPressedTime( const U32 buttonId ) const;

	//! Keyboard style typematic rate and delay. Returns true for debounced key, then waits for the 
	//! 'initialDelay' time.  It will then returns true every 'repeatTime' in Update cycles.
	const bool GetButtonAutoRepeat( const U32 buttonId, const U16 initialDelay, const U16 repeatTime ) const;

	//! Returns the current number of axis controls available for this device.
	const S32 GetAxisCount() const;

	//! Gets the current C_AxisInput that is providing the state for axis 'axisId'.
	C_AxisInput* GetAxisInput( const U32 axisId ) const;

	//! Sets the current C_AxisInput that is providing the state for axis 'axisId'. This provides the mapping to the physical devices.
	//! When an axis input is assigned using this function it will be reference counted by this system and deleted when C_GameInput is destroyed.
	void SetAxisInput( const U32 axisId, C_AxisInput* pInput);

	//! Returns the current axis value between 0.0f and 1.0f, centre is (0.5f)
	const F32 GetAxisValue( const U32 axisId ) const;

	//! Returns the current axis value between -1.0f and 1.0f (centre is 0.0f)
	const F32 GetAxisSignedValue( const U32 axisId ) const;

	//! Sets the threshold 0->1 for ButtonPressed and GetButtonDebounce queries.
	void SetButtonPressedThreshold( const F32 threshold );

private:
	void Update();

private:
	S32 m_UserIndex;

	F32 m_ButtonPressedThreshold;

	Ut::Vector<C_GameInputButton> m_GameButtonList;

	Ut::Vector<C_GameInputAxis> m_GameAxisList;
};

//! This class manages the instances of C_GameInput.
class C_GameInputManager
{
public:
	C_GameInputManager( const S32 userCount, const S32 buttonCount, const S32 axisCount );

	~C_GameInputManager();

	void Resize( const S32 userCount, const S32 buttonCount, const S32 axisCount );

	//! This class is not a singleton although only one instance should be created at once. 
	//! GetInstance can be used to return the currently created instance of the class. 
	//! If the class has not been created GetInstance will return NULL.
	static C_GameInputManager* GetInstance();

	//! This returns the maximum number of signed in user controls we are supporting.
	const S32 GetUserIndexCount();

	//! The helper function Input::GetGameInput() can be used to more easily get a C_GameInput from the user index.
	C_GameInput* GetGameInput( const S32 userIndex );

	//! This should be called once per game tick and will update the state of each C_GameInput.
	void Update();

private:
	Ut::Vector<C_GameInput> m_GameInputList;
};

//! A helper function to get the C_GameInput from a user index.
//! Typical use case:
//! Input::GetGameInput( 0 )->GetButtonPressed( GameControl::E_Button_Accelerate );
C_GameInput* GetGameInput( const S32 userIndex );

C_GameInputManager* GetGameInputManager();

//! @}

} // namespace Input

#endif

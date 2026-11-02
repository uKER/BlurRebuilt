#ifndef INPUT_KEYBOARDPC_H
#define INPUT_KEYBOARDPC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\Device.h>
#include <input\Keyboard.h>

struct IDirectInputDevice8;
typedef IDirectInputDevice8           _IDirectInputDevice;

namespace Input
{

//! @addtogroup Input
//! @{

//! The PC Implementation of a C_Keyboard. 
class C_KeyboardPC : public C_Keyboard
{
public:
	//! Create a C_KeyboardPC using the C_DeviceManager class
	C_KeyboardPC( C_DeviceManager* pDeviceManager, const S32 userIndex );
	~C_KeyboardPC();

	//! Inherited from C_Device
	void Update();
    // [Blur2PC] Unicode characters from WM_CHAR, transferred once per input update.
    static void UpdateTextCharacters(); // [Blur2PC] called once by the device manager
    static void PushTextCharacter(WCHAR character);
    static U32 ReadTextCharacters(WCHAR* output, U32 capacity, U32& generation);


private:
	bool m_Acquired;

	_IDirectInputDevice* m_pDirectInputKeyboard;
};

//! @}

} // namespace Input

#endif

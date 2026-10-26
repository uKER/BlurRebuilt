#ifndef INPUT_GAMEINPUTMAPPING_H
#define INPUT_GAMEINPUTMAPPING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <input\Input.h>
#include <input\GameInput.h>

namespace Input
{

//! @addtogroup Input
//! @{

class C_Device;

//! C_ButtonInputFromDevice
class C_ButtonInputFromDevice : public C_ButtonInput
{
public:
	C_ButtonInputFromDevice( C_Device* pDevice, const S32 deviceButtonIndex );
	~C_ButtonInputFromDevice();

	//! Inherited from I_ButtonInput.
	const F32 GetValue() const;
	const F32 GetValueAndDevice( S32* pDevice ) const;	// [Blur2PC]

private:
	C_Device* m_pDevice;
	const S32 m_DeviceButtonIndex;
};

//! C_ButtonInputCombiner
class C_ButtonInputCombiner : public C_ButtonInput
{
public:
	enum E_Operation
	{
		E_Operation_Max = 0,
		E_Operation_Min
	};

	C_ButtonInputCombiner( const E_Operation operation = E_Operation_Max );
	~C_ButtonInputCombiner();

	void AddButtonInput(C_ButtonInput* pButtonInput);
	const S32 GetButtonInputCount() const;
	C_ButtonInput* GetButtonInput(const S32 buttonIndex) const;

	void SetOperation( const E_Operation operation );

	//! Inherited from I_ButtonInput.
	const F32 GetValue() const;
	const F32 GetValueAndDevice( S32* pDevice ) const;	// [Blur2PC]

private:
	Ut::Vector<C_ButtonInput*> m_ButtonList;
	E_Operation m_Operation;
};

//! C_ButtonInputFromAxis
class C_ButtonInputFromAxis : public C_ButtonInput
{
public:
	C_ButtonInputFromAxis(C_AxisInput* pAxisMap, const bool invert = false, const float threshold = 0.0f);
	~C_ButtonInputFromAxis();

	//! Inherited from I_ButtonInput.
	const F32 GetValue() const;

private :
	C_AxisInput* m_pAxisMap;
	bool m_Invert;
	float m_Threshold;
};

//! C_AxisInputFromDevice
class C_AxisInputFromDevice : public C_AxisInput
{
public:
	C_AxisInputFromDevice( C_Device* pDevice, const S32 deviceAxisIndex );
	~C_AxisInputFromDevice();

	//! Inherited from I_AxisInput.
	const F32 GetValue() const;

private:
	C_Device* m_pDevice;
	const S32 m_DeviceAxisIndex;
};

//! C_AxisMapCombiner
class C_AxisInputCombiner : public C_AxisInput
{
public:
	C_AxisInputCombiner();
	~C_AxisInputCombiner();

	void AddAxisInput(C_AxisInput* pAxisInput);
	const S32 GetAxisInputCount() const;
	C_AxisInput* GetAxisInput(const S32 axisIndex) const;

	//! Inherited from I_ButtonInput.
	const F32 GetValue() const;

private:
	Ut::Vector<C_AxisInput*> m_AxisList;

};

//! C_AxisMapFromButtons
class C_AxisInputFromButtons : public C_AxisInput
{
public:
	C_AxisInputFromButtons( C_ButtonInput* pButtonNegative, C_ButtonInput* pButtonPositive );
	~C_AxisInputFromButtons();

	//! Inherited from I_AxisInput.
	const F32 GetValue() const;

private:
	C_ButtonInput* m_pButtonNegative;
	C_ButtonInput* m_pButtonPositive;
};

//! C_AxisMapFromDigitalButtons - (ignores pressure values from buttons)
class C_AxisInputFromDigitalButtons : public C_AxisInput
{
public:
	C_AxisInputFromDigitalButtons( C_ButtonInput* pButtonNegative, C_ButtonInput* pButtonPositive, F32 fButtonPressedThreshold = 0.1f );
	~C_AxisInputFromDigitalButtons();

	//! Inherited from I_AxisInput.
	const F32 GetValue() const;

private:
	C_ButtonInput* m_pButtonNegative;
	C_ButtonInput* m_pButtonPositive;
	F32 m_fButtonPressedThreshold;
};

//! C_AxisInputShifted. This axis returns zero unless the button is pressed
class C_AxisInputShifted : public C_AxisInput
{
public:
	C_AxisInputShifted( C_AxisInput* pAxisInput, C_ButtonInput* pButtonInput );
	~C_AxisInputShifted();

	//! Inherited from I_AxisInput.
	virtual const F32 GetValue() const;

	void SetThreshold( const F32 threshold );

private:
	C_AxisInput* m_pAxisInput;
	C_ButtonInput* m_pButtonInput;
	F32 m_Threshold;
};

//! C_AxisInputScaled
class C_AxisInputScaled : public C_AxisInput
{
public:
	C_AxisInputScaled( C_AxisInput* pAxisInput, const F32 scale );
	~C_AxisInputScaled();

	//! Inherited from I_AxisInput.
	const F32 GetValue() const;

private:
	C_AxisInput* m_pAxisInput;
	F32 m_Scale;
};

//! @}

} // namespace Input

#endif

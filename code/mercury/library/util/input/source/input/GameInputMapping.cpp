#include "stdafx.h"

#include <input\GameInputMapping.h>
#include <input\Device.h>
#include <core\maths\MathsUtil.h>

namespace Input
{

// C_ButtonInputFromDevice

C_ButtonInputFromDevice::C_ButtonInputFromDevice( C_Device* pDevice, const S32 deviceButtonIndex )
	: m_pDevice( pDevice )
	, m_DeviceButtonIndex( deviceButtonIndex )
{
}

C_ButtonInputFromDevice::~C_ButtonInputFromDevice()
{
}

const F32 C_ButtonInputFromDevice::GetValue() const
{
	return m_pDevice->GetButtonValue( m_DeviceButtonIndex );
}

// [Blur2PC] retail Blur PC 0x00843c90: reports the type of the device the button belongs to
const F32 C_ButtonInputFromDevice::GetValueAndDevice( S32* pDevice ) const
{
	if( pDevice )
		*pDevice = m_pDevice->GetDeviceType();
	return GetValue();
}

// C_ButtonInputCombiner


C_ButtonInputCombiner::C_ButtonInputCombiner( const E_Operation operation ) 
	: m_Operation( operation )
{
}

C_ButtonInputCombiner::~C_ButtonInputCombiner()
{
	for(S32 i=0; i<m_ButtonList.Size(); i++)
		if(m_ButtonList[i] && m_ButtonList[i]->DecRefCount())
			SAFE_DELETE_PTR(m_ButtonList[i]);
}

void C_ButtonInputCombiner::AddButtonInput(C_ButtonInput* pButtonInput)
{
	assert(pButtonInput);
	pButtonInput->IncRefCount();
	m_ButtonList.PushBack(pButtonInput);
}

const S32 C_ButtonInputCombiner::GetButtonInputCount() const
{
	return m_ButtonList.Size();
}

C_ButtonInput* C_ButtonInputCombiner::GetButtonInput(const S32 buttonIndex) const
{
	return m_ButtonList[buttonIndex];
}

void C_ButtonInputCombiner::SetOperation( const E_Operation operation )
{
	m_Operation = operation;
}

const F32 C_ButtonInputCombiner::GetValue() const
{
	const S32 buttonCount = m_ButtonList.Size();

	F32 value = 0;
	switch( m_Operation )
	{
		default:
		case E_Operation_Max:
		{
			value = 0;
			for(S32 i = 0; i < buttonCount; i++)
			{
				// TODO : replace with Max
				const F32 buttonValue = m_ButtonList[ i ]->GetValue();
				if( buttonValue > value  )
				{
					value = buttonValue;
				}		
			}
		}
		break;

		case E_Operation_Min:
		{
			value = 1.0f;
			for(S32 i = 0; i < buttonCount; i++)
			{
				// TODO : replace with Min
				const F32 buttonValue = m_ButtonList[ i ]->GetValue();
				if( buttonValue < value  )
				{
					value = buttonValue;
				}		
			}
		}
		break;
    }

    return value;
}

// [Blur2PC] retail Blur PC 0x00844090: the device of the input that sets the result (the first input holding
// the maximum, or the minimum) is reported. Retail reuses its argument slot as the children's out value, so a
// child that reports no device (a button from an axis) passes on the previous child's device; -1 here where
// retail starts from that slot's pointer value.
const F32 C_ButtonInputCombiner::GetValueAndDevice( S32* pDevice ) const
{
	const S32 buttonCount = m_ButtonList.Size();
	S32 childDevice = -1;

	F32 value;
	if( m_Operation == E_Operation_Min )
	{
		value = 1.0f;
		for(S32 i = 0; i < buttonCount; i++)
		{
			const F32 buttonValue = m_ButtonList[ i ]->GetValueAndDevice( &childDevice );
			if( buttonValue < value )
			{
				value = buttonValue;
				if( pDevice )
					*pDevice = childDevice;
			}
		}
	}
	else
	{
		value = 0;
		for(S32 i = 0; i < buttonCount; i++)
		{
			const F32 buttonValue = m_ButtonList[ i ]->GetValueAndDevice( &childDevice );
			if( buttonValue > value )
			{
				value = buttonValue;
				if( pDevice )
					*pDevice = childDevice;
			}
		}
	}

	return value;
}

// C_ButtonInputFromAxis

C_ButtonInputFromAxis::C_ButtonInputFromAxis( C_AxisInput* pAxisMap, const bool invert, const float threshold )
	: m_pAxisMap( pAxisMap )
	, m_Invert( invert )
	, m_Threshold( threshold )
{
	assert( ( threshold >= 0.0f ) && ( threshold < 1.0f ) );
	pAxisMap->IncRefCount();
}

C_ButtonInputFromAxis::~C_ButtonInputFromAxis()
{
	if(m_pAxisMap->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pAxisMap);
	}
}

const F32 C_ButtonInputFromAxis::GetValue() const
{
	F32 value = m_pAxisMap->GetValue();
	
	if(m_Invert)
		value *= -1.0f;
	
	return Maths::Clamp((value - m_Threshold) / (1.0f - m_Threshold), 0.0f, 1.0f);
}

// C_AxisInputFromDevice
C_AxisInputFromDevice::C_AxisInputFromDevice( C_Device* pDevice, const S32 deviceAxisIndex )
	: m_pDevice( pDevice )
	, m_DeviceAxisIndex( deviceAxisIndex )
{
}

C_AxisInputFromDevice::~C_AxisInputFromDevice()
{
}

const F32 C_AxisInputFromDevice::GetValue() const
{
	return m_pDevice->GetAxisSignedValue( m_DeviceAxisIndex );
}

// C_AxisInputCombiner
C_AxisInputCombiner::C_AxisInputCombiner()
{
}

C_AxisInputCombiner::~C_AxisInputCombiner()
{
	const S32 axisCount = m_AxisList.Size();
	for(S32 axisNo=0; axisNo<axisCount; axisNo++)
	{
		C_AxisInput* pAxisInput = m_AxisList[axisNo];
		if(pAxisInput->DecRefCount())
		{
			SAFE_DELETE_PTR(pAxisInput);
		}
	}
}

void C_AxisInputCombiner::AddAxisInput(C_AxisInput* pAxisInput)
{
	pAxisInput->IncRefCount();
	m_AxisList.PushBack( pAxisInput );
}

const S32 C_AxisInputCombiner::GetAxisInputCount() const
{
	return m_AxisList.Size();
}

C_AxisInput* C_AxisInputCombiner::GetAxisInput(const S32 axisIndex) const
{
	return m_AxisList[axisIndex];
}

const F32 C_AxisInputCombiner::GetValue() const
{
	const S32 axisCount = m_AxisList.Size();

	F32 value = 0.0f;
    for(S32 i = 0; i < axisCount; i++)
    {
		value = value + m_AxisList[i]->GetValue();
    }

    return value;
}

// C_AxisInputFromButtons
C_AxisInputFromButtons::C_AxisInputFromButtons( C_ButtonInput* pButtonNegative, C_ButtonInput* pButtonPositive )
	: m_pButtonNegative(pButtonNegative)
	, m_pButtonPositive(pButtonPositive)
{
	pButtonNegative->IncRefCount();
	pButtonPositive->IncRefCount();
}

C_AxisInputFromButtons::~C_AxisInputFromButtons()
{
	if(m_pButtonNegative->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pButtonNegative);
	}

	if(m_pButtonPositive->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pButtonPositive);
	}
}

const F32 C_AxisInputFromButtons::GetValue() const
{
	return (m_pButtonPositive->GetValue() - m_pButtonNegative->GetValue());
}

// C_AxisInputFromDigitalButtons
C_AxisInputFromDigitalButtons::C_AxisInputFromDigitalButtons( C_ButtonInput* pButtonNegative, C_ButtonInput* pButtonPositive, F32 fButtonPressedThreshold )
: m_pButtonNegative(pButtonNegative)
, m_pButtonPositive(pButtonPositive)
, m_fButtonPressedThreshold(fButtonPressedThreshold)
{
	pButtonNegative->IncRefCount();
	pButtonPositive->IncRefCount();
}

C_AxisInputFromDigitalButtons::~C_AxisInputFromDigitalButtons()
{
	if(m_pButtonNegative->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pButtonNegative);
	}

	if(m_pButtonPositive->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pButtonPositive);
	}
}

const F32 C_AxisInputFromDigitalButtons::GetValue() const
{
	F32 fValue = 0.0f;
	if( m_pButtonPositive->GetValue() > m_fButtonPressedThreshold )
		fValue += 1.0f;
	if( m_pButtonNegative->GetValue() > m_fButtonPressedThreshold )
		fValue -= 1.0f;
	return fValue;
}

// C_AxisInputShifted

C_AxisInputShifted::C_AxisInputShifted( C_AxisInput* pAxisInput, C_ButtonInput* pButtonInput )
	: m_pAxisInput( pAxisInput )
	, m_pButtonInput( pButtonInput )
{
	m_pAxisInput->IncRefCount();
	m_pButtonInput->IncRefCount();

	m_Threshold = 0.5f;
}

C_AxisInputShifted::~C_AxisInputShifted()
{
	if(m_pAxisInput->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pAxisInput);
	}

	if(m_pButtonInput->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pButtonInput);
	}
}

const F32 C_AxisInputShifted::GetValue() const
{
	if(m_pButtonInput->GetValue() < m_Threshold)
	{
		return 0.0f;	
	}

	return m_pAxisInput->GetValue();
}

void C_AxisInputShifted::SetThreshold( const F32 threshold )
{
	m_Threshold = threshold;
}

// C_AxisInputScaled

C_AxisInputScaled::C_AxisInputScaled( C_AxisInput* pAxisInput, const F32 scale ) :
	m_pAxisInput(pAxisInput), 
	m_Scale(scale)
{
	pAxisInput->IncRefCount();
}

C_AxisInputScaled::~C_AxisInputScaled()
{
	if (m_pAxisInput->DecRefCount())
	{
		SAFE_DELETE_PTR(m_pAxisInput);
	}
}

const F32 C_AxisInputScaled::GetValue() const
{
	return m_Scale * m_pAxisInput->GetValue();
}

} // namespace Input

#include "StdAfx.h"
#include "Samples/System/PS3Pad.h"

namespace Samples
{

C_PS3Pad::C_PS3Pad()
{
}

C_PS3Pad::~C_PS3Pad()
{
}

void C_PS3Pad::Initialise()
{
	if (cellPadInit(MAX_PADS))
	{
		printf("Error Initialising Pad\n");
	}
	for (int i = 0; i < MAX_PADS; i++)
	{
		m_Status[i] = 0;
	}
}

void C_PS3Pad::Update()
{
	int32_t ret = 0;
#if CELL_SDK_VERSION >= 0x250000
	if ((ret = cellPadGetInfo2 (&m_Info))) 
#else
	if ((ret = cellPadGetInfo (&m_Info))) 
#endif
	{
		printf ("Error%d : cellPadGetPadInfo\n", ret);
		return;
	}

#if CELL_SDK_VERSION >= 0x250000
	if ((ret = cellPadGetInfo2 (&m_Info))) 
#else
	if ((ret = cellPadGetInfo (&m_Info))) 
#endif 
	{
		printf ("Error%d : cellPadGetPadInfo\n", ret);
		return;
	}

	memcpy(&m_Prev, &m_Next, sizeof(m_Next));

	for (int i = 0; i < MAX_PADS; i++) 
	{
#if CELL_SDK_VERSION >= 0x250000
		if (m_Info.port_status[i] == 0) 
#else
		if (m_Info.status[i] == 0) 
#endif
		{
			continue;
		}

		/* Gamepad i has been connected */
		if (m_Status[i] == 0) 
		{
			/* the previous data shows that gamepad was not
				* yet connected, so it is a new connection
				*/
		/*	printf ("New gamepad %d is connected: VENDOR_ID=%d PRODUCT_ID=%d\n",
				i, m_Info.vendor_id[i], m_Info.product_id[i]);*/

		}
		ret = cellPadGetData (i, &m_Next[i]);
		if (0 != ret) 
		{
			continue;
		}

		if (m_Next[i].len == 0)
			continue;
#if DEBUG_PAD
		printf ("Pad[%d] data =", i);
		for (int j = 0; j < 8; j++) printf (" %2.2x", m_Next[i].button[j]);
		printf ("\n");
#endif
	}

    /* save the connection PadInformation, so we can compare it next time around */
    for (int i = 0; i < MAX_PADS; i++) 
    {
#if CELL_SDK_VERSION >= 0x250000
        m_Status[i] = m_Info.port_status[i];
#else
		m_Status[i] = m_Info.status[i];
#endif
    }
}

bool C_PS3Pad::IsFirstDn(const U32 pad, const U32 button, const U32 mask)
{
	return PrevUp(pad, button, mask) && NextDn(pad, button, mask);
}

bool C_PS3Pad::IsStillDn(const U32 pad, const U32 button, const U32 mask)
{
	return PrevUp(pad, button, mask) && NextUp(pad, button, mask);
}

bool C_PS3Pad::PrevDn(const U32 pad, const U32 button, const U32 mask)
{
	return m_Prev[pad].button[button] & mask;
}

bool C_PS3Pad::PrevUp(const U32 pad, const U32 button, const U32 mask)
{
	return !(m_Prev[pad].button[button] & mask);
}

bool C_PS3Pad::NextDn(const U32 pad, const U32 button, const U32 mask)
{
	return m_Next[pad].button[button] & mask;
}

bool C_PS3Pad::NextUp(const U32 pad, const U32 button, const U32 mask)
{
	return !(m_Next[pad].button[button] & mask);
}

}; // namespace Samples

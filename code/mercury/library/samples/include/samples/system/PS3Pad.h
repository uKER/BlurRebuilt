#ifndef SAMPLES_PS3PAD_H
#define SAMPLES_PS3PAD_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Samples
{

#define MAX_PADS 4

class C_PS3Pad
{
public:

	C_PS3Pad();
	~C_PS3Pad();

	void Initialise();
	void Update();

	bool IsFirstDn(const U32 pad, const U32 button, const U32 mask);
	bool IsStillDn(const U32 pad, const U32 button, const U32 mask);
	
	bool PrevDn(const U32 pad, const U32 button, const U32 mask);
	bool PrevUp(const U32 pad, const U32 button, const U32 mask);
	bool NextDn(const U32 pad, const U32 button, const U32 mask);
	bool NextUp(const U32 pad, const U32 button, const U32 mask);

public:

	uint8_t		m_Status[MAX_PADS];
#if CELL_SDK_VERSION >= 0x250000
	CellPadInfo2 m_Info; // connection padinfo buffer
#else
	CellPadInfo m_Info; // connection padinfo buffer
#endif

	CellPadData m_Next[MAX_PADS];
	CellPadData m_Prev[MAX_PADS];

};

}; // namespace Samples

#endif // SAMPLES_PS3PAD_H

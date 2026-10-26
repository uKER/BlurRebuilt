#ifndef ACTORSYSTEM_HANDLEDATA_H
#define ACTORSYSTEM_HANDLEDATA_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Util
{

/**
@brief Internal data format for handles.

Munges together an array index, a cyclical counter that spins each time the
array slot is reused, and a fixed counter that identifies the owner of the 
array.
*/
struct C_HandleData
{
	static const U32 kIndexBits = 16U;
	static const U32 kIndexMask = (1U << kIndexBits) - 1U;

	static const U32 kCycleShift = kIndexBits;
	static const U32 kCycleBits = 14U;
	static const U32 kCycleMask = (1U << kCycleBits) - 1U;

	static const U32 kFixedShift = kIndexBits + kCycleBits;
	static const U32 kFixedBits = 32U - kFixedShift;
	static const U32 kFixedMask = (1U << kFixedBits) - 1U;

public:
	static const U32 kIndexSize = (1U << kIndexBits); ///< index values lie in range [0,kIndexSize)
	static const U32 kCycleSize = (1U << kCycleBits); ///< cycle values lie in range (0,kCycleSize)
	static const U32 kFixedSize = (1U << kFixedBits); ///< fixed values lie in range [0,kFixedSize)

	C_HandleData();
	~C_HandleData();

	C_HandleData(const C_HandleData& copy);
	C_HandleData& operator = (const C_HandleData& copy);

	void SetIndex(const U32 value);
	void SetCycle(const U32 value);
	void SetFixed(const U32 value);

	void SpinCycle();

	void Clear() {m_Data = 0;}

	void Init(const U32 index, const U32 cycle, const U32 fixed);

	U32 GetIndex() const {return (m_Data & kIndexMask);}
	U32 GetCycle() const {return ((m_Data >> kCycleShift) & kCycleMask);}
	U32 GetFixed() const {return ((m_Data >> kFixedShift) & kFixedMask);}

	bool IsNull() const {return (m_Data == 0U);}

	U32 AsU32() const {return m_Data;}

private:
	U32 m_Data; ///< bit-packed data

};

inline C_HandleData::C_HandleData()
	: m_Data(0) 
{
}

inline C_HandleData::~C_HandleData() 
{
}

inline C_HandleData::C_HandleData(const C_HandleData& copy)
	: m_Data(copy.m_Data)
{
}

inline C_HandleData& C_HandleData::operator = (const C_HandleData& copy)
{
	m_Data = copy.m_Data; 
	return *this;
}

inline void C_HandleData::Init(const U32 index, const U32 cycle, const U32 fixed)
{
	BIZ_ASSERT(index < kIndexSize);
	m_Data = index | (Maths::Max(cycle & kCycleMask, 1U) << kCycleShift) | ((fixed & kFixedMask) << kFixedShift);
}

inline void C_HandleData::SetIndex(const U32 value)
{
	BIZ_ASSERT(value < kIndexSize);
	m_Data = value | (m_Data & ((kCycleMask << kCycleShift) | (kFixedMask << kFixedShift)));
}

inline void C_HandleData::SetCycle(const U32 value)
{
	const U32 cycle = Maths::Max(value & kCycleMask, 1U); // 0 is reserved for NULL handle
	m_Data = (cycle << kCycleShift) | (m_Data & (kIndexMask | (kFixedMask << kFixedShift)));
}

inline void C_HandleData::SetFixed(const U32 value)
{
	const U32 fixed = value & kFixedMask;
	m_Data = (fixed << kFixedShift) | (m_Data & (kIndexMask | (kCycleMask << kCycleShift)));
}

inline void C_HandleData::SpinCycle()
{
	SetCycle(GetCycle() + 1U);
}

} // namespace Util


#endif // ACTORSYSTEM_HANDLEDATA_H

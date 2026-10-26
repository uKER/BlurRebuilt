#ifndef FIXED_VECTOR_H
#define FIXED_VECTOR_H

#ifdef _MSC_VER
#pragma once
#endif


namespace Ut 
{

// Fixed Vector: provides pushback, where max size is known 			
// at compile-time. saves reallocation checks etc.						
//																		
// many cases involve fixed maximum sizes, e.g. SPU's assume workload	
// is pre-cut up into 256k chunks 										


template<typename T, int MAX, typename COUNT_T = int>
class	FixedVector
{
	T		m_List[MAX];
	COUNT_T	m_Size;
public:
	int	Size() { return	m_Size; }

	void	Resize(int newSize) 
	{
		assert(newSize < MAX);
		m_Size = newSize;
	}

	T&	operator[](int index) {
		return	m_List[index];
	}
	const T&	operator[](int index) const {
		return	m_List[index];
	}

	void	PushBack(const T& src) {
		*Append(1) = src;
	}

	int	Remaining() const {
		return	MAX-m_Size;
	}

	// allocate space at end - allowing 'push back' in-place.
	// returns ptr to where to place new items.
	inline T*	Append(int newCount=1)
	{
		T*	ret = m_List+m_Size;
		m_Size+=newCount;
		assert(m_Size<=MAX);
		return	ret;
	};

	// No effect.
	void	Reserve(int size)
	{
		assert(size<=MAX)
	}

	FixedVector() {
		m_Size=0;
	}
};

}; // namespace Ut

#endif
















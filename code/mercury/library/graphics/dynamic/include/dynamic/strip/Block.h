#ifndef RENDER_DYNAMICEFFECT_STRIP_BLOCK_H
#define RENDER_DYNAMICEFFECT_STRIP_BLOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Dynamic/Block.h>

namespace DynamicEffect
{
namespace Strip
{

// minimal class
class ElementBlockBase
{
public:
	ElementBlockBase() /*: m_nCount(0)*/ {};

	void Reset()
	{
		//m_nCount = 0;
	}

	U32		m_iElementSize;
	//U32		m_nCount;
};

// derive off empty class
//template <int ElementSize>
class ElementBlock : public Ut::ListElementDouble<ElementBlock>, public ElementBlockBase
{
public:
	ElementBlock() : ElementBlockBase() {};

	void Reset()
	{
		m_Element.Clear();
		ElementBlockBase::Reset();
	};

	void Clear()
	{
		m_Element.Clear();
	}

	void Destroy()
	{
	};

	ElementVector	m_Element; 
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_BLOCK_H

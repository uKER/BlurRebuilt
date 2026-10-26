#ifndef DISPLAY_RESOURCE_RESOURCE_INL
#define DISPLAY_RESOURCE_RESOURCE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE const S32 C_Resource::RefCount() const
{
	return m_RefCount;
}

INLINE const S32 C_Resource::Acquire()
{
	return ++m_RefCount;
}

INLINE const S32 C_Resource::Release()
{
	if(m_RefCount > 0)
		--m_RefCount;
	return m_RefCount;
}

}; // namespace Display

#endif // DISPLAY_RESOURCE_RESOURCE_H

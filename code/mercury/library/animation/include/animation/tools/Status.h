#ifndef ANIMATION_TOOLS_STATUS_H
#define ANIMATION_TOOLS_STATUS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/utility/string.h>

namespace AnimationTools
{
	/**
	Status class.
	*/
	class C_Status
	{
	public:
	
		C_Status(): m_ok(false) {}
	
		explicit C_Status( bool ok ):
			m_ok(ok)
		{
		}

		explicit C_Status( bool ok, const Btl::String& message ):
			m_message(message),
			m_ok(ok)
		{
		}

		bool operator !() const	{ return !m_ok; }
		
		operator bool () const	{ return m_ok; }
		
		const Btl::String&	GetErrorMessage() const
		{
			return m_message;
		}
	
	private:
		Btl::String		m_message;
		bool			m_ok;
	};
}

#endif

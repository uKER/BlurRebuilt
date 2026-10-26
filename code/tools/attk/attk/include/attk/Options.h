#ifndef ATTK_OPTIONS_H
#define ATTK_OPTIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	class C_Options
	{
	public:
		void AddAttribute(const C_OptionAttribute& attribute);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	private:
		Ut::UtVector< Ut::DynamicVector<C_OptionAttribute> > m_optionList;
	};
#endif
}

#endif

#ifndef EDITOR_LIB_STRING_UTIL_H
#define EDITOR_LIB_STRING_UTIL_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	namespace StringUtil
	{
		void StripIdFromName(Btl::String& name, const I_DgNode& node);
		
		void StripNumericSuffixFromName(Btl::String& name, const I_DgNode& node);
		bool SplitName(const Btl::String& name, Btl::String& prefix, U32& suffix);
		

		#include "StringUtil.inl"
	}
}

#endif

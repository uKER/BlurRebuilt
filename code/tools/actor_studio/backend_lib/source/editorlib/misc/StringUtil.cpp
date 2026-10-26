#include "stdafx.h"
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/StringUtil.h>
#include <cctype>
#include <sstream>

namespace EditorLib
{

namespace StringUtil
{

void StripNumericSuffixFromName(Btl::String& name, const I_DgNode& node)
{
	U32 suffix;
	SplitName(node.GetName(), name, suffix);
}

bool SplitName(const Btl::String& name, Btl::String& prefix, U32& suffix)
{
	//get numeric suffix
	const char* pChar = name.Chars();
	const char* pSuffixStart = 0;
	while (*pChar)
	{
		if (*pChar >= '0' && *pChar <='9')
		{
			if (!pSuffixStart && *pChar!='0') pSuffixStart = pChar;	//skip any preceding 0's
		}
		else
		{
			pSuffixStart = 0;
		}
		pChar++;
	}

	if (pSuffixStart)
	{
		prefix = name(0, pSuffixStart-name.Chars());
		suffix= atoi(pSuffixStart);
		return true;
	}
	else
		return false;
}
}

}
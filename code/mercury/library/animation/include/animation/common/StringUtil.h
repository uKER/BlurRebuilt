#ifndef ANIMATION_STRINGUTIL_H
#define ANIMATION_STRINGUTIL_H

namespace Animation
{

namespace StringUtil
{
	/// Returns true if string matches pattern.
	bool Match(const CHAR8* str, const CHAR8* pattern);

	/// Returns true if string matches pattern, igoring case.
	bool MatchAnyCase(const CHAR8* str, const CHAR8* pattern);


	/// Converts character c to lower-case.
	inline CHAR8 ToLower(const CHAR8 c)
	{
		return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
	}

	/// Returns true if string has a particular prefix (case-insensitive).
	inline bool HasPrefix(const CHAR8* str, const CHAR8* prefix)
	{
		while (*str != 0 && *prefix != 0 && ToLower(*str) == ToLower(*prefix))
		{
			++str;
			++prefix;
		}
		return *prefix == 0;
	}
	
	template <typename TStringA, typename TStringB>
	void ExtractFilename(TStringA& outFilename, const TStringB& path)
	{
		S32 offset = 0;
		S32 extOffset = path.Length();
		for (U32 i = path.Length(); i != 0; --i)
		{
			CHAR8 c = path[i-1];
			if (c == '.')
			{
				extOffset = i - 1;
			}
			else if (c == '\\' || c == '/')
			{
				offset = i;
				break;
			}						
		}
		outFilename = TStringA(path.Chars() + offset, extOffset - offset);
	}
}

}

#endif

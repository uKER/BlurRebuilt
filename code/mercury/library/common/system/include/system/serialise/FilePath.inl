INLINE void _FormatChar(char* pCursor)
{
	if (*pCursor >= 'A' && *pCursor <= 'Z') 
		*pCursor += 0x20;

	if (*pCursor == GetNotLocalDirToken())
		*pCursor = GetLocalDirToken();
}

C_FilePath::C_FilePath()
#ifdef FILEPATH_DEFER_FORMAT
:
m_isDirty(0)
#endif
{
}

C_FilePath::C_FilePath(const C_FilePath& rhs)
{
	m_formattedFilePath = rhs.m_formattedFilePath;
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = rhs.m_isDirty;
#endif
}

C_FilePath::C_FilePath(const Util::C_String& filePath)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(filePath.Chars());
#else
	FormatAndStoreFilePath(filePath.Chars());
#endif
}

C_FilePath::C_FilePath(const Btl::String& filePath)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(filePath.Chars());
#else
	FormatAndStoreFilePath(filePath.Chars());
#endif
}

C_FilePath::C_FilePath(const Btl::String260& filePath)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(filePath.Chars());
#else
	FormatAndStoreFilePath(filePath.Chars());
#endif
}

C_FilePath::C_FilePath(const char* filePath)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(filePath);
#else
	FormatAndStoreFilePath(filePath);
#endif
}

C_FilePath::C_FilePath(const char filePath)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(filePath);
#else
	FormatAndStoreFilePath(filePath);
#endif
}

S32 C_FilePath::Length() const
{
	return m_formattedFilePath.Length();
}

const C_FilePath& C_FilePath::operator=(const C_FilePath& rhs)
{
	m_formattedFilePath = rhs.m_formattedFilePath;
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = rhs.m_isDirty;
#endif
	return *this;
}

const C_FilePath& C_FilePath::operator=(const Util::C_String& rhs)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(rhs.Chars());
#else
	FormatAndStoreFilePath(rhs.Chars());
#endif
	return *this;
}

const C_FilePath& C_FilePath::operator=(const Btl::String& rhs)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(rhs.Chars());
#else
	FormatAndStoreFilePath(rhs.Chars());
#endif
	return *this;
}

const C_FilePath& C_FilePath::operator=(const Btl::String260& rhs)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(rhs.Chars());
#else
	FormatAndStoreFilePath(rhs.Chars());
#endif
	return *this;
}

const C_FilePath& C_FilePath::operator=(const char* rhs)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(rhs);
#else
	FormatAndStoreFilePath(rhs);
#endif
	return *this;
}

const C_FilePath& C_FilePath::operator=(const char rhs)
{
#ifdef FILEPATH_DEFER_FORMAT
	StoreFilePathAndDirty(rhs);
#else
	FormatAndStoreFilePath(rhs);
#endif
	return *this;
}

void C_FilePath::SetFormatted(const Util::C_String& filePath)
{
	m_formattedFilePath = filePath.Chars();
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = false;
#endif
}

void C_FilePath::SetFormatted(const Btl::String& filePath)
{
	m_formattedFilePath = filePath;
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = false;
#endif
}

void C_FilePath::SetFormatted(const Btl::String260& filePath)
{
	m_formattedFilePath = filePath;
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = false;
#endif
}

void C_FilePath::SetFormatted(const char* filePath)
{
	m_formattedFilePath = filePath;
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = false;
#endif
}

C_FilePath C_FilePath::operator + (const C_FilePath& rhs) const
{
	if (rhs.Length() == 0)
		return m_formattedFilePath;

	if (Length() > 0)
	{
#ifdef FILEPATH_DEFER_FORMAT
		m_isDirty = m_isDirty || rhs.m_isDirty;
#endif
		char trailingChar = m_formattedFilePath[Length()-1];
		if (trailingChar == '/' || trailingChar == '\\')
		{
			S32 iCharOffset = 0;
			for (; iCharOffset < rhs.Length(); ++iCharOffset)
			{
				char rhsLeadingChar = rhs.m_formattedFilePath[iCharOffset];
				if (rhsLeadingChar != '/' && rhsLeadingChar != '\\')
					//We've run out of leading directory separators. Break out.
					break;
			}
			//If the entire rhs path is directory separators return.
			if (iCharOffset == rhs.Length())
				return m_formattedFilePath;

			//Concat truncated rhs path.
			return m_formattedFilePath + (rhs.m_formattedFilePath.Chars() + iCharOffset);
		}
		//Concat rhs path.
		return m_formattedFilePath + rhs.m_formattedFilePath;
	}

#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = rhs.m_isDirty;
#endif
	return rhs.m_formattedFilePath;
}

const C_FilePath& C_FilePath::operator += (const C_FilePath& rhs)
{
	if (rhs.Length() == 0)
		return *this;

	if (Length() > 0)
	{
#ifdef FILEPATH_DEFER_FORMAT
		m_isDirty = m_isDirty || rhs.m_isDirty;
#endif
		char trailingChar = m_formattedFilePath[Length()-1];
		if (trailingChar == '/' || trailingChar == '\\')
		{
			S32 iCharOffset = 0;
			for (; iCharOffset < rhs.Length(); ++iCharOffset)
			{
				char rhsLeadingChar = rhs.m_formattedFilePath[iCharOffset];
				if (rhsLeadingChar != '/' && rhsLeadingChar != '\\')
					//We've run out of leading directory separators. Break out.
					break;
			}
			//If the entire rhs path is directory separators return.
			if (iCharOffset == rhs.Length())
				return *this;

			//Concat truncated rhs path.
			m_formattedFilePath += (rhs.m_formattedFilePath.Chars() + iCharOffset);
			return *this;
		}
		//Concat rhs path.
		m_formattedFilePath += rhs.m_formattedFilePath;
		return *this;
	}

#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = rhs.m_isDirty;
#endif
	m_formattedFilePath = rhs.m_formattedFilePath;
	return *this;
}

const bool C_FilePath::operator == (const C_FilePath& rhs) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
	rhs.FormatFilePath();
#endif
	return m_formattedFilePath == rhs.m_formattedFilePath;
}

const bool C_FilePath::operator != (const C_FilePath& rhs) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
	rhs.FormatFilePath();
#endif
	return m_formattedFilePath != rhs.m_formattedFilePath;
}

const bool C_FilePath::operator < (const C_FilePath& rhs) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
	rhs.FormatFilePath();
#endif
	return m_formattedFilePath < rhs.m_formattedFilePath;
}

const bool C_FilePath::operator > (const C_FilePath& rhs) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
	rhs.FormatFilePath();
#endif
	return m_formattedFilePath > rhs.m_formattedFilePath;
}

C_FilePath C_FilePath::operator () ( S32 lo, S32 hi ) const
{
	C_FilePath r;
	r.m_formattedFilePath = m_formattedFilePath(lo, hi);
	return r;
}

const char* C_FilePath::Chars() const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath.Chars();
}

const Btl::String260& C_FilePath::String() const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath;
}

S32 C_FilePath::Search(char c, S32 offset) const
{
	_FormatChar(&c);
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath.Search(c, offset);
}

S32 C_FilePath::RevSearch(char c, S32 offset) const
{
	_FormatChar(&c);
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath.RevSearch(c, offset);
}

S32 C_FilePath::Search(const C_FilePath& rhs, S32 offset) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath.Search(rhs.m_formattedFilePath, offset);
}

C_FilePath C_FilePath::Replace(const C_FilePath& src, const C_FilePath& dst) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath.Replace(src.Chars(), dst.Chars());
}

const char& C_FilePath::operator [] (S32 i) const
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath[i];
}

char& C_FilePath::operator [] (S32 i)
{
#ifdef FILEPATH_DEFER_FORMAT
	FormatFilePath();
#endif
	return m_formattedFilePath[i];
}

void C_FilePath::Clear()
{
	m_formattedFilePath.Clear();
#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = false;
#endif
}

#ifndef FILEPATH_DEFER_FORMAT
void C_FilePath::FormatAndStoreFilePath(const char toFormat)
{
	m_formattedFilePath.Resize(1);
	m_formattedFilePath[0] = toFormat;
	_FormatChar(m_formattedFilePath.NonConstChars());
}

void C_FilePath::FormatAndStoreFilePath(const char* toFormat)
{
	m_formattedFilePath = toFormat;
	FormatFilePath();
}
#endif

#ifdef FILEPATH_DEFER_FORMAT
void C_FilePath::FormatFilePath() const
#else
void C_FilePath::FormatFilePath()
#endif
{
#ifdef FILEPATH_DEFER_FORMAT
	if (!m_isDirty)
		return;
#endif

	S32 start = 0;

#if defined(PLATFORM_PC) && defined(TOOL)
	//PC hackery to support share paths.
	{
		char sharePath[] = "\\\\";
		const char* strstrResult = strstr(m_formattedFilePath.Chars(), sharePath);
		if (strstrResult != 0)
		{
			U32 offsetBytes = strstrResult - m_formattedFilePath.Chars();
			if (offsetBytes == 0)
				start = lengthof(sharePath)-1;
		}
	}
#elif defined(PLATFORM_PS3) || defined(PLATFORM_SPU)
	{
		//PS3 hackery due to mixed case usage.
		//Games are mounted from /%mountpoint%/%game_name%/USRDIR
		//E.g. /dev/bdvd/AMAX/USRDIR
		char usrDir[] = "/USRDIR";
		const char* strstrResult = strstr(m_formattedFilePath.Chars(), usrDir);
		if (strstrResult != 0)
		{
			U32 offsetBytes = strstrResult - m_formattedFilePath.Chars();
			start = offsetBytes + lengthof(usrDir)-1;
		}
		else
		{
#ifdef PLATFORM_PS3
			extern char g_ps3_titleId[];
			const char* strstrResult = strstr(m_formattedFilePath.Chars(), PlatformPS3::g_ps3_titleId);
			if (strstrResult != 0)
			{
				U32 offsetBytes = strstrResult - m_formattedFilePath.Chars();
				start = offsetBytes + strlen(PlatformPS3::g_ps3_titleId);
			}
#endif
		}
	}
#endif

	for (char* pCursor = m_formattedFilePath.NonConstChars()+start; *pCursor; pCursor++)
		_FormatChar(pCursor);

#ifdef FILEPATH_DEFER_FORMAT
	m_isDirty = false;
#endif
}


#ifdef FILEPATH_DEFER_FORMAT

void C_FilePath::StoreFilePathAndDirty(const char toFormat)
{
	m_formattedFilePath.Resize(1);
	m_formattedFilePath[0] = toFormat;
	m_isDirty = true;
}

void C_FilePath::StoreFilePathAndDirty(const char* toFormat)
{
	S32 start = 0;
	m_formattedFilePath = toFormat;
	m_isDirty = true;
}

#endif

char GetLocalDirToken() 
{
	#if defined(PLATFORM_PC)
	return '\\';
	#elif defined(PLATFORM_PS3) || defined(PLATFORM_SPU) || defined(PLATFORM_LINUX)  
	return '/';
	#else
	#error ("*** ERROR: Unsupported platform")
	#endif
	return '/';
}

char GetNotLocalDirToken()
{
	#if defined(PLATFORM_PC)
	return '/';
	#elif defined(PLATFORM_PS3) || defined(PLATFORM_SPU) || defined(PLATFORM_LINUX) 
	return '\\';
	#else
	#error ("*** ERROR: Unsupported platform")
	#endif
	return '\\';
}

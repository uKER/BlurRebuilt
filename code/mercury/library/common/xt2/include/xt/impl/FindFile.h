#ifndef XT_IMPL_FINDFILE_H
#define XT_IMPL_FINDFILE_H

struct FindFile
{
	WIN32_FIND_DATA	wfd;
	HANDLE			hdl;

public:
	inline FindFile() :
		hdl(INVALID_HANDLE_VALUE)
	{
	}

	inline ~FindFile()
	{
		End();
	}

public:
	inline bool Begin(const char* path)
	{
		return (hdl = FindFirstFile(path, &wfd)) != INVALID_HANDLE_VALUE;
	}

	inline bool Next()
	{
		if (hdl == INVALID_HANDLE_VALUE)
			return false;

		return FindNextFile(hdl, &wfd) != FALSE;
	}

	inline void End()
	{
		if (hdl != INVALID_HANDLE_VALUE)
		{
			FindClose(hdl);
			hdl = INVALID_HANDLE_VALUE;
		}
	}

public:
	inline WIN32_FIND_DATA* operator->()
	{
		return &wfd;
	}
};

#endif // XT_IMPL_FINDFILE_H
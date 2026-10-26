#ifndef REGWRAP_H
#define REGWRAP_H

#ifdef _MSC_VER
#pragma once
#endif

class RegWrap
{
public:
	RegWrap(HKEY root, const char* subKey);
	~RegWrap();

public:
	void Set(const char* name, const char* value);
	void SetMulti(const char* name, const XtString& multiValue);

public:
	XtString Get(const char* name);
	XtString GetMulti(const char* name);

private:
	XtString GetImpl(const char* name, DWORD type);

public:
	LONG MyRegGetValue(HKEY key, const char* name, DWORD type, BYTE* data, DWORD* length);
	
public:
	inline operator HKEY()
	{
		return _key;
	}

private:
	HKEY _key;
};

#endif // REGWRAP_H
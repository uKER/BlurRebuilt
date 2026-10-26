#pragma once

inline void Clear(FILETIME* f)
{
	f->dwHighDateTime = 0;
	f->dwLowDateTime = 0;
}

inline FILETIME MakeFiletime(DWORD low, DWORD high)
{
	FILETIME f;

	f.dwHighDateTime = low;
	f.dwLowDateTime = high;

	return f;
}

inline bool operator == (const FILETIME& t1, const FILETIME& t2)
{
	if (t1.dwHighDateTime == t2.dwHighDateTime
		&& t1.dwLowDateTime == t2.dwLowDateTime)
		return true;
	else
		return false;
}

inline bool operator != (const FILETIME& t1, const FILETIME& t2)
{
	if (t1.dwHighDateTime != t2.dwHighDateTime
		|| t1.dwLowDateTime != t2.dwLowDateTime)
		return true;
	else
		return false;
}

inline bool operator > (const FILETIME& t1, const FILETIME& t2)
{
	if (t1.dwHighDateTime > t2.dwHighDateTime)
		return true;
	else if (t2.dwHighDateTime > t1.dwHighDateTime)
		return false;

	if (t1.dwLowDateTime > t2.dwLowDateTime)
		return true;
	else
		return false;
}

inline bool operator >= (const FILETIME& t1, const FILETIME& t2)
{
	return !(t2 > t1);
}

inline bool operator <= (const FILETIME& t1, const FILETIME& t2)
{
	return !(t1 > t2);
}

inline bool operator < (const FILETIME& t1, const FILETIME& t2)
{
	return t2 > t1;
}

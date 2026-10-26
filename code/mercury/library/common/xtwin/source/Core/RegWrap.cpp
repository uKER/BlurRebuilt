#include "Core/XtWinPch.h"
#include "XtWin/WndCore.h"
#include "Core/List.h"
#include "XtWin/RegWrap.h"

template<typename T> __forceinline T Max(const T& Lhs, const T& Rhs)
{
	return (Rhs > Lhs) ? Rhs : Lhs;
}

RegWrap::RegWrap(HKEY root, const char* subKey) :
	_key(0)
{
	if ((root == 0) || (RegCreateKeyEx(root, subKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &_key, 0) != ERROR_SUCCESS))
		_key = 0;
}

RegWrap::~RegWrap()
{
	if (_key != 0)
		RegCloseKey(_key);
}

void RegWrap::Set(const char* name, const char* value)
{
	if (_key == 0)
		return;

	RegSetValueEx(_key, name, 0, REG_SZ, (BYTE*)value, (DWORD)(strlen(value) + 1));
}

void RegWrap::SetMulti(const char* name, const XtString& multiValue)
{
	if (_key == 0)
		return;

	RegSetValueEx(_key, name, 0, REG_MULTI_SZ, (BYTE*)(const char*)multiValue, (DWORD)multiValue.Length() + 1);
}

XtString RegWrap::Get(const char* name)
{
	return GetImpl(name, REG_SZ);
}

XtString RegWrap::GetMulti(const char* name)
{
	return GetImpl(name, REG_MULTI_SZ);
}

XtString RegWrap::GetImpl(const char* name, DWORD type)
{
	if (_key == 0)
		return "";

	char	data[64];
	DWORD	length = sizeof(data);

	switch(MyRegGetValue(_key, name, type, (BYTE*)data, &length))
	{
		case ERROR_SUCCESS:
			if ((length > 0) && (data[length - 1] == '\0'))
				length--;
		return XtString(data, length);

		case ERROR_MORE_DATA:
		{
			XtString result;

			for(;;)
			{
				char* ptr = result.Capture(length, false);

				if (!ptr)
					return "";

				switch(MyRegGetValue(_key, name, type, (BYTE*)ptr, &length))
				{
					case ERROR_SUCCESS:
						result.Truncate(Max((int)length - 1, 0));
					return result;

					case ERROR_MORE_DATA:
						result.Truncate(0);
					break;

					default:
						result.Truncate(0);
					return result;
				}
			}
		}
		break;

		default:
		return "";
	}
}

LONG RegWrap::MyRegGetValue(HKEY key, const char* name, DWORD type, BYTE* data, DWORD* length)
{
	DWORD regType	= 0;
	DWORD regLength = *length;

	LONG result = RegQueryValueEx(key, name, 0, &regType, data, &regLength);

	if (result == ERROR_SUCCESS)
	{
		if (regType != type)
			return ERROR_INVALID_DATA;

		if (type == REG_SZ)
		{
			bool needsTermination = false;

			if (regLength > 0)
			{
				if (data[regLength - 1] != '\0')
					needsTermination = true;
			}
			else
				needsTermination = true;

			if (needsTermination)
			{
				if ((regLength + 1) > *length)
				{
					*length = regLength + 1;

					return ERROR_MORE_DATA;
				}

				data[regLength] = '\0';

				regLength++;
			}

			*length = regLength;
		}
		else if (type == REG_MULTI_SZ)
		{
			bool needsTermination = false;

			if (regLength > 1)
			{
				if ((data[regLength - 2] != '\0') || (data[regLength - 1] != '\0'))
					needsTermination = true;
			}
			else
				needsTermination = true;

			if (needsTermination)
			{
				if ((regLength + 2) > *length)
				{
					*length = regLength + 2;

					return ERROR_MORE_DATA;
				}

				data[regLength] = '\0';
				data[regLength + 1] = '\0';

				regLength += 2;
			}

			*length = regLength;
		}

		*length = regLength;
	}
	else if (result == ERROR_MORE_DATA)
	{
		if (regType != type)
			return ERROR_INVALID_DATA;

		*length = regLength;
	}

	return result;
}
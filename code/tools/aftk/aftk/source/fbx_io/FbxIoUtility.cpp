#include "stdafx.h"
#include "../Aftk/Ftk.h"
#include "FbxIoUtility.h"

namespace FbxIo
{
	bool GetOptionValueString(const char* pStr, std::string &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		value = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		return true;
	}

	bool GetOptionValueBool(const char* pStr, bool &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		tokenStd = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		if (tokenStd == "false" || tokenStd == "0")
		{
			value = false;
			return true;
		}

		value = true;
		return true;
	}

	bool GetOptionValueF32(const char* pStr, F32 &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		tokenStd = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		std::stringstream ss;
		ss << tokenStd;
		ss >> value;
		return true;
	}

	bool GetOptionValueU32(const char* pStr, U32 &value)
	{
		char delimiter[] = "=";

		std::string tokenStd(pStr);

		size_t pos = tokenStd.find(delimiter);
		if (pos == std::string::npos)
		{
			return false;
		}
		tokenStd = tokenStd.substr(pos+1, tokenStd.length()-pos-1);
		std::stringstream ss;
		ss << tokenStd;
		ss >> value;
		return true;
	}
}
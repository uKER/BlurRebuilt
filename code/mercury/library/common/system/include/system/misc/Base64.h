#ifndef SYSTEM_BASE64_H
#define SYSTEM_BASE64_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{
	namespace Base64
	{
		Util::C_String Encode(const Util::C_String& string);

		Util::C_String Decode(const Util::C_String& encodedString);
	}
}

#endif

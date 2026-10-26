#include "SystemPch.h"
#include <system/misc/Base64.h>

namespace Util
{
namespace Base64
{

static const char base64_chars[] = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

INLINE bool IsBase64(unsigned char c) 
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

// encode a base-64 string
Util::C_String Encode(const Util::C_String& string)
{
	char const* bytes_to_encode = string.Chars();
	unsigned int in_len = string.Length();
	Util::C_String ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				ret = ret + Util::C_String(base64_chars[char_array_4[i]]);
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret = ret + Util::C_String(base64_chars[char_array_4[j]]);

		while((i++ < 3))
			ret = ret + Util::C_String('=');

	}

	return ret;
}

// decode a base-64 string
Util::C_String Decode(const Util::C_String& encoded_string)
{
  int in_len = encoded_string.Length()-4;
  int i = 0;
  int j = 0;
  int in_ = 4;
  unsigned char char_array_4[4], char_array_3[3];
  Util::C_String ret;

  while (in_len-- && ( encoded_string[in_] != '=') && Util::Base64::IsBase64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = (char)(strchr(base64_chars, char_array_4[i]) - base64_chars);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
		ret = ret + Util::C_String(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = (char)(strchr(base64_chars, char_array_4[j]) - base64_chars);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret = ret + Util::C_String(char_array_3[j]);
  }

  return ret;
}

}
}

// [Blur2PC] Optional offline player name; no profile/save or account changes.
#ifndef UI_PC_DRIVER_NAME_H
#define UI_PC_DRIVER_NAME_H

#include <cstdio>
#include <cstring>

namespace UI
{

// Read 1-16 printable ASCII bytes, without BOM, terminator or line ending.
// Missing/invalid files retain the original "Player" default.
inline bool ReadPcDriverName(const char* path, char (&name)[17])
{
	std::memset(name, 0, sizeof(name));
	std::memcpy(name, "Player", 6);
	if (!path)
		return false;

	FILE* file = NULL;
#ifdef _MSC_VER
	if (fopen_s(&file, path, "rb") != 0)
		return false;
#else
	file = std::fopen(path, "rb");
#endif
	if (!file)
		return false;

	char candidate[17] = {};
	const size_t count = std::fread(candidate, 1, 16, file);
	const int extra = std::fgetc(file);
	bool valid = count != 0 && extra == EOF && !std::ferror(file);
	bool hasVisibleCharacter = false;
	for (size_t i = 0; valid && i < count; ++i)
	{
		const unsigned char c = static_cast<unsigned char>(candidate[i]);
		valid = c >= 0x20 && c <= 0x7e;
		hasVisibleCharacter = hasVisibleCharacter || c > 0x20;
	}
	std::fclose(file);
	if (!valid || !hasVisibleCharacter)
		return false;

	std::memcpy(name, candidate, sizeof(name));
	return true;
}

} // namespace UI

#endif

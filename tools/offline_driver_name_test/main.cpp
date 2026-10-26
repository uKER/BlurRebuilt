#include "ui/Game/Profiles/PcDriverName.h"
#include <fstream>
#include <iostream>
#include <string>

static int failures = 0;
static int cases = 0;
static const char* fileName = "driver-name-test.tmp";

static void Check(const char* label, const std::string& bytes, bool expectedValid)
{
	{
		std::ofstream file(fileName, std::ios::binary | std::ios::trunc);
		file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
	}
	char name[17];
	const bool valid = UI::ReadPcDriverName(fileName, name);
	const std::string expected = expectedValid ? bytes : "Player";
	++cases;
	if (valid != expectedValid || std::string(name) != expected || name[16] != '\0')
	{
		std::cerr << "FAIL: " << label << '\n';
		++failures;
	}
	std::remove(fileName);
}

int main()
{
	char name[17];
	std::remove(fileName);
	++cases;
	if (UI::ReadPcDriverName(fileName, name) || std::string(name) != "Player")
		++failures;
	++cases;
	if (UI::ReadPcDriverName(NULL, name) || std::string(name) != "Player")
		++failures;
	Check("one byte", "A", true);
	Check("normal LAN name", "LAN_Driver", true);
	Check("exactly 16 bytes", "1234567890abcdef", true);
	Check("internal space", "LAN Driver", true);
	Check("literal percent", "Pilot%25", true);
	Check("empty", "", false);
	Check("spaces only", "   ", false);
	Check("17 bytes", "1234567890abcdefg", false);
	Check("line ending", "LAN_Driver\r\n", false);
	Check("UTF-8 BOM", std::string("\xef\xbb\xbf") + "LAN_Driver", false);
	Check("embedded NUL", std::string("LAN\0Driver", 10), false);
	Check("non-ASCII", std::string("LAN\xc3\xbc"), false);
	std::cout << cases << " offline-name cases; " << failures << " failures\n";
	return failures ? 1 : 0;
}

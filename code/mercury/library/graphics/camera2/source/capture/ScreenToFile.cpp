
#include "stdafx.h"
#include <camera2/capture/ScreenToFile.h>
#include <camera2/capture/LockScreen.h>
#pragma warning (push, 3)
#include <Framework/application/CoreSystems.h>
#include <System/Serialise/I_FileSystem.h>
#pragma warning (pop)

namespace Cam2
{

static inline S32 CalcAddr(S32 x, S32 y, S32 pitch)
{
	return y * pitch + (x << 2);
}

S32 ScreenToTgaFile(Sys::I_File* file)
{
	C_LockScreenInfo si;
	S32 ret = LockScreen(si);

	if (ret != 0)
		return ret;
	
	// TGA header:
	U8 head[0x12];
	memset(head, 0, sizeof(head));
	head[0x02] = 2; // Uncompressed RGB
	head[0x0c] = (U8) si.width;
	head[0x0d] = (U8)(si.width >> 8);
	head[0x0e] = (U8) si.height;
	head[0x0f] = (U8)(si.height >> 8);
	head[0x10] = 24;
	head[0x11] = 0x20; // Top down image
	file->Write(head, sizeof(head));

	// Allocate space for one line
	S32 lineLen = si.width * 3;
	U8* line = new U8[lineLen];
	if (line == 0)
	{
		delete[] file;
		UnlockScreen();
		return 1;
	}
	memset(line, 0, lineLen);

	// Output all the lines of the texture:
	S32 y = 0;
	for (y = 0; y < si.height; y++)
	{
		S32 x = 0;
		U8* pd = line;
		
		switch (si.format)
		{
			default:
			case 0:
				// Little Endian 8,8,8
				for (x = 0; x < si.width; x++, pd += 3)
				{
					U8* ps = (U8*)si.bits + CalcAddr(x, y, si.pitch);
					pd[2] = ps[2]; // Red
					pd[1] = ps[1]; // Green
					pd[0] = ps[0]; // Blue
				}
			break;
			case 3:
				// PS3 - Big Endian 8,8,8
				for (x = 0; x < si.width; x++, pd += 3)
				{
					U8* ps = (U8*)si.bits + CalcAddr(x, y, si.pitch);
					pd[2] = ps[1]; // Red
					pd[1] = ps[2]; // Green
					pd[0] = ps[3]; // Blue
				}
			break;
		}
		
		file->Write(line, lineLen);
	}
	
	delete[] line;
	UnlockScreen();
	return 0;
}

// Grab a screenshot and save it to disk:
S32 ScreenToTgaFile(const char* string)
{
	// Open output image:
	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	Sys::I_File* file = fs.Open(string, Sys::E_FileAccess_Write);
	if (file == 0)
	{
		return 1;
	}

	ScreenToTgaFile(file);

	delete file;
	return 0;
}

// This function will write out an image to the next free filename:
bool ScreenToNextTga()
{
	Sys::I_FileSystem& fileSystem = *Sys::GetDefaultFileSystem();
	
	fileSystem.CreateSingleDirectory("images");
	Util::C_String name;
	
	for (int i = 0; i < 1000000; i++)
	{
		static int tgaNumber = 0;
		name.Printf("images/image_%.6d.tga", tgaNumber);
		if (!fileSystem.FileExists(name))
			break;

		tgaNumber++;
		tgaNumber %= 1000000;
	}
	
	Sys::I_File* file = fileSystem.Open(name, Sys::E_FileAccess_Write);
	if (file == 0)
		return false;

	ScreenToTgaFile(file);

	Debug::Logger& log = Debug::GetLogger("mercury.camera2");
	LOGINFO(log, "Wrote screenshot to %s", name.Chars());

	delete file;
	return true;
}

} // End of namespace

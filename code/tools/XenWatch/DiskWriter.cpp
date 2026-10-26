
#include "stdafx.h"
#include "DiskWriter.h"

C_DiskWriter::C_DiskWriter()
{
	m_outFile = 0;
	m_screenNumber = 0;
	m_lastTick = 0;
	m_imagePeriod = 0.f;
}

C_DiskWriter::~C_DiskWriter()
{
	if (m_outFile)
		fclose(m_outFile);
	m_outFile = 0;
}

void C_DiskWriter::BeginImage(S32 width, S32 height)
{
	SkipExisting();

	Util::C_String filename;
	filename.Printf("Screen_%.6d.tmp", m_screenNumber);
	
	if (m_path.Length() > 0)
	{
		CreateDirectory(m_path.Chars(), NULL);
		filename = m_path + "\\" + filename;
	}
	
	m_outFile = fopen(filename.Chars(), "wb");
	if (m_outFile == 0)
		return;

	// TGA header:
	unsigned char head[0x12];
	memset(head, 0, sizeof(head));
	head[0x02] = 2; // Uncompressed RGB
	head[0x0c] = (unsigned char) width;
	head[0x0d] = (unsigned char)(width >> 8);
	head[0x0e] = (unsigned char) height;
	head[0x0f] = (unsigned char)(height >> 8);
	head[0x10] = 24;
	head[0x11] = 0x20; // Top down image
	fwrite(head, 1, sizeof(head), m_outFile);
}

void C_DiskWriter::WriteLine(U8* data, S32 len)
{
	if (m_outFile)
		fwrite(data, 1, len, m_outFile);
}

void C_DiskWriter::EndImage()
{
	if (m_outFile)
		fclose(m_outFile);
	m_outFile = 0;

	// Move temp file to TGA:
	Util::C_String src, dest;
	src.Printf ("Screen_%.6d.tmp", m_screenNumber);
	dest.Printf("Screen_%.6d.tga", m_screenNumber);

	if (m_path.Length() > 0)
	{
		src  = m_path + "\\" + src;
		dest = m_path + "\\" + dest;
	}
	
	DeleteFile(dest.Chars());
	MoveFile(src.Chars(), dest.Chars());

	m_screenNumber++;
	m_lastFilename = dest;

	// Find out how long it's taking to capture an image:
	int now = GetTickCount();
	float period = 0;
	period = (float)(now - m_lastTick) * 0.001f;
	m_lastTick = now;
	
	if (period < 5.f)
		m_imagePeriod = m_imagePeriod * 0.75f + period * 0.25f; // Smooth value
}

float C_DiskWriter::ImagesPerSecond()
{
	if (m_imagePeriod < 0.00001f)
		return 0.f;

	return 1.f / m_imagePeriod;
}

void C_DiskWriter::SkipExisting()
{
	WIN32_FIND_DATA found;
	memset(&found, 0, sizeof(found));
	
	// See if current screen number exists
	Util::C_String search;
	search.Printf("Screen_%.6d.tga", m_screenNumber);
	if (m_path.Length() > 0)
		search = m_path + "\\" + search;

	HANDLE find = FindFirstFile(search.Chars(), &found);
	if (find == INVALID_HANDLE_VALUE)
		return; // Nope - all clear

	// Search for all of the screenshots:
	search = "Screen_*.tga";
	if (m_path.Length() > 0)
		search = m_path + "\\" + search;

	S32 highest = 0;
	find = FindFirstFile(search.Chars(), &found);
	
	if (find != INVALID_HANDLE_VALUE)
	{
		do
		{
			S32 num = 0;
			sscanf(found.cFileName, "Screen_%d.tga", &num);

			if (highest < num)
				highest = num;
		}
		while (FindNextFile(find, &found));
	}
		
	FindClose(find);
	
	m_screenNumber = highest + 1;
}
